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
        m->comp->accept(*this);
        indent_space--;
    }
}

void Visitor::visit(Expression_Node *m) {
    printf("expression\n");
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

void Visitor::visit(Binary_Operator_Node *m) {
    space_plus();
    cout << "binary operator <line: " << m->line_num << ", col: " << m->col_num << "> " << m->oper << '\n';
    indent_space++;
    m->leftoperand->accept(*this);
    m->rightoperand->accept(*this);
    indent_space--;
}

void Visitor::visit(Unary_Operator_Node *m) {
    space_plus();
    cout << "unary operator <line: " << m->line_num << ", col: " << m->col_num << "> " << m->oper << '\n';
    indent_space++;
    m->operand->accept(*this);
    indent_space--;
}

void Visitor::visit(Variable_Reference_Node *m) {
    space_plus();
    cout << "variable reference <line: " << m->line_num << ", col: " << m->col_num << "> " << m->ident->identifier << '\n';
    if(m->expr_list != 0) {
        for (auto tmp : *m->expr_list) {
            space_plus();
            cout << "[\n";
            indent_space++;
            tmp->accept(*this);
            indent_space--;
            space_plus();
            cout << "]\n";
        }
    }
}

void Visitor::visit(Function_Call_expr_Node *m) {
    space_plus();
    cout << "function call statement <line: " << m->line_num << ", col: " << m->col_num << "> " << m->ident->identifier << '\n';
    if(m->expr_list != 0) {
        for (auto tmp : *m->expr_list) {
            indent_space++;
            tmp->accept(*this);
            indent_space--;
        }
    }
}

void Visitor::visit(Statement_Node *m) {
    printf("statement\n");
}

void Visitor::visit(Compound_Node *m) {
    space_plus();
    cout << "compound statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    vector<Declaration_Node *>* tmp = m->decl_list;
    vector<Statement_Node *>* tmp2 = m->stat_list;
    if(tmp != 0) {
        for(auto v : *tmp) {
            indent_space++;
            v->accept(*this);
            indent_space--;
        }
    }
    if(tmp2 != 0) {
        for(auto v2 : *tmp2) {
            indent_space++;
            v2->accept(*this);
            indent_space--;
        }
    }
}

void Visitor::visit(Assignment_Node *m) {
    space_plus();
    cout << "assignment statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->var->accept(*this);
    m->expr->accept(*this);
    indent_space--;
}

void Visitor::visit(Print_Node *m) {
    space_plus();
    cout << "print statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->expr->accept(*this);
    indent_space--;
}

void Visitor::visit(Read_Node *m) {
    space_plus();
    cout << "read statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->var->accept(*this);
    indent_space--;
}

void Visitor::visit(If_Node *m) {
    space_plus();
    cout << "if statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->expr->accept(*this);
    vector<Statement_Node *>* tmp = m->stat_list;
    vector<Statement_Node *>* tmp2 = m->stat2_list;
    for (auto v : *tmp) {
        v->accept(*this);
    }
    indent_space--;
    if (tmp2 == 0) return;
    space_plus();
    cout << "else\n";
    indent_space++;
    for (auto v2 : *tmp2) {
        v2->accept(*this);
    }
    indent_space--;
}

void Visitor::visit(While_Node *m) {
    space_plus();
    cout << "while statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->expr->accept(*this);
    vector<Statement_Node *>* tmp = m->stat_list;
    for (auto v : *tmp) {
        v->accept(*this);
    }
    indent_space--;
}

void Visitor::visit(For_Node *m) {
    space_plus();
    cout << "for statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    space_plus();
    cout << "declaration <line: " << m->ident->line_num << ", col: " << m->ident->col_num << ">\n";
    indent_space++;
    m->val2->state = 6;
    m->ident->const_val = m->val2;
    m->ident->accept(*this);
    m->val2->state = 1;
    indent_space--;
    m->val->accept(*this);
    m->val3->accept(*this);
    vector<Statement_Node *>* tmp = m->stat_list;
    if(tmp != 0) {
        for (auto v : *tmp) {
            v->accept(*this);
        }
    }
    indent_space--;
}

void Visitor::visit(Return_Node *m) {
    space_plus();
    cout << "return statement <line: " << m->line_num << ", col: " << m->col_num << ">\n";
    indent_space++;
    m->expr->accept(*this);
    indent_space--;
}

void Visitor::visit(Function_Call_Node *m) {
    space_plus();
    cout << "function call statement <line: " << m->line_num << ", col: " << m->col_num << "> " << m->ident->identifier << '\n';
    if(m->expr_list != 0) {
        for (auto tmp : *m->expr_list) {
            indent_space++;
            tmp->accept(*this);
            indent_space--;
        }
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
    } else if(tmp->state == 6) {
        cout << "integer\n";
    }
}

void Visitor::visit(Array_Node *m) {
    cout << "[" << m->str_start << "..." << m->str_end << "]";
}

void Visitor::visit(Formal_Node *m) { //確認function_prototype 是要印什麼
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
