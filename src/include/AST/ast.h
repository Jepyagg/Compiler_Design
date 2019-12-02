#ifndef __AST_H
#define __AST_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string.h>
using namespace std;

class AstNode;
class AstProgram;
class Program_body;
class Declaration_Node;
class Function_Node;

// Expression
class Expression_Node;
class Const_Node;
class Binary_Operator_Node;
class Unary_Operator_Node;
class Variable_Reference_Node;
class Function_Call_expr_Node;

//Statement
class Statement_Node;
class Compound_Node;
class Assignment_Node;
class Print_Node;
class Read_Node;
class If_Node;
class While_Node;
class For_Node;
class Return_Node;
class Function_Call_Node;
class Id_Node;
class Array_Node;
class Formal_Node;

class VisitorBase {
    public:
        virtual void visit(class  AstProgram*e) = 0;
        virtual void visit(class  Program_body*e) = 0;
        virtual void visit(class  Declaration_Node*e) = 0;
        virtual void visit(class  Function_Node*e) = 0;
        virtual void visit(class  Expression_Node*e) = 0;
        virtual void visit(class  Const_Node*e) = 0;
        virtual void visit(class  Binary_Operator_Node*e) = 0;
        virtual void visit(class  Unary_Operator_Node*e) = 0;
        virtual void visit(class  Variable_Reference_Node*e) = 0;
        virtual void visit(class  Function_Call_expr_Node*e) = 0;
        virtual void visit(class  Statement_Node*e) = 0;
        virtual void visit(class  Compound_Node*e) = 0;
        virtual void visit(class  Assignment_Node*e) = 0;
        virtual void visit(class  Print_Node*e) = 0;
        virtual void visit(class  Read_Node*e) = 0;
        virtual void visit(class  If_Node*e) = 0;
        virtual void visit(class  While_Node*e) = 0;
        virtual void visit(class  For_Node*e) = 0;
        virtual void visit(class  Return_Node*e) = 0;
        virtual void visit(class  Function_Call_Node*e) = 0;
        virtual void visit(class  Id_Node*e) = 0;
        virtual void visit(class  Array_Node*e) = 0;
        virtual void visit(class  Formal_Node*e) = 0;
};

class AstNode {
    public:
        AstNode(){};
        virtual void accept(class VisitorBase &v) = 0;
        ~AstNode(){};
};

class Expression_Node : public AstNode {
    public:
        virtual void accept(class VisitorBase &v) = 0;
};

class Const_Node : public Expression_Node { //save type and const
    public:
        char* str;
        int state = 0;
        int line_num, col_num;
        vector<Array_Node *> * arr_list;
        Const_Node(char*s, int sel, int line, int col): str(s), state(sel), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Binary_Operator_Node : public Expression_Node {
    public:
        char* oper;
        int line_num, col_num;
        Expression_Node *leftoperand, *rightoperand;
        Binary_Operator_Node(Expression_Node *left, Expression_Node *right, char *opt, int line, int col):leftoperand(left), rightoperand(right), oper(opt), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Unary_Operator_Node : public Expression_Node {
    public:
        char* oper;
        int line_num, col_num;
        Expression_Node *operand;
        Unary_Operator_Node(Expression_Node *opd, char *opt, int line, int col):operand(opd), oper(opt), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};


class Variable_Reference_Node : public Expression_Node {
    public:
        int line_num, col_num;
        Id_Node* ident;
        vector<Expression_Node*>* expr_list;
        Variable_Reference_Node(Id_Node *name, vector<Expression_Node*> *ind, int line, int col):ident(name), expr_list(ind), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Function_Call_expr_Node : public Expression_Node {
    public:
        int line_num, col_num;
        Id_Node* ident;
        vector<Expression_Node*>* expr_list;
        Function_Call_expr_Node(Id_Node* id, vector<Expression_Node*> *expression, int line, int col):ident(id), expr_list(expression), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Statement_Node : public AstNode {
    public:
        virtual void accept(class VisitorBase &v) = 0;
};

class Compound_Node : public Statement_Node {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        vector<Statement_Node *>* stat_list;
        Compound_Node(vector<Declaration_Node *> *decl, vector<Statement_Node *> *stat, int line, int col):decl_list(decl), stat_list(stat), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Assignment_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Variable_Reference_Node *var;
        Expression_Node *expr;
        Assignment_Node(Variable_Reference_Node *value, Expression_Node *expression, int line, int col):var(value), expr(expression), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Print_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Expression_Node *expr;
        Print_Node(Expression_Node *expr, int line, int col):expr(expr), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Read_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Variable_Reference_Node *var;
        Read_Node(Variable_Reference_Node *value, int line, int col):var(value), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class If_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Expression_Node *expr;
        vector<Statement_Node *>* stat_list;
        vector<Statement_Node *>* stat2_list;
        If_Node(Expression_Node *expression, vector<Statement_Node *>* stat, vector<Statement_Node *>* stat2, int line, int col):expr(expression), stat_list(stat), stat2_list(stat2), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class While_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Expression_Node *expr;
        vector<Statement_Node *>* stat_list;
        While_Node(Expression_Node *expression, vector<Statement_Node *>* stat, int line, int col):expr(expression), stat_list(stat), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class For_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Id_Node* ident;
        Assignment_Node* val;
        Const_Node* val2, *val3;
        vector<Statement_Node *>* stat_list;
        For_Node(Id_Node *id, Assignment_Node* const1, Const_Node* const2, Const_Node* const3, vector<Statement_Node *>* stat, int line, int col):ident(id), val(const1), val2(const2), val3(const3), stat_list(stat), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Return_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Expression_Node *expr;
        Return_Node(Expression_Node *expression, int line, int col):expr(expression), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Function_Call_Node : public Statement_Node {
    public:
        int line_num, col_num;
        Id_Node* ident;
        vector<Expression_Node*>* expr_list;
        Function_Call_Node(Id_Node* id, vector<Expression_Node*> *expression, int line, int col):ident(id), expr_list(expression), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Id_Node : public AstNode {
    public:
        char* identifier;
        int line_num, col_num;
        Const_Node* const_val;
        Id_Node(char* id, int line, int col):identifier(id), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Array_Node : public AstNode {
    public:
        char* str_start;
        char* str_end;
        Array_Node(char*s, char*e):str_start(s), str_end(e){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Formal_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * id_list;
        Const_Node* type_val;
        Formal_Node(vector<Id_Node *> * id, Const_Node* type, int line, int col):id_list(id), type_val(type), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Function_Node : public AstNode {
    public:
        char* name;
        char* returntype;
        int line_num, col_num;
        Id_Node* ident;
        Compound_Node* comp;
        vector<Formal_Node *>* form_list;
        Function_Node(Id_Node* id, vector<Formal_Node *>* form, Compound_Node* com, char* type, int line, int col): ident(id), form_list(form), comp(com), returntype(type), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * id_list;
        Const_Node* type_val;
        Declaration_Node(vector<Id_Node *> * list, Const_Node* type, int line, int col) : id_list(list), type_val(type), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        vector<Function_Node *>* func_list;
        Compound_Node* comp;
        Program_body(vector<Declaration_Node *> *decl, vector<Function_Node *>* func, Compound_Node* com):decl_list(decl), func_list(func), comp(com){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class AstProgram : public AstNode {
    public:
        char* name;
        int line_num, col_num;
        Id_Node* identifier;
        Program_body* p_body;
        AstProgram(Id_Node* id, int line, int col, Program_body* pb): identifier(id), line_num(line), col_num(col), p_body(pb){};
        void accept(VisitorBase &v) { v.visit(this); }

};

#endif
