%{
#include "AST/ast.hpp"
#include "AST/program.hpp"
#include "AST/declaration.hpp"
#include "AST/variable.hpp"
#include "AST/constant_value.hpp"
#include "AST/function.hpp"
#include "AST/compound_statement.hpp"
#include "AST/assignment.hpp"
#include "AST/print.hpp"
#include "AST/read.hpp"
#include "AST/variable_reference.hpp"
#include "AST/binary_operator.hpp"
#include "AST/unary_operator.hpp"
#include "AST/if.hpp"
#include "AST/while.hpp"
#include "AST/for.hpp"
#include "AST/return.hpp"
#include "AST/function_call.hpp"
#include "AST/ASTDumper.hpp"
#include "AST/symbol_entry.hpp"
#include "AST/symbol_table.hpp"
#include "codegen/codegen.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "symbol/TableConstructor.hpp"
#include "core/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstring>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

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
extern int32_t OptTable;

/* Declared by lex */
extern FILE *yyin;
extern char *yytext;
extern int arr_cnt;
extern char* arr_token[512];

extern "C" int yylex(void);
extern "C" int yyparse();

static void yyerror(const char *msg);

extern int yylex_destroy(void); // Memory_Free

/* Datatype for Return Non-terminals */
struct id_info{
    string name;
    uint32_t line_number;
    uint32_t col_number;
};

static Node AST;
string file_name = "";
string file_name2 = "";
string output_dir = "";
int error_find = 0;

%}
    /* Useful Header */
%code requires { #include "AST/ast.hpp" }
%code requires { #include "AST/program.hpp" }
%code requires { #include "AST/declaration.hpp" }
%code requires { #include "AST/variable.hpp" }
%code requires { #include "AST/constant_value.hpp" }
%code requires { #include "AST/function.hpp" }
%code requires { #include "AST/compound_statement.hpp" }
%code requires { #include "AST/assignment.hpp" }
%code requires { #include "AST/print.hpp" }
%code requires { #include "AST/read.hpp" }
%code requires { #include "AST/variable_reference.hpp" }
%code requires { #include "AST/binary_operator.hpp" }
%code requires { #include "AST/unary_operator.hpp" }
%code requires { #include "AST/if.hpp" }
%code requires { #include "AST/while.hpp" }
%code requires { #include "AST/for.hpp" }
%code requires { #include "AST/return.hpp" }
%code requires { #include "AST/function_call.hpp" }
%code requires { #include "AST/symbol_entry.hpp" }
%code requires { #include "AST/symbol_table.hpp"}
%code requires { #include "visitor/visitor.hpp" }

    /* Union Define */
%union {
    int                                 val;
    double                              dval;
    char*                               text;

    enum enumOperator                   op_type;

    Node                                node;
    NodeList*                           node_list_ptr;

    struct NodeWithTypeList*            node_w_type_list_ptr;
    vector<struct NodeWithTypeList*>*   node_w_type_list_ptr_list_ptr;

    vector<struct id_info>*             id_list_ptr;
    VariableInfo*                       variable_info_ptr;
}

%locations

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token <op_type> ASSIGN
%left <op_type> OR
%left <op_type> AND
%right <op_type> NOT
%left <op_type> LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left <op_type> PLUS MINUS
%left <op_type> MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token <text> ID

    /* Literal */
%token <val> INT_LITERAL
%token <dval> REAL_LITERAL
%token <text> STRING_LITERAL

    /* Type of Nonterminals */
%type <text> ProgramName
%type <text> FunctionName

%type <node> Declaration
%type <node_list_ptr> Declarations
%type <node_list_ptr> DeclarationList

%type <node> FunctionDeclaration
%type <node_list_ptr> Functions
%type <node_list_ptr> FunctionList

%type <node_w_type_list_ptr> FormalArg
%type <node_w_type_list_ptr_list_ptr> FormalArgs
%type <node_w_type_list_ptr_list_ptr> FormalArgList

%type <node> Statement
%type <node_list_ptr> Statements
%type <node_list_ptr> StatementList

%type <node> Expression
%type <node_list_ptr> Expressions
%type <node_list_ptr> ExpressionList

%type <node> VariableReference
%type <node_list_ptr> ArrForm

%type <node> Condition
%type <node_list_ptr> ElseOrNot

%type <node> While
%type <node> For
%type <node> Return
%type <node> FunctionInvokation
%type <node> FunctionCall

%type <node> Simple
%type <node> CompoundStatement

%type <id_list_ptr> IdList
%type <variable_info_ptr> ReturnType
%type <variable_info_ptr> Type
%type <variable_info_ptr> ScalarType
%type <variable_info_ptr> ArrType
%type <variable_info_ptr> ArrDecl
%type <variable_info_ptr> TypeOrConstant
%type <variable_info_ptr> LiteralConstant

%%
    /*
       Program Units
                     */

Program     : ProgramName SEMICOLON DeclarationList FunctionList CompoundStatement END ProgramName {
              string return_type = "void"; 
              AST = new ProgramNode(@1.first_line, @1.first_column, string($1), return_type, $3, $4, $5, @7.first_line, @7.first_column, string($7));
              free($1); free($7);}
            ;

ProgramName     : ID {$$ = strdup($1); free($1);}
                ;

    /*
    ProgramBody     : DeclarationList FunctionList CompoundStatement
                    ;
                        */

DeclarationList     : Epsilon {$$ = nullptr;}
                    | Declarations {$$ = $1;}
                    ;

Declarations    : Declaration{$$ = new NodeList(); $$->push_back($1);}
                | Declarations Declaration {$1->push_back($2); $$ = $1;}
                ;

FunctionList    : Epsilon {$$ = nullptr;}
                | Functions {$$ = $1;}
                ;

Functions       : FunctionDeclaration {$$ = new NodeList(); $$->push_back($1);}
                | Functions FunctionDeclaration {$1->push_back($2); $$ = $1;}
                ;

FunctionDeclaration     : FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON CompoundStatement END FunctionName {
                            vector<VariableInfo*> prototype;
                            //NodeList* parameters = new NodeList();
                            
                            // Disassemble FormalArgList (node w type list ptr list ptr)
                            if ($3 != nullptr) {
                                for(uint i = 0; i < $3->size(); ++i) { 
                                    // put node (Node)
                                    //parameters->push_back((*$3)[i]->node);
                                    for(uint j = 0; j < (*$3)[i]->counter; ++j) {
                                        // put type (VariableInfo*)
                                        // duplicate (eliminate hierarchy problem)
                                        VariableInfo* dupTemp = new VariableInfo();
                                        dupTemp->type_set = (*$3)[i]->type->type_set;
                                        dupTemp->type = (*$3)[i]->type->type;
                                        dupTemp->array_range = (*$3)[i]->type->array_range;
                                        dupTemp->int_literal = (*$3)[i]->type->int_literal;
                                        dupTemp->real_literal = (*$3)[i]->type->real_literal;
                                        dupTemp->string_literal = (*$3)[i]->type->string_literal;
                                        dupTemp->boolean_literal = (*$3)[i]->type->boolean_literal;
                                        
                                        prototype.push_back(dupTemp);
                                    }   
                                } 
                            } else {
                                //delete parameters;
                                //parameters = nullptr;
                            }

                            $$ = new FunctionNode(@1.first_line, @1.first_column, string($1), $3, $5, $7, @9.first_line, @9.first_column, string($9), prototype);

                            free($1); free($9);
                        }
                        ;

FunctionName    : ID {$$ = strdup($1); free($1);}
                ;

FormalArgList       : Epsilon {$$ = nullptr;}
                    | FormalArgs {$$ = $1;}
                    ;

FormalArgs      : FormalArg {$$ = new vector<NodeWithTypeList*>(); $$->push_back($1);}
                | FormalArgs SEMICOLON FormalArg {$1->push_back($3); $$ = $1;}
                ;

FormalArg       : IdList COLON Type { // Declaration Node (but location is not KWvar)
                    $$ = new NodeWithTypeList(); $$->type = $3; $$->counter = 0;
                    NodeList* var_list = new NodeList();
                    for(uint i = 0; i < $1->size(); ++i) {
                        // Duplicate $3 (eliminate hierarchy problem)
                        VariableInfo* dupTemp = new VariableInfo();
                        dupTemp->type_set = $3->type_set;
                        dupTemp->type = $3->type;
                        dupTemp->array_range = $3->array_range;
                        dupTemp->int_literal = $3->int_literal;
                        dupTemp->real_literal = $3->real_literal;
                        dupTemp->string_literal = $3->string_literal;
                        dupTemp->boolean_literal = $3->boolean_literal;

                        VariableNode* variable_node = new VariableNode((*$1)[i].line_number, (*$1)[i].col_number, (*$1)[i].name, dupTemp, nullptr);

                        var_list->push_back(variable_node);
                        $$->counter++;
                    }

                    $$->node = new DeclarationNode(@1.first_line, @1.first_column, var_list);

                    delete $1;
                }
                ;

IdList      : ID {$$ = new vector<id_info>(); $$->push_back(id_info{string($1), @1.first_line, @1.first_column}); free($1);}
            | IdList COMMA ID {$1->push_back(id_info{string($3), @3.first_line, @3.first_column}); $$ = $1; free($3);}
            ;

ReturnType      : COLON ScalarType {$$ = $2;}
                | Epsilon {$$ = new VariableInfo(); $$->type_set = UNKNOWN_SET; $$->type = TYPE_VOID;}
                ;

    /*
       Data Types and Declarations
                                   */

Declaration     : VAR IdList COLON TypeOrConstant SEMICOLON {
                    NodeList* var_list = new NodeList();
                    for(uint i = 0; i < $2->size(); ++i) {
                        if( $4->type_set == SET_CONSTANT_LITERAL ) {  // Literal Constant
                            // duplicate $4 (eliminate hierarchy problem)
                            VariableInfo* dupTemp = new VariableInfo();
                            dupTemp->type_set = $4->type_set;
                            dupTemp->type = $4->type;
                            dupTemp->array_range = $4->array_range;
                            dupTemp->int_literal = $4->int_literal;
                            dupTemp->real_literal = $4->real_literal;
                            dupTemp->string_literal = $4->string_literal;
                            dupTemp->boolean_literal = $4->boolean_literal;

                            ConstantValueNode* constant_value_node = new ConstantValueNode(@4.first_line, @4.first_column, dupTemp);

                            // duplicate $4 (eliminate hierarchy problem)
                            VariableInfo* dupTemp2 = new VariableInfo();
                            dupTemp2->type_set = $4->type_set;
                            dupTemp2->type = $4->type;
                            dupTemp2->array_range = $4->array_range;
                            dupTemp2->int_literal = $4->int_literal;
                            dupTemp2->real_literal = $4->real_literal;
                            dupTemp2->string_literal = $4->string_literal;
                            dupTemp2->boolean_literal = $4->boolean_literal;

                            VariableNode* variable_node = new VariableNode((*$2)[i].line_number, (*$2)[i].col_number, (*$2)[i].name, dupTemp2, constant_value_node);
                            var_list->push_back(variable_node);
                            
                        } else {  // Type
                            // duplicate $4 (eliminate hierarchy problem)
                            VariableInfo* dupTemp = new VariableInfo();
                            dupTemp->type_set = $4->type_set;
                            dupTemp->type = $4->type;
                            dupTemp->array_range = $4->array_range;
                            dupTemp->int_literal = $4->int_literal;
                            dupTemp->real_literal = $4->real_literal;
                            dupTemp->string_literal = $4->string_literal;
                            dupTemp->boolean_literal = $4->boolean_literal;

                            VariableNode* variable_node = new VariableNode((*$2)[i].line_number, (*$2)[i].col_number, (*$2)[i].name, dupTemp, nullptr);
                            var_list->push_back(variable_node);
                        }
                    }

                    $$ = new DeclarationNode(@1.first_line, @1.first_column, var_list);
                    delete $2; delete $4;
                }
                ;

TypeOrConstant      : Type {$$ = $1;}
                    | LiteralConstant {$$ = $1;}
                    ;

Type    : ScalarType {$$ = $1;}
        | ArrType {$$ = $1;}
        ;

ScalarType      : INTEGER {$$ = new VariableInfo(); $$->type_set = SET_SCALAR; $$->type = TYPE_INTEGER;}
                | REAL {$$ = new VariableInfo(); $$->type_set = SET_SCALAR; $$->type = TYPE_REAL;}
                | STRING {$$ = new VariableInfo(); $$->type_set = SET_SCALAR; $$->type = TYPE_STRING;}
                | BOOLEAN {$$ = new VariableInfo(); $$->type_set = SET_SCALAR; $$->type = TYPE_BOOLEAN;}
                ;

ArrType     : ArrDecl ScalarType { $$ = new VariableInfo(); $$->type_set = SET_ACCUMLATED; $$->type = $2->type; $$->array_range = $1->array_range;
              delete $1; delete $2;}
            ;

ArrDecl     : ARRAY INT_LITERAL TO INT_LITERAL OF {$$ = new VariableInfo(); $$->type_set = SET_ACCUMLATED; $$->type = UNKNOWN_TYPE;
              $$->array_range.push_back(IntPair{$2, $4});}
            | ArrDecl ARRAY INT_LITERAL TO INT_LITERAL OF {$1->array_range.push_back(IntPair{$3, $5}); $$ = $1;}
            ;

LiteralConstant     : INT_LITERAL{$$ = new VariableInfo(); $$->type_set = SET_CONSTANT_LITERAL; $$->type = TYPE_INTEGER; $$->int_literal = $1;}
                    | REAL_LITERAL {$$ = new VariableInfo(); $$->type_set = SET_CONSTANT_LITERAL; $$->type = TYPE_REAL; $$->real_literal = $1;}
                    | STRING_LITERAL {$$ = new VariableInfo(); $$->type_set = SET_CONSTANT_LITERAL; $$->type = TYPE_STRING; $$->string_literal = string($1); free($1);}
                    | TRUE {$$ = new VariableInfo(); $$->type_set = SET_CONSTANT_LITERAL; $$->type = TYPE_BOOLEAN; $$->boolean_literal = Boolean_TRUE;}
                    | FALSE {$$ = new VariableInfo(); $$->type_set = SET_CONSTANT_LITERAL; $$->type = TYPE_BOOLEAN; $$->boolean_literal = Boolean_FALSE;}
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

CompoundStatement       : BEGIN_ DeclarationList StatementList END {$$ = new CompoundStatementNode(@1.first_line,  @1.first_column,  $2, $3);}
                        ;

Simple      : VariableReference ASSIGN Expression SEMICOLON {$$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);}
            | PRINT Expression SEMICOLON {$$ = new PrintNode(@1.first_line, @1.first_column, $2);}
            | READ VariableReference SEMICOLON {$$ = new ReadNode(@1.first_line, @1.first_column, $2  );}
            ;

VariableReference       : ID {$$ = new VariableReferenceNode(@1.first_line, @1.first_column, string($1), nullptr); free($1);}
                        | ID ArrForm {$$ = new VariableReferenceNode(@1.first_line, @1.first_column, string($1), $2); free($1);}
                        ;

ArrForm     : L_BRACKET Expression R_BRACKET {$$ = new NodeList(); $$->push_back($2);}
            | ArrForm L_BRACKET Expression R_BRACKET {$1->push_back($3); $$ = $1;}
            ;

Condition       : IF Expression THEN StatementList ElseOrNot END IF {$$ = new IfNode( @1.first_line, @1.first_column, $2, $4, $5);}
                ;

ElseOrNot       : ELSE StatementList {$$ = $2;}
                | Epsilon {$$ = nullptr;}
                ;

While       : WHILE Expression DO StatementList END DO {$$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);}
            ;

For     : FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO StatementList END DO { // loop_variable_declaration : a declaration node
            VariableInfo* var_info = new VariableInfo();
            var_info->type_set = SET_SCALAR;
            var_info->type = TYPE_INTEGER;
            VariableNode* variable_node = new VariableNode(@2.first_line, @2.first_column, string($2), var_info, nullptr);

            NodeList* var_list = new NodeList();
            var_list->push_back(variable_node);
            DeclarationNode* declaration_node = new DeclarationNode(@2.first_line, @2.first_column, var_list);

            // initial: an assignment node
            VariableReferenceNode* variable_reference_node = new VariableReferenceNode(@2.first_line, @2.first_column, string($2), nullptr);

            VariableInfo* constant_value_info = new VariableInfo();
            constant_value_info->type_set = SET_CONSTANT_LITERAL;
            constant_value_info->type = TYPE_INTEGER;
            constant_value_info->int_literal = $4;
            ConstantValueNode* constant_value_node = new ConstantValueNode(@4.first_line, @4.first_column, constant_value_info);

            AssignmentNode* assignment_node = new AssignmentNode(@3.first_line, @3.first_column, variable_reference_node, constant_value_node);

            // condition: an expression node ( constant value node 1)
            VariableInfo* constant_value_info3 = new VariableInfo();
            constant_value_info3->type_set = SET_CONSTANT_LITERAL;
            constant_value_info3->type = TYPE_INTEGER;
            constant_value_info3->int_literal = $4;
            ConstantValueNode* constant_value_node3 = new ConstantValueNode(@4.first_line, @4.first_column, constant_value_info3);

            // condition: an expression node ( constant value node 2)
            VariableInfo* constant_value_info2 = new VariableInfo();
            constant_value_info2->type_set = SET_CONSTANT_LITERAL;
            constant_value_info2->type = TYPE_INTEGER;
            constant_value_info2->int_literal = $6;
            ConstantValueNode* constant_value_node2 = new ConstantValueNode(@6.first_line, @6.first_column, constant_value_info2);

            // For Node
            $$ = new ForNode(@1.first_line, @1.first_column, declaration_node, assignment_node, constant_value_node3, constant_value_node2, $8);

            free($2);
        }
        ;

Return      : RETURN Expression SEMICOLON {$$ = new ReturnNode(@1.first_line, @1.first_column, $2);}
            ;

FunctionInvokation      : FunctionCall SEMICOLON {$$ = $1;}
                        ;

FunctionCall    : ID L_PARENTHESIS ExpressionList R_PARENTHESIS {$$ = new FunctionCallNode(@1.first_line, @1.first_column, string($1), $3);
                  free($1);}
                ;

ExpressionList      : Epsilon {$$ = nullptr;}
                    | Expressions {$$ = $1;}
                    ;

Expressions     : Expression {$$ = new NodeList(); $$->push_back($1);}
                | Expressions COMMA Expression {$1->push_back($3); $$ = $1;}
                ;

StatementList       : Epsilon {$$ = nullptr;}
                    | Statements {$$ = $1;}
                    ;

Statements      : Statement {$$ = new NodeList(); $$->push_back($1);}
                | Statements Statement {$1->push_back($2); $$ = $1;}
                ;

Expression      : L_PARENTHESIS Expression R_PARENTHESIS {$$ = $2;}
                | MINUS Expression %prec UNARY_MINUS {$$ = new UnaryOperatorNode(@1.first_line, @1.first_column, $1, $2);}            
                | Expression MULTIPLY Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression DIVIDE Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression MOD Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression PLUS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression MINUS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression LESS Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression LESS_OR_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression GREATER Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression GREATER_OR_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression NOT_EQUAL Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | NOT Expression {$$ = new UnaryOperatorNode(@1.first_line, @1.first_column, $1, $2);}
                | Expression AND Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | Expression OR Expression {$$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $2, $1, $3);}
                | LiteralConstant {$$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);}
                | VariableReference {$$ = $1;}
                | FunctionCall {$$ = $1;}
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
    CHECK((argc >= 2) && (argc <= 4), "Usage: ./parser <filename> [--dump-ast][--output_code_dir]\n");
    
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

    // construct a Table
    TableConstructor constor;
    AST->accept(constor);

    // Dump AST
    if(argc == 3 && isDumpNeed == 0) {
        ASTDumper visitor;
        AST->accept(visitor);
    }

	// record file name
    string tmp(argv[1]);
    for(int i = tmp.size() - 3; i >= 0; --i) {
        if(tmp[i] == '.' || tmp[i] == '/') {
            break;
        } else {
            file_name = tmp[i] + file_name;
        }
    }

    // construct a SemanticAnalyzer to analyze the AST
    SemanticAnalyzer analyzer;
    AST->accept(analyzer);

    // dup record file name
    // record file name, output_dir
    string tmp2(argv[1]);
    string tmp3(argv[3]);
    output_dir = tmp3;
    for(int i = tmp2.size() - 3; i >= 0; --i) {
        if(tmp2[i] == '.' || tmp2[i] == '/') {
            break;
        } else {
            file_name2 = tmp2[i] + file_name2;
        }
    }

    // code generation
    int codegen_check = 0;
    if(error_find == 0) {
        codegen_check = 1;
        codegen gen;
        AST->accept(gen);
    }

    delete AST;
    fclose(fp);
    yylex_destroy();

    // free arr_token
    for(int i = 1; i < arr_cnt; ++i) {
        free(arr_token[i]);
    }

    if(error_find == 0 && codegen_check == 0) {
        printf("\n"
               "|---------------------------------------------|\n"
               "|  There is no syntactic and semantic error!  |\n"
               "|---------------------------------------------|\n");
    }
    return 0;
}
