%code requires {
typedef struct {
    int isFloat;
    long long integerValue;
    double floatValue;
} Value;
}

%code {
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

extern int yylineno;
extern FILE *yyin;

int yylex(void);
void yyerror(const char *message);
void lexicalError(const char *text);

static std::map<std::string, Value> symbolTable;
static int evaluationFailed = 0;
static int errorCount = 0;

static void reportError(const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    std::printf("Error (line %d): ", yylineno);
    std::vprintf(format, arguments);
    std::printf("\n");
    va_end(arguments);
    ++errorCount;
}

static Value makeInteger(long long number) {
    Value value;
    value.isFloat = 0;
    value.integerValue = number;
    value.floatValue = 0.0;
    return value;
}

static double asDouble(Value value) {
    return value.isFloat ? value.floatValue : static_cast<double>(value.integerValue);
}

static bool isZero(Value value) {
    return value.isFloat ? value.floatValue == 0.0 : value.integerValue == 0;
}

static std::string formatValue(Value value) {
    if (!value.isFloat) {
        return std::to_string(value.integerValue);
    }
    std::ostringstream stream;
    stream << std::setprecision(10) << value.floatValue;
    std::string text = stream.str();
    if (text.find_first_of(".en") == std::string::npos) {
        text += ".0";
    }
    return text;
}

static Value negate(Value value) {
    if (value.isFloat) {
        value.floatValue = -value.floatValue;
    } else {
        value.integerValue = -value.integerValue;
    }
    return value;
}

static Value applyOperator(char symbol, Value left, Value right) {
    if (evaluationFailed) {
        return makeInteger(0);
    }
    if (symbol == '/' && isZero(right)) {
        reportError("division by zero");
        evaluationFailed = 1;
        return makeInteger(0);
    }
    if (!left.isFloat && !right.isFloat) {
        switch (symbol) {
            case '+': return makeInteger(left.integerValue + right.integerValue);
            case '-': return makeInteger(left.integerValue - right.integerValue);
            case '*': return makeInteger(left.integerValue * right.integerValue);
            default: return makeInteger(left.integerValue / right.integerValue);
        }
    }
    double a = asDouble(left);
    double b = asDouble(right);
    Value result;
    result.isFloat = 1;
    result.integerValue = 0;
    switch (symbol) {
        case '+': result.floatValue = a + b; break;
        case '-': result.floatValue = a - b; break;
        case '*': result.floatValue = a * b; break;
        default: result.floatValue = a / b; break;
    }
    return result;
}
}

%union {
    Value value;
    char *name;
}

%define parse.error verbose

%token <value> INTEGER_LITERAL "integer"
%token <value> FLOAT_LITERAL "float"
%token <name> IDENTIFIER "identifier"
%token PRINT "print"

%type <value> expression

%destructor { std::free($$); } <name>

%left '+' '-'
%left '*' '/'
%right UMINUS

%%

program
    : %empty
    | program statement
    ;

statement
    : IDENTIFIER '=' expression ';' {
        if (!evaluationFailed) {
            symbolTable[$1] = $3;
            std::printf("%s = %s\n", $1, formatValue($3).c_str());
        }
        evaluationFailed = 0;
        std::free($1);
    }
    | PRINT expression ';' {
        if (!evaluationFailed) {
            std::printf("Result = %s\n", formatValue($2).c_str());
        }
        evaluationFailed = 0;
    }
    | error ';' {
        evaluationFailed = 0;
        yyerrok;
    }
    ;

expression
    : INTEGER_LITERAL { $$ = $1; }
    | FLOAT_LITERAL { $$ = $1; }
    | IDENTIFIER {
        auto found = symbolTable.find($1);
        if (found == symbolTable.end()) {
            if (!evaluationFailed) {
                reportError("undefined variable '%s'", $1);
            }
            evaluationFailed = 1;
            $$ = makeInteger(0);
        } else {
            $$ = found->second;
        }
        std::free($1);
    }
    | expression '+' expression { $$ = applyOperator('+', $1, $3); }
    | expression '-' expression { $$ = applyOperator('-', $1, $3); }
    | expression '*' expression { $$ = applyOperator('*', $1, $3); }
    | expression '/' expression { $$ = applyOperator('/', $1, $3); }
    | '-' expression %prec UMINUS { $$ = negate($2); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

void yyerror(const char *message) {
    std::printf("Syntax error (line %d): %s\n", yylineno, message);
    ++errorCount;
}

void lexicalError(const char *text) {
    std::printf("Lexical error (line %d): unexpected character '%s'\n", yylineno, text);
    ++errorCount;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::fprintf(stderr, "Usage: %s [input_file]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        yyin = std::fopen(argv[1], "r");
        if (yyin == nullptr) {
            std::perror(argv[1]);
            return 1;
        }
    }
    int status = yyparse();
    if (yyin != stdin) {
        std::fclose(yyin);
    }
    return (status != 0 || errorCount > 0) ? 2 : 0;
}
