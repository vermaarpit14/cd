#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::string END_MARKER = "$";
const std::string AUGMENTED_START = "S'";

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
};

struct Item {
    int production;
    int dot;
    bool operator<(const Item &other) const {
        return production != other.production ? production < other.production : dot < other.dot;
    }
    bool operator==(const Item &other) const { return production == other.production && dot == other.dot; }
};

using ItemSet = std::set<Item>;

const std::vector<Production> PRODUCTIONS = {
    {"S'", {"S"}},
    {"S", {"A"}},
    {"A", {"A", "b"}},
    {"A", {"a"}}};

const std::vector<std::string> NON_TERMINALS = {"S", "A"};
const std::vector<std::string> TERMINALS = {"a", "b"};

bool isNonTerminal(const std::string &symbol) {
    return symbol == AUGMENTED_START || std::find(NON_TERMINALS.begin(), NON_TERMINALS.end(), symbol) != NON_TERMINALS.end();
}

std::string productionText(int index) {
    const Production &production = PRODUCTIONS[index];
    std::string text = production.lhs + " -> ";
    for (const std::string &symbol : production.rhs) {
        text += symbol;
    }
    return text;
}

std::string itemText(const Item &item) {
    const Production &production = PRODUCTIONS[item.production];
    std::string text = production.lhs + " -> ";
    for (int index = 0; index <= static_cast<int>(production.rhs.size()); ++index) {
        if (index == item.dot) {
            text += ".";
        }
        if (index < static_cast<int>(production.rhs.size())) {
            text += production.rhs[index];
        }
    }
    return text;
}

ItemSet closure(const ItemSet &items) {
    ItemSet result = items;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Item> snapshot(result.begin(), result.end());
        for (const Item &item : snapshot) {
            const Production &production = PRODUCTIONS[item.production];
            if (item.dot >= static_cast<int>(production.rhs.size())) {
                continue;
            }
            const std::string &next = production.rhs[item.dot];
            if (!isNonTerminal(next)) {
                continue;
            }
            for (std::size_t index = 0; index < PRODUCTIONS.size(); ++index) {
                if (PRODUCTIONS[index].lhs == next && result.insert({static_cast<int>(index), 0}).second) {
                    changed = true;
                }
            }
        }
    }
    return result;
}

ItemSet gotoSet(const ItemSet &items, const std::string &symbol) {
    ItemSet moved;
    for (const Item &item : items) {
        const Production &production = PRODUCTIONS[item.production];
        if (item.dot < static_cast<int>(production.rhs.size()) && production.rhs[item.dot] == symbol) {
            moved.insert({item.production, item.dot + 1});
        }
    }
    return moved.empty() ? moved : closure(moved);
}

struct Automaton {
    std::vector<ItemSet> states;
    std::map<std::pair<int, std::string>, int> transitions;
};

Automaton buildAutomaton() {
    Automaton automaton;
    automaton.states.push_back(closure({{0, 0}}));
    std::vector<std::string> symbols = NON_TERMINALS;
    symbols.insert(symbols.end(), TERMINALS.begin(), TERMINALS.end());
    for (std::size_t current = 0; current < automaton.states.size(); ++current) {
        for (const std::string &symbol : symbols) {
            ItemSet target = gotoSet(automaton.states[current], symbol);
            if (target.empty()) {
                continue;
            }
            auto found = std::find(automaton.states.begin(), automaton.states.end(), target);
            int targetIndex = static_cast<int>(found - automaton.states.begin());
            if (found == automaton.states.end()) {
                automaton.states.push_back(target);
            }
            automaton.transitions[{static_cast<int>(current), symbol}] = targetIndex;
        }
    }
    return automaton;
}

std::map<std::string, std::set<std::string>> computeFirst() {
    std::map<std::string, std::set<std::string>> first;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production &production : PRODUCTIONS) {
            const std::string &head = production.rhs[0];
            std::set<std::string> additions;
            if (isNonTerminal(head)) {
                additions = first[head];
            } else {
                additions.insert(head);
            }
            for (const std::string &entry : additions) {
                if (first[production.lhs].insert(entry).second) {
                    changed = true;
                }
            }
        }
    }
    return first;
}

std::map<std::string, std::set<std::string>> computeFollow() {
    std::map<std::string, std::set<std::string>> first = computeFirst();
    std::map<std::string, std::set<std::string>> follow;
    follow[AUGMENTED_START].insert(END_MARKER);
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production &production : PRODUCTIONS) {
            for (std::size_t index = 0; index < production.rhs.size(); ++index) {
                const std::string &symbol = production.rhs[index];
                if (!isNonTerminal(symbol)) {
                    continue;
                }
                std::set<std::string> additions;
                if (index + 1 < production.rhs.size()) {
                    const std::string &following = production.rhs[index + 1];
                    if (isNonTerminal(following)) {
                        additions = first[following];
                    } else {
                        additions.insert(following);
                    }
                } else {
                    additions = follow[production.lhs];
                }
                for (const std::string &entry : additions) {
                    if (follow[symbol].insert(entry).second) {
                        changed = true;
                    }
                }
            }
        }
    }
    return follow;
}

using ActionCell = std::vector<std::string>;

struct ParsingTable {
    std::vector<std::map<std::string, ActionCell>> action;
    std::vector<std::map<std::string, int>> gotoTable;
};

void addAction(ActionCell &cell, const std::string &entry) {
    if (std::find(cell.begin(), cell.end(), entry) == cell.end()) {
        cell.push_back(entry);
        std::stable_partition(cell.begin(), cell.end(), [](const std::string &text) { return text[0] == 's'; });
    }
}

ParsingTable buildTable(const Automaton &automaton, bool useFollow,
                        const std::map<std::string, std::set<std::string>> &follow) {
    ParsingTable table;
    table.action.resize(automaton.states.size());
    table.gotoTable.resize(automaton.states.size());
    std::vector<std::string> lookaheads = TERMINALS;
    lookaheads.push_back(END_MARKER);
    for (std::size_t state = 0; state < automaton.states.size(); ++state) {
        for (const Item &item : automaton.states[state]) {
            const Production &production = PRODUCTIONS[item.production];
            if (item.dot < static_cast<int>(production.rhs.size())) {
                const std::string &next = production.rhs[item.dot];
                auto transition = automaton.transitions.find({static_cast<int>(state), next});
                if (transition == automaton.transitions.end()) {
                    continue;
                }
                if (isNonTerminal(next)) {
                    table.gotoTable[state][next] = transition->second;
                } else {
                    addAction(table.action[state][next], "s" + std::to_string(transition->second));
                }
            } else if (item.production == 0) {
                addAction(table.action[state][END_MARKER], "acc");
            } else {
                for (const std::string &lookahead : lookaheads) {
                    if (!useFollow || follow.at(production.lhs).count(lookahead)) {
                        addAction(table.action[state][lookahead], "r" + std::to_string(item.production));
                    }
                }
            }
        }
    }
    return table;
}

std::string cellText(const ActionCell &cell) {
    std::string text;
    for (std::size_t index = 0; index < cell.size(); ++index) {
        text += (index ? "/" : "") + cell[index];
    }
    return text;
}

std::vector<std::string> findConflicts(const ParsingTable &table) {
    std::vector<std::string> conflicts;
    for (std::size_t state = 0; state < table.action.size(); ++state) {
        for (const auto &entry : table.action[state]) {
            if (entry.second.size() < 2) {
                continue;
            }
            bool hasShift = false;
            bool hasReduce = false;
            std::string reduceText;
            for (const std::string &action : entry.second) {
                if (action[0] == 's') {
                    hasShift = true;
                } else if (action[0] == 'r') {
                    if (hasReduce) {
                        reduceText += ", ";
                    }
                    hasReduce = true;
                    reduceText += productionText(std::stoi(action.substr(1)));
                }
            }
            std::string kind = hasShift && hasReduce ? "shift-reduce" : "reduce-reduce";
            conflicts.push_back("State " + std::to_string(state) + ", symbol '" + entry.first + "': " + kind +
                                " conflict, cell = " + cellText(entry.second) +
                                (hasReduce ? " (reduce by " + reduceText + ")" : ""));
        }
    }
    return conflicts;
}

void printTable(const ParsingTable &table, const std::string &title) {
    std::vector<std::string> lookaheads = TERMINALS;
    lookaheads.push_back(END_MARKER);
    std::cout << title << "\n";
    std::cout << "  " << std::left << std::setw(7) << "State";
    for (const std::string &symbol : lookaheads) {
        std::cout << std::setw(9) << symbol;
    }
    std::cout << "| ";
    for (const std::string &symbol : NON_TERMINALS) {
        std::cout << std::setw(5) << symbol;
    }
    std::cout << "\n  " << std::string(7 + 9 * lookaheads.size() + 2 + 5 * NON_TERMINALS.size(), '-') << "\n";
    for (std::size_t state = 0; state < table.action.size(); ++state) {
        std::cout << "  " << std::left << std::setw(7) << state;
        for (const std::string &symbol : lookaheads) {
            auto cell = table.action[state].find(symbol);
            std::cout << std::setw(9) << (cell == table.action[state].end() ? "" : cellText(cell->second));
        }
        std::cout << "| ";
        for (const std::string &symbol : NON_TERMINALS) {
            auto target = table.gotoTable[state].find(symbol);
            std::cout << std::setw(5) << (target == table.gotoTable[state].end() ? "" : std::to_string(target->second));
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

std::string joinStack(const std::vector<std::string> &stack) {
    std::string text;
    for (const std::string &entry : stack) {
        text += (text.empty() ? "" : " ") + entry;
    }
    return text;
}

bool parseInput(const ParsingTable &table, const std::string &input) {
    for (char character : input) {
        if (std::find(TERMINALS.begin(), TERMINALS.end(), std::string(1, character)) == TERMINALS.end()) {
            std::cout << "  Rejected: '" << character << "' is not a terminal of the grammar\n";
            return false;
        }
    }
    std::vector<std::string> stack = {"0"};
    std::size_t position = 0;
    std::cout << "  " << std::left << std::setw(20) << "Stack" << std::setw(12) << "Input" << "Action\n";
    while (true) {
        int state = std::stoi(stack.back());
        std::string lookahead = position < input.size() ? std::string(1, input[position]) : END_MARKER;
        std::string remaining = input.substr(position) + END_MARKER;
        auto cell = table.action[state].find(lookahead);
        std::string stackText = joinStack(stack);
        if (cell == table.action[state].end() || cell->second.empty()) {
            std::cout << "  " << std::setw(20) << stackText << std::setw(12) << remaining
                      << "error: no action in state " << state << " on '" << lookahead << "'\n";
            return false;
        }
        if (cell->second.size() > 1) {
            std::cout << "  " << std::setw(20) << stackText << std::setw(12) << remaining
                      << "error: conflicting actions " << cellText(cell->second) << "\n";
            return false;
        }
        const std::string &action = cell->second[0];
        if (action == "acc") {
            std::cout << "  " << std::setw(20) << stackText << std::setw(12) << remaining << "accept\n";
            return true;
        }
        if (action[0] == 's') {
            std::cout << "  " << std::setw(20) << stackText << std::setw(12) << remaining << "shift to state "
                      << action.substr(1) << "\n";
            stack.push_back(lookahead);
            stack.push_back(action.substr(1));
            ++position;
        } else {
            int productionIndex = std::stoi(action.substr(1));
            const Production &production = PRODUCTIONS[productionIndex];
            std::cout << "  " << std::setw(20) << stackText << std::setw(12) << remaining << "reduce by "
                      << productionText(productionIndex) << "\n";
            stack.resize(stack.size() - 2 * production.rhs.size());
            int exposed = std::stoi(stack.back());
            auto target = table.gotoTable[exposed].find(production.lhs);
            if (target == table.gotoTable[exposed].end()) {
                std::cout << "  error: missing GOTO entry\n";
                return false;
            }
            stack.push_back(production.lhs);
            stack.push_back(std::to_string(target->second));
        }
    }
}

}

int main(int argc, char *argv[]) {
    std::cout << "Original grammar:\n  S -> A\n  A -> Ab | a\n\nAugmented grammar:\n";
    for (std::size_t index = 0; index < PRODUCTIONS.size(); ++index) {
        std::cout << "  (" << index << ") " << productionText(static_cast<int>(index)) << "\n";
    }

    Automaton automaton = buildAutomaton();
    std::cout << "\nCanonical LR(0) collection of item sets:\n";
    for (std::size_t state = 0; state < automaton.states.size(); ++state) {
        std::cout << "  I" << state << ":\n";
        for (const Item &item : automaton.states[state]) {
            std::cout << "      " << itemText(item) << "\n";
        }
    }
    std::cout << "\nGOTO transitions:\n";
    for (const auto &entry : automaton.transitions) {
        std::cout << "  GOTO(I" << entry.first.first << ", " << entry.first.second << ") = I" << entry.second << "\n";
    }
    std::cout << "\n";

    std::map<std::string, std::set<std::string>> follow = computeFollow();
    ParsingTable lr0Table = buildTable(automaton, false, follow);
    printTable(lr0Table, "LR(0) parsing table (reduce on every terminal):");

    std::vector<std::string> conflicts = findConflicts(lr0Table);
    if (conflicts.empty()) {
        std::cout << "The grammar is LR(0): no conflicts.\n\n";
    } else {
        std::cout << "The grammar is NOT LR(0). Conflicts found:\n";
        for (const std::string &conflict : conflicts) {
            std::cout << "  " << conflict << "\n";
        }
        std::cout << "Explanation: a state that contains a completed item (a reduce) together with an item\n"
                     "that can still shift a terminal cannot be resolved without lookahead. An LR(0) parser\n"
                     "reduces on every terminal, so it cannot choose between shifting and reducing.\n\n";
    }

    std::cout << "FOLLOW sets used for SLR(1):\n";
    for (const auto &entry : follow) {
        std::cout << "  FOLLOW(" << entry.first << ") = { ";
        bool first = true;
        for (const std::string &symbol : entry.second) {
            std::cout << (first ? "" : ", ") << symbol;
            first = false;
        }
        std::cout << " }\n";
    }
    std::cout << "\n";

    ParsingTable slrTable = buildTable(automaton, true, follow);
    printTable(slrTable, "SLR(1) parsing table (reduce only on FOLLOW of the left-hand side):");
    std::vector<std::string> slrConflicts = findConflicts(slrTable);
    std::cout << (slrConflicts.empty() ? "The grammar is SLR(1): the SLR(1) table has no conflicts.\n\n"
                                       : "The SLR(1) table still has conflicts.\n\n");
    if (!slrConflicts.empty()) {
        return 1;
    }

    std::vector<std::string> inputs;
    for (int index = 1; index < argc; ++index) {
        inputs.push_back(argv[index]);
    }
    if (inputs.empty()) {
        inputs = {"abb", "a", "ba", "ab"};
    }
    int rejected = 0;
    for (const std::string &input : inputs) {
        std::cout << "Parsing \"" << input << "\" with the SLR(1) table:\n";
        bool accepted = parseInput(slrTable, input);
        std::cout << "  Result: " << (accepted ? "ACCEPTED" : "REJECTED") << "\n\n";
        if (!accepted) {
            ++rejected;
        }
    }
    return rejected > 0 && argc > 1 ? 2 : 0;
}
