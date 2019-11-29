#include "../../include/AST/ast.h"
#include "../../include/AST/visitor.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>

int indent_space = 0, form_state = 0, con = 0;

void space_plus() {
    for(int i = 0; i < indent_space; ++i) {
        cout << "  ";
    }
}

void Visitor::visit(AstProgram *m) { 
    cout << "program <line: " << m->line_num << ", col: " << m->col_num << "> " << m->identifier->identifier << " void\n";
    m->p_body->accept(*this);
}

void Visitor::visit(Program_body *m) {
    indent_space++;
    vector<Declaration_Node *>* tmp = m->decl_list;
    vector<Function_Node *>* tmp2 = m->func_list;
    if(tmp != 0) {
        for(auto v : *tmp) {
            v->accept(*this);
        }
    }
    if(tmp2 != 0) {
        for(auto v2 : *tmp2) {
            v2->accept(*this);
        }
    }
    if(m->comp != 0) {
        space_plus();
        m->comp->accept(*this);
    }
}

void Visitor::visit(Declaration_Node *m) { 
    space_plus();
    cout << "declaration <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    vector<Id_Node *>* tmp = m->id_list;
    for(auto v : *tmp) {
        v->const_val = m->type_val;
        indent_space++;
        v->accept(*this);
        indent_space--;
    }
}

void Visitor::visit(Const_Node *m) {
    if(m->state != 5) {
        space_plus();
        cout << "constant <line: " << m->line_num << ", col: " << m->col_num << "> ";
        if(m->state == 1) {
            int tmp = 0;
            if(m->str[0] == '0') {
                tmp = strtol(m->str, NULL, 8);
                cout << fixed << setprecision(6) << tmp << '\n';
            } else {
                tmp = atoi(m->str);
                cout << tmp << '\n';
            }
        } else if(m->state == 2) {
            double tmp = atof(m->str);
            cout << fixed << setprecision(6) << tmp << '\n';
        } else if(m->state == 3 || m->state == 4) {
            cout << m->str << '\n';
        }
    } else if(m->state == 5) {
        cout << m->str;
        vector<Array_Node*>* tmp = m->arr_list;
        for(auto v : *tmp) {
            v->accept(*this);
        }
        cout << '\n';
    }
}

void Visitor::visit(Compound_Node *m) {
    cout << "compound statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
}

void Visitor::visit(Array_Node *m) {
    cout << "[" << m->str_start << "..." << m->str_end << "]";
}

void Visitor::visit(Formal_Node *m) {
    if(form_state != 0) {
        space_plus();
        cout << "declaration <line: " << m->line_num << ", col: " << m->col_num << ">\n";
        vector<Id_Node *>* tmp = m->id_list;
        for(auto v : *tmp) {
            v->const_val = m->type_val;
            indent_space++;
            v->accept(*this);
            indent_space--;
        }
    } else {
        vector<Id_Node *>* tmp = m->id_list;
        for(auto v : *tmp) {
            v->const_val = m->type_val;
            if(con > 0 ) {
                cout << ", ";
            }
            con++;
            cout << v->const_val->str;
            if(v->const_val->state == 5) {
                vector<Array_Node*>* tmp2 = v->const_val->arr_list;
                for(auto v2 : *tmp2) {
                    int end = atoi(v2->str_end);
                    int start = atoi(v2->str_start);
                    cout << "[" << end - start << "]";
                }
            }
        }
    }
}

void Visitor::visit(Statement_Node *m) {
    printf("statement\n");
}

void Visitor::visit(Function_Node *m) {
    space_plus();
    cout << "function declaration <line: " << m->line_num << ", col: " << m->col_num << "> " << m->ident->identifier << " ";
    if(m->returntype != 0) {
        cout << m->returntype << " (";
    } else {
        cout << "void (";
    }
    vector<Formal_Node *>* tmp = m->form_list;
    if(tmp != 0) {
        for(auto v : *tmp) {
            v->accept(*this);
        }
        cout << ")\n";
        form_state = 1;
        for(auto v : *tmp) {
            indent_space++;
            v->accept(*this);
            indent_space--;
        }
        form_state = 0;
        con = 0;
    } else {
        cout << ")\n";
    }
    if(m->comp != 0) {
        indent_space++;
        space_plus();
        m->comp->accept(*this);
        indent_space--;
    }
}

void Visitor::visit(Id_Node *m) {
    Const_Node* tmp = m->const_val;
    space_plus();
    cout << "variable <line: " << m->line_num << ", col: " << m->col_num << "> " << m->identifier << " ";
    if(tmp->state == 0) {
        cout << tmp->str << '\n';
    } else if(tmp->state == 1) {
        cout << "integer\n";
        indent_space++;
        tmp->accept(*this);
        indent_space--;
    } else if(tmp->state == 2) {
        cout << "real\n";
        indent_space++;
        tmp->accept(*this);
        indent_space--;
    } else if(tmp->state == 3) {
        cout << "string\n";
        indent_space++;
        tmp->accept(*this);
        indent_space--;
    } else if(tmp->state == 4) {
        cout << "boolean\n";
        indent_space++;
        tmp->accept(*this);
        indent_space--;
    } else if(tmp->state == 5) {
        tmp->accept(*this); //array
    }
}
