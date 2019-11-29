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


enum class Datatype {
    int_type,
    void_type,
    bool_type
};

class AstNode {
    public:
        AstNode(){};
        ~AstNode(){};
};

class AstProgram : public AstNode {
    public:
        int line_num, col_num;
        char* name;
        Id_Node* identifier;
        AstProgram(Id_Node* id): identifier(id){};
};

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *>* decl_list;
        Program_body(vector<Declaration_Node *> *decl):decl_list(decl){};
};

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        vector<Id_Node *> * data_type;
        vector<Id_Node *> * id_list;
        Declaration_Node(vector<Id_Node *> * list, vector<Id_Node *> * type) : id_list(list), data_type(type){};
};

class Id_Node : public AstNode {
    public:
        char* identifier;
        int line_num, col_num;
        Id_Node(char* id):identifier(strdup(id)){};
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
