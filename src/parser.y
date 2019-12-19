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

static AstProgram *root;

%}

%union {
  int                                       int_val;
  double                                    dou_val;
  char*                                     str_val;

  AstProgram*                               prog;
  Program_body*                             prog_body;
  Declaration_Node*                         decl;
  Function_Node*                            func;
  Statement_Node*                           stat;
  Expression_Node*                          expr;
  Id_Node*                                  id;
  Const_Node*                               Const;
  Compound_Node*                            compound;
  Array_Node*                               arr;
  Formal_Node*                              form;
  Variable_Reference_Node*                  vari;
  Function_Call_Node*                       func_call;
  Function_Call_expr_Node*                  func_call_expr;
  If_Node*                                  iff;

  vector<Declaration_Node *>*               decl_list;
  vector<Function_Node *>*                  func_list;
  vector<Statement_Node *>*                 stat_list;
  vector<Expression_Node *>*                expr_list;
  vector<Id_Node *>*                        id_list;
  vector<Array_Node *>*                     arr_list;
  vector<Formal_Node *>*                    form_list;
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
%token ARRAY
%token <str_val> BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token <str_val> FALSE TRUE
%token PRINT READ

    /* Identifier */
%token <str_val> ID

    /* Literal */
%token <str_val> INT_LITERAL
%token <str_val> REAL_LITERAL
%token <str_val> STRING_LITERAL

/* non-terminals */

%type <prog>                        Program
%type <prog_body>                   ProgramBody
%type <id>                          ProgramName FunctionName
%type <compound>                    CompoundStatement
%type <decl_list>                   DeclarationList Declarations
%type <decl>                        Declaration
%type <func_list>                   FunctionList Functions
%type <func>                        FunctionDeclaration
%type <stat_list>                   StatementList Statements ElseOrNot
%type <stat>                        Statement Simple Condition While For Return FunctionInvokation
%type <expr_list>                   ExpressionList Expressions ArrForm
%type <expr>                        Expression
%type <id_list>                     IdList
%type <Const>                       TypeOrConstant Type LiteralConstant ArrType
%type <func_call>                   FunctionCall
%type <func_call_expr>              FunctionCall_expr
%type <vari>                        VariableReference
%type <str_val>                     ScalarType ReturnType
%type <arr_list>                    ArrDecl
%type <form_list>                   FormalArgList FormalArgs
%type <form>                        FormalArg

%%
    /*
       Program Units
                     */

Program     : ProgramName SEMICOLON ProgramBody END ProgramName {$$ = new AstProgram($1, $5, $3, @1.first_line, @1.first_column); root = $$;}
            ;

ProgramName     : ID {$$ = new Id_Node($1, @1.first_line, @1.first_column);}
                ;

ProgramBody     : DeclarationList FunctionList CompoundStatement {$$ = new Program_body($1, $2, $3);}
                ;

DeclarationList     : Epsilon {$$ = NULL;}
                    | Declarations {$$ = $1;}
                    ;

Declarations    : Declaration {$$ = new vector<Declaration_Node *>(); $$->push_back($1);}
                | Declarations Declaration {$1->push_back($2); $$ = $1;}
                ;

FunctionList    : Epsilon {$$ = NULL;}
                | Functions {$$ = $1;}
                ;

Functions       : FunctionDeclaration {$$ = new vector<Function_Node *>(); $$->push_back($1);}
                | Functions FunctionDeclaration {$1->push_back($2); $$ = $1;}
                ;

FunctionDeclaration     : FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON CompoundStatement END FunctionName {
                          $$ = new Function_Node($1, $9, $3, $7, $5, @1.first_line, @1.first_column);}
                        ;

FunctionName    : ID {$$ = new Id_Node($1, yylloc.first_line, yylloc.first_column);}
                ;

FormalArgList       : Epsilon {$$ = NULL;}
                    | FormalArgs {$$ = $1;}
                    ;

FormalArgs      : FormalArg {$$ = new vector<Formal_Node *>(); $$->push_back($1);}
                | FormalArgs SEMICOLON FormalArg {$1->push_back($3); $$ = $1;}
                ;

FormalArg       : IdList COLON Type {$$ = new Formal_Node($1, $3, @1.first_line, @1.first_column);}
                ;

IdList      : ID {$$ = new vector<Id_Node *>(); Id_Node* tmp = new Id_Node($1, yylloc.first_line, yylloc.first_column); $$->push_back(tmp);}
            | IdList COMMA ID {Id_Node* tmp = new Id_Node($3, yylloc.first_line, yylloc.first_column); $1->push_back(tmp); $$ = $1;}
            ;

ReturnType      : COLON ScalarType {$$ = $2;}
                | Epsilon {$$ = NULL;}
                ;

    /*
       Data Types and Declarations
                                   */

Declaration     : VAR IdList COLON TypeOrConstant SEMICOLON {$$ = new Declaration_Node($2, $4, @1.first_line, @1.first_column);}
                ;

TypeOrConstant      : Type {$$ = $1;}
                    | LiteralConstant {$$ = $1;}
                    ;

Type    : ScalarType {$$ = new Const_Node($1, 0, yylloc.first_line, yylloc.first_column);}
        | ArrType {$$ = $1;}
        ;

ScalarType      : INTEGER {$$ = $1;}
                | REAL {$$ = $1;}
                | STRING {$$ = $1;}
                | BOOLEAN {$$ = $1;}
                ;

ArrType     : ArrDecl ScalarType {Const_Node* tmp = new Const_Node($2, 5, yylloc.first_line, yylloc.first_column); tmp->arr_list = $1; $$ = tmp;}
            ;

ArrDecl     : ARRAY INT_LITERAL TO INT_LITERAL OF {$$ = new vector<Array_Node *>(); Array_Node* tmp = new Array_Node($2, $4); $$->push_back(tmp);}
            | ArrDecl ARRAY INT_LITERAL TO INT_LITERAL OF {Array_Node* tmp = new Array_Node($3, $5); $1->push_back(tmp); $$ = $1;}
;

LiteralConstant     : INT_LITERAL {$$ = new Const_Node($1, 1, yylloc.first_line, yylloc.first_column);}
                    | REAL_LITERAL {$$ = new Const_Node($1, 2, yylloc.first_line, yylloc.first_column);}
                    | STRING_LITERAL {$$ = new Const_Node($1, 3, yylloc.first_line, yylloc.first_column);}
                    | TRUE {$$ = new Const_Node($1, 4, yylloc.first_line, yylloc.first_column);}
                    | FALSE {$$ = new Const_Node($1, 4, yylloc.first_line, yylloc.first_column);}
                    ;

    /*
       Statements
                  */

Statement       : CompoundStatement {$$ = $1;}
                | Simple {$$ = $1;}
                | Condition {$$ = $1;}
                | While {$$ = $1;}
                | For {$$ = $1;}
                | Return {$$ = $1;}
                | FunctionInvokation {$$ = $1;}
                ;

CompoundStatement       : BEGIN_ DeclarationList StatementList END {$$ = new Compound_Node($2, $3, @1.first_line, @1.first_column);}
                        ;

Simple      : VariableReference ASSIGN Expression SEMICOLON {$$ = new Assignment_Node($1, $3, @2.first_line, @2.first_column);}
            | PRINT Expression SEMICOLON {$$ = new Print_Node($2, @1.first_line, @1.first_column);}
            | READ VariableReference SEMICOLON { $$ = new Read_Node($2, @1.first_line, @1.first_column);}
            ;

VariableReference       : ID {Id_Node* tmp = new Id_Node($1, yylloc.first_line, yylloc.first_column); $$ = new Variable_Reference_Node(tmp, 
                        NULL, @1.first_line, @1.first_column);}
                        | ID ArrForm {Id_Node* tmp = new Id_Node($1, yylloc.first_line, yylloc.first_column); $$ = new Variable_Reference_Node(tmp, $2, @1.first_line, @1.first_column);}
                        ;

ArrForm     : L_BRACKET Expression R_BRACKET {$$ = new vector<Expression_Node*>(); $$->push_back($2);}
            | ArrForm L_BRACKET Expression R_BRACKET {$1->push_back($3); $$ = $1;}
            ;

Condition       : IF Expression THEN StatementList ElseOrNot END IF {$$ = new If_Node($2, $4, $5, @1.first_line, @1.first_column);}
                ;

ElseOrNot       : ELSE StatementList {$$ = $2;}
                | Epsilon {$$ = NULL;}
                ;

While       : WHILE Expression DO StatementList END DO {$$ = new While_Node($2, $4, @1.first_line, @1.first_column);}
            ;

For     : FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO StatementList END DO {Id_Node* tmp = new Id_Node($2, @2.first_line, @2.first_column);
        Variable_Reference_Node* tmp2 = new Variable_Reference_Node(tmp, NULL, @2.first_line, @2.first_column);
        Const_Node* tmp4 = new Const_Node($4, 1, @4.first_line, @4.first_column);
        Assignment_Node* tmp3 = new Assignment_Node(tmp2, tmp4, @3.first_line, @3.first_column);
        Const_Node* tmp5 = new Const_Node($6, 1, @6.first_line, @6.first_column);
        $$ = new For_Node(tmp, tmp3, tmp4, tmp5, $8, @1.first_line, @1.first_column);}
        ;

Return      : RETURN Expression SEMICOLON {$$ = new Return_Node($2, @1.first_line, @1.first_column);}
            ;

FunctionInvokation      : FunctionCall SEMICOLON {$$ = $1;}
                        ;

FunctionCall    : ID L_PARENTHESIS ExpressionList R_PARENTHESIS {Id_Node* tmp = new Id_Node($1, yylloc.first_line, yylloc.first_column); 
                $$ = new Function_Call_Node(tmp, $3, @1.first_line, @1.first_column);}
                ;

FunctionCall_expr       : ID L_PARENTHESIS ExpressionList R_PARENTHESIS {Id_Node* tmp = new Id_Node($1, yylloc.first_line, yylloc.first_column); 
                        $$ = new Function_Call_expr_Node(tmp, $3, @1.first_line, @1.first_column);}
                        ;

ExpressionList      : Epsilon {$$ = NULL;}
                    | Expressions {$$ = $1;}
                    ;

Expressions     : Expression {$$ = new vector<Expression_Node *>(); $$->push_back($1);}
                | Expressions COMMA Expression {$1->push_back($3); $$ = $1;}
                ;

StatementList       : Epsilon {$$ = NULL;}
                    | Statements {$$ = $1;}
                    ;

Statements      : Statement {$$ = new vector<Statement_Node *>(); $$->push_back($1);}
                | Statements Statement {$1->push_back($2); $$ = $1;}
                ;

Expression      : L_PARENTHESIS Expression R_PARENTHESIS {$$ = $2;}
                | MINUS Expression %prec UNARY_MINUS {$$ = new Unary_Operator_Node($2, strdup("neg"), @1.first_line, @1.first_column);}
                | Expression MULTIPLY Expression {$$ = new Binary_Operator_Node($1, $3, strdup("*"), @2.first_line, @2.first_column);}
                | Expression DIVIDE Expression {$$ = new Binary_Operator_Node($1, $3, strdup("/"), @2.first_line, @2.first_column);}
                | Expression MOD Expression {$$ = new Binary_Operator_Node($1, $3, strdup("mod"), @2.first_line, @2.first_column);}
                | Expression PLUS Expression {$$ = new Binary_Operator_Node($1, $3, strdup("+"), @2.first_line, @2.first_column);}
                | Expression MINUS Expression {$$ = new Binary_Operator_Node($1, $3, strdup("-"), @2.first_line, @2.first_column);}
                | Expression LESS Expression {$$ = new Binary_Operator_Node($1, $3, strdup("<"), @2.first_line, @2.first_column);}
                | Expression LESS_OR_EQUAL Expression {$$ = new Binary_Operator_Node($1, $3, strdup("<="), @2.first_line, @2.first_column);}
                | Expression GREATER Expression {$$ = new Binary_Operator_Node($1, $3, strdup(">"), @2.first_line, @2.first_column);}
                | Expression GREATER_OR_EQUAL Expression {$$ = new Binary_Operator_Node($1, $3, strdup(">="), @2.first_line, @2.first_column);}
                | Expression EQUAL Expression {$$ = new Binary_Operator_Node($1, $3, strdup("="), @2.first_line, @2.first_column);}
                | Expression NOT_EQUAL Expression {$$ = new Binary_Operator_Node($1, $3, strdup("<>"), @2.first_line, @2.first_column);}
                | NOT Expression {$$ = new Unary_Operator_Node($2, strdup("not"), @1.first_line, @1.first_column);}
                | Expression AND Expression {$$ = new Binary_Operator_Node($1, $3, strdup("and"), @2.first_line, @2.first_column);}
                | Expression OR Expression {$$ = new Binary_Operator_Node($1, $3, strdup("or"), @2.first_line, @2.first_column);}
                | LiteralConstant {$$ = $1;}
                | VariableReference {$$ = $1;}
                | FunctionCall_expr {$$ = $1;}
                ;

    /*
       misc
            */
Epsilon     :
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
    CHECK((argc >= 2) && (argc<=3), "Usage: ./parser <filename> [--dump-ast]\n");
    
    int isDumpNeed;
    if(argc == 3){
        isDumpNeed = strcmp(argv[2], "--dump-ast");
        if(isDumpNeed != 0){
            fprintf(stderr, "Usage: ./parser <filename> [--dump-ast]\n");
            exit(-1);                                                          
        }
    }
        
    FILE *fp = fopen(argv[1], "r");

    CHECK(fp != NULL, "fopen() fails.\n");
    yyin = fp;
    yyparse();

    if(argc == 3 && isDumpNeed == 0) {
        Visitor vs;
        root->accept(vs);
    }

    delete root;
    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    fclose(fp);
    return 0;
}
            