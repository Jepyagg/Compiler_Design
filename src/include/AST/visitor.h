#ifndef __VISITOR_H
#define __VISITOR_H

#include "AST/ast.h"

class Visitor : public VisitorBase {
    public:
        void visit(class  AstProgram*e) override;
        void visit(class  Program_body*e) override;
        void visit(class  Declaration_Node*e) override;
        void visit(class  Const_Node*e) override;
        void visit(class  Id_Node*e) override;
        void visit(class  Compound_Node*e) override;
        void visit(class  Statement_Node*e) override;
        void visit(class  Array_Node*e) override;
};

#endif
