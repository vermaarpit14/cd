%code {
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(const char *text);
void yy_delete_buffer(YY_BUFFER_STATE buffer);

int yylex(void);
void yyerror(const char *message);

int openParenthesisCount = 0;
int unmatchedCloseCount = 0;
std::string lexicalMessage;
static int divisionByZeroDetected = 0;
static double expressionResult = 0.0;
static std::string syntaxMessage;
}

%union {
    double number;
}

%define parse.error verbose

%token <number> NUMBER "number"
%token INVALID "invalid character"

%type <number> expression

%left '+' '-'
%left '*' '/'
%right UMINUS

%%

input
    : expression { expressionResult = $1; }
    ;

expression
    : NUMBER { $$ = $1; }
    | expression '+' expression { $$ = $1 + $3; }
    | expression '-' expression { $$ = $1 - $3; }
    | expression '*' expression { $$ = $1 * $3; }
    | expression '/' expression {
        if ($3 == 0.0) {
            divisionByZeroDetected = 1;
            $$ = 0.0;
        } else {
            $$ = $1 / $3;
        }
    }
    | '-' expression %prec UMINUS { $$ = -$2; }
    | '(' expression ')' { $$ = $2; }
    ;

%%

void yyerror(const char *message) {
    if (syntaxMessage.empty()) {
        syntaxMessage = message;
    }
}

static std::string trim(const std::string &text) {
    std::size_t begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) {
        return "";
    }
    std::size_t end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

static std::string formatNumber(double value) {
    std::ostringstream stream;
    stream.precision(12);
    stream << value;
    return stream.str();
}

static int evaluateLine(const std::string &line) {
    openParenthesisCount = 0;
    unmatchedCloseCount = 0;
    divisionByZeroDetected = 0;
    expressionResult = 0.0;
    lexicalMessage.clear();
    syntaxMessage.clear();

    YY_BUFFER_STATE buffer = yy_scan_string(line.c_str());
    int status = yyparse();
    if (status != 0) {
        while (yylex() != 0) {
        }
    }
    yy_delete_buffer(buffer);

    std::cout << "Expression: " << line << "\n";
    bool balanced = openParenthesisCount == 0 && unmatchedCloseCount == 0;
    if (balanced) {
        std::cout << "Parentheses Balanced\n";
    } else {
        std::cout << "Parentheses Unbalanced: unmatched parentheses";
        if (openParenthesisCount > 0) {
            std::cout << " (" << openParenthesisCount << " unmatched '(')";
        }
        if (unmatchedCloseCount > 0) {
            std::cout << " (" << unmatchedCloseCount << " unmatched ')')";
        }
        std::cout << "\n";
    }
    if (status != 0) {
        std::string reason = lexicalMessage.empty() ? syntaxMessage : lexicalMessage;
        std::cout << "Syntax Invalid: " << reason << "\n";
        return 1;
    }
    std::cout << "Syntax Valid\n";
    if (divisionByZeroDetected) {
        std::cout << "Error: Division by zero\n";
        return 1;
    }
    std::cout << "Result = " << formatNumber(expressionResult) << "\n";
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [input_file]\n";
        return 1;
    }
    std::ifstream file;
    if (argc == 2) {
        file.open(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open input file '" << argv[1] << "'\n";
            return 1;
        }
    }
    std::istream &source = argc == 2 ? static_cast<std::istream &>(file) : std::cin;

    int failures = 0;
    int evaluated = 0;
    std::string line;
    while (std::getline(source, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        if (evaluated > 0) {
            std::cout << "\n";
        }
        ++evaluated;
        failures += evaluateLine(line);
    }
    if (evaluated == 0) {
        std::cerr << "Error: no expressions to evaluate\n";
        return 1;
    }
    return failures > 0 ? 2 : 0;
}
