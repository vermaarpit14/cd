#include <iostream>
#include <string>
#include <vector>

namespace {

std::string repeat(const std::string &unit, int count) {
    std::string result;
    for (int index = 0; index < count; ++index) {
        result += unit;
    }
    return result;
}

std::string buildLanguageString(int n) {
    return repeat("aab", n) + repeat("a", n);
}

bool isValidInput(const std::string &text) {
    if (text.empty()) {
        return false;
    }
    for (char character : text) {
        if (character != 'a' && character != 'b') {
            return false;
        }
    }
    return true;
}

bool derivableFromStart(const std::string &text, int &depth) {
    if (text == "aaba") {
        depth = 1;
        return true;
    }
    if (text.size() >= 8 && text.compare(0, 3, "aab") == 0 && text.back() == 'a') {
        int innerDepth = 0;
        if (derivableFromStart(text.substr(3, text.size() - 4), innerDepth)) {
            depth = innerDepth + 1;
            return true;
        }
    }
    return false;
}

std::vector<std::string> buildDerivation(int n) {
    std::vector<std::string> steps;
    steps.push_back("S");
    for (int level = 1; level < n; ++level) {
        steps.push_back(repeat("aab", level) + "S" + repeat("a", level));
    }
    steps.push_back(buildLanguageString(n));
    return steps;
}

void printDerivation(int n) {
    std::vector<std::string> steps = buildDerivation(n);
    std::cout << "  " << steps.front() << "\n";
    for (std::size_t index = 1; index < steps.size(); ++index) {
        bool lastStep = index + 1 == steps.size();
        std::cout << "  => " << steps[index] << "   [" << (lastStep ? "S -> aaba" : "S -> aabSa") << "]\n";
    }
}

}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [terminal_string]\n";
        return 1;
    }
    std::string input = argc == 2 ? argv[1] : "aabaabaa";
    if (!isValidInput(input)) {
        std::cerr << "Error: input must be a non-empty string over {a, b}\n";
        return 1;
    }

    std::cout << "Terminal string : " << input << "\n\n";
    std::cout << "Language pattern:\n  L = { (aab)^n a^n | n > 0 }\n\n";
    std::cout << "Context-free grammar:\n  S -> aabSa | aaba\n\n";

    std::cout << "Derivation for n = 2:\n";
    printDerivation(2);
    std::string expected = buildLanguageString(2);
    std::cout << "\nVerification for n = 2:\n";
    std::cout << "  (aab)^2 a^2 = " << expected << "\n";
    std::cout << "  Equals aabaabaa : " << (expected == "aabaabaa" ? "yes" : "no") << "\n\n";

    int depth = 0;
    bool member = derivableFromStart(input, depth);
    std::cout << "Membership of \"" << input << "\" in L: " << (member ? "ACCEPTED" : "REJECTED") << "\n";
    if (member) {
        std::cout << "  Matches the pattern with n = " << depth << "\n";
        std::cout << "Derivation of the input:\n";
        printDerivation(depth);
    }

    std::cout << "\nFirst strings of L:\n";
    for (int n = 1; n <= 4; ++n) {
        std::cout << "  n = " << n << " : " << buildLanguageString(n) << "\n";
    }
    return member ? 0 : 2;
}
