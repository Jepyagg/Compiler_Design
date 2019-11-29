#include "../../include/AST/ast.h"
#include <iostream>

int space_cnt = 0;

void AstProgram::print() {
    cout << "program <line: " << line_num << ", col: " << col_num << "> " << identifier->identifier << " void\n";
    p_body->print();
}
void Program_body::print() {
    for(auto i : *decl_list) {
        i->print();
    }
}
void Const_Node::print(){printf("hello\n");}
void Declaration_Node::print() {}
void Id_Node::print() {cout << "variable <line: " << line_num << ", col: " << col_num << "> " << identifier << '\n';}
