#include "symbol/TableConstructor.hpp"
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

extern int OptTable;

vector<SymbolTableNode*> symbol_table_list;
vector<string> for_name_check;
SymbolTableNode* current_table = nullptr;
int table_cnt = 0, level_cnt = 0, func_param = 0, for_check = 0;

void dumpDemarcation(const char chr) {
    for (size_t i = 0; i < 110; ++i) {
        std::cout << chr;
    }
    cout << '\n';
}

void dumpSymbol(SymbolTableNode* symbol_table) {
    if (OptTable == 0) return;
    dumpDemarcation('=');
    printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type", "Attribute");
    dumpDemarcation('-');
    for(uint i = 0; i < symbol_table->entries->size(); ++i) {
        std::cout << std::left << std::setw(33) << (*(symbol_table->entries))[i]->sym_name;
        switch((*(symbol_table->entries))[i]->sym_kind) {
            case KIND_PROG: 
                std::cout << std::left << std::setw(11) << "program"; break;
            case KIND_FUNC: 
                std::cout << std::left << std::setw(11) << "function"; break;
            case KIND_PARAM: 
                std::cout << std::left << std::setw(11) << "parameter"; break;
            case KIND_VAR: 
                std::cout << std::left << std::setw(11) << "variable"; break;
            case KIND_LP_VAR:
                std::cout << std::left << std::setw(11) << "loop_var"; break;
            case KIND_CONST: 
                std::cout << std::left << std::setw(11) << "constant"; break;
            default: 
                std::cout << "something wrong"; break;
        }
        string tmp = to_string((*(symbol_table->entries))[i]->sym_level);
        if((*(symbol_table->entries))[i]->sym_level > 0) {
            tmp += "(local)";
            std::cout << std::left << std::setw(11) << tmp;
        } else {
            tmp += "(global)";
            std::cout << std::left << std::setw(11) << tmp;
        }
        tmp = "";
        switch((*(symbol_table->entries))[i]->sym_type->type_set){
            case SET_SCALAR:
                switch((*(symbol_table->entries))[i]->sym_type->type) {
                    case TYPE_INTEGER: 
                        tmp += "integer"; break;
                    case TYPE_REAL:    
                        tmp += "real"; break;
                    case TYPE_STRING:  
                        tmp += "string"; break;
                    case TYPE_BOOLEAN: 
                        tmp += "boolean"; break;
                    default:           
                        std::cout << "something wrong"; break;
                }
                break;
            case SET_CONSTANT_LITERAL:
                switch((*(symbol_table->entries))[i]->sym_type->type) {
                    case TYPE_INTEGER: 
                        tmp += "integer"; break;
                    case TYPE_REAL:    
                        tmp += "real"; break;
                    case TYPE_STRING:  
                        tmp += "string"; break;
                    case TYPE_BOOLEAN: 
                        tmp += "boolean"; break;
                    default:           
                        std::cout << "something wrong"; break;
                }
                break;
            case SET_ACCUMLATED:
                switch((*(symbol_table->entries))[i]->sym_type->type) {
                    case TYPE_INTEGER: 
                        tmp += "integer"; break;
                    case TYPE_REAL:    
                        tmp += "real"; break;
                    case TYPE_STRING:  
                        tmp += "string"; break;
                    case TYPE_BOOLEAN: 
                        tmp += "boolean"; break;
                    default:           
                        std::cout << "something wrong"; break;
                }
                tmp += " ";
                for(uint j = 0; j < (*(symbol_table->entries))[i]->sym_type->array_range.size(); ++j) {
                    tmp += "[";
                    tmp += to_string((*(symbol_table->entries))[i]->sym_type->array_range[j].end - (*(symbol_table->entries))[i]->sym_type->array_range[j].start);
                    tmp += "]";
                }
                break;
            case UNKNOWN_SET:
                switch((*(symbol_table->entries))[i]->sym_type->type){
                    case TYPE_VOID: tmp += "void"; break;
                    default:           std::cout << "something wrong"; break;
                }
                break;
            default:
                std::cout << "can't find type"; break;
        }
        std::cout << std::left << std::setw(17) << tmp;
        tmp = "";
        int check = 0;
        if((*(symbol_table->entries))[i]->sym_attribute.size() != 0) {
            for(uint k = 0; k < (*(symbol_table->entries))[i]->sym_attribute.size(); ++k) {
                if(k != 0) tmp += ", ";
                switch((*(symbol_table->entries))[i]->sym_attribute[k]->type_set) {
                    case SET_SCALAR:
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: 
                                tmp += "integer"; break;
                            case TYPE_REAL:    
                                tmp += "real"; break;
                            case TYPE_STRING:  
                                tmp += "string"; break;
                            case TYPE_BOOLEAN: 
                                tmp += "boolean"; break;
                            default:           
                                std::cout << "something wrong"; break;
                        }
                        break;
                    case SET_CONSTANT_LITERAL:
                        check = 1;
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: 
                                std::cout << std::left << std::setw(11) << (*(symbol_table->entries))[i]->sym_attribute[k]->int_literal; break;
                            case TYPE_REAL:    
                                std::cout << std::left << std::setw(11) << fixed << setprecision(6) << (*(symbol_table->entries))[i]->sym_attribute[k]->real_literal; break;
                            case TYPE_STRING:  
                                std::cout << std::left << std::setw(11) << (*(symbol_table->entries))[i]->sym_attribute[k]->string_literal; break;
                            case TYPE_BOOLEAN: 
                                switch((*(symbol_table->entries))[i]->sym_attribute[k]->boolean_literal){
                                    case Boolean_TRUE:  
                                        std::cout << std::left << std::setw(11) << "true"; break;
                                    case Boolean_FALSE: 
                                        std::cout << std::left << std::setw(11) << "false"; break;
                                    default: 
                                        std::cout << "something wrong"; break;
                                } 
                                break;
                            default:
                                std::cout << "not const"; break;
                        }
                        break;
                    case SET_ACCUMLATED:
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: 
                                tmp += "integer"; break;
                            case TYPE_REAL:    
                                tmp += "real"; break;
                            case TYPE_STRING:  
                                tmp += "string"; break;
                            case TYPE_BOOLEAN: 
                                tmp += "boolean"; break;
                            default:           
                                std::cout << "something wrong"; break;
                        }
                        tmp += " ";
                        for(uint j = 0; j < (*(symbol_table->entries))[i]->sym_attribute[k]->array_range.size(); ++j){
                            tmp += "[";
                            tmp += to_string((*(symbol_table->entries))[i]->sym_attribute[k]->array_range[j].end - (*(symbol_table->entries))[i]->sym_attribute[k]->array_range[j].start);
                            tmp += "]";
                        }
                        break;
                    default:
                        std::cout << "something wrong"; break;
                }
            }
        }
        if(check == 0) {
            std::cout << std::left << std::setw(11) << tmp;
        }
        std::cout << '\n';
    }
    dumpDemarcation('-');
}


void TableConstructor::visit(ProgramNode *m) {
    vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
    SymbolTableNode* symbol_table = new SymbolTableNode(entries);
    symbol_table_list.push_back(symbol_table);

    VariableInfo* tmp = new VariableInfo(); 
    vector<VariableInfo*> tmp2;
    tmp->type_set = UNKNOWN_SET;
    tmp->type = TYPE_VOID;
    tmp->var_name = m->program_name;
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(m->program_name, KIND_PROG, 0, tmp, tmp2);
    symbol_table_list[0]->entries->push_back(symbol_entry);
    current_table = symbol_table;

    if(m->declaration_node_list != nullptr) {
        for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }
    if(m->function_node_list != nullptr) {
        for(uint i = 0; i < m->function_node_list->size(); ++i) {
            (*(m->function_node_list))[i]->accept(*this);
        }
    }
    current_table = symbol_table;
    if(m->compound_statement_node != nullptr) {
        m->compound_statement_node->accept(*this);
    }

    m->symbol_table_node = symbol_table;
    dumpSymbol(m->symbol_table_node);
}

void TableConstructor::visit(DeclarationNode *m) {   
    if(m->variables_node_list != nullptr) {
        for(uint i = 0; i < m->variables_node_list->size(); ++i) {
            (*(m->variables_node_list))[i]->accept(*this);
        }
    }
}

void TableConstructor::visit(VariableNode *m) {
    vector<VariableInfo*> tmp2;
    string name_len_check = m->variable_name; //check name length, bigger than 32 will ignore
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->variable_name, 0, 32);
    }
    m->type->var_name = name_len_check;
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(name_len_check, KIND_VAR, level_cnt, m->type, tmp2);
    if(m->constant_value_node != nullptr) {
        symbol_entry->sym_attribute.push_back(m->type);
        symbol_entry->sym_kind = KIND_CONST;
    }
    int check_redeclar = 0; // 0 : no redecl, 1 : redecl
    for(uint i = 0; i < current_table->entries->size(); ++i) {
        string tmp = (*current_table->entries)[i]->sym_name;
        if(name_len_check == tmp) {
            check_redeclar = 1;
        }
    }
    if(for_check == 1) {
        for(uint i = 0; i < for_name_check.size(); ++i) {
            string tmp = for_name_check[i];
            if(name_len_check == tmp) {
                check_redeclar = 1;
            }
        }
        if(check_redeclar == 0) {
            for_name_check.push_back(symbol_entry->sym_name);
        }
    }

    if(check_redeclar == 0) {
        current_table->entries->push_back(symbol_entry);
    }
}

void TableConstructor::visit(ConstantValueNode *m) {
    // (*(*symbol_table_list)[table_cnt]->entries)[(*symbol_table_list)[table_cnt]->entries->size() - 1]->sym_attribute.push_back(m->constant_value);
}

void TableConstructor::visit(FunctionNode *m) {
    string name_len_check = m->function_name; //check name length, can't bigger than 32
    if(name_len_check.length() > 32) {
        name_len_check = name_len_check.assign(m->function_name, 0, 32);
    }

    m->return_type->var_name = name_len_check;
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(
                name_len_check, 
                KIND_FUNC, 
                0, 
                m->return_type,
                m->prototype);

    int check_redeclar = 0; // 0 : no redecl, 1 : redecl 
    for(uint i = 0; i < symbol_table_list[0]->entries->size(); ++i) {
        string tmp = (*symbol_table_list[0]->entries)[i]->sym_name;
        if(name_len_check == tmp) {
            check_redeclar = 1;
        }
    }
    if(check_redeclar == 0) {
        symbol_table_list[0]->entries->push_back(symbol_entry);
    }

    if (m->parameters != nullptr) {

        // build table to store param
        vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
        SymbolTableNode* symbol_table = new SymbolTableNode(entries);
        m->symbol_table_node = symbol_table; //error detect use
        // symbol_table_list.push_back(symbol_table);
        current_table = symbol_table; // point to table
        level_cnt++; // level add

        for(uint i = 0; i < m->parameters->size(); ++i){
            (*(m->parameters))[i]->node->accept(*this);
        }
        for(uint i = 0; i < current_table->entries->size(); ++i) {
            (*current_table->entries)[i]->sym_kind = KIND_PARAM;
        }
        func_param = 1; // check func have param
    }
    if (m->body != nullptr) {
        m->body->accept(*this);
    }
    m->symbol_table_node = current_table;
    // dumpSymbol(m->symbol_table_node);
    // level_cnt--;
}

void TableConstructor::visit(CompoundStatementNode *m) {

    vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
    SymbolTableNode* symbol_table = new SymbolTableNode(entries);
    // symbol_table_list.push_back(symbol_table);
    
    if(func_param == 0) { // 表示沒參數，所以改用 compound 建立的table
        level_cnt++;
        current_table = symbol_table;
    }

    m->symbol_table_node = current_table;
    symbol_table_list.push_back(m->symbol_table_node);
    
    // cout << level_cnt << '\n';
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }

    func_param = 0;

    // if(func_param == 1) {
    //     level_cnt++;
    // }
    if (m->statement_node_list != nullptr) {
        for(uint i = 0; i < m->statement_node_list->size(); ++i) {
            (*(m->statement_node_list))[i]->accept(*this);
        }
    }
    // cout << level_cnt << '\n'
    dumpSymbol(m->symbol_table_node);
    current_table = m->symbol_table_node;
    level_cnt--;
}

void TableConstructor::visit(AssignmentNode *m) {
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }
    
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }
}

void TableConstructor::visit(PrintNode *m) {
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }
}

void TableConstructor::visit(ReadNode *m) {
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }
}

void TableConstructor::visit(VariableReferenceNode *m) {
    if (m->expression_node_list != nullptr) {
        for(uint i = 0; i < m->expression_node_list->size(); ++i) {
            (*(m->expression_node_list))[i]->accept(*this);
        }
    }
}

void TableConstructor::visit(BinaryOperatorNode *m) {
    if (m->left_operand != nullptr) {
        m->left_operand->accept(*this);
    }

    if (m->right_operand != nullptr) {
        m->right_operand->accept(*this);
    }
}

void TableConstructor::visit(UnaryOperatorNode *m) {
    if (m->operand != nullptr) {
        m->operand->accept(*this);
    }
}

void TableConstructor::visit(IfNode *m) {
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    if (m->body_of_else != nullptr){
        std::cout << "else\n";
        for(uint i = 0; i < m->body_of_else->size(); ++i) {
            (*(m->body_of_else))[i]->accept(*this);
        }
    }
}

void TableConstructor::visit(WhileNode *m) {
    if (m->condition != nullptr) {
        m->condition->accept(*this);
    }

    if (m->body != nullptr) {
        for(uint i = 0; i< m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }
}

void TableConstructor::visit(ForNode *m) {

    vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
    SymbolTableNode* symbol_table = new SymbolTableNode(entries);
    symbol_table_list.push_back(symbol_table);
    current_table = symbol_table;
    m->symbol_table_node = symbol_table;

    level_cnt++;
    for_check = 1;
    int loop_var_size = 0;
    if (m->loop_variable_declaration != nullptr) {
        m->loop_variable_declaration->accept(*this);
        for(uint i = 0; i < current_table->entries->size(); ++i) {
            (*m->symbol_table_node->entries)[0]->sym_kind = KIND_LP_VAR;
            loop_var_size = 1;
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
    for_check = 0;
    level_cnt--;
    if(loop_var_size == 1) {
        for_name_check.pop_back();
    }
    dumpSymbol(m->symbol_table_node);
}

void TableConstructor::visit(ReturnNode *m) {
    if (m->return_value != nullptr) {
        m->return_value->accept(*this);
    }
}

void TableConstructor::visit(FunctionCallNode *m) {
    if (m->arguments != nullptr) {
        for(uint i=0; i< m->arguments->size(); i++) {
            (*(m->arguments))[i]->accept(*this);
        }
    }
}
