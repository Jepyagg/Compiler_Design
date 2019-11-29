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
class Id_Node;
class Const_Node;

enum class Datatype {
    int_type,
    void_type,
    bool_type
};

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
        Id_Node(char* id, int line, int col):identifier(strdup(id)), line_num(line), col_num(col){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();
};

class Const_Node : public AstNode {
    public:
        char* name;
        int line_num, col_num;
        Const_Node(char* id, int line, int col): name(id), line_num(line), col_num(col){};
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

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        Program_body(vector<Declaration_Node *> *decl):decl_list(decl){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print(VisitorBase &v);
};

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * id_list;
        Const_Node * data_type;
        Declaration_Node(vector<Id_Node *> * list, Const_Node* type) : id_list(list), data_type(type){};
        void accept(VisitorBase &v) { v.visit(this); }
        void print();
};


// class Function_Node : public AstNode {
//     public:
//         int line_num, col_num;
//         char* name;
//         Function_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

//         vector<Declaration_Node *> decl_list;
//         Compound_Statement_Node *comp_list;
// };

// class Expression_Node : public AstNode {
//     public:
// };

// class Const_Node : public Expression_Node {
//     public:
//         int line_num, col_num;
//         char* name;
//         Const_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

//         void accept(Visitor &v){};
// };
// class Boolean_Node : public Expression_Node {
//     public:
//         int line_num, col_num;
//         char* name;
//         Boolean_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

//         void accept(Visitor &v){};
// };

// class Int_Node : public Expression_Node {
//     public:
//         int line_num, col_num;
//         char* name;
//         Int_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

// };

// class Compound_Statement_Node : public AstNode {
//     vector<Declaration_Node *> decl_list;
//     public:
//         Compound_Statement_Node();
// };

// class Constant_Value_Node : public AstNode {
//     public:
//         weeawf()
// };



#endif
