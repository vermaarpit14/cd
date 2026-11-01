#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum class TokenKind { Identifier, Number, Plus, Minus, Star, Slash, Assign, LeftParen, RightParen, End };

struct Token {
    TokenKind kind;
    std::string text;
    std::size_t position;
};

struct Quadruple {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

class CompileError : public std::runtime_error {
public:
    explicit CompileError(const std::string &message) : std::runtime_error(message) {}
};

std::vector<Token> tokenize(const std::string &text) {
    std::vector<Token> tokens;
    std::size_t position = 0;
    while (position < text.size()) {
        char current = text[position];
        if (std::isspace(static_cast<unsigned char>(current))) {
            ++position;
        } else if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
            std::size_t start = position;
            while (position < text.size() &&
                   (std::isalnum(static_cast<unsigned char>(text[position])) || text[position] == '_')) {
                ++position;
            }
            tokens.push_back({TokenKind::Identifier, text.substr(start, position - start), start});
        } else if (std::isdigit(static_cast<unsigned char>(current)) || current == '.') {
            std::size_t start = position;
            int dots = 0;
            int digits = 0;
            while (position < text.size() &&
                   (std::isdigit(static_cast<unsigned char>(text[position])) || text[position] == '.')) {
                if (text[position] == '.') {
                    ++dots;
                } else {
                    ++digits;
                }
                ++position;
            }
            if (dots > 1 || digits == 0) {
                throw CompileError("malformed number '" + text.substr(start, position - start) + "' at position " +
                                   std::to_string(start));
            }
            tokens.push_back({TokenKind::Number, text.substr(start, position - start), start});
        } else {
            TokenKind kind;
            switch (current) {
                case '+': kind = TokenKind::Plus; break;
                case '-': kind = TokenKind::Minus; break;
                case '*': kind = TokenKind::Star; break;
                case '/': kind = TokenKind::Slash; break;
                case '=': kind = TokenKind::Assign; break;
                case '(': kind = TokenKind::LeftParen; break;
                case ')': kind = TokenKind::RightParen; break;
                default:
                    throw CompileError(std::string("invalid character '") + current + "' at position " +
                                       std::to_string(position));
            }
            tokens.push_back({kind, std::string(1, current), position});
            ++position;
        }
    }
    tokens.push_back({TokenKind::End, "end of input", text.size()});
    return tokens;
}

class CodeGenerator {
public:
    explicit CodeGenerator(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
        for (const Token &token : tokens_) {
            if (token.kind == TokenKind::Identifier) {
                usedNames_.insert(token.text);
            }
        }
    }

    void generate() {
        if (current().kind == TokenKind::End) {
            throw CompileError("empty input");
        }
        if (current().kind == TokenKind::Identifier && tokens_[index_ + 1].kind == TokenKind::Assign) {
            std::string target = advance().text;
            advance();
            std::string value = parseExpression();
            emit("=", value, "-", target);
        } else {
            parseExpression();
        }
        if (current().kind != TokenKind::End) {
            throw CompileError(describeUnexpected("end of input"));
        }
    }

    const std::vector<Quadruple> &quadruples() const { return quadruples_; }

    bool isTemporary(const std::string &name) const { return temporaryIndex_.count(name) > 0; }

    std::size_t temporaryIndex(const std::string &name) const { return temporaryIndex_.at(name); }

private:
    const Token &current() const { return tokens_[index_]; }

    Token advance() {
        Token token = tokens_[index_];
        if (token.kind != TokenKind::End) {
            ++index_;
        }
        return token;
    }

    std::string describeUnexpected(const std::string &expected) const {
        return "expected " + expected + " but found '" + current().text + "' at position " +
               std::to_string(current().position);
    }

    std::string emit(const std::string &op, const std::string &arg1, const std::string &arg2,
                     const std::string &result) {
        quadruples_.push_back({op, arg1, arg2, result});
        return result;
    }

    std::string newTemporary() {
        std::string name;
        do {
            name = "t" + std::to_string(++temporaryCount_);
        } while (usedNames_.count(name));
        temporaryIndex_[name] = quadruples_.size();
        return name;
    }

    std::string parseExpression() {
        std::string left = parseTerm();
        while (current().kind == TokenKind::Plus || current().kind == TokenKind::Minus) {
            std::string op = advance().text;
            std::string right = parseTerm();
            std::string result = newTemporary();
            emit(op, left, right, result);
            left = result;
        }
        return left;
    }

    std::string parseTerm() {
        std::string left = parseFactor();
        while (current().kind == TokenKind::Star || current().kind == TokenKind::Slash) {
            std::string op = advance().text;
            std::string right = parseFactor();
            std::string result = newTemporary();
            emit(op, left, right, result);
            left = result;
        }
        return left;
    }

    std::string parseFactor() {
        const Token &token = current();
        if (token.kind == TokenKind::Minus) {
            advance();
            std::string operand = parseFactor();
            std::string result = newTemporary();
            emit("uminus", operand, "-", result);
            return result;
        }
        if (token.kind == TokenKind::Identifier || token.kind == TokenKind::Number) {
            return advance().text;
        }
        if (token.kind == TokenKind::LeftParen) {
            advance();
            std::string inner = parseExpression();
            if (current().kind != TokenKind::RightParen) {
                throw CompileError(describeUnexpected("')'"));
            }
            advance();
            return inner;
        }
        throw CompileError(describeUnexpected("an operand or '('"));
    }

    std::vector<Token> tokens_;
    std::size_t index_ = 0;
    std::size_t temporaryCount_ = 0;
    std::vector<Quadruple> quadruples_;
    std::map<std::string, std::size_t> temporaryIndex_;
    std::set<std::string> usedNames_;
};

std::string tripleOperand(const CodeGenerator &generator, const std::string &operand) {
    if (generator.isTemporary(operand)) {
        return "(" + std::to_string(generator.temporaryIndex(operand)) + ")";
    }
    return operand;
}

void printThreeAddressCode(const std::vector<Quadruple> &quadruples) {
    std::cout << "Three Address Code:\n";
    for (const Quadruple &quad : quadruples) {
        if (quad.op == "=") {
            std::cout << "  " << quad.result << " = " << quad.arg1 << "\n";
        } else if (quad.op == "uminus") {
            std::cout << "  " << quad.result << " = -" << quad.arg1 << "\n";
        } else {
            std::cout << "  " << quad.result << " = " << quad.arg1 << " " << quad.op << " " << quad.arg2 << "\n";
        }
    }
}

void printQuadruples(const std::vector<Quadruple> &quadruples) {
    std::cout << "Quadruples:\n";
    for (const Quadruple &quad : quadruples) {
        std::cout << "  (" << quad.op << ", " << quad.arg1 << ", " << quad.arg2 << ", " << quad.result << ")\n";
    }
}

void printTriples(const CodeGenerator &generator) {
    std::cout << "Triples:\n";
    const std::vector<Quadruple> &quadruples = generator.quadruples();
    for (std::size_t index = 0; index < quadruples.size(); ++index) {
        const Quadruple &quad = quadruples[index];
        std::cout << "  " << index << ": (" << quad.op << ", " << tripleOperand(generator, quad.arg1);
        if (quad.op == "=") {
            std::cout << ", " << quad.result << ")\n";
        } else if (quad.op == "uminus") {
            std::cout << ")\n";
        } else {
            std::cout << ", " << tripleOperand(generator, quad.arg2) << ")\n";
        }
    }
}

bool processStatement(const std::string &statement) {
    std::cout << "Input: " << statement << "\n";
    try {
        CodeGenerator generator(tokenize(statement));
        generator.generate();
        printThreeAddressCode(generator.quadruples());
        printQuadruples(generator.quadruples());
        printTriples(generator);
        std::cout << "\n";
        return true;
    } catch (const CompileError &error) {
        std::cout << "Error: " << error.what() << "\n\n";
        return false;
    }
}

}

int main(int argc, char *argv[]) {
    std::vector<std::string> statements;
    if (argc == 2 && std::string(argv[1]) == "-") {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.find_first_not_of(" \t\r") != std::string::npos) {
                statements.push_back(line);
            }
        }
        if (statements.empty()) {
            std::cerr << "Error: no statements on standard input\n";
            return 1;
        }
    } else if (argc > 1) {
        for (int index = 1; index < argc; ++index) {
            statements.push_back(argv[index]);
        }
    } else {
        statements = {"a = b + c * d", "x = (a + b) * (c - d) / e", "r = p - q - s", "y = -a + 5",
                      "total = 7", "b + c * d", "a = b + * c", "a = (b + c", "a = b $ c", "a = b c", ""};
    }

    int failures = 0;
    for (const std::string &statement : statements) {
        if (!processStatement(statement)) {
            ++failures;
        }
    }
    return failures > 0 && argc > 1 ? 2 : 0;
}
