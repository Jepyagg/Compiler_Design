#ifndef __AST_H
#define __AST_H

typedef struct __AstNode {
  // TODO
} AstNode;

class AstNode {
    public:
        virtual void visit() = 0;
        virtual void visit() = 0;
};

class AstProgram : public AstNode {
    public:
        virtual void accept(class AstNode &v) = 0;
        virtual ~AstProgram(){};
};

class Declaration_Node : public AstProgram {
    public:
        void accept(AstNode &v) { v.visit(this); }
        void make_sound();
};

class Function_Node : public AstProgram {
    public:
        void accept(AstNode &v) { v.visit(this); }
        void make_sound();
};

class Variable_Node : public AstNode {
    public:
        weeawf()
};

class Constant_Value_Node : public AstNode {
    public:
        weeawf()
};

class Compound_Statement_Node : public AstNode {
    public:
        weeawf()
};

class Assignment_Node : public AstNode {
    public:
        weeawf()
};

class Print_Node : public AstNode {
    public:
        weeawf()
};

class Read_Node : public AstNode {
    public:
        weeawf()
};

class Variable_Reference_Node : public AstNode {
    public:
        weeawf()
};

class Binary_Operator_Node : public AstNode {
    public:
        weeawf()
};

class Unary_Operator_Node : public AstNode {
    public:
        weeawf()
};

class If_Node : public AstNode {
    public:
        weeawf()
};

class While_Node : public AstNode {
    public:
        weeawf()
};

class For_Node : public AstNode {
    public:
        weeawf()
};

class Return_Node : public AstNode {
    public:
        weeawf()
};

class Function_Call_Node : public AstNode {
    public:
        weeawf()
};



#endif
