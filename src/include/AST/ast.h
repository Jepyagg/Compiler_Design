#ifndef __AST_H
#define __AST_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;

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
        AstProgram(char* id, int line, int col): name = strdup(id), line_num(line), col_num(col);
};

class Program_body : public AstNode {
    public:
        int line_num, col_num;
        vector<Declaration_Node *> decl_list;
        vector<Function_Node *> func_list;
        Compound_Statement_Node *comp;
        Program_body(vector<Declaration_Node *> *decl, vector<Function_Node *> *func, Compound_Statement_Node *comp):decl_list(decl), func_list(func), comp(comp);
}

class Declaration_Node : public AstNode {
    public:
        int line_num, col_num;
        int data_type;
        vector<Id_Node *> * id_list;
        Declaration_Node(vector<Id_Node *> * list, int type) : id_list(list), data_type(type);
};

class Id_Node : public AstNode {
    public:
        char* identifier;
        int line_num, col_num;
        Id_Node(char* id, int line, int col):identifier = strdup(id), line_num(line), col_num(col);
}

class Formal_Arg : public AstNode {
    public:
        int line_num, col_num;
        int data_type;
        vector<Id_Node *> *id_list;
        Formal_Arg(vector<Id_Node *> * list, int type) : id_list(list), data_type(type);
}





class Function_Node : public AstNode {
    public:
        int line_num, col_num;
        char* name;
        Function_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

        vector<Declaration_Node *> decl_list;
        Compound_Statement_Node *comp_list;
};

class Expression_Node : public AstNode {
    public:
};

class Const_Node : public Expression_Node {
    public:
        int line_num, col_num;
        char* name;
        Const_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

        void accept(Visitor &v){};
};
class Boolean_Node : public Expression_Node {
    public:
        int line_num, col_num;
        char* name;
        Boolean_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

        void accept(Visitor &v){};
};

class Int_Node : public Expression_Node {
    public:
        int line_num, col_num;
        char* name;
        Int_Node(char* id, int line, int col):name = strdup(id), line_num(line), col_num(col);

};

class Compound_Statement_Node : public AstNode {
    vector<Declaration_Node *> decl_list;
    public:
        Compound_Statement_Node();
};

class Constant_Value_Node : public AstNode {
    public:
        weeawf()
};



#endif
