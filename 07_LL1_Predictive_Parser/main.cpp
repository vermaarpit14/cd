#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr char EPSILON = '#';
constexpr char END_MARKER = '$';

struct Production {
    char lhs;
    std::string rhs;
};

const std::vector<Production> PRODUCTIONS = {
    {'S', "aABe"},
    {'A', "bAc"},
    {'A', "d"},
    {'B', "f"},
    {'B', ""}};

const char START_SYMBOL = 'S';

bool isNonTerminal(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }

std::size_t displayWidth(const std::string &text) {
    std::size_t width = 0;
    for (unsigned char character : text) {
        if ((character & 0xC0) != 0x80) {
            ++width;
        }
    }
    return width;
}

std::string pad(const std::string &text, std::size_t width) {
    std::size_t current = displayWidth(text);
    return current >= width ? text : text + std::string(width - current, ' ');
}

std::string symbolText(char symbol) {
    return symbol == EPSILON ? std::string("\xCE\xB5") : std::string(1, symbol);
}

std::string rhsText(const std::string &rhs) { return rhs.empty() ? std::string("\xCE\xB5") : rhs; }

std::string productionText(const Production &production) {
    return std::string(1, production.lhs) + " \xE2\x86\x92 " + rhsText(production.rhs);
}

std::string setText(const std::set<char> &symbols) {
    std::string text = "{ ";
    bool first = true;
    for (char symbol : symbols) {
        if (!first) {
            text += ", ";
        }
        text += symbolText(symbol);
        first = false;
    }
    return text + " }";
}

class LL1Grammar {
public:
    LL1Grammar() {
        for (const Production &production : PRODUCTIONS) {
            nonTerminals_.insert(production.lhs);
            for (char symbol : production.rhs) {
                if (isNonTerminal(symbol)) {
                    nonTerminals_.insert(symbol);
                } else {
                    terminals_.insert(symbol);
                }
            }
        }
        computeFirst();
        computeFollow();
        buildTable();
    }

    std::set<char> firstOfSequence(const std::string &sequence) const {
        std::set<char> result;
        bool allNullable = true;
        for (char symbol : sequence) {
            if (!isNonTerminal(symbol)) {
                result.insert(symbol);
                allNullable = false;
                break;
            }
            const std::set<char> &symbolFirst = first_.at(symbol);
            for (char entry : symbolFirst) {
                if (entry != EPSILON) {
                    result.insert(entry);
                }
            }
            if (!symbolFirst.count(EPSILON)) {
                allNullable = false;
                break;
            }
        }
        if (allNullable) {
            result.insert(EPSILON);
        }
        return result;
    }

    void printSets() const {
        std::cout << "FIRST sets:\n";
        for (char nonTerminal : nonTerminals_) {
            std::cout << "  FIRST(" << nonTerminal << ")  = " << setText(first_.at(nonTerminal)) << "\n";
        }
        std::cout << "\nFOLLOW sets:\n";
        for (char nonTerminal : nonTerminals_) {
            std::cout << "  FOLLOW(" << nonTerminal << ") = " << setText(follow_.at(nonTerminal)) << "\n";
        }
        std::cout << "\n";
    }

    void printTable() const {
        std::vector<char> columns(terminals_.begin(), terminals_.end());
        columns.push_back(END_MARKER);
        std::cout << "LL(1) parsing table:\n";
        std::cout << "  " << pad("", 4);
        for (char column : columns) {
            std::cout << pad(std::string(1, column), 12);
        }
        std::cout << "\n  " << std::string(4 + 12 * columns.size(), '-') << "\n";
        for (char nonTerminal : nonTerminals_) {
            std::cout << "  " << pad(std::string(1, nonTerminal), 4);
            for (char column : columns) {
                auto entry = table_.find({nonTerminal, column});
                std::string cell = entry == table_.end() ? "" : productionText(PRODUCTIONS[entry->second]);
                std::cout << pad(cell, 12);
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        if (conflicts_.empty()) {
            std::cout << "The grammar is LL(1): no table cell holds more than one production.\n\n";
        } else {
            std::cout << "The grammar is NOT LL(1). Conflicts:\n";
            for (const std::string &conflict : conflicts_) {
                std::cout << "  " << conflict << "\n";
            }
            std::cout << "\n";
        }
    }

    bool isLL1() const { return conflicts_.empty(); }

    bool parse(const std::string &input, bool verbose) const {
        for (char symbol : input) {
            if (!terminals_.count(symbol)) {
                std::cout << "  Rejected: symbol '" << symbol << "' is not a terminal of the grammar\n";
                return false;
            }
        }
        std::vector<char> stack = {END_MARKER, START_SYMBOL};
        std::string remaining = input + END_MARKER;
        std::size_t position = 0;
        std::size_t step = 1;
        if (verbose) {
            std::cout << "  " << pad("Step", 6) << pad("Stack (top at left)", 22) << pad("Input", 14) << "Action\n";
        }
        while (!stack.empty()) {
            char top = stack.back();
            char lookahead = remaining[position];
            std::string stackText(stack.rbegin(), stack.rend());
            std::string action;
            bool failed = false;
            bool finished = false;
            if (top == END_MARKER && lookahead == END_MARKER) {
                action = "accept";
                finished = true;
            } else if (!isNonTerminal(top)) {
                if (top == lookahead) {
                    action = std::string("match ") + top;
                    stack.pop_back();
                    ++position;
                } else {
                    action = std::string("error: expected '") + top + "', found '" + lookahead + "'";
                    failed = true;
                }
            } else {
                auto entry = table_.find({top, lookahead});
                if (entry == table_.end()) {
                    action = std::string("error: no rule for ") + top + " on '" + lookahead + "'";
                    failed = true;
                } else {
                    const Production &production = PRODUCTIONS[entry->second];
                    action = "output " + productionText(production);
                    stack.pop_back();
                    for (auto iterator = production.rhs.rbegin(); iterator != production.rhs.rend(); ++iterator) {
                        stack.push_back(*iterator);
                    }
                }
            }
            if (verbose) {
                std::cout << "  " << pad(std::to_string(step), 6) << pad(stackText, 22)
                          << pad(remaining.substr(position), 14) << action << "\n";
            }
            ++step;
            if (failed) {
                return false;
            }
            if (finished) {
                return true;
            }
        }
        return false;
    }

private:
    void computeFirst() {
        for (char nonTerminal : nonTerminals_) {
            first_[nonTerminal];
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (const Production &production : PRODUCTIONS) {
                for (char entry : firstOfSequence(production.rhs)) {
                    if (first_[production.lhs].insert(entry).second) {
                        changed = true;
                    }
                }
            }
        }
    }

    void computeFollow() {
        for (char nonTerminal : nonTerminals_) {
            follow_[nonTerminal];
        }
        follow_[START_SYMBOL].insert(END_MARKER);
        bool changed = true;
        while (changed) {
            changed = false;
            for (const Production &production : PRODUCTIONS) {
                for (std::size_t index = 0; index < production.rhs.size(); ++index) {
                    char symbol = production.rhs[index];
                    if (!isNonTerminal(symbol)) {
                        continue;
                    }
                    std::set<char> trailing = firstOfSequence(production.rhs.substr(index + 1));
                    for (char entry : trailing) {
                        if (entry != EPSILON && follow_[symbol].insert(entry).second) {
                            changed = true;
                        }
                    }
                    if (trailing.count(EPSILON)) {
                        for (char entry : follow_[production.lhs]) {
                            if (follow_[symbol].insert(entry).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }

    void place(char nonTerminal, char terminal, int productionIndex) {
        auto key = std::make_pair(nonTerminal, terminal);
        auto existing = table_.find(key);
        if (existing != table_.end() && existing->second != productionIndex) {
            conflicts_.push_back(std::string("M[") + nonTerminal + ", " + terminal + "] has " +
                                 productionText(PRODUCTIONS[existing->second]) + " and " +
                                 productionText(PRODUCTIONS[productionIndex]));
            return;
        }
        table_[key] = productionIndex;
    }

    void buildTable() {
        for (std::size_t index = 0; index < PRODUCTIONS.size(); ++index) {
            const Production &production = PRODUCTIONS[index];
            std::set<char> selection = firstOfSequence(production.rhs);
            for (char terminal : selection) {
                if (terminal != EPSILON) {
                    place(production.lhs, terminal, static_cast<int>(index));
                }
            }
            if (selection.count(EPSILON)) {
                for (char terminal : follow_[production.lhs]) {
                    place(production.lhs, terminal, static_cast<int>(index));
                }
            }
        }
    }

    std::set<char> nonTerminals_;
    std::set<char> terminals_;
    std::map<char, std::set<char>> first_;
    std::map<char, std::set<char>> follow_;
    std::map<std::pair<char, char>, int> table_;
    std::vector<std::string> conflicts_;
};

}

int main(int argc, char *argv[]) {
    LL1Grammar grammar;

    std::cout << "Grammar:\n";
    for (const Production &production : PRODUCTIONS) {
        std::cout << "  " << productionText(production) << "\n";
    }
    std::cout << "\n";
    grammar.printSets();
    grammar.printTable();

    if (!grammar.isLL1()) {
        return 1;
    }

    std::vector<std::string> inputs;
    for (int index = 1; index < argc; ++index) {
        inputs.push_back(argv[index]);
    }
    if (inputs.empty()) {
        inputs = {"adfe", "abdcfe", "ade", "abdfe", "adf", "adfee"};
    }

    int rejected = 0;
    for (const std::string &input : inputs) {
        std::cout << "Parsing \"" << input << "\":\n";
        bool accepted = grammar.parse(input, true);
        std::cout << "  Result: " << (accepted ? "ACCEPTED" : "REJECTED") << "\n\n";
        if (!accepted) {
            ++rejected;
        }
    }
    return rejected > 0 && argc > 1 ? 2 : 0;
}
