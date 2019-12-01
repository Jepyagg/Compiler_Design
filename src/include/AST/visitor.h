#ifndef __VISITOR_H
#define __VISITOR_H

#include "AST/ast.h"

class Visitor : public VisitorBase {
    public:
        void visit(class  AstProgram*e) override;
        void visit(class  Program_body*e) override;
        void visit(class  Declaration_Node*e) override;
        void visit(class  Function_Node*e) override;
        void visit(class  Statement_Node*e) override;
        void visit(class  Compound_Node*e) override;
        void visit(class  Assignment_Node*e) override;
        void visit(class  Print_Node*e) override;
        void visit(class  Read_Node*e) override;
        void visit(class  If_Node*e) override;
        void visit(class  Return_Node*e) override;
        void visit(class  Function_Call_Node*e) override;
        void visit(class  Expression_Node*e) override;
        void visit(class  Const_Node*e) override;
        void visit(class  Binary_Operator_Node*e) override;
        void visit(class  Unary_Operator_Node*e) override;
        void visit(class  Variable_Reference_Node*e) override;
        void visit(class  Id_Node*e) override;
        void visit(class  Array_Node*e) override;
        void visit(class  Formal_Node*e) override;
};

#endif
