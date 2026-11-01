#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

enum class DataType { Int, Float, Char, Error };

std::string typeName(DataType type) {
    switch (type) {
        case DataType::Int: return "int";
        case DataType::Float: return "float";
        case DataType::Char: return "char";
        case DataType::Error: return "<error>";
    }
    return "<error>";
}

enum class TokenKind {
    KeywordInt,
    KeywordFloat,
    KeywordChar,
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    CharLiteral,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Assign,
    LeftParen,
    RightParen,
    Semicolon,
    EndOfFile,
    Invalid
};

struct Token {
    TokenKind kind;
    std::string text;
    int line;
};

std::vector<Token> tokenize(const std::string &source) {
    std::vector<Token> tokens;
    std::size_t position = 0;
    int line = 1;
    while (position < source.size()) {
        char current = source[position];
        if (current == '\n') {
            ++line;
            ++position;
        } else if (std::isspace(static_cast<unsigned char>(current))) {
            ++position;
        } else if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
            std::size_t start = position;
            while (position < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[position])) || source[position] == '_')) {
                ++position;
            }
            std::string word = source.substr(start, position - start);
            TokenKind kind = TokenKind::Identifier;
            if (word == "int") {
                kind = TokenKind::KeywordInt;
            } else if (word == "float") {
                kind = TokenKind::KeywordFloat;
            } else if (word == "char") {
                kind = TokenKind::KeywordChar;
            }
            tokens.push_back({kind, word, line});
        } else if (std::isdigit(static_cast<unsigned char>(current))) {
            std::size_t start = position;
            bool isFloat = false;
            while (position < source.size() && std::isdigit(static_cast<unsigned char>(source[position]))) {
                ++position;
            }
            if (position + 1 < source.size() && source[position] == '.' &&
                std::isdigit(static_cast<unsigned char>(source[position + 1]))) {
                isFloat = true;
                ++position;
                while (position < source.size() && std::isdigit(static_cast<unsigned char>(source[position]))) {
                    ++position;
                }
            }
            tokens.push_back({isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral,
                              source.substr(start, position - start), line});
        } else if (current == '\'') {
            if (position + 2 < source.size() && source[position + 2] == '\'' && source[position + 1] != '\n') {
                tokens.push_back({TokenKind::CharLiteral, source.substr(position, 3), line});
                position += 3;
            } else {
                tokens.push_back({TokenKind::Invalid, "'", line});
                ++position;
            }
        } else {
            TokenKind kind = TokenKind::Invalid;
            switch (current) {
                case '+': kind = TokenKind::Plus; break;
                case '-': kind = TokenKind::Minus; break;
                case '*': kind = TokenKind::Star; break;
                case '/': kind = TokenKind::Slash; break;
                case '%': kind = TokenKind::Percent; break;
                case '=': kind = TokenKind::Assign; break;
                case '(': kind = TokenKind::LeftParen; break;
                case ')': kind = TokenKind::RightParen; break;
                case ';': kind = TokenKind::Semicolon; break;
                default: break;
            }
            tokens.push_back({kind, std::string(1, current), line});
            ++position;
        }
    }
    tokens.push_back({TokenKind::EndOfFile, "end of input", line});
    return tokens;
}

enum class NodeKind {
    Program,
    Declaration,
    Assignment,
    BinaryOperation,
    UnaryMinus,
    IntegerLiteral,
    FloatLiteral,
    CharLiteral,
    Identifier
};

struct Node {
    NodeKind kind;
    std::string text;
    int line;
    DataType declaredType = DataType::Error;
    DataType type = DataType::Error;
    bool typed = false;
    std::vector<std::unique_ptr<Node>> children;

    Node(NodeKind nodeKind, std::string nodeText, int nodeLine)
        : kind(nodeKind), text(std::move(nodeText)), line(nodeLine) {}
};

using NodePointer = std::unique_ptr<Node>;

struct SyntaxError {
    std::string message;
    int line;
};

class Parser {
public:
    Parser(std::vector<Token> tokens, std::vector<std::string> &errors)
        : tokens_(std::move(tokens)), errors_(errors) {}

    NodePointer parseProgram() {
        NodePointer program = std::make_unique<Node>(NodeKind::Program, "Program", 1);
        while (current().kind != TokenKind::EndOfFile) {
            try {
                program->children.push_back(parseStatement());
            } catch (const SyntaxError &error) {
                errors_.push_back("Syntax error (line " + std::to_string(error.line) + "): " + error.message);
                synchronize();
            }
        }
        return program;
    }

private:
    const Token &current() const { return tokens_[index_]; }

    Token advance() {
        Token token = tokens_[index_];
        if (token.kind != TokenKind::EndOfFile) {
            ++index_;
        }
        return token;
    }

    bool match(TokenKind kind) {
        if (current().kind == kind) {
            advance();
            return true;
        }
        return false;
    }

    Token expect(TokenKind kind, const std::string &description) {
        if (current().kind != kind) {
            throw SyntaxError{"expected " + description + " but found '" + current().text + "'", current().line};
        }
        return advance();
    }

    void synchronize() {
        while (current().kind != TokenKind::EndOfFile) {
            if (advance().kind == TokenKind::Semicolon) {
                return;
            }
        }
    }

    static bool isTypeKeyword(TokenKind kind) {
        return kind == TokenKind::KeywordInt || kind == TokenKind::KeywordFloat || kind == TokenKind::KeywordChar;
    }

    NodePointer parseStatement() {
        if (isTypeKeyword(current().kind)) {
            Token keyword = advance();
            Token name = expect(TokenKind::Identifier, "a variable name");
            NodePointer declaration = std::make_unique<Node>(NodeKind::Declaration, name.text, name.line);
            declaration->declaredType = keyword.kind == TokenKind::KeywordInt     ? DataType::Int
                                        : keyword.kind == TokenKind::KeywordFloat ? DataType::Float
                                                                                  : DataType::Char;
            if (match(TokenKind::Assign)) {
                declaration->children.push_back(parseExpression());
            }
            expect(TokenKind::Semicolon, "';'");
            return declaration;
        }
        if (current().kind == TokenKind::Identifier) {
            Token name = advance();
            Token assign = expect(TokenKind::Assign, "'='");
            NodePointer assignment = std::make_unique<Node>(NodeKind::Assignment, "=", assign.line);
            assignment->children.push_back(std::make_unique<Node>(NodeKind::Identifier, name.text, name.line));
            assignment->children.push_back(parseExpression());
            expect(TokenKind::Semicolon, "';'");
            return assignment;
        }
        throw SyntaxError{"unexpected '" + current().text + "' at start of statement", current().line};
    }

    NodePointer parseExpression() {
        NodePointer left = parseTerm();
        while (current().kind == TokenKind::Plus || current().kind == TokenKind::Minus) {
            Token op = advance();
            NodePointer node = std::make_unique<Node>(NodeKind::BinaryOperation, op.text, op.line);
            node->children.push_back(std::move(left));
            node->children.push_back(parseTerm());
            left = std::move(node);
        }
        return left;
    }

    NodePointer parseTerm() {
        NodePointer left = parseUnary();
        while (current().kind == TokenKind::Star || current().kind == TokenKind::Slash ||
               current().kind == TokenKind::Percent) {
            Token op = advance();
            NodePointer node = std::make_unique<Node>(NodeKind::BinaryOperation, op.text, op.line);
            node->children.push_back(std::move(left));
            node->children.push_back(parseUnary());
            left = std::move(node);
        }
        return left;
    }

    NodePointer parseUnary() {
        if (current().kind == TokenKind::Minus) {
            Token op = advance();
            NodePointer node = std::make_unique<Node>(NodeKind::UnaryMinus, "-", op.line);
            node->children.push_back(parseUnary());
            return node;
        }
        return parsePrimary();
    }

    NodePointer parsePrimary() {
        const Token &token = current();
        switch (token.kind) {
            case TokenKind::IntegerLiteral: {
                Token literal = advance();
                return std::make_unique<Node>(NodeKind::IntegerLiteral, literal.text, literal.line);
            }
            case TokenKind::FloatLiteral: {
                Token literal = advance();
                return std::make_unique<Node>(NodeKind::FloatLiteral, literal.text, literal.line);
            }
            case TokenKind::CharLiteral: {
                Token literal = advance();
                return std::make_unique<Node>(NodeKind::CharLiteral, literal.text, literal.line);
            }
            case TokenKind::Identifier: {
                Token name = advance();
                return std::make_unique<Node>(NodeKind::Identifier, name.text, name.line);
            }
            case TokenKind::LeftParen: {
                advance();
                NodePointer inner = parseExpression();
                expect(TokenKind::RightParen, "')'");
                return inner;
            }
            default:
                throw SyntaxError{"unexpected '" + token.text + "' in expression", token.line};
        }
    }

    std::vector<Token> tokens_;
    std::vector<std::string> &errors_;
    std::size_t index_ = 0;
};

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(std::vector<std::string> &errors) : errors_(errors) {}

    void analyze(Node &program) {
        for (NodePointer &statement : program.children) {
            if (statement->kind == NodeKind::Declaration) {
                analyzeDeclaration(*statement);
            } else {
                analyzeAssignment(*statement);
            }
        }
    }

    const std::vector<std::pair<std::string, DataType>> &symbolTable() const { return symbols_; }

private:
    void report(int line, const std::string &message) {
        errors_.push_back("Semantic error (line " + std::to_string(line) + "): " + message);
    }

    DataType lookup(const std::string &name) const {
        for (const auto &entry : symbols_) {
            if (entry.first == name) {
                return entry.second;
            }
        }
        return DataType::Error;
    }

    bool isDeclared(const std::string &name) const {
        for (const auto &entry : symbols_) {
            if (entry.first == name) {
                return true;
            }
        }
        return false;
    }

    void checkAssignable(DataType target, DataType source, int line, const std::string &name) {
        if (target == DataType::Error || source == DataType::Error || target == source) {
            return;
        }
        if (target == DataType::Float && source == DataType::Int) {
            return;
        }
        if (target == DataType::Int && source == DataType::Float) {
            report(line, "incompatible assignment to '" + name + "': cannot assign float to int without loss of data");
            return;
        }
        report(line, "incompatible assignment to '" + name + "': cannot assign " + typeName(source) + " to " +
                         typeName(target));
    }

    void analyzeDeclaration(Node &node) {
        if (isDeclared(node.text)) {
            report(node.line, "variable '" + node.text + "' is already declared");
        } else {
            symbols_.push_back({node.text, node.declaredType});
        }
        node.type = node.declaredType;
        node.typed = true;
        if (!node.children.empty()) {
            DataType valueType = analyzeExpression(*node.children[0]);
            checkAssignable(node.declaredType, valueType, node.line, node.text);
        }
    }

    void analyzeAssignment(Node &node) {
        Node &target = *node.children[0];
        DataType targetType = DataType::Error;
        if (!isDeclared(target.text)) {
            report(target.line, "undeclared variable '" + target.text + "'");
        } else {
            targetType = lookup(target.text);
        }
        target.type = targetType;
        target.typed = true;
        DataType valueType = analyzeExpression(*node.children[1]);
        checkAssignable(targetType, valueType, node.line, target.text);
        node.type = targetType;
        node.typed = true;
    }

    static bool isZeroLiteral(const Node &node) {
        if (node.kind != NodeKind::IntegerLiteral && node.kind != NodeKind::FloatLiteral) {
            return false;
        }
        return std::stod(node.text) == 0.0;
    }

    DataType analyzeExpression(Node &node) {
        node.typed = true;
        switch (node.kind) {
            case NodeKind::IntegerLiteral: node.type = DataType::Int; break;
            case NodeKind::FloatLiteral: node.type = DataType::Float; break;
            case NodeKind::CharLiteral: node.type = DataType::Char; break;
            case NodeKind::Identifier:
                if (!isDeclared(node.text)) {
                    report(node.line, "undeclared variable '" + node.text + "'");
                    node.type = DataType::Error;
                } else {
                    node.type = lookup(node.text);
                }
                break;
            case NodeKind::UnaryMinus: {
                DataType operand = analyzeExpression(*node.children[0]);
                if (operand == DataType::Char) {
                    report(node.line, "invalid arithmetic operation: unary '-' cannot be applied to char");
                    node.type = DataType::Error;
                } else {
                    node.type = operand;
                }
                break;
            }
            case NodeKind::BinaryOperation: node.type = analyzeBinary(node); break;
            default: node.type = DataType::Error; break;
        }
        return node.type;
    }

    DataType analyzeBinary(Node &node) {
        DataType left = analyzeExpression(*node.children[0]);
        DataType right = analyzeExpression(*node.children[1]);
        if (left == DataType::Error || right == DataType::Error) {
            return DataType::Error;
        }
        if (left == DataType::Char && right == DataType::Char) {
            report(node.line, "invalid arithmetic operation: '" + node.text + "' is not defined for char operands");
            return DataType::Error;
        }
        if (left == DataType::Char || right == DataType::Char) {
            report(node.line, "incompatible operand types for '" + node.text + "': " + typeName(left) + " and " +
                                  typeName(right));
            return DataType::Error;
        }
        if (node.text == "%" && (left == DataType::Float || right == DataType::Float)) {
            report(node.line, "invalid arithmetic operation: '%' requires int operands but got " + typeName(left) +
                                  " and " + typeName(right));
            return DataType::Error;
        }
        if ((node.text == "/" || node.text == "%") && isZeroLiteral(*node.children[1])) {
            report(node.line, "invalid arithmetic operation: division by zero");
            return DataType::Error;
        }
        return left == DataType::Float || right == DataType::Float ? DataType::Float : DataType::Int;
    }

    std::vector<std::string> &errors_;
    std::vector<std::pair<std::string, DataType>> symbols_;
};

std::string nodeLabel(const Node &node) {
    std::string label;
    switch (node.kind) {
        case NodeKind::Program: return "Program";
        case NodeKind::Declaration: label = "Declaration " + typeName(node.declaredType) + " " + node.text; break;
        case NodeKind::Assignment: label = "Assignment (=)"; break;
        case NodeKind::BinaryOperation: label = "BinaryOp (" + node.text + ")"; break;
        case NodeKind::UnaryMinus: label = "UnaryMinus (-)"; break;
        case NodeKind::IntegerLiteral: label = "IntegerLiteral " + node.text; break;
        case NodeKind::FloatLiteral: label = "FloatLiteral " + node.text; break;
        case NodeKind::CharLiteral: label = "CharLiteral " + node.text; break;
        case NodeKind::Identifier: label = "Identifier " + node.text; break;
    }
    if (node.typed && node.kind != NodeKind::Declaration) {
        label += " : " + typeName(node.type);
    }
    return label;
}

void printTree(const Node &node, const std::string &prefix, bool isLast, bool isRoot) {
    if (isRoot) {
        std::cout << nodeLabel(node) << "\n";
    } else {
        std::cout << prefix << (isLast ? "`-- " : "|-- ") << nodeLabel(node) << "\n";
    }
    std::string childPrefix = isRoot ? "" : prefix + (isLast ? "    " : "|   ");
    for (std::size_t index = 0; index < node.children.size(); ++index) {
        printTree(*node.children[index], childPrefix, index + 1 == node.children.size(), false);
    }
}

bool runAnalysis(const std::string &title, const std::string &source) {
    std::cout << "============================================================\n";
    std::cout << title << "\n";
    std::cout << "============================================================\n";
    std::cout << "Source:\n";
    std::istringstream lines(source);
    std::string line;
    int number = 1;
    while (std::getline(lines, line)) {
        std::cout << "  " << number++ << " | " << line << "\n";
    }

    std::vector<std::string> errors;
    std::vector<Token> tokens = tokenize(source);
    for (const Token &token : tokens) {
        if (token.kind == TokenKind::Invalid) {
            errors.push_back("Lexical error (line " + std::to_string(token.line) + "): invalid character '" +
                             token.text + "'");
        }
    }
    std::vector<Token> usable;
    for (const Token &token : tokens) {
        if (token.kind != TokenKind::Invalid) {
            usable.push_back(token);
        }
    }

    std::vector<std::string> syntaxErrors;
    Parser parser(usable, syntaxErrors);
    NodePointer program = parser.parseProgram();
    SemanticAnalyzer analyzer(errors);
    analyzer.analyze(*program);
    errors.insert(errors.end(), syntaxErrors.begin(), syntaxErrors.end());

    std::cout << "\nAbstract Syntax Tree:\n";
    printTree(*program, "", true, true);

    std::cout << "\nSymbol table:\n";
    if (analyzer.symbolTable().empty()) {
        std::cout << "  (empty)\n";
    }
    for (const auto &entry : analyzer.symbolTable()) {
        std::cout << "  " << entry.first << " : " << typeName(entry.second) << "\n";
    }

    std::cout << "\nSemantic analysis result:\n";
    if (errors.empty()) {
        std::cout << "  No errors. The program is semantically valid.\n\n";
        return true;
    }
    for (const std::string &error : errors) {
        std::cout << "  " << error << "\n";
    }
    std::cout << "  " << errors.size() << " error(s) found.\n\n";
    return false;
}

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [source_file]\n";
        return 1;
    }
    if (argc == 2) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open source file '" << argv[1] << "'\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return runAnalysis(std::string("Analysis of ") + argv[1], buffer.str()) ? 0 : 2;
    }

    runAnalysis("Test 1: valid program",
                "int a = 4;\n"
                "int b = 6;\n"
                "float rate = 2.5;\n"
                "char grade = 'A';\n"
                "float total = a + b * rate;\n"
                "int remainder = (a + b) % 3;\n"
                "rate = a;\n"
                "grade = 'B';\n");

    runAnalysis("Test 2: undeclared variables",
                "int count = 1;\n"
                "total = count + 2;\n"
                "int result = count * missing;\n");

    runAnalysis("Test 3: incompatible and invalid operations",
                "int number = 10;\n"
                "float ratio = 1.5;\n"
                "char letter = 'x';\n"
                "int sum = number + letter;\n"
                "char both = letter * letter;\n"
                "int narrow = ratio;\n"
                "float mod = ratio % 2;\n"
                "letter = number;\n"
                "int zero = number / 0;\n"
                "int number = 3;\n");

    runAnalysis("Test 4: syntax errors",
                "int x = ;\n"
                "int y = (1 + 2;\n"
                "y = 5 @ 3;\n"
                "int z = 7;\n");
    return 0;
}
