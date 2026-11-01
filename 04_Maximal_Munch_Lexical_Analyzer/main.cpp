#include <cctype>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

using Matcher = std::function<std::size_t(const std::string &, std::size_t)>;

struct Rule {
    std::string name;
    int priority;
    Matcher match;
};

const std::set<std::string> KEYWORDS = {"int", "float", "char", "void", "if", "else", "while", "for",
                                        "do", "return", "break", "continue", "double", "long", "short"};

const std::vector<std::string> OPERATORS = {
    "<<=", ">>=", "++", "--", "+=", "-=", "*=", "/=", "%=", "==", "!=", "<=", ">=", "&&", "||",
    "<<", ">>", "->", "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "^", "~", "?", ":", "."};

const std::string DELIMITERS = "(){}[];,";

bool isDigitAt(const std::string &text, std::size_t position) {
    return position < text.size() && std::isdigit(static_cast<unsigned char>(text[position]));
}

std::size_t scanDigits(const std::string &text, std::size_t position) {
    while (isDigitAt(text, position)) {
        ++position;
    }
    return position;
}

std::size_t matchWord(const std::string &text, std::size_t start) {
    if (start >= text.size() || !(std::isalpha(static_cast<unsigned char>(text[start])) || text[start] == '_')) {
        return 0;
    }
    std::size_t position = start;
    while (position < text.size() &&
           (std::isalnum(static_cast<unsigned char>(text[position])) || text[position] == '_')) {
        ++position;
    }
    return position - start;
}

std::size_t matchKeyword(const std::string &text, std::size_t start) {
    std::size_t length = matchWord(text, start);
    if (length > 0 && KEYWORDS.count(text.substr(start, length))) {
        return length;
    }
    return 0;
}

std::size_t matchInteger(const std::string &text, std::size_t start) {
    return scanDigits(text, start) - start;
}

std::size_t matchFloat(const std::string &text, std::size_t start) {
    std::size_t position = scanDigits(text, start);
    bool hasIntegerPart = position > start;
    bool isFloat = false;
    if (position < text.size() && text[position] == '.') {
        std::size_t afterDot = scanDigits(text, position + 1);
        if (afterDot > position + 1) {
            position = afterDot;
            isFloat = true;
        } else if (hasIntegerPart) {
            return 0;
        }
    } else if (!hasIntegerPart) {
        return 0;
    }
    if (position < text.size() && (text[position] == 'e' || text[position] == 'E')) {
        std::size_t exponent = position + 1;
        if (exponent < text.size() && (text[exponent] == '+' || text[exponent] == '-')) {
            ++exponent;
        }
        std::size_t afterExponent = scanDigits(text, exponent);
        if (afterExponent > exponent) {
            position = afterExponent;
            isFloat = true;
        }
    }
    return isFloat ? position - start : 0;
}

std::size_t matchOperator(const std::string &text, std::size_t start) {
    std::size_t best = 0;
    for (const std::string &candidate : OPERATORS) {
        if (candidate.size() > best && text.compare(start, candidate.size(), candidate) == 0) {
            best = candidate.size();
        }
    }
    return best;
}

std::size_t matchDelimiter(const std::string &text, std::size_t start) {
    return DELIMITERS.find(text[start]) != std::string::npos ? 1 : 0;
}

const std::vector<Rule> &rules() {
    static const std::vector<Rule> table = {
        {"KEYWORD", 1, matchKeyword},
        {"IDENTIFIER", 2, matchWord},
        {"FLOAT", 3, matchFloat},
        {"INTEGER", 4, matchInteger},
        {"OPERATOR", 5, matchOperator},
        {"DELIMITER", 6, matchDelimiter}};
    return table;
}

struct Candidate {
    std::string name;
    int priority;
    std::size_t length;
};

struct Token {
    std::string lexeme;
    std::string type;
    int line;
    std::vector<Candidate> candidates;
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
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(current))) {
            ++position;
            continue;
        }
        std::vector<Candidate> candidates;
        const Rule *winner = nullptr;
        std::size_t bestLength = 0;
        for (const Rule &rule : rules()) {
            std::size_t length = rule.match(source, position);
            if (length == 0) {
                continue;
            }
            candidates.push_back({rule.name, rule.priority, length});
            if (length > bestLength || (length == bestLength && rule.priority < winner->priority)) {
                bestLength = length;
                winner = &rule;
            }
        }
        if (winner == nullptr) {
            tokens.push_back({std::string(1, current), "ERROR", line, candidates});
            ++position;
        } else {
            tokens.push_back({source.substr(position, bestLength), winner->name, line, candidates});
            position += bestLength;
        }
    }
    return tokens;
}

std::string describeCandidates(const std::vector<Candidate> &candidates) {
    std::ostringstream text;
    for (std::size_t index = 0; index < candidates.size(); ++index) {
        if (index > 0) {
            text << ", ";
        }
        text << candidates[index].name << "(" << candidates[index].length << ")";
    }
    return text.str();
}

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

    std::cout << "Rule priority (lower number wins an equal-length match):\n";
    for (const Rule &rule : rules()) {
        std::cout << "  " << rule.priority << ". " << rule.name << "\n";
    }
    std::cout << "\n";

    std::vector<Token> tokens = tokenize(buffer.str());

    std::cout << std::left << std::setw(6) << "Line" << std::setw(14) << "Lexeme" << std::setw(14)
              << "Token Type" << "Matching rules (match length)\n";
    std::cout << std::string(78, '-') << "\n";
    int errorCount = 0;
    for (const Token &token : tokens) {
        if (token.type == "ERROR") {
            ++errorCount;
        }
        std::cout << std::left << std::setw(6) << token.line << std::setw(14) << token.lexeme
                  << std::setw(14) << token.type << describeCandidates(token.candidates) << "\n";
    }
    std::cout << "\nTotal tokens: " << tokens.size() << ", errors: " << errorCount << "\n";
    return errorCount > 0 ? 2 : 0;
}
