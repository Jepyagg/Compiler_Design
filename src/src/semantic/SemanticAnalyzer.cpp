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
int for_check_p = 0, first_op = 0, second_op = 0, use_opd = 0, varref_check = 0, read_check = 0;
int if_cond = 0;
int arr_line = 0, arr_col = 0, arr_ref_check = 0;
VariableInfo* first_type = nullptr;
VariableInfo* second_type = nullptr;
VariableInfo* tmp_type = nullptr;
vector<VariableInfo*> type_list;


VariableInfo* search_table(VariableInfo* target) {

    VariableInfo* find_type = nullptr;
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        if((*(sem_table->entries))[i]->sym_name == target->var_name && (*(sem_table->entries))[i]->decl_check == 1) {
            return (*(sem_table->entries))[i]->sym_type;
        }
    }
    for(uint i = 0; i < symbol_table_list[0]->entries->size(); ++i) {
        if((*symbol_table_list[0]->entries)[i]->sym_name == target->var_name && (*symbol_table_list[0]->entries)[i]->decl_check == 1) {
            return (*symbol_table_list[0]->entries)[i]->sym_type;
        }
    }
    return find_type;
}

string type_return(VariableInfo* info) {
    string s = "";
    switch(info->type) {
        case TYPE_INTEGER: 
            s += "integer"; break;
        case TYPE_REAL:    
            s += "real"; break;
        case TYPE_STRING:  
            s += "string"; break;
        case TYPE_BOOLEAN: 
            s += "boolean"; break;
        case TYPE_VOID: 
            s += "void"; break;
        default:           
            s += "something wrong"; break;
    }
    if(info->type_set == SET_ACCUMLATED) {
        VariableInfo* find_type = search_table(info);
        if(find_type->array_range.size() != info->array_range.size()) {
            s += " ";
            for(uint i = 0; i < info->array_range.size(); ++i){
                s += "[";
                s += to_string(find_type->array_range[i].end - find_type->array_range[i].start);
                s += "]";
            }
        }
    }
    return s;
}

void clear_tmp() {
    first_op = 0;
    second_op = 0;
    first_type = nullptr;
    second_type = nullptr;
    tmp_type = nullptr;
}

void space_arrow(int len) {
    error_find = 1;
    for(int i = 1; i < len + 4; ++i) {
        std::cerr << " ";
    }
    std::cerr << "^\n";
}

void SemanticAnalyzer::visit(ProgramNode *m) {

    // error detect
    if(m->program_name != file_name) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": program name must be the same as filename\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
    }

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

    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        if((*(sem_table->entries))[i]->sym_type->type_set == SET_ACCUMLATED) {
            for(uint j = 0; j < (*(sem_table->entries))[i]->sym_type->array_range.size(); ++j) {
                uint arr_start = (*(sem_table->entries))[i]->sym_type->array_range[j].start;
                uint arr_end = (*(sem_table->entries))[i]->sym_type->array_range[j].end;
                if(arr_start > arr_end) {
                    std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": '" << (*(sem_table->entries))[i]->sym_name << "' declared as an array with a lower bound greater or equal to upper bound\n";
                    std::cerr << "    " << arr_token[m->line_number] << '\n';
                    space_arrow(m->col_number);
                }
            }
        }
    }

    // for loop redeclare detect
    int check_redecl = 0;
    if(for_check_p == 1) {
        for(uint i = 0; i < for_check_vec.size(); ++i) {
            string tmp = for_check_vec[i];
            if(name_len_check == tmp) {
                check_redecl = 1;
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << name_len_check << "' is redeclared\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
                for_check_p = 0;
                break;
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
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << name_len_check << "' is redeclared\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
        }
    }
}

void SemanticAnalyzer::visit(ConstantValueNode *m) {
    if(use_opd != 0) {
        type_list.push_back(m->constant_value);
        return;
    }
    if(arr_ref_check == 1) {
        tmp_type = m->constant_value;
        arr_line = m->line_number;
        arr_col = m->col_number;
    }
}

void SemanticAnalyzer::visit(FunctionNode *m) {

    // error detect
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if(m->function_name == tmp && (*sem_table->entries)[i]->decl_check == 0) {
            (*sem_table->entries)[i]->decl_check = 1;
            break;
        } else if(m->function_name == tmp && (*sem_table->entries)[i]->decl_check == 1) { //redeclare
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << m->function_name << "' is redeclared\n";
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
            sem_table = m->symbol_table_node;
            (*(m->statement_node_list))[i]->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(AssignmentNode *m) {

    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }
    first_type = tmp_type;
    tmp_type = nullptr;
        
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }
    second_type = tmp_type;
    tmp_type = nullptr;

    // error detect
    int type_con = 0;
    if(first_type != nullptr && second_type != nullptr) {
        if(first_type->type == TYPE_INTEGER || first_type->type == TYPE_REAL) {
            if(second_type->type == TYPE_INTEGER || second_type->type == TYPE_REAL) {
                type_con = 1;
            }
        }
        if(first_type->type != second_type->type && type_con == 0) {
            std::cerr << "<Error> Found in line " << m->line_number + 2 << ", column " << m->col_number + 2 << ": assigning to '";
            switch(first_type->type) {
                case TYPE_INTEGER: 
                    std::cerr << "integer"; break;
                case TYPE_REAL:    
                    std::cerr << "real"; break;
                case TYPE_STRING:  
                    std::cerr << "string"; break;
                case TYPE_BOOLEAN: 
                    std::cerr << "boolean"; break;
                default:           
                    std::cerr << "something wrong"; break;
            }
            std::cerr << "' from incompatible type '";
            switch(second_type->type) {
                case TYPE_INTEGER: 
                    std::cerr << "integer"; break;
                case TYPE_REAL:    
                    std::cerr << "real"; break;
                case TYPE_STRING:  
                    std::cerr << "string"; break;
                case TYPE_BOOLEAN: 
                    std::cerr << "boolean"; break;
                default:           
                    std::cerr << "something wrong"; break;
            }
            std::cerr << "'\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            clear_tmp();
            return ; // undeclare not need to check arguements
        }
    }

    clear_tmp();
}

void SemanticAnalyzer::visit(PrintNode *m) {
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }
}

void SemanticAnalyzer::visit(ReadNode *m) {
    if (m->variable_reference_node != nullptr) {
        read_check = 1;
        m->variable_reference_node->accept(*this);
        read_check = 0;
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode *m) {

    VariableInfo* tmp_arr = nullptr;

    if(use_opd != 0) {
        tmp_arr = new VariableInfo();
    }

    // error detect
    int error_p = 0, dimension_cnt = 0;
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if((*sem_table->entries)[i]->sym_kind == KIND_VAR || (*sem_table->entries)[i]->sym_kind == KIND_CONST || (*sem_table->entries)[i]->sym_kind == KIND_PARAM || (*sem_table->entries)[i]->sym_kind == KIND_LP_VAR) {
            if(m->variable_name == tmp && (*sem_table->entries)[i]->decl_check == 1) { // have declare
                tmp_type = (*sem_table->entries)[i]->sym_type;
                if(use_opd != 0) {
                    tmp_arr->type_set = (*sem_table->entries)[i]->sym_type->type_set;
                    tmp_arr->type = (*sem_table->entries)[i]->sym_type->type;
                    tmp_arr->var_name = m->variable_name;
                }
                error_p = 1;
                dimension_cnt = (*sem_table->entries)[i]->sym_type->array_range.size();
                break;
            }
        }
    }
    if(error_p == 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": use of undeclared identifier '" << m->variable_name << "'\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        clear_tmp();
        return ; // undeclare not need to check arguements
    }
    if(tmp_type->type_set == SET_CONSTANT_LITERAL && first_type == nullptr) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": cannot assign to variable '" << m->variable_name << "' which is a constant\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        clear_tmp();
        return;
    }
    if(tmp_type->type_set == SET_ACCUMLATED && m->expression_node_list == nullptr) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": array assignment is not allowed\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        clear_tmp();
        return;
    }
    if(read_check == 1) {
        if(tmp_type->type_set == SET_ACCUMLATED && m->expression_node_list == nullptr) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": variable reference of read statement must be scalar type\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            clear_tmp();
            return;
        }
    }
    if (m->expression_node_list != nullptr) {
        arr_ref_check = 1;
        for(uint i = 0; i < m->expression_node_list->size(); ++i) {
            (*(m->expression_node_list))[i]->accept(*this);
            if(tmp_type->type != TYPE_INTEGER) {
                std::cerr << "<Error> Found in line " << arr_line << ", column " << arr_col << ": index of array reference must be an integer\n";
                std::cerr << "    " << arr_token[arr_line] << '\n';
                space_arrow(arr_col);
                clear_tmp();
                arr_line = 0;
                arr_col = 0;
                return;
            }
            if(use_opd != 0) {
                tmp_arr->array_range.push_back({1, 1});
            }
            arr_line = 0;
            arr_col = 0;
            dimension_cnt--;
        }
        arr_ref_check = 0;
    }
    type_list.push_back(tmp_arr);
    if(dimension_cnt != 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": there is an over array subscript\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        clear_tmp();
        return;
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode *m) {
    string opt = "";
    switch(m->op) {
        case OP_PLUS: use_opd = 5; opt = "+"; break;
        case OP_MINUS: use_opd = 1; opt = "-"; break;
        case OP_MULTIPLY: use_opd = 1; opt = "*"; break;
        case OP_DIVIDE: use_opd = 1; opt = "/"; break;
        case OP_MOD: use_opd = 2; opt = "mod"; break;
        case OP_AND: use_opd = 3; opt = "and"; break;
        case OP_OR: use_opd = 3; opt = "or"; break;
        case OP_LESS: use_opd = 4; opt = "<"; break;
        case OP_LESS_OR_EQUAL: use_opd = 4; opt = "<="; break;
        case OP_EQUAL: use_opd = 4; opt = "="; break;
        case OP_GREATER: use_opd = 4; opt = ">"; break;
        case OP_GREATER_OR_EQUAL: use_opd = 4; opt = ">="; break;
        case OP_NOT_EQUAL: use_opd = 4; opt = "<>"; break;
        default: break;
    }
    if (m->left_operand != nullptr) {
        m->left_operand->accept(*this);
    }
    if (m->right_operand != nullptr) {
        m->right_operand->accept(*this);
    }

    VariableInfo* left_op = nullptr;
    VariableInfo* right_op = nullptr;

    right_op = type_list.back();
    type_list.pop_back();
    left_op = type_list.back();
    type_list.pop_back();

    int str_plus = 0;
    string left_out = type_return(left_op), right_out = type_return(right_op);
    if(use_opd == 1 || use_opd == 5) {
        if(left_op->type != TYPE_INTEGER && left_op->type != TYPE_REAL) {
            if(right_op->type != TYPE_INTEGER && right_op->type != TYPE_REAL) {
                if(use_opd == 5 && (left_op->type == TYPE_STRING && right_op->type == TYPE_STRING)) {
                    str_plus = 1;
                }
                if(str_plus == 0) {
                    std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
                    std::cerr << left_out << "' and '" << right_out << "')\n";
                    std::cerr << "    " << arr_token[m->line_number] << '\n';
                    space_arrow(m->col_number);
                    clear_tmp();
                    str_plus = 0;
                    return;
                }
            }
        }
        if(left_op->type == TYPE_REAL || right_op->type == TYPE_REAL) {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_REAL;
            type_list.push_back(tmp);
        } else if(left_op->type == TYPE_STRING && right_op->type == TYPE_STRING && use_opd == 5) {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_STRING;
            type_list.push_back(tmp);
        } else {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_INTEGER;
            type_list.push_back(tmp);
        }
    } else if(use_opd == 2) {
        if(left_op->type != TYPE_INTEGER || right_op->type != TYPE_INTEGER) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
            std::cerr << left_out << "' and '" << right_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            clear_tmp();
            str_plus = 0;
            return;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_INTEGER;
        type_list.push_back(tmp);
    } else if(use_opd == 3) {
        if(left_op->type != TYPE_BOOLEAN || right_op->type != TYPE_BOOLEAN) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
            std::cerr << left_out << "' and '" << right_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            clear_tmp();
            str_plus = 0;
            return;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_BOOLEAN;
        type_list.push_back(tmp);
    } else if(use_opd == 4) {
        if(left_op->type != TYPE_INTEGER && left_op->type != TYPE_REAL) {
            if(right_op->type != TYPE_INTEGER && right_op->type != TYPE_REAL) {
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
                std::cerr << left_out << "' and '" << right_out << "')\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
                clear_tmp();
                str_plus = 0;
                return;
            }
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_BOOLEAN;
        type_list.push_back(tmp);
    }

    if(if_cond == 1 && (use_opd != 3 && use_opd != 4)) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": the expression of condition must be boolean type\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        clear_tmp();
        if_cond = 0;
        return;
    } 

    if (m->left_operand != nullptr) {
        m->left_operand->accept(*this);
    }

    if (m->right_operand != nullptr) {
        m->right_operand->accept(*this);
    }
    use_opd = 0;
    first_op = 0;
    second_op = 0;
}

void SemanticAnalyzer::visit(UnaryOperatorNode *m) {
    first_op = 1;
    if (m->operand != nullptr) {
        m->operand->accept(*this);
        first_type = tmp_type;
        tmp_type = nullptr;
    }
    switch(m->op) {
        case OP_MINUS: use_opd = 1; break;
        case OP_NOT: use_opd = 3; break;
        default: break;
    }
    if (m->operand != nullptr) {
        m->operand->accept(*this);
    }
    use_opd = 0;
    first_op = 0;
}

void SemanticAnalyzer::visit(IfNode *m) {
    
    if_cond = 1;
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    if(if_cond == 1) {
        if (m->body != nullptr) {
            for(uint i = 0; i < m->body->size(); ++i) {
                (*(m->body))[i]->accept(*this);
            }
        }

        if (m->body_of_else != nullptr) {
            for(uint i = 0; i < m->body_of_else->size(); ++i) {
                (*(m->body_of_else))[i]->accept(*this);
            }
        }
    }
    if_cond = 0;
}

void SemanticAnalyzer::visit(WhileNode *m) {
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(ForNode *m) {

    // error detect
    sem_table = m->symbol_table_node;
    for_check_p = 1;
    if (m->loop_variable_declaration != nullptr) {
        m->loop_variable_declaration->accept(*this);
        if(for_check_p == 0) {
            return;
        }
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

void SemanticAnalyzer::visit(ReturnNode *m) {
    if (m->return_value != nullptr) {
        m->return_value->accept(*this);
    }
}

void SemanticAnalyzer::visit(FunctionCallNode *m) {

    // error detect
    int error_p = 0;
    for(uint i = 0; i < symbol_table_list[0]->entries->size(); ++i) {
        string tmp = (*symbol_table_list[0]->entries)[i]->sym_name;
        if((*symbol_table_list[0]->entries)[i]->sym_kind == KIND_FUNC) {
            if(m->function_name == tmp && (*symbol_table_list[0]->entries)[i]->decl_check == 1) { // have declare
                second_type = (*symbol_table_list[0]->entries)[i]->sym_type;
                error_p = 1;
                if(use_opd != 0) {
                    type_list.push_back((*symbol_table_list[0]->entries)[i]->sym_type);
                    return;
                }
                break;
            }
        }
    }
    if(error_p == 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": used of undeclared function '" << m->function_name << "'\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        return ; // undeclare not need to check arguements
    }

    if (m->arguments != nullptr) {
        for(uint i = 0; i < m->arguments->size(); ++i) {
            (*(m->arguments))[i]->accept(*this);
        }
    }
}
