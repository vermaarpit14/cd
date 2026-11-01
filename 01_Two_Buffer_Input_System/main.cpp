#include <cctype>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr char SENTINEL = '\0';
constexpr int END_OF_INPUT = -1;
constexpr std::size_t DEFAULT_BUFFER_SIZE = 16;
constexpr std::size_t MINIMUM_BUFFER_SIZE = 2;

const std::set<std::string> KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while"};

const char *const SAMPLE_SOURCE =
    "int main() {\n"
    "    float total_amount = 3.14;\n"
    "    int counter = 0;\n"
    "    while (counter < 10) {\n"
    "        counter = counter + 1;\n"
    "    }\n"
    "    return counter;\n"
    "}\n";

class TwoBufferInput {
public:
    TwoBufferInput(const std::string &path, std::size_t bufferSize)
        : file_(path, std::ios::binary),
          bufferSize_(bufferSize),
          storage_(2 * (bufferSize + 1), SENTINEL),
          forward_(0),
          loadCount_(0) {
        if (file_.is_open()) {
            loadHalf(0);
        }
    }

    bool isOpen() const { return file_.is_open(); }

    int peek() {
        while (true) {
            char current = storage_[forward_];
            if (current != SENTINEL) {
                return static_cast<unsigned char>(current);
            }
            if (forward_ == halfEnd(0)) {
                loadHalf(1);
                forward_ = halfStart(1);
            } else if (forward_ == halfEnd(1)) {
                loadHalf(0);
                forward_ = halfStart(0);
            } else {
                return END_OF_INPUT;
            }
        }
    }

    void advance() { ++forward_; }

    std::size_t loadCount() const { return loadCount_; }

    const std::vector<std::string> &loadLog() const { return loadLog_; }

private:
    std::size_t halfStart(std::size_t half) const { return half * (bufferSize_ + 1); }

    std::size_t halfEnd(std::size_t half) const { return halfStart(half) + bufferSize_; }

    void loadHalf(std::size_t half) {
        std::size_t start = halfStart(half);
        file_.read(&storage_[start], static_cast<std::streamsize>(bufferSize_));
        std::size_t bytesRead = static_cast<std::size_t>(file_.gcount());
        for (std::size_t index = 0; index < bytesRead; ++index) {
            if (storage_[start + index] == SENTINEL) {
                storage_[start + index] = ' ';
            }
        }
        storage_[start + bytesRead] = SENTINEL;
        ++loadCount_;
        loadLog_.push_back("load #" + std::to_string(loadCount_) + ": " +
                           std::to_string(bytesRead) + " byte(s) into buffer " +
                           std::to_string(half));
    }

    std::ifstream file_;
    std::size_t bufferSize_;
    std::vector<char> storage_;
    std::size_t forward_;
    std::size_t loadCount_;
    std::vector<std::string> loadLog_;
};

struct TokenRecord {
    std::string lexeme;
    std::string type;
    bool spansBoundary;
};

bool isIdentifierStart(int character) {
    return character >= 0 && (std::isalpha(character) || character == '_');
}

bool isIdentifierPart(int character) {
    return character >= 0 && (std::isalnum(character) || character == '_');
}

bool isDigit(int character) { return character >= 0 && std::isdigit(character); }

bool isWhitespace(int character) { return character >= 0 && std::isspace(character); }

std::vector<TokenRecord> scanAll(TwoBufferInput &input) {
    std::vector<TokenRecord> tokens;
    while (true) {
        int first = input.peek();
        if (first == END_OF_INPUT) {
            break;
        }
        if (isWhitespace(first)) {
            input.advance();
            continue;
        }
        std::string lexeme;
        std::size_t loadsAtStart = input.loadCount();
        std::size_t loadsAtLastCharacter = loadsAtStart;
        auto consume = [&]() {
            lexeme.push_back(static_cast<char>(input.peek()));
            loadsAtLastCharacter = input.loadCount();
            input.advance();
        };
        std::string type;
        if (isIdentifierStart(first)) {
            while (isIdentifierPart(input.peek())) {
                consume();
            }
            type = KEYWORDS.count(lexeme) ? "KEYWORD" : "IDENTIFIER";
        } else if (isDigit(first)) {
            while (isDigit(input.peek())) {
                consume();
            }
            if (input.peek() == '.') {
                consume();
                while (isDigit(input.peek())) {
                    consume();
                }
            }
            type = "NUMBER";
        } else {
            consume();
            type = "SYMBOL";
        }
        tokens.push_back({lexeme, type, loadsAtLastCharacter != loadsAtStart});
    }
    return tokens;
}

bool writeSampleFile(const std::string &path) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }
    output << SAMPLE_SOURCE;
    return static_cast<bool>(output);
}

bool parseBufferSize(const std::string &text, std::size_t &result) {
    try {
        std::size_t consumed = 0;
        unsigned long value = std::stoul(text, &consumed);
        if (consumed != text.size() || value < MINIMUM_BUFFER_SIZE) {
            return false;
        }
        result = static_cast<std::size_t>(value);
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

}

int main(int argc, char *argv[]) {
    std::string path = "sample_input.c";
    std::size_t bufferSize = DEFAULT_BUFFER_SIZE;

    if (argc > 3) {
        std::cerr << "Usage: " << argv[0] << " [source_file] [buffer_size]\n";
        return 1;
    }
    if (argc >= 2) {
        path = argv[1];
    } else if (!writeSampleFile(path)) {
        std::cerr << "Error: cannot create sample file '" << path << "'\n";
        return 1;
    }
    if (argc == 3 && !parseBufferSize(argv[2], bufferSize)) {
        std::cerr << "Error: buffer size must be an integer >= " << MINIMUM_BUFFER_SIZE << "\n";
        return 1;
    }

    TwoBufferInput input(path, bufferSize);
    if (!input.isOpen()) {
        std::cerr << "Error: cannot open source file '" << path << "'\n";
        return 1;
    }

    std::cout << "Source file : " << path << "\n";
    std::cout << "Buffer size : " << bufferSize << " byte(s) per buffer (+1 sentinel each)\n\n";

    std::vector<TokenRecord> tokens = scanAll(input);

    std::cout << std::left << std::setw(6) << "No." << std::setw(20) << "Lexeme"
              << std::setw(14) << "Token Type" << "Spans Buffer Boundary\n";
    std::cout << std::string(60, '-') << "\n";
    std::size_t spanning = 0;
    for (std::size_t index = 0; index < tokens.size(); ++index) {
        const TokenRecord &token = tokens[index];
        if (token.spansBoundary) {
            ++spanning;
        }
        std::cout << std::left << std::setw(6) << index + 1 << std::setw(20) << token.lexeme
                  << std::setw(14) << token.type << (token.spansBoundary ? "yes" : "no") << "\n";
    }

    std::cout << "\nTotal tokens                     : " << tokens.size() << "\n";
    std::cout << "Tokens spanning a buffer boundary: " << spanning << "\n";
    std::cout << "Buffer loads                     : " << input.loadCount() << "\n";
    for (const std::string &entry : input.loadLog()) {
        std::cout << "  " << entry << "\n";
    }
    std::cout << "End of input reached.\n";
    return 0;
}
