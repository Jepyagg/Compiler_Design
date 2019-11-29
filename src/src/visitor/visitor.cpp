#include "../../include/AST/ast.h"
#include "../../include/AST/visitor.h"
#include <iostream>
#include <stdlib.h>

void Visitor::visit(AstProgram *m) { 
    cout << "program <line: " << m->line_num << ", col: " << m->col_num << "> " << m->identifier->identifier << " void\n";
    m->p_body->accept(*this);
}
void Visitor::visit(Program_body *m) {
    vector<Declaration_Node *>* tmp = m->decl_list;
    for(auto v : *tmp) {
        v->accept(*this);
    }
}
void Visitor::visit(Declaration_Node *m) { 
    cout << "declaration <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    vector<Id_Node *>* tmp = m->id_list;
    for(auto v : *tmp) {
        v->const_val = m->type_val;
        v->accept(*this);
    }
}
void Visitor::visit(Const_Node *m) {
    cout << "constant <line: " << m->line_num << ", col: " << m->col_num << "> ";
    if(m->state == 1) {
        int tmp = 0;
        if(m->str[0] == '0') {
            tmp = strtol(m->str, NULL, 8);
            cout << tmp << '\n';
        } else {
            tmp = atoi(m->str);
            cout << tmp << '\n';
        }
    } else if(m->state == 2) {
        double tmp = atof(m->str);
        cout << tmp << '\n';
    } else if(m->state == 3 || m->state == 4) {
        cout << m->str << '\n';
    }
}
void Visitor::visit(Id_Node *m) {
    Const_Node* tmp = m->const_val;
    cout << "variable <line: " << m->line_num << ", col: " << m->col_num << "> " << m->identifier << " ";
    if(tmp->state == 0) {
        cout << tmp->str << '\n';
    } else if(tmp->state == 1) {
        cout << "integer\n";
        tmp->accept(*this);
    } else if(tmp->state == 2) {
        cout << "real\n";
        tmp->accept(*this);
    } else if(tmp->state == 3) {
        cout << "string\n";
        tmp->accept(*this);
    } else if(tmp->state == 4) {
        cout << "bool\n";
        tmp->accept(*this);
    }
}
