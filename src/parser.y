%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "error.h"

extern int32_t LineNum;   /* declared in scanner.l */
extern char Buffer[512];  /* declared in scanner.l */
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */

extern int yylex(void); 
static void yyerror(const char *msg);
%}

    /* operator */
%token MOD ASSIGN LE NE GE AND OR NOT

    /* keyword */
%token ARRAY BEG TYPE DEF DO ELSE END FALSE FOR IF OF PRINT READ THEN TO TRUE RETURN VAR WHILE

    /* Identifier */
%token IDENTIFIER INT FLOAT SCIENTIFIC STRING OCTAL

%left NOT
%left AND
%left OR
%left NE GE LE '>' '<' '='
%left '*' '/' MOD
%left '+' '-'
%left NEG

%%

    /* program */
program     : IDENTIFIER ';' programbody END IDENTIFIER
            ;

programbody     : variables functions compound
                ;

    /* function */
functions       : function_declaration
                | 
                ;

function_declaration    : function function_declaration
                        | function
                        ;

function    : IDENTIFIER '(' arguments ')' ':' TYPE ';' compound END IDENTIFIER
            | IDENTIFIER '(' arguments ')' ';' compound END IDENTIFIER
            ;

identifier_list     : multi_identifier
                    | 
                    ;

multi_identifier    : multi_identifier ',' IDENTIFIER
                    | IDENTIFIER
                    ;

    /* arguments */
arguments       : have_arguments
                | 
                ;

have_arguments      : identifier_list ':' TYPE ';' have_arguments
                    | identifier_list ':' TYPE
                    | identifier_list ':' ARRAY INT TO INT OF array_types ';' have_arguments
                    | identifier_list ':' ARRAY INT TO INT OF array_types
                    ;

    /* variable */
variables       : variable_declaration
                |
                ;

variable_declaration    : variable_declaration variable
                        | variable
                        ;

variable    : VAR identifier_list ':' TYPE ';'
            | VAR identifier_list ':' ARRAY INT TO INT OF array_types';'
            | VAR identifier_list ':' literal_constant ';'
            ;

array_types     : ARRAY INT TO INT OF array_types
                | TYPE
                ;

    /* constant */
literal_constant    : number
                    | STRING
                    | TRUE
                    | FALSE
                    ;

number      : INT
            | OCTAL
            | SCIENTIFIC
            | FLOAT
            ;

    /* statement */
statements      : nonEmptystatements
                |
                ;

nonEmptystatements      : nonEmptystatements statement
                        | statement
                        ;

statement       : compound
                | simple
                | expression
                | conditional
                | while
                | for
                | return
                | function_invocation ';'
                ;

    /* compound */
compound    : BEG variables statements END
            ;

    /* simple */
simple      : variable_reference ASSIGN expression ';'
            | PRINT variable_reference ';'
            | PRINT expression ';'
            | READ variable_reference ';'
            ;

variable_reference      : array_reference
                        | IDENTIFIER
                        ;

    /* expression */
expression      : '-' expression %prec NEG
                | expression '>' expression
                | expression '<' expression
                | expression '=' expression
                | expression LE expression
                | expression GE expression
                | expression NE expression
                | expression AND expression
                | expression OR  expression
                | NOT expression %prec NOT
                | expression '+' expression
                | expression '-' expression
                | expression '*' expression
                | expression '/' expression
                | expression MOD expression
                | '(' expression ')' %prec '*'
                | number
                | IDENTIFIER
                | function_invocation
                | STRING
                | array_reference
                | literal_constant
                ;

array_reference     : IDENTIFIER multi_array_reference
                    ;

multi_array_reference       : '[' expression ']' multi_array_reference
                            | '[' expression ']'
                            ;

function_invocation     : IDENTIFIER '(' expression_list ')'
                        | IDENTIFIER '(' ')'
                        ;

expression_list     : expression ',' expression_list
                    | expression
                    ;

    /* conditional */
conditional     : IF expression THEN conditional_body END IF
                ;

conditional_body    : statements
                    | statements ELSE statements

    /* while */
while       : WHILE expression DO statements END DO
            ;

    /* for */
for     : FOR IDENTIFIER ASSIGN INT TO INT DO statements END DO
        ;

    /* return */
return      : RETURN expression ';'
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
    CHECK(argc == 2, "Usage: ./parser <filename>\n");

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
