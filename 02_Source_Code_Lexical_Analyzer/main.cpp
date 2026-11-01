#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

enum class TokenType {
    Keyword,
    Identifier,
    IntegerConstant,
    FloatConstant,
    StringLiteral,
    CharacterLiteral,
    Operator,
    SpecialSymbol,
    Preprocessor,
    Error
};

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::Keyword: return "KEYWORD";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::IntegerConstant: return "INTEGER_CONSTANT";
        case TokenType::FloatConstant: return "FLOAT_CONSTANT";
        case TokenType::StringLiteral: return "STRING_LITERAL";
        case TokenType::CharacterLiteral: return "CHARACTER_LITERAL";
        case TokenType::Operator: return "OPERATOR";
        case TokenType::SpecialSymbol: return "SPECIAL_SYMBOL";
        case TokenType::Preprocessor: return "PREPROCESSOR";
        case TokenType::Error: return "ERROR";
    }
    return "UNKNOWN";
}

struct Token {
    std::string lexeme;
    TokenType type;
    int line;
    std::string detail;
};

const std::set<std::string> KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while"};

const std::vector<std::string> OPERATORS = {
    "<<=", ">>=", "++", "--", "+=", "-=", "*=", "/=", "%=", "==", "!=", "<=",
    ">=", "&&", "||", "<<", ">>", "->", "&=", "|=", "^=", "+", "-", "*", "/",
    "%", "=", "<", ">", "!", "&", "|", "^", "~", "?", ":", "."};

const std::string SPECIAL_SYMBOLS = "(){}[];,";

class Lexer {
public:
    explicit Lexer(const std::string &source) : source_(source) {}

    std::vector<Token> tokenize() {
        while (position_ < source_.size()) {
            scanNext();
        }
        return tokens_;
    }

private:
    char at(std::size_t offset) const {
        return position_ + offset < source_.size() ? source_[position_ + offset] : '\0';
    }

    static bool isIdentifierStart(char character) {
        return std::isalpha(static_cast<unsigned char>(character)) || character == '_';
    }

    static bool isIdentifierPart(char character) {
        return std::isalnum(static_cast<unsigned char>(character)) || character == '_';
    }

    static bool isDigit(char character) {
        return std::isdigit(static_cast<unsigned char>(character)) != 0;
    }

    void emit(const std::string &lexeme, TokenType type, int line, const std::string &detail = "") {
        tokens_.push_back({lexeme, type, line, detail});
    }

    void skipBlockComment() {
        std::size_t end = source_.find("*/", position_ + 2);
        int startLine = line_;
        std::size_t stop = end == std::string::npos ? source_.size() : end + 2;
        for (std::size_t index = position_; index < stop; ++index) {
            if (source_[index] == '\n') {
                ++line_;
            }
        }
        if (end == std::string::npos) {
            emit("/*", TokenType::Error, startLine, "unterminated comment");
        }
        position_ = stop;
    }

    bool atLineStart() const {
        for (std::size_t index = lineBegin_; index < position_; ++index) {
            if (!std::isspace(static_cast<unsigned char>(source_[index]))) {
                return false;
            }
        }
        return true;
    }

    void scanPreprocessor() {
        std::size_t end = source_.find('\n', position_);
        if (end == std::string::npos) {
            end = source_.size();
        }
        std::string text = source_.substr(position_, end - position_);
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
            text.pop_back();
        }
        emit(text, TokenType::Preprocessor, line_);
        position_ = end;
    }

    void scanWord() {
        std::size_t start = position_;
        while (isIdentifierPart(at(0))) {
            ++position_;
        }
        std::string word = source_.substr(start, position_ - start);
        emit(word, KEYWORDS.count(word) ? TokenType::Keyword : TokenType::Identifier, line_);
    }

    void scanNumber() {
        std::size_t start = position_;
        bool isFloat = false;
        if (at(0) == '0' && (at(1) == 'x' || at(1) == 'X') && std::isxdigit(static_cast<unsigned char>(at(2)))) {
            position_ += 2;
            while (std::isxdigit(static_cast<unsigned char>(at(0)))) {
                ++position_;
            }
        } else {
            while (isDigit(at(0))) {
                ++position_;
            }
            if (at(0) == '.') {
                isFloat = true;
                ++position_;
                while (isDigit(at(0))) {
                    ++position_;
                }
            }
            if (at(0) == 'e' || at(0) == 'E') {
                std::size_t offset = 1;
                if (at(offset) == '+' || at(offset) == '-') {
                    ++offset;
                }
                if (isDigit(at(offset))) {
                    isFloat = true;
                    position_ += offset;
                    while (isDigit(at(0))) {
                        ++position_;
                    }
                }
            }
        }
        if (isFloat) {
            if (at(0) == 'f' || at(0) == 'F' || at(0) == 'l' || at(0) == 'L') {
                ++position_;
            }
        } else {
            while (at(0) == 'u' || at(0) == 'U' || at(0) == 'l' || at(0) == 'L') {
                ++position_;
            }
        }
        if (isIdentifierPart(at(0))) {
            while (isIdentifierPart(at(0))) {
                ++position_;
            }
            emit(source_.substr(start, position_ - start), TokenType::Error, line_, "malformed numeric constant");
            return;
        }
        emit(source_.substr(start, position_ - start),
             isFloat ? TokenType::FloatConstant : TokenType::IntegerConstant, line_);
    }

    void scanQuoted(char quote) {
        std::size_t start = position_;
        ++position_;
        while (position_ < source_.size() && source_[position_] != quote && source_[position_] != '\n') {
            if (source_[position_] == '\\' && position_ + 1 < source_.size() && source_[position_ + 1] != '\n') {
                ++position_;
            }
            ++position_;
        }
        bool closed = position_ < source_.size() && source_[position_] == quote;
        if (closed) {
            ++position_;
        }
        std::string lexeme = source_.substr(start, position_ - start);
        if (!closed) {
            emit(lexeme, TokenType::Error, line_, quote == '"' ? "unterminated string literal"
                                                                 : "unterminated character literal");
            return;
        }
        if (quote == '"') {
            emit(lexeme, TokenType::StringLiteral, line_);
            return;
        }
        std::string body = lexeme.substr(1, lexeme.size() - 2);
        bool valid = body.size() == 1 && body[0] != '\\';
        if (!valid && body.size() >= 2 && body.size() <= 4 && body[0] == '\\') {
            valid = true;
        }
        if (valid) {
            emit(lexeme, TokenType::CharacterLiteral, line_);
        } else {
            emit(lexeme, TokenType::Error, line_, "invalid character literal");
        }
    }

    void scanOperatorOrSymbol() {
        char current = at(0);
        if (SPECIAL_SYMBOLS.find(current) != std::string::npos) {
            emit(std::string(1, current), TokenType::SpecialSymbol, line_);
            ++position_;
            return;
        }
        for (const std::string &candidate : OPERATORS) {
            if (source_.compare(position_, candidate.size(), candidate) == 0) {
                emit(candidate, TokenType::Operator, line_);
                position_ += candidate.size();
                return;
            }
        }
        emit(std::string(1, current), TokenType::Error, line_, "unrecognized character");
        ++position_;
    }

    void scanNext() {
        char current = at(0);
        if (current == '\n') {
            ++line_;
            ++position_;
            lineBegin_ = position_;
        } else if (std::isspace(static_cast<unsigned char>(current))) {
            ++position_;
        } else if (current == '/' && at(1) == '/') {
            while (position_ < source_.size() && source_[position_] != '\n') {
                ++position_;
            }
        } else if (current == '/' && at(1) == '*') {
            skipBlockComment();
        } else if (current == '#' && atLineStart()) {
            scanPreprocessor();
        } else if (isIdentifierStart(current)) {
            scanWord();
        } else if (isDigit(current) || (current == '.' && isDigit(at(1)))) {
            scanNumber();
        } else if (current == '"' || current == '\'') {
            scanQuoted(current);
        } else {
            scanOperatorOrSymbol();
        }
    }

    std::string source_;
    std::size_t position_ = 0;
    std::size_t lineBegin_ = 0;
    int line_ = 1;
    std::vector<Token> tokens_;
};

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [source_file]\n";
        return 1;
    }
    std::string path = argc == 2 ? argv[1] : "input.c";
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: cannot open source file '" << path << "'\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    if (source.find('\0') != std::string::npos) {
        std::cerr << "Error: '" << path << "' is not a text source file\n";
        return 1;
    }

    std::vector<Token> tokens = Lexer(source).tokenize();

    std::cout << std::left << std::setw(6) << "Line" << std::setw(34) << "Lexeme"
              << "Token Type\n";
    std::cout << std::string(66, '-') << "\n";
    std::map<std::string, int> counts;
    int errorCount = 0;
    for (const Token &token : tokens) {
        std::string typeText = tokenTypeName(token.type);
        if (token.type == TokenType::Error) {
            typeText += " (" + token.detail + ")";
            ++errorCount;
        }
        ++counts[tokenTypeName(token.type)];
        std::cout << std::left << std::setw(6) << token.line << std::setw(34) << token.lexeme << typeText << "\n";
    }

    std::cout << "\nSummary\n" << std::string(30, '-') << "\n";
    for (const auto &entry : counts) {
        std::cout << std::left << std::setw(22) << entry.first << entry.second << "\n";
    }
    std::cout << std::left << std::setw(22) << "TOTAL" << tokens.size() << "\n";
    if (errorCount > 0) {
        std::cout << "\nLexical errors found: " << errorCount << "\n";
        return 2;
    }
    return 0;
}
