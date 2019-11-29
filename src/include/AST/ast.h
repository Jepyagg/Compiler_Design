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
class Type_Node;
class Id_Node;

class VisitorBase {
    public:
        virtual void visit(class  AstProgram*e) = 0;
        virtual void visit(class  Program_body*e) = 0;
        virtual void visit(class  Declaration_Node*e) = 0;
        virtual void visit(class  Const_Node*e) = 0;
        virtual void visit(class  Id_Node*e) = 0;
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
        void print();
};

//no use
// class Type_Node : public AstNode {
//     public:
//         char* name;
//         Type_Node(char* id): name(id){};
//         void accept(VisitorBase &v) { v.visit(this); }
//         void print();
// };

class Const_Node : public AstNode { //save type and const
    public:
        char* str;
        int line_num, col_num, state = 0;
        Const_Node(char*s, int sel, int line, int col): str(s), state(sel), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();
};

class AstProgram : public AstNode {
    public:
        char* name;
        int line_num, col_num;
        Id_Node* identifier;
        Program_body* p_body;
        AstProgram(Id_Node* id, int line, int col, Program_body* pb): identifier(id), line_num(line), col_num(col), p_body(pb){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();

};

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * id_list;
        Const_Node* type_val;
        Declaration_Node(vector<Id_Node *> * list, Const_Node* type, int line, int col) : id_list(list), type_val(type), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();
};

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        Program_body(vector<Declaration_Node *> *decl):decl_list(decl){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();
};


#endif
