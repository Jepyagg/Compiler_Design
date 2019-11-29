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
class Const_Node;
class Id_Node;
class Compound_Node;
class Statement_Node;
class Array_Node;

class VisitorBase {
    public:
        virtual void visit(class  AstProgram*e) = 0;
        virtual void visit(class  Program_body*e) = 0;
        virtual void visit(class  Declaration_Node*e) = 0;
        virtual void visit(class  Const_Node*e) = 0;
        virtual void visit(class  Id_Node*e) = 0;
        virtual void visit(class  Compound_Node*e) = 0;
        virtual void visit(class  Statement_Node*e) = 0;
        virtual void visit(class  Array_Node*e) = 0;
};

class AstNode {
    public:
        AstNode(){};
        virtual void accept(class VisitorBase &v) = 0;
        ~AstNode(){};
};

class Id_Node : public AstNode {
    public:
        char* identifier;
        int line_num, col_num;
        Const_Node* const_val;
        Id_Node(char* id, int line, int col):identifier(strdup(id)), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Const_Node : public AstNode { //save type and const
    public:
        char* str;
        int line_num, col_num, state = 0;
        vector<Array_Node *> * arr_list;
        Const_Node(char*s, int sel, int line, int col): str(s), state(sel), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Array_Node : public AstNode {
    public:
        char* str_start;
        char* str_end;
        Array_Node(char*s, char*e):str_start(s), str_end(e){};
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

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * id_list;
        Const_Node* type_val;
        Declaration_Node(vector<Id_Node *> * list, Const_Node* type, int line, int col) : id_list(list), type_val(type), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Statement_Node : public AstNode {
    public:
        int line_num, col_num;
};

class Compound_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        vector<Statement_Node *>* stat_list;
        Compound_Node(vector<Declaration_Node *> *decl, vector<Statement_Node *> *stat, int line, int col):decl_list(decl), stat_list(stat), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
};

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        Compound_Node* comp;
        Program_body(vector<Declaration_Node *> *decl, Compound_Node* com):decl_list(decl), comp(com){};
        void accept(VisitorBase &v) { v.visit(this); }
};


#endif
