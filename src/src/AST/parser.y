%{
#include "include/AST/ast.h"
#include "include/AST/visitor.h"
#include "include/core/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

/* Declared by scanner.l */
extern int32_t LineNum;
extern char Buffer[512];

/* Declared by lex */
extern FILE *yyin;
extern char *yytext;

extern int yylex(void);
static void yyerror(const char *msg);

Visitor vs;

%}

%union {
  int                   int_val;
  double                dou_val;
  char*                 str_val;

  AstProgram*           prog;
  Program_body*         prog_body;
  Declaration_Node*     decl;
  Id_Node*              id;
  Const_Node*           Const;
  vector<Declaration_Node *>*               decl_list;
  vector<Id_Node *> *                       id_list;
}

%locations

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token <str_val> ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

/* non-terminals */

%type <prog>                        Program
%type <prog_body>                   ProgramBody
%type <id>                          ProgramName
%type <decl_list>                   DeclarationList Declarations
%type <decl>                        Declaration
%type <func>                        FunctionList
%type <decl>                        CompoundStatement
%type <id_list>                     IdList
%type <Const>                       TypeOrConstant Type
%type <str_val>                     ScalarType INTEGER REAL STRING BOOLEAN

%%
    /*
       Program Units
                     */

Program:
    ProgramName SEMICOLON ProgramBody END ProgramName {$$ = new AstProgram($1, @1.first_line, @1.first_column); $$->accept(vs);}
;

ProgramName:
    ID {$$ = new Id_Node($1, yylloc.first_line, yylloc.first_column); $$->accept(vs);}
;

ProgramBody:
    DeclarationList FunctionList CompoundStatement {$$ = new Program_body($1);}
;

DeclarationList:
    Epsilon {$$ = NULL;}
    |
    Declarations {$$ = $1;}
;

Declarations:
    Declaration {$$ = new vector<Declaration_Node *>(); $$->push_back($1);}
    |
    Declarations Declaration {$1->push_back($2); $$ = $1;}
;

FunctionList:
    Epsilon
    |
    Functions
;

Functions:
    FunctionDeclaration
    |
    Functions FunctionDeclaration
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON
    CompoundStatement
    END FunctionName
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon
    |
    FormalArgs
;

FormalArgs:
    FormalArg
    |
    FormalArgs SEMICOLON FormalArg
;

FormalArg:
    IdList COLON Type
;

IdList:
    ID
    |
    IdList COMMA ID
;

ReturnType:
    COLON ScalarType
    |
    Epsilon
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON TypeOrConstant SEMICOLON {$$ = new Declaration_Node($2, $4);}
;

TypeOrConstant:
    Type {$$ = $1;}
    |
    LiteralConstant
;

Type:
    ScalarType {$$ = new Const_Node($1, yylloc.first_line, yylloc.first_column);}
    |
    ArrType
;

ScalarType:
    INTEGER {$$ = $1;}
    |
    REAL {$$ = $1;}
    |
    STRING {$$ = $1;}
    |
    BOOLEAN {$$ = $1;}
;

ArrType:
    ArrDecl ScalarType
;

ArrDecl:
    ARRAY INT_LITERAL TO INT_LITERAL OF
    |
    ArrDecl ARRAY INT_LITERAL TO INT_LITERAL OF
;

LiteralConstant:
    INT_LITERAL
    |
    REAL_LITERAL
    |
    STRING_LITERAL
    |
    TRUE
    |
    FALSE
;

    /*
       Statements
                  */

Statement:
    CompoundStatement
    |
    Simple
    |
    Condition
    |
    While
    |
    For
    |
    Return
    |
    FunctionInvokation
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON
    |
    PRINT Expression SEMICOLON
    |
    READ VariableReference SEMICOLON
;

VariableReference:
    ID
    |
    ID ArrForm
;

ArrForm:
    L_BRACKET Expression R_BRACKET
    |
    ArrForm L_BRACKET Expression R_BRACKET
;

Condition:
    IF Expression THEN
    StatementList
    ElseOrNot
    END IF
;

ElseOrNot:
    ELSE
    StatementList
    |
    Epsilon
;

While:
    WHILE Expression DO
    StatementList
    END DO
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    StatementList
    END DO
;

Return:
    RETURN Expression SEMICOLON
;

FunctionInvokation:
    FunctionCall SEMICOLON
;

FunctionCall:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS
;

ExpressionList:
    Epsilon
    |
    Expressions
;

Expressions:
    Expression
    |
    Expressions COMMA Expression
;

StatementList:
    Epsilon
    |
    Statements
;

Statements:
    Statement
    |
    Statements Statement
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS
    |
    MINUS Expression %prec UNARY_MINUS
    |
    Expression MULTIPLY Expression
    |
    Expression DIVIDE Expression
    |
    Expression MOD Expression
    |
    Expression PLUS Expression
    |
    Expression MINUS Expression
    |
    Expression LESS Expression
    |
    Expression LESS_OR_EQUAL Expression
    |
    Expression GREATER Expression
    |
    Expression GREATER_OR_EQUAL Expression
    |
    Expression EQUAL Expression
    |
    Expression NOT_EQUAL Expression
    |
    NOT Expression
    |
    Expression AND Expression
    |
    Expression OR Expression
    |
    LiteralConstant
    |
    VariableReference
    |
    FunctionCall
;

    /*
       misc
            */
Epsilon:
;
%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            LineNum, Buffer, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    CHECK(argc >= 2, "Usage: ./parser <filename>\n");

    FILE *fp = fopen(argv[1], "r");

    CHECK(fp != NULL, "fopen() fails.\n");
    yyin = fp;
    yyparse();


    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
