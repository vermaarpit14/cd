#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

using Symbols = std::vector<std::string>;

struct Grammar {
    std::vector<std::string> order;
    std::map<std::string, std::vector<Symbols>> rules;
};

const char *const DEFAULT_GRAMMAR =
    "E -> E + T | T\n"
    "T -> T * F | F\n"
    "F -> ( E ) | id\n";

const char *const EXPECTED_GRAMMAR =
    "E -> T E'\n"
    "E' -> + T E' | eps\n"
    "T -> F T'\n"
    "T' -> * F T' | eps\n"
    "F -> ( E ) | id\n";

std::vector<std::string> splitWords(const std::string &text) {
    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;
    while (stream >> word) {
        words.push_back(word);
    }
    return words;
}

bool parseGrammar(const std::string &text, Grammar &grammar, std::string &error) {
    std::istringstream stream(text);
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        if (splitWords(line).empty()) {
            continue;
        }
        std::size_t arrow = line.find("->");
        if (arrow == std::string::npos) {
            error = "line " + std::to_string(lineNumber) + ": missing '->'";
            return false;
        }
        std::vector<std::string> head = splitWords(line.substr(0, arrow));
        if (head.size() != 1) {
            error = "line " + std::to_string(lineNumber) + ": left side must be one non-terminal";
            return false;
        }
        const std::string &name = head[0];
        if (!grammar.rules.count(name)) {
            grammar.order.push_back(name);
        }
        std::string body = line.substr(arrow + 2);
        std::size_t start = 0;
        while (true) {
            std::size_t bar = body.find('|', start);
            std::string alternative = body.substr(start, bar == std::string::npos ? std::string::npos : bar - start);
            Symbols symbols = splitWords(alternative);
            if (symbols.empty()) {
                error = "line " + std::to_string(lineNumber) + ": empty alternative (use eps)";
                return false;
            }
            if (symbols.size() == 1 && symbols[0] == "eps") {
                symbols.clear();
            }
            grammar.rules[name].push_back(symbols);
            if (bar == std::string::npos) {
                break;
            }
            start = bar + 1;
        }
    }
    if (grammar.order.empty()) {
        error = "grammar is empty";
        return false;
    }
    return true;
}

std::string alternativeText(const Symbols &symbols) {
    if (symbols.empty()) {
        return "\xCE\xB5";
    }
    std::string text;
    for (std::size_t index = 0; index < symbols.size(); ++index) {
        text += (index ? " " : "") + symbols[index];
    }
    return text;
}

void printGrammar(const Grammar &grammar) {
    for (const std::string &name : grammar.order) {
        std::cout << "  " << name << " \xE2\x86\x92 ";
        const std::vector<Symbols> &alternatives = grammar.rules.at(name);
        for (std::size_t index = 0; index < alternatives.size(); ++index) {
            std::cout << (index ? " | " : "") << alternativeText(alternatives[index]);
        }
        std::cout << "\n";
    }
}

bool eliminateImmediate(Grammar &grammar, const std::string &name, std::string &error) {
    std::vector<Symbols> recursive;
    std::vector<Symbols> others;
    for (const Symbols &alternative : grammar.rules[name]) {
        if (!alternative.empty() && alternative[0] == name) {
            recursive.emplace_back(alternative.begin() + 1, alternative.end());
        } else {
            others.push_back(alternative);
        }
    }
    if (recursive.empty()) {
        return true;
    }
    if (others.empty()) {
        error = "non-terminal " + name + " has no non-recursive alternative";
        return false;
    }
    std::string freshName = name + "'";
    while (grammar.rules.count(freshName)) {
        freshName += "'";
    }
    std::vector<Symbols> updated;
    for (Symbols alternative : others) {
        alternative.push_back(freshName);
        updated.push_back(alternative);
    }
    std::vector<Symbols> tail;
    for (Symbols alternative : recursive) {
        alternative.push_back(freshName);
        tail.push_back(alternative);
    }
    tail.push_back({});
    grammar.rules[name] = updated;
    grammar.rules[freshName] = tail;
    auto position = std::find(grammar.order.begin(), grammar.order.end(), name);
    grammar.order.insert(position + 1, freshName);
    return true;
}

bool eliminateLeftRecursion(Grammar &grammar, std::string &error) {
    std::vector<std::string> original = grammar.order;
    for (std::size_t i = 0; i < original.size(); ++i) {
        const std::string &current = original[i];
        for (std::size_t j = 0; j < i; ++j) {
            const std::string &earlier = original[j];
            std::vector<Symbols> rewritten;
            for (const Symbols &alternative : grammar.rules[current]) {
                if (!alternative.empty() && alternative[0] == earlier) {
                    for (const Symbols &replacement : grammar.rules[earlier]) {
                        Symbols combined = replacement;
                        combined.insert(combined.end(), alternative.begin() + 1, alternative.end());
                        rewritten.push_back(combined);
                    }
                } else {
                    rewritten.push_back(alternative);
                }
            }
            grammar.rules[current] = rewritten;
        }
        if (!eliminateImmediate(grammar, current, error)) {
            return false;
        }
    }
    return true;
}

std::string canonicalForm(const Grammar &grammar) {
    std::string text;
    for (const std::string &name : grammar.order) {
        std::set<std::string> alternatives;
        for (const Symbols &alternative : grammar.rules.at(name)) {
            alternatives.insert(alternativeText(alternative));
        }
        text += name + ":";
        for (const std::string &alternative : alternatives) {
            text += "[" + alternative + "]";
        }
        text += ";";
    }
    return text;
}

std::set<std::string> enumerateLanguage(const Grammar &grammar, std::size_t maxTerminals) {
    std::set<std::string> language;
    std::set<Symbols> visited;
    std::vector<Symbols> queue = {{grammar.order.front()}};
    visited.insert(queue.front());
    std::size_t maxLength = 3 * maxTerminals + 3;
    for (std::size_t head = 0; head < queue.size(); ++head) {
        Symbols form = queue[head];
        std::size_t firstNonTerminal = form.size();
        std::size_t terminalCount = 0;
        for (std::size_t index = 0; index < form.size(); ++index) {
            if (grammar.rules.count(form[index])) {
                if (firstNonTerminal == form.size()) {
                    firstNonTerminal = index;
                }
            } else {
                ++terminalCount;
            }
        }
        if (firstNonTerminal == form.size()) {
            language.insert(alternativeText(form));
            continue;
        }
        for (const Symbols &alternative : grammar.rules.at(form[firstNonTerminal])) {
            Symbols next(form.begin(), form.begin() + firstNonTerminal);
            next.insert(next.end(), alternative.begin(), alternative.end());
            next.insert(next.end(), form.begin() + firstNonTerminal + 1, form.end());
            std::size_t nextTerminals = 0;
            for (const std::string &symbol : next) {
                if (!grammar.rules.count(symbol)) {
                    ++nextTerminals;
                }
            }
            if (nextTerminals > maxTerminals || next.size() > maxLength) {
                continue;
            }
            if (visited.insert(next).second) {
                queue.push_back(next);
            }
        }
        (void)terminalCount;
    }
    return language;
}

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [grammar_file]\n";
        return 1;
    }
    std::string text = DEFAULT_GRAMMAR;
    if (argc == 2) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open grammar file '" << argv[1] << "'\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        text = buffer.str();
    }

    Grammar original;
    std::string error;
    if (!parseGrammar(text, original, error)) {
        std::cerr << "Error: " << error << "\n";
        return 1;
    }

    Grammar transformed = original;
    if (!eliminateLeftRecursion(transformed, error)) {
        std::cerr << "Error: " << error << "\n";
        return 1;
    }

    std::cout << "Original grammar:\n";
    printGrammar(original);
    std::cout << "\nTransformed grammar (left recursion removed):\n";
    printGrammar(transformed);

    if (argc == 1) {
        Grammar expected;
        parseGrammar(EXPECTED_GRAMMAR, expected, error);
        std::cout << "\nMatches the required LL(1) grammar: "
                  << (canonicalForm(expected) == canonicalForm(transformed) ? "yes" : "no") << "\n";
    }

    const std::size_t limit = 4;
    std::set<std::string> before = enumerateLanguage(original, limit);
    std::set<std::string> after = enumerateLanguage(transformed, limit);
    std::cout << "Equivalence check on all sentences with at most " << limit << " terminals: "
              << before.size() << " sentences from the original, " << after.size()
              << " from the transformed grammar, " << (before == after ? "identical" : "DIFFERENT") << "\n";
    return before == after ? 0 : 2;
}
