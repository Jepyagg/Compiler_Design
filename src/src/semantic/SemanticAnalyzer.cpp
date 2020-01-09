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
#include <cstdio>
#include <iomanip>
#include <stack>

extern char* arr_token[512];                            // scanner token
extern string file_name;                                // check program name whether same as filename or not
extern int error_find;                                  // let parser know whether have error or not

SymbolTableNode* sem_table = nullptr;                   // point to current node symbol table
SymbolTableNode* func_table = nullptr;                  // if in func, point to func symbol table
vector<SymbolTableNode*> table_list;                    // symbol table list
vector<SymbolTableNode*> for_lp_table;                  // record for_loop_var's symbol table
vector<string> for_check_vec;                           // record for_loop_var's name
vector<VariableInfo*> type_list;                        // record node's type info
int for_check_p = 0;                                    // check loop variable
string func_name = "", forloop_var = "";                // record function name and for loop variable


void delete_info(VariableInfo* target) {
    if(target != nullptr) {
        delete target;
        target = nullptr;
    } 
}

VariableInfo* search_table(VariableInfo* target) {

    VariableInfo* find_type = nullptr;

    // search table list
    for(int i = table_list.size() - 1; i >= 0; --i) {
        for(uint j = 0; j < table_list[i]->entries->size(); ++j) {
            if((*table_list[i]->entries)[j]->sym_name == target->var_name && (*table_list[i]->entries)[j]->decl_check == 1) {
                return (*table_list[i]->entries)[j]->sym_type;
            }
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
            for(uint i = info->array_range.size(); i < find_type->array_range.size(); ++i){
                s += "[";
                s += to_string(find_type->array_range[i].end - find_type->array_range[i].start);
                s += "]";
            }
        }
    }
    return s;
}

string function_call_return(VariableInfo* info) {
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
        s += " ";
        for(uint i = 0; i < info->array_range.size(); ++i){
            s += "[";
            s += to_string(info->array_range[i].end - info->array_range[i].start);
            s += "]";
        }
    }
    return s;
}

void space_arrow(int len) {
    error_find = 1;
    for(int i = 1; i < len + 4; ++i) {
        std::cerr << " ";
    }
    std::cerr << "^\n";
}

void SemanticAnalyzer::visit(ProgramNode *m) {

    // check name length, bigger than 32 will ignore
    string name_len_check = m->program_name;
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->program_name, 0, 32);
    }

    string name_len_check2 = m->end_name;
    if(name_len_check2.length() > 32) {
        name_len_check2 = name_len_check2.assign(m->end_name, 0, 32);
    }

    // point to current node table and push table
    sem_table = m->symbol_table_node;
    table_list.push_back(m->symbol_table_node);

    // visit declaration
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i< m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }

    // visit function
    if (m->function_node_list != nullptr) {
        for(uint i=0; i< m->function_node_list->size(); i++){
            sem_table = m->symbol_table_node;
            (*(m->function_node_list))[i]->accept(*this);
        }
    }

    // visit compound
    if (m->compound_statement_node != nullptr) {
        m->compound_statement_node->accept(*this);
    }

    // check Program Node related error
    // check program name same as file name
    if(name_len_check != file_name) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": program name must be the same as filename\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
    }

    // check end name same as program name
    if(name_len_check != name_len_check2) {
        std::cerr << "<Error> Found in line " << m->end_line_number << ", column " << m->end_col_number;
        std::cerr << ": identifier at the end of program must be the same as identifier at the beginning of program\n";
        std::cerr << "    " << arr_token[m->end_line_number] << '\n';
        space_arrow(m->end_col_number);
    }

    // pop table in list
    table_list.pop_back();

    // check type_list, and free all
    for(uint i = 0; i < type_list.size(); ++i) {
        VariableInfo* tmp = type_list.back();
        type_list.pop_back();
        delete_info(tmp);
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

    // check name length, bigger than 32 will ignore
    string name_len_check = m->variable_name;
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->variable_name, 0, 32);
    }

    // search current table to check whether redeclare or not
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 0) {
            (*sem_table->entries)[i]->decl_check = 1;
            break;
        } else if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 1) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << name_len_check << "' is redeclared\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            return ;
        }
    }

    // for loop redeclare
    int check_redecl = 0;
    if(for_check_p == 1) {
        for(uint i = 0; i < for_check_vec.size(); ++i) {
            string tmp = for_check_vec[i];
            if(name_len_check == tmp) {
                check_redecl = 1;
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << name_len_check << "' is redeclared\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
                for_check_p = 2;
                return ;
            }
        }
        if(check_redecl == 0) {
            forloop_var = name_len_check;
        }
    }

    // check whether array lower bound smaller than up bound or not
    if(m->type->type_set == SET_ACCUMLATED) {
        for(uint j = 0; j < m->type->array_range.size(); ++j) {
            uint arr_start = m->type->array_range[j].start;
            uint arr_end = m->type->array_range[j].end;
            if(arr_start >= arr_end) {
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": '" << name_len_check;
                std::cerr << "' declared as an array with a lower bound greater or equal to upper bound\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
                return ;
            }
        }
    }
}

void SemanticAnalyzer::visit(ConstantValueNode *m) {

    // return constant information
    VariableInfo* const_info = new VariableInfo();
    const_info->type_set = m->constant_value->type_set;
    const_info->type = m->constant_value->type;
    const_info->array_range = m->constant_value->array_range;
    const_info->int_literal = m->constant_value->int_literal;
    const_info->real_literal = m->constant_value->real_literal;
    const_info->string_literal = m->constant_value->string_literal;
    const_info->boolean_literal = m->constant_value->boolean_literal;
    const_info->var_line = m->line_number;
    const_info->var_col = m->col_number;
    type_list.push_back(const_info);
}

void SemanticAnalyzer::visit(FunctionNode *m) {

    // check name length, bigger than 32 will ignore
    string name_len_check = m->function_name;
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->function_name, 0, 32);
    }
    string name_len_check2 = m->end_name;
    if(name_len_check2.length() > 32) {
        name_len_check2 = name_len_check2.assign(m->end_name, 0, 32);
    }

    // search program table to check whether have function redeclared or not
    for(uint i = 0; i < sem_table->entries->size(); ++i) {
        string tmp = (*sem_table->entries)[i]->sym_name;
        if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 0) {
            (*sem_table->entries)[i]->decl_check = 1;
            break;
        } else if(name_len_check == tmp && (*sem_table->entries)[i]->decl_check == 1) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": symbol '" << name_len_check << "' is redeclared\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
        }
    }

    sem_table = m->symbol_table_node; // current table is function(first compound) table
    func_table = m->symbol_table_node; // point to function(first compound) table

    if (m->parameters != nullptr) {
        for(uint i = 0; i < m->parameters->size(); ++i) {
            (*(m->parameters))[i]->node->accept(*this);
        }
    }

    func_name = m->function_name; // record function name, to search the program table
    
    if (m->body != nullptr) {
        m->body->accept(*this);
    }

    func_table = nullptr; // leave function, so no need to point to function(first compound) table

    // check Function Node related error
    // check function end name same as function name

    if(name_len_check != name_len_check2) {
        std::cerr << "<Error> Found in line " << m->end_line_number << ", column " << m->end_col_number;
        std::cerr << ": identifier at the end of function must be the same as identifier at the beginning of function\n";
        std::cerr << "    " << arr_token[m->end_line_number] << '\n';
        space_arrow(m->end_col_number);
    }

    func_name = ""; // clear tmp variable
}

void SemanticAnalyzer::visit(CompoundStatementNode *m) {

    // point to current node table and push table
    sem_table = m->symbol_table_node;
    table_list.push_back(m->symbol_table_node);
    
    // visit declaration
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }

    // visit statement
    if (m->statement_node_list != nullptr) {
        for(uint i = 0; i < m->statement_node_list->size(); ++i) {
            sem_table = m->symbol_table_node;
            (*(m->statement_node_list))[i]->accept(*this);
        }
    }

    // pop table in list;
    table_list.pop_back();
}

void SemanticAnalyzer::visit(AssignmentNode *m) {

    // record current type_list size, whether variable and expression have error or not
    int cnt = type_list.size(), variable_error = 0, expression_error = 0;

    // visit variable reference node
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }

    VariableInfo* left_type = nullptr;
    VariableInfo* right_type = nullptr;

    // check no error
    if((type_list.size() - cnt) < 1) {
        variable_error = 1;
    }

    if(variable_error == 0) {
        left_type = type_list.back();
        type_list.pop_back();
    }

    // visit expression node
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }

    // check no error
    if((type_list.size() - cnt) < 1) {
        expression_error = 1;
    }

    if(expression_error == 0) {
        right_type = type_list.back();
        type_list.pop_back();
    }

    // check Assignment Node related error
    if(variable_error == 0) {

        // check variable is not constant
        if(left_type->type_set == SET_CONSTANT_LITERAL) {
            std::cerr << "<Error> Found in line " << left_type->var_line << ", column " << left_type->var_col << ": cannot assign to variable '";
            std::cerr << left_type->var_name << "' which is a constant\n";
            std::cerr << "    " << arr_token[left_type->var_line] << '\n';
            space_arrow(left_type->var_col);
            delete_info(left_type); // free memory
            if(expression_error == 0) {
                delete_info(right_type); // free memory
            }
            return ;
        }

        // check variable is not array
        if(left_type->type_set == SET_ACCUMLATED && left_type->array_range.size() != left_type->var_dim) {
            std::cerr << "<Error> Found in line " << left_type->var_line << ", column " << left_type->var_col << ": array assignment is not allowed\n";
            std::cerr << "    " << arr_token[left_type->var_line] << '\n';
            space_arrow(left_type->var_col);
            delete_info(left_type); // free memory
            if(expression_error == 0) {
                delete_info(right_type); // free memory
            }
            return ;
        }

        // if in for_lp, check variable is not assign
        if(for_check_vec.size() != 0 && for_check_p != 2) {
            for(uint i = 0; i < for_check_vec.size(); ++i) {
                if(left_type->var_name == for_check_vec[i]) {
                    std::cerr << "<Error> Found in line " << left_type->var_line << ", column " << left_type->var_col;
                    std::cerr << ": the value of loop variable cannot be modified inside the loop\n";
                    std::cerr << "    " << arr_token[left_type->var_line] << '\n';
                    space_arrow(left_type->var_col);
                    delete_info(left_type); // free memory
                    if(expression_error == 0) {
                        delete_info(right_type); // free memory
                    }
                    return ;
                }
            }
        }
    } else {

        // left node has error, if right child not error, need to free memory
        if(expression_error == 0) {
            delete_info(right_type); // free memory
        }
        return ;
    }

    if(expression_error == 0) {

        // check variable is not array
        if(right_type->type_set == SET_ACCUMLATED && right_type->array_range.size() != right_type->var_dim) {
            std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col << ": array assignment is not allowed\n";
            std::cerr << "    " << arr_token[right_type->var_line] << '\n';
            space_arrow(right_type->var_col);
            if(variable_error == 0) {
                delete_info(left_type); // free memory
            }
            delete_info(right_type); // free memory
            return ;
        }  

        // check left child type same as right child type
        int type_con = 0;
        if(left_type != nullptr && right_type != nullptr && variable_error == 0 && expression_error == 0) {
            if(left_type->type == TYPE_REAL && right_type->type == TYPE_INTEGER) {
                type_con = 1;
            }
            if(left_type->type != right_type->type && type_con == 0) {
                string first_ans = type_return(left_type), second_ans = type_return(right_type);
                std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col << ": assigning to '";
                std::cerr << left_type->var_name << "' from incompatible type '" << second_ans << "'\n";
                std::cerr << "    " << arr_token[right_type->var_line] << '\n';
                space_arrow(right_type->var_col);
                if(variable_error == 0) {
                    delete_info(left_type); // free memory
                }
                delete_info(right_type); // free memory
                return ; // undeclare not need to check arguements
            }
        }
    } else {
        if(variable_error == 0) {
            delete_info(left_type); // free memory
        }
        return ;
    }

    // free memory
    delete_info(left_type);
    delete_info(right_type);
}

void SemanticAnalyzer::visit(PrintNode *m) {

    int cnt = type_list.size();

    // visit child
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }

    // check no error
    if((type_list.size() - cnt) < 1) {
        return ;
    }

    VariableInfo* right_type = nullptr;
    right_type = type_list.back();
    type_list.pop_back();

    string print_type_check = type_return(right_type);
    if(print_type_check != "integer" && print_type_check != "real" && print_type_check != "string" && print_type_check != "boolean") {
        std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col;
        std::cerr << ": variable reference of print statement must be scalar type\n";
        std::cerr << "    " << arr_token[right_type->var_line] << '\n';
        space_arrow(right_type->var_col);
        delete_info(right_type); // free memory
        return ;
    }

    // free memory
    delete_info(right_type);
}

void SemanticAnalyzer::visit(ReadNode *m) {
    
    int cnt = type_list.size();

    // visit child
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }

    // check no error
    if((type_list.size() - cnt) < 1) {
        return ;
    }

    VariableInfo* right_type = nullptr;
    right_type = type_list.back();
    type_list.pop_back();

    // check variable not be the constant
    if(right_type->type_set == SET_CONSTANT_LITERAL) {
        std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col;
        std::cerr << ": variable reference of read statement cannot be a constant variable reference\n";
        std::cerr << "    " << arr_token[right_type->var_line] << '\n';
        space_arrow(right_type->var_col);
        delete_info(right_type); // free memory
        return ;
    }

    // check variable not be the loop var
    for(uint i = 0; i < for_check_vec.size(); ++i) {
        if(right_type->var_name == for_check_vec[i]) {
            std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col;
            std::cerr << ": the value of loop variable cannot be modified inside the loop\n";
            std::cerr << "    " << arr_token[right_type->var_line] << '\n';
            space_arrow(right_type->var_col);
            delete_info(right_type); // free memory
            return ;
        }
    }

    // check variable be scalar type
    if(right_type->type_set == SET_ACCUMLATED && right_type->array_range.size() != right_type->var_dim) {
        std::cerr << "<Error> Found in line " << right_type->var_line << ", column " << right_type->var_col;
        std::cerr << ": variable reference of read statement must be scalar type\n";
        std::cerr << "    " << arr_token[right_type->var_line] << '\n';
        space_arrow(right_type->var_col);
        delete_info(right_type); // free memory
        return ;
    }

    // free memory
    delete_info(right_type);
}

void SemanticAnalyzer::visit(VariableReferenceNode *m) {

    // check name length, bigger than 32 will ignore
    string name_len_check = m->variable_name;
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->variable_name, 0, 32);
    }

    VariableInfo* tmp_arr = new VariableInfo();
    int error_p = 0, dimension_cnt = 0;

    // search table list to check declare
    for(int i = table_list.size() - 1; i >= 0; --i) {
        int find = 0;
        for(uint j = 0; j < table_list[i]->entries->size(); ++j) {
            string tmp = (*table_list[i]->entries)[j]->sym_name;
            if((*table_list[i]->entries)[j]->sym_kind != KIND_FUNC) {
                if(name_len_check == tmp && (*table_list[i]->entries)[j]->decl_check == 1) { // have declare
                    tmp_arr->type_set = (*table_list[i]->entries)[j]->sym_type->type_set;
                    tmp_arr->type = (*table_list[i]->entries)[j]->sym_type->type;
                    tmp_arr->var_name = name_len_check;
                    tmp_arr->var_line = m->line_number;
                    tmp_arr->var_col = m->col_number;
                    error_p = 1;
                    dimension_cnt = (*table_list[i]->entries)[j]->sym_type->array_range.size();
                    tmp_arr->var_dim = dimension_cnt;
                    find = 1;
                    break;
                }
            }
        }
        if(find == 1) {
            break;
        }
    }

    // undeclared in any table
    if(error_p == 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": use of undeclared identifier '" << name_len_check << "'\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        delete_info(tmp_arr); // free memory
        return ; // undeclare not need to check arguements
    }

    // check array reference
    if (m->expression_node_list != nullptr) {
        vector<bool> expr_list; expr_list.clear();
        int expr_cnt = 0;
        for(uint i = 0; i < m->expression_node_list->size(); ++i) {
            int cnt = type_list.size();
            (*(m->expression_node_list))[i]->accept(*this);

            // check expression child node error
            if((type_list.size() - cnt) < 1) {
                expr_list.push_back(false);
            } else {
                expr_list.push_back(true);
                expr_cnt++;
            }
        }
        stack<VariableInfo*> expr_param;
        for(uint i = 0; i < expr_cnt; ++i) {
            VariableInfo* tmp = type_list.back();
            type_list.pop_back();
            expr_param.push(tmp);
        }
        for(int i = 0; i < expr_list.size(); ++i) {
            if(expr_list[i]) {
                VariableInfo* tmp_type = expr_param.top();
                expr_param.pop();
                if(tmp_type->type != TYPE_INTEGER) {
                    std::cerr << "<Error> Found in line " << tmp_type->var_line << ", column " << tmp_type->var_col;
                    std::cerr << ": index of array reference must be an integer\n";
                    std::cerr << "    " << arr_token[tmp_type->var_line] << '\n';
                    space_arrow(tmp_type->var_col);

                    // free memory
                    delete_info(tmp_type);
                    delete_info(tmp_arr);
                    while (!expr_param.empty()) { 
                        VariableInfo* tmp_type2 = expr_param.top();
                        expr_param.pop(); 
                        delete_info(tmp_type2);
                    }
                    return ;
                }
                tmp_arr->array_range.push_back({1, 1});
                dimension_cnt--;
                delete_info(tmp_type); // free memory
            } else {
                // free memory
                delete_info(tmp_arr);
                while (!expr_param.empty()) { 
                    VariableInfo* tmp_type2 = expr_param.top();
                    expr_param.pop(); 
                    delete_info(tmp_type2);
                }
                return ;
            }
        }
    }

    // check array
    if(dimension_cnt < 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": there is an over array subscript\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        delete_info(tmp_arr); // free memory
        return ;
    }
    type_list.push_back(tmp_arr);
}

void SemanticAnalyzer::visit(BinaryOperatorNode *m) {

    int use_opd = 0, opr_cnt = type_list.size();
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

    // search child node
    if (m->left_operand != nullptr) {
        m->left_operand->accept(*this);
    }
    if (m->right_operand != nullptr) {
        m->right_operand->accept(*this);
    }

    // check left && right child no error
    if((type_list.size() - opr_cnt) < 2) {
        for(int i = 0; i < type_list.size() - opr_cnt; ++i) {
            VariableInfo* delete_tmp = type_list.back();
            type_list.pop_back();
            delete_info(delete_tmp); // free memory
        }
        return ;
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
        if((left_out != "integer" && left_out != "real") || (right_out != "integer" && right_out != "real")) {
            if(use_opd == 5 && (left_out == "string" && right_out == "string")) {
                str_plus = 1;
            }
            if(str_plus == 0) {
                std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number;
                std::cerr << ": invalid operands to binary operation '" << opt << "' ('";
                std::cerr << left_out << "' and '" << right_out << "')\n";
                std::cerr << "    " << arr_token[m->line_number] << '\n';
                space_arrow(m->col_number);
                delete_info(left_op); // free memory
                delete_info(right_op); // free memory
                return ;
            }
        }
        if(left_op->type == TYPE_REAL || right_op->type == TYPE_REAL) {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_REAL;
            tmp->var_name = opt;
            tmp->var_line = m->line_number;
            tmp->var_col = m ->col_number;
            type_list.push_back(tmp);
        } else if(left_op->type == TYPE_STRING && right_op->type == TYPE_STRING && use_opd == 5) {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_STRING;
            tmp->var_name = opt;
            tmp->var_line = m->line_number;
            tmp->var_col = m ->col_number;
            type_list.push_back(tmp);
        } else {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_INTEGER;
            tmp->var_name = opt;
            tmp->var_line = m->line_number;
            tmp->var_col = m ->col_number;
            type_list.push_back(tmp);
        }
    } else if(use_opd == 2) {
        if(left_out != "integer" || right_out != "integer") {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
            std::cerr << left_out << "' and '" << right_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            delete_info(left_op); // free memory
            delete_info(right_op); // free memory
            return ;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_INTEGER;
        tmp->var_name = opt;
        tmp->var_line = m->line_number;
        tmp->var_col = m ->col_number;
        type_list.push_back(tmp);
    } else if(use_opd == 3) {
        if(left_out != "boolean" || right_out != "boolean") {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
            std::cerr << left_out << "' and '" << right_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            delete_info(left_op); // free memory
            delete_info(right_op); // free memory
            return ;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_BOOLEAN;
        tmp->var_name = opt;
        tmp->var_line = m->line_number;
        tmp->var_col = m ->col_number;
        type_list.push_back(tmp);
    } else if(use_opd == 4) {
        if((left_out != "integer" && left_out != "real") || (right_out != "integer" && right_out != "real")) {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operands to binary operation '" << opt << "' ('";
            std::cerr << left_out << "' and '" << right_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            delete_info(left_op); // free memory
            delete_info(right_op); // free memory
            return ;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_BOOLEAN;
        tmp->var_name = opt;
        tmp->var_line = m->line_number;
        tmp->var_col = m ->col_number;
        type_list.push_back(tmp);
    }

    // free memory
    delete_info(left_op);
    delete_info(right_op);
}

void SemanticAnalyzer::visit(UnaryOperatorNode *m) {

    int use_opd = 0, opr_cnt = type_list.size();
    string opt = "";
    switch(m->op) {
        case OP_MINUS: use_opd = 1; opt += "-"; break;
        case OP_NOT: use_opd = 3; opt += "not"; break;
        default: break;
    }

    // visit child node
    if (m->operand != nullptr) {
        m->operand->accept(*this);
    }

    // check right child no error
    if((type_list.size() - opr_cnt) < 1) {
        return ;
    }

    VariableInfo* left_op = nullptr;
    left_op = type_list.back();
    type_list.pop_back();
    
    string left_out = type_return(left_op);
    if(use_opd == 1) {
        if(left_out != "integer" && left_out != "real") {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operand to unary operation '" << opt << "' ('";
            std::cerr << left_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            delete_info(left_op); // free memory
            return;
        }
        if(left_op->type == TYPE_REAL) {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_REAL;
            tmp->var_name = opt;
            tmp->var_line = m->line_number;
            tmp->var_col = m ->col_number;
            type_list.push_back(tmp);
        } else {
            VariableInfo* tmp = new VariableInfo();
            tmp->type = TYPE_INTEGER;
            tmp->var_name = opt;
            tmp->var_line = m->line_number;
            tmp->var_col = m ->col_number;
            type_list.push_back(tmp);
        }
    } else if(use_opd == 3) {
        if(left_out != "boolean") {
            std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": invalid operand to unary operation '" << opt << "' ('";
            std::cerr << left_out << "')\n";
            std::cerr << "    " << arr_token[m->line_number] << '\n';
            space_arrow(m->col_number);
            delete_info(left_op); // free memory
            return;
        }
        VariableInfo* tmp = new VariableInfo();
        tmp->type = TYPE_BOOLEAN;
        tmp->var_name = opt;
        tmp->var_line = m->line_number;
        tmp->var_col = m ->col_number;
        type_list.push_back(tmp);
    }
    
    // free memory
    delete_info(left_op);
}

void SemanticAnalyzer::visit(IfNode *m) {
    
    int cnt = type_list.size(), condition_error = 0;

    // visit condition
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    // check no error
    if((type_list.size() - cnt) < 1) {
        condition_error = 1;
    }

    VariableInfo* if_condition_type = nullptr;

    // visit body
    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    // visit body of else
    if (m->body_of_else != nullptr) {
        for(uint i = 0; i < m->body_of_else->size(); ++i) {
            (*(m->body_of_else))[i]->accept(*this);
        }
    }

    // check If Node related error
    if(condition_error == 0) {
        if_condition_type = type_list.back();
        type_list.pop_back();

        // check conditional expression be boolean type
        if(if_condition_type->type != TYPE_BOOLEAN) {
            std::cerr << "<Error> Found in line " << if_condition_type->var_line << ", column " << if_condition_type->var_col;
            std::cerr << ": the expression of condition must be boolean type\n";
            std::cerr << "    " << arr_token[if_condition_type->var_line] << '\n';
            space_arrow(if_condition_type->var_col);
            delete_info(if_condition_type); // free memory
            return ;
        }

        // free memory
        delete_info(if_condition_type);
    }
}

void SemanticAnalyzer::visit(WhileNode *m) {

    int cnt = type_list.size(), condition_error = 0;
    
    // visit condition
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    // check no error
    if((type_list.size() - cnt) < 1) {
        condition_error = 1;
    }

    VariableInfo* while_condition_type = nullptr;

    // visit body
    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    // check While Node related error
    if(condition_error == 0) {
        while_condition_type = type_list.back();
        type_list.pop_back();

        // check conditional expression be boolean type
        if(while_condition_type->type != TYPE_BOOLEAN) {
            std::cerr << "<Error> Found in line " << while_condition_type->var_line << ", column " << while_condition_type->var_col;
            std::cerr << ": the expression of condition must be boolean type\n";
            std::cerr << "    " << arr_token[while_condition_type->var_line] << '\n';
            space_arrow(while_condition_type->var_col);
            delete_info(while_condition_type); // free memory
            return;
        }

        // free memory
        delete_info(while_condition_type);
    }
}

void SemanticAnalyzer::visit(ForNode *m) {

    // point to current node table and push table
    sem_table = m->symbol_table_node;
    table_list.push_back(m->symbol_table_node);
    
    for_check_p = 1;

    // visit loop_var
    if (m->loop_variable_declaration != nullptr) {
        m->loop_variable_declaration->accept(*this);
    }

    // visit assignment
    if (m->initial_statement != nullptr) {
        m->initial_statement->accept(*this);
    }
    for_check_vec.push_back(forloop_var);
    forloop_var = "";
    for_check_p = 1;

    // visit first integer
    if (m->condition2 != nullptr) {
        m->condition2->accept(*this);
    }

    // visit second integer
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    VariableInfo* first_value = nullptr;
    VariableInfo* second_value = nullptr;
    second_value = type_list.back();
    type_list.pop_back();
    first_value = type_list.back();
    type_list.pop_back();
    
    // visit body
    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    // check left integer not greater than right integer
    if(first_value->int_literal > second_value->int_literal) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number;
        std::cerr << ": the lower bound of iteration count must be smaller than or equal to the upper bound\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        table_list.pop_back();
        for_check_vec.pop_back();
        for_check_p = 0;
        delete_info(first_value); // free memory
        delete_info(second_value); // free memory
        return ;
    }

    // leave current forloop need to pop some information and clear information
    table_list.pop_back();
    for_check_vec.pop_back();
    for_check_p = 0;

    // free memory
    delete_info(first_value);
    delete_info(second_value);
}

void SemanticAnalyzer::visit(ReturnNode *m) {

    int opr_cnt = type_list.size();
    // search child node
    if (m->return_value != nullptr) {
        m->return_value->accept(*this);
    }

    // check child no error
    if((type_list.size() - opr_cnt) < 1) {
        return ;
    }

    VariableInfo* return_type = nullptr;
    return_type = type_list.back();
    type_list.pop_back();

    string ret_type = type_return(return_type), func_type = "";
    for(uint i = 0; i < table_list[0]->entries->size(); ++i) {
        string tmp = (*table_list[0]->entries)[i]->sym_name;
        if((*table_list[0]->entries)[i]->sym_kind == KIND_FUNC) {
            if(func_name == tmp && (*table_list[0]->entries)[i]->decl_check == 1) { // have declare
                func_type = type_return((*table_list[0]->entries)[i]->sym_type); // get func type
            }
        }
    }

    // check Return Node related error
    // no func_table mean return not in function
    if(func_table == nullptr) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": program/procedure should not return a value\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);
        delete_info(return_type); // free memory
        return ;
    }

    // check return type same as function type
    if(ret_type != func_type) {
        int type_ok = 0;
        if(func_type == "real" && ret_type == "integer") {
            type_ok = 1;
        }
        if(type_ok == 0) {
            std::cerr << "<Error> Found in line " << return_type->var_line << ", column " << return_type->var_col << ": return '" << ret_type;
            std::cerr << "' from a function with return type '";
            std::cerr << func_type << "'\n";
            std::cerr << "    " << arr_token[return_type->var_line] << '\n';
            space_arrow(return_type->var_col);
            delete_info(return_type); // free memory
            return ;
        }
    }

     // free memory
    delete_info(return_type);
}

void SemanticAnalyzer::visit(FunctionCallNode *m) {

    // check name length, bigger than 32 will ignore
    string name_len_check = m->function_name;
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->function_name, 0, 32);
    }

    // check func whether have declared or not
    int error_p = 0;
    VariableInfo* funccall_type = new VariableInfo();
    vector<VariableInfo*> param_info;
    for(uint i = 0; i < table_list[0]->entries->size(); ++i) {
        string tmp = (*table_list[0]->entries)[i]->sym_name;
        if((*table_list[0]->entries)[i]->sym_kind == KIND_FUNC) {
            if(name_len_check == tmp && (*table_list[0]->entries)[i]->decl_check == 1) { // have declare
                funccall_type->type_set = (*table_list[0]->entries)[i]->sym_type->type_set;
                funccall_type->type = (*table_list[0]->entries)[i]->sym_type->type;
                funccall_type->array_range = (*table_list[0]->entries)[i]->sym_type->array_range;
                funccall_type->int_literal = (*table_list[0]->entries)[i]->sym_type->int_literal;
                funccall_type->real_literal = (*table_list[0]->entries)[i]->sym_type->real_literal;
                funccall_type->string_literal = (*table_list[0]->entries)[i]->sym_type->string_literal;
                funccall_type->boolean_literal = (*table_list[0]->entries)[i]->sym_type->boolean_literal;
                param_info = (*table_list[0]->entries)[i]->sym_attribute;
                error_p = 1;
                break;
            }
        }
    }

    // check parameters
    int param_cnt = 0, param_cnt2 = 0;
    vector<bool> param_list; param_list.clear(); 
    if (m->arguments != nullptr) {
        for(uint i = 0; i < m->arguments->size(); ++i) {
            int cnt = type_list.size();
            (*(m->arguments))[i]->accept(*this);
            param_cnt++;
            if(type_list.size() - cnt < 1) {
                param_list.push_back(false);
            } else {
                param_cnt2++;
                param_list.push_back(true);
            }
        }
    }

    if(error_p == 0) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": used of undeclared function '" << name_len_check << "'\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);

        // free memory
        for(uint i = 0; i < param_cnt2; ++i) {
            VariableInfo* tmp = type_list.back();
            type_list.pop_back();
            delete_info(tmp);
        }
        delete_info(funccall_type);
        return ; // undeclare not need to check arguements
    }

    if(param_cnt != param_info.size()) {
        std::cerr << "<Error> Found in line " << m->line_number << ", column " << m->col_number << ": too few/much arguments to function invocation\n";
        std::cerr << "    " << arr_token[m->line_number] << '\n';
        space_arrow(m->col_number);

        // free memory
        for(uint i = 0; i < param_cnt2; ++i) {
            VariableInfo* tmp = type_list.back();
            type_list.pop_back();
            delete_info(tmp);
        }
        delete_info(funccall_type);
        return ; // function call param need more
    }

    // check actual parameters type whether have same as the formal parameters type in the function declaration
    stack<VariableInfo*> func_param;
    for(uint i = 0; i < param_cnt; ++i) {
        if(param_list[i]) {
            VariableInfo* tmp = type_list.back();
            type_list.pop_back();
            func_param.push(tmp);
        }
    }

    int check_param = 0;
    for(uint i = 0; i < param_cnt; ++i) {
        if(param_list[i]) {
            VariableInfo* tmp = func_param.top();
            func_param.pop();
            string call_param = type_return(tmp), acc_param = function_call_return(param_info[i]);
            if(call_param != acc_param) {
                int type_ok = 0;
                if(call_param == "integer" && acc_param == "real") {
                    type_ok = 1;
                }
                if(type_ok == 0) {
                    std::cerr << "<Error> Found in line " << tmp->var_line << ", column " << tmp->var_col << ": incompatible types passing '" << call_param;
                    std::cerr << "' to parameter of type '" << acc_param << "'\n";
                    std::cerr << "    " << arr_token[tmp->var_line] << '\n';
                    space_arrow(tmp->var_col);
                    check_param = 1;
                }
            }
            delete_info(tmp); // free memory
        }
    }

    // return functioncall information
    if(check_param == 0) {
        funccall_type->var_name = name_len_check;
        funccall_type->var_line = m->line_number;
        funccall_type->var_col = m->col_number;
        type_list.push_back(funccall_type);
    } else {
        delete_info(funccall_type);
    }

    // clear pointer
    funccall_type = nullptr;
}