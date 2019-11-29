#include "../../include/AST/visitor.h"
#include "../../include/AST/ast.h"
#include <iostream>

void Visitor::visit(AstProgram *m) { m->print(); }
void Visitor::visit(Program_body *m) { m->print(&this); }
void Visitor::visit(Declaration_Node *m) { m->print(); }
void Visitor::visit(Const_Node *m) { m->print(); }
void Visitor::visit(Id_Node *m) { m->print(); }
