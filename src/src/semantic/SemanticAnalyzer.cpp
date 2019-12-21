#include "semantic/SemanticAnalyzer.hpp"
#include "AST/program.hpp"
#include "AST/declaration.hpp"
#include "AST/variable.hpp"
#include "AST/constant_value.hpp"
#include "AST/function.hpp"
#include "AST/compound_statement.hpp"
#include "AST/assignment.hpp"
#include "AST/print.hpp"
#include "AST/read.hpp"
#include "AST/variable_reference.hpp"
#include "AST/binary_operator.hpp"
#include "AST/unary_operator.hpp"
#include "AST/if.hpp"
#include "AST/while.hpp"
#include "AST/for.hpp"
#include "AST/return.hpp"
#include "AST/function_call.hpp"
#include "AST/symbol_entry.hpp"
#include "AST/symbol_table.hpp"

#include <iostream>
#include <iomanip>
#include <cstdio>

extern char* arr_token[512];
extern string file_name;
extern int error_find;
extern vector<SymbolTableNode*> symbol_table_list;

SymbolTableNode* sem_table = nullptr;
vector<string> for_check_vec;
int for_check_p;

void space_arrow(int len) {
    error_find = 1;
    for(int i = 1; i < len + 4; ++i) {
        std::cerr << " ";
    }
    std::cerr << "^\n";
}


void SemanticAnalyzer::visit(ProgramNode *m) {
    // 晚點處理
    // if(m->program_name != file_name) {
    //     std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": program name must be the same as filename\n";
    //     std::cerr << "    " << arr_token[m->line_number] << '\n';
    //     space_arrow(m->col_number);
    // }
    sem_table = m->symbol_table_node;
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i< m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }
    if (m->function_node_list != nullptr) {
        for(uint i=0; i< m->function_node_list->size(); i++){
            sem_table = m->symbol_table_node;
            (*(m->function_node_list))[i]->accept(*this);
        }
    }
    if (m->compound_statement_node != nullptr) {
        m->compound_statement_node->accept(*this);
    }

    // error detect
    if(m->program_name != m->end_name) {
        std::cerr << "<Error> Found in line " << m->end_line_number << ", column " << m->end_col_number << ": identifier at the end of program must be the same as identifier at the beginning of program\n";
        std::cerr << "    " << arr_token[m->end_line_number] << '\n';
        space_arrow(m->end_col_number);
    }
}

void SemanticAnalyzer::visit(DeclarationNode *m) {
    if (m->variables_node_list != nullptr) {
        for(uint i = 0; i < m->variables_node_list->size(); ++i) {
            (*(m->variables_node_list))[i]->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(VariableNode *m) {
    string name_len_check = m->variable_name; //check name length, bigger than 32 will ignore
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->variable_name, 0, 32);
    }

    int check_redecl = 0;
    if(for_check_p == 1) {
        for(uint i = 0; i < for_check_vec.size(); ++i) {
            string tmp = for_check_vec[i];
            if(name_len_check == tmp) {
                check_redecl = 1;
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << " symbol '" << name_len_check << "' is redeclared\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
            }
        }
        if(check_redecl == 0) {
            for_check_vec.push_back(name_len_check);
            // (*sem_table->entries)[0]->decl_check = 1;
        }
    }

    // error detect
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 0) {
            (*sem_table->entries)[i]->decl_check = 1;
            break;
        } else if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 1) { //redeclare
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << " symbol '" << name_len_check << "' is redeclared\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
        }
    }
}

void SemanticAnalyzer::visit(ConstantValueNode *m) {}

void SemanticAnalyzer::visit(FunctionNode *m) {

    // error detect
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if(m->function_name == tmp && (*sem_table->entries)[i]->decl_check == 0) {
            (*sem_table->entries)[i]->decl_check = 1;
            break;
        } else if(m->function_name == tmp && (*sem_table->entries)[i]->decl_check == 1) { //redeclare
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << " symbol '" << m->function_name << "' is redeclared\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
        }
    }

    sem_table = m->symbol_table_node;
    if (m->parameters != nullptr) {
        for(uint i=0; i< m->parameters->size(); i++){
            (*(m->parameters))[i]->node->accept(*this);
        }
    }
        
    if (m->body != nullptr) {
        m->body->accept(*this);
    }

    // error detect
    if(m->function_name != m->end_name) {
        std::cerr << "<Error> Found in line " << m->end_line_number << ", column " << m->end_col_number << ": identifier at the end of function must be the same as identifier at the beginning of function\n";
        std::cerr << "    " << arr_token[m->end_line_number] << '\n';
        space_arrow(m->end_col_number);
    }
}

void SemanticAnalyzer::visit(CompoundStatementNode *m) {

    sem_table = m->symbol_table_node;
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }
    if (m->statement_node_list != nullptr) {
        for(uint i = 0; i < m->statement_node_list->size(); ++i) {
            (*(m->statement_node_list))[i]->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(AssignmentNode *m) {}

void SemanticAnalyzer::visit(PrintNode *m) {}

void SemanticAnalyzer::visit(ReadNode *m) {}

void SemanticAnalyzer::visit(VariableReferenceNode *m) {}

void SemanticAnalyzer::visit(BinaryOperatorNode *m) {}

void SemanticAnalyzer::visit(UnaryOperatorNode *m) {}

void SemanticAnalyzer::visit(IfNode *m) {}

void SemanticAnalyzer::visit(WhileNode *m) {}

void SemanticAnalyzer::visit(ForNode *m) {

    sem_table = m->symbol_table_node;
    for_check_p = 1;
    if (m->loop_variable_declaration != nullptr) {
        m->loop_variable_declaration->accept(*this);
    }
    
    if (m->initial_statement != nullptr) {
        m->initial_statement->accept(*this);
    }

    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }
    for_check_p = 0;
}

void SemanticAnalyzer::visit(ReturnNode *m) {}

void SemanticAnalyzer::visit(FunctionCallNode *m) {}
