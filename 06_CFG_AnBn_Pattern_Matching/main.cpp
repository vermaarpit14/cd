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

bool matchesAnB(std::string S, std::string A, std::string B, int n) {
    if (n <= 0 || A.empty() || B.empty()) {
        return false;
    }
    return S == repeat(A, n) + repeat(B, n);
}

int deriveCount(const std::string &S, const std::string &A, const std::string &B) {
    if (S == A + B) {
        return 1;
    }
    std::size_t minimum = A.size() + B.size();
    if (S.size() > minimum && S.compare(0, A.size(), A) == 0 &&
        S.compare(S.size() - B.size(), B.size(), B) == 0) {
        int inner = deriveCount(S.substr(A.size(), S.size() - minimum), A, B);
        return inner > 0 ? inner + 1 : 0;
    }
    return 0;
}

void printGrammar(const std::string &A, const std::string &B) {
    std::cout << "Context-free grammar:\n";
    std::cout << "  S -> A S B | A B\n";
    std::cout << "  A -> " << A << "\n";
    std::cout << "  B -> " << B << "\n\n";
    std::cout << "Language pattern:\n  L = { A^n B^n | n > 0 } = { (" << A << ")^n (" << B << ")^n | n > 0 }\n\n";
}

void printDerivation(const std::string &A, const std::string &B, int n) {
    std::cout << "Derivation for n = " << n << ":\n";
    std::cout << "  S\n";
    for (int level = 1; level < n; ++level) {
        std::cout << "  => " << (level > 1 ? repeat("A ", level) : std::string("A ")) << "S "
                  << repeat("B ", level) << "  [S -> A S B]\n";
    }
    std::string symbolic = repeat("A ", n) + repeat("B ", n);
    std::cout << "  => " << symbolic << "  [S -> A B]\n";
    std::cout << "  => " << repeat(A, n) << repeat(B, n) << "  [A -> " << A << ", B -> " << B << "]\n\n";
}

void runCase(const std::string &S, const std::string &A, const std::string &B, int n) {
    std::cout << "Test: S = \"" << S << "\", A = \"" << A << "\", B = \"" << B << "\", n = " << n << "\n";
    bool result = matchesAnB(S, A, B, n);
    std::cout << "  matchesAnB(S, A, B, n) = " << (result ? "true" : "false") << "\n";
    if (!A.empty() && !B.empty()) {
        int derived = deriveCount(S, A, B);
        if (derived > 0) {
            std::cout << "  Grammar derives S with n = " << derived << "\n";
        } else {
            std::cout << "  Grammar cannot derive S\n";
        }
    }
    std::cout << "\n";
}

bool parseInteger(const std::string &text, int &value) {
    try {
        std::size_t consumed = 0;
        value = std::stoi(text, &consumed);
        return consumed == text.size();
    } catch (const std::exception &) {
        return false;
    }
}

}

int main(int argc, char *argv[]) {
    if (argc != 1 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [S A B n]\n";
        return 1;
    }

    std::string S = "aabaabaa";
    std::string A = "aab";
    std::string B = "a";
    int n = 2;
    if (argc == 5) {
        S = argv[1];
        A = argv[2];
        B = argv[3];
        if (!parseInteger(argv[4], n)) {
            std::cerr << "Error: n must be an integer\n";
            return 1;
        }
        if (A.empty() || B.empty()) {
            std::cerr << "Error: A and B must be non-empty strings\n";
            return 1;
        }
    }

    printGrammar(A, B);
    if (n > 0) {
        printDerivation(A, B, n);
    }

    std::cout << "Matching result:\n";
    runCase(S, A, B, n);

    if (argc == 1) {
        std::cout << "Additional test cases:\n";
        runCase("aabaabaabaaa", "aab", "a", 3);
        runCase("aabaabaabaaa", "aab", "a", 2);
        runCase("aabaabaa", "aab", "a", 0);
        runCase("aaba", "aab", "a", 1);
        runCase("aabaaba", "aab", "a", 2);
        runCase("abab", "ab", "ab", 1);
    }
    return matchesAnB(S, A, B, n) ? 0 : 2;
}
