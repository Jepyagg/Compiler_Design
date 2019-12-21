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

vector<SymbolTableNode*> *symbol_table_list = new vector<SymbolTableNode*>();
int table_cnt = 0, level_cnt = 0;

void dumpDemarcation(const char chr) {
    for (size_t i = 0; i < 110; ++i) {
        std::cout << chr;
    }
    cout << '\n';
}

void dumpSymbol(SymbolTableNode* symbol_table) {
    dumpDemarcation('=');
    printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type", "Attribute");
    dumpDemarcation('-');
    for(uint i = 0; i < symbol_table->entries->size(); ++i) {
        std::cout << std::left << std::setw(33) << (*(symbol_table->entries))[i]->sym_name;
        switch((*(symbol_table->entries))[i]->sym_kind) {
            case KIND_PROG: std::cout << std::left << std::setw(11) << "program"; break;
            case KIND_FUNC: std::cout << std::left << std::setw(11) << "function"; break;
            case KIND_PARAM: std::cout << std::left << std::setw(11) << "parameter"; break;
            case KIND_VAR: std::cout << std::left << std::setw(11) << "variable"; break;
            case KIND_LP_VAR: std::cout << std::left << std::setw(11) << "loop_var"; break;
            case KIND_CONST: std::cout << std::left << std::setw(11) << "constant"; break;
            default: std::cout << "something wrong"; break;
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
                    case TYPE_INTEGER: tmp += "integer"; break;
                    case TYPE_REAL:    tmp += "real"; break;
                    case TYPE_STRING:  tmp += "string"; break;
                    case TYPE_BOOLEAN: tmp += "boolean"; break;
                    default:           std::cout << "something wrong"; break;
                }
                break;
            case SET_CONSTANT_LITERAL:
                switch((*(symbol_table->entries))[i]->sym_type->type) {
                    case TYPE_INTEGER: tmp += "integer"; break;
                    case TYPE_REAL:    tmp += "real"; break;
                    case TYPE_STRING:  tmp += "string"; break;
                    case TYPE_BOOLEAN: tmp += "boolean"; break;
                    default:           std::cout << "something wrong"; break;
                }
                break;
            case SET_ACCUMLATED:
                switch((*(symbol_table->entries))[i]->sym_type->type) {
                    case TYPE_INTEGER: tmp += "integer"; break;
                    case TYPE_REAL:    tmp += "real"; break;
                    case TYPE_STRING:  tmp += "string"; break;
                    case TYPE_BOOLEAN: tmp += "boolean"; break;
                    default:           std::cout << "something wrong"; break;
                }
                tmp += " ";
                for(uint j = 0; j < (*(symbol_table->entries))[i]->sym_type->array_range.size(); ++j){
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
            // cout << "test";
            for(uint k = 0; k < (*(symbol_table->entries))[i]->sym_attribute.size(); ++k) {
                // cout << "test2";
                if(k != 0) tmp += ", ";
                switch((*(symbol_table->entries))[i]->sym_attribute[k]->type_set) {
                    case SET_SCALAR:
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: tmp += "integer"; break;
                            case TYPE_REAL:    tmp += "real"; break;
                            case TYPE_STRING:  tmp += "string"; break;
                            case TYPE_BOOLEAN: tmp += "boolean"; break;
                            default:           std::cout << "something wrong"; break;
                        }
                        break;
                    case SET_CONSTANT_LITERAL:
                        check = 1;
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: std::cout << std::left << std::setw(11) << (*(symbol_table->entries))[i]->sym_attribute[k]->int_literal; break;
                            case TYPE_REAL:    std::cout << std::left << std::setw(11) << fixed << setprecision(6) << (*(symbol_table->entries))[i]->sym_attribute[k]->real_literal; break;
                            case TYPE_STRING:  std::cout << std::left << std::setw(11) << (*(symbol_table->entries))[i]->sym_attribute[k]->string_literal; break;
                            case TYPE_BOOLEAN: 
                                switch((*(symbol_table->entries))[i]->sym_attribute[k]->boolean_literal){
                                    case Boolean_TRUE:  std::cout << std::left << std::setw(11) << "true"; break;
                                    case Boolean_FALSE: std::cout << std::left << std::setw(11) << "false"; break;
                                    default: std::cout << "unknown"; break;
                                } 
                                break;
                            default:
                                std::cout << "not const"; break;
                        }
                        break;
                    case SET_ACCUMLATED:
                        switch((*(symbol_table->entries))[i]->sym_attribute[k]->type) {
                            case TYPE_INTEGER: tmp += "integer"; break;
                            case TYPE_REAL:    tmp += "real"; break;
                            case TYPE_STRING:  tmp += "string"; break;
                            case TYPE_BOOLEAN: tmp += "boolean"; break;
                            default:           std::cout << "something wrong"; break;
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
    symbol_table_list->push_back(symbol_table);

    VariableInfo* tmp = new VariableInfo(); 
    vector<VariableInfo*> tmp2;
    tmp->type_set = UNKNOWN_SET;
    tmp->type = TYPE_VOID;
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(m->program_name, KIND_PROG, 0, tmp, tmp2);
    (*symbol_table_list)[table_cnt]->entries->push_back(symbol_entry);
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
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(m->variable_name, KIND_VAR, level_cnt, m->type, tmp2);
    if(m->constant_value_node != nullptr) {
        symbol_entry->sym_attribute.push_back(m->type);
        symbol_entry->sym_kind = KIND_CONST;
    }
    (*symbol_table_list)[table_cnt]->entries->push_back(symbol_entry);
}

void TableConstructor::visit(ConstantValueNode *m) {
    // (*(*symbol_table_list)[table_cnt]->entries)[(*symbol_table_list)[table_cnt]->entries->size() - 1]->sym_attribute.push_back(m->constant_value);
}

void TableConstructor::visit(FunctionNode *m) {
    SymbolEntryNode* symbol_entry = new SymbolEntryNode(
                m->function_name, 
                KIND_FUNC, 
                0, 
                m->return_type,
                m->prototype);
    (*symbol_table_list)[0]->entries->push_back(symbol_entry);
    vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
    SymbolTableNode* symbol_table = new SymbolTableNode(entries);
    symbol_table_list->push_back(symbol_table);

    m->symbol_table_node = symbol_table;
    table_cnt++;
    level_cnt++;
    if (m->parameters != nullptr) {
        for(uint i = 0; i < m->parameters->size(); ++i){
            (*(m->parameters))[i]->node->accept(*this);
        }
        for(uint i = 0; i < (*symbol_table_list)[table_cnt]->entries->size(); ++i) {
            (*(*symbol_table_list)[table_cnt]->entries)[i]->sym_kind = KIND_PARAM;
        }
    }
    if (m->body != nullptr) {
        m->body->accept(*this);    
    }
    dumpSymbol(m->symbol_table_node);
    level_cnt--;
}

void TableConstructor::visit(CompoundStatementNode *m) {
    if(m->declaration_node_list != nullptr || m->statement_node_list != nullptr) {
        vector<SymbolEntryNode*>* entries = new vector<SymbolEntryNode*>();
        SymbolTableNode* symbol_table = new SymbolTableNode(entries);
        symbol_table_list->push_back(symbol_table);
        table_cnt++;
        level_cnt++;
        if (m->declaration_node_list != nullptr) {
            for(uint i=0; i< m->declaration_node_list->size(); i++){
                (*(m->declaration_node_list))[i]->accept(*this);
            }
        }
        if (m->statement_node_list != nullptr) {
            for(uint i=0; i< m->statement_node_list->size(); i++){
                (*(m->statement_node_list))[i]->accept(*this);
            }
        }
        dumpSymbol(symbol_table);
        level_cnt--;
    }
}

void TableConstructor::visit(AssignmentNode *m) {
        if (m->variable_reference_node != nullptr)
            m->variable_reference_node->accept(*this);
        
        if (m->expression_node != nullptr)
            m->expression_node->accept(*this);  
}

void TableConstructor::visit(PrintNode *m) {
        if (m->expression_node != nullptr)
            m->expression_node->accept(*this);  
}

void TableConstructor::visit(ReadNode *m) {
        if (m->variable_reference_node != nullptr)
            m->variable_reference_node->accept(*this);    
}

void TableConstructor::visit(VariableReferenceNode *m) {
    
    if (m->expression_node_list != nullptr)
        for(uint i=0; i< m->expression_node_list->size(); i++){
        
            std::cout<<"["<<std::endl;
            
                (*(m->expression_node_list))[i]->accept(*this);
    
            
        
            std::cout<<"]"<<std::endl;
        }
}

void TableConstructor::visit(BinaryOperatorNode *m) {
        if (m->left_operand != nullptr)
            m->left_operand->accept(*this);

        if (m->right_operand != nullptr)
            m->right_operand->accept(*this);
}

void TableConstructor::visit(UnaryOperatorNode *m) {
        if (m->operand != nullptr)
            m->operand->accept(*this);
}

void TableConstructor::visit(IfNode *m) {
        if (m->condition != nullptr)
            m->condition->accept(*this);

        if (m->body != nullptr)
            for(uint i=0; i< m->body->size(); i++)
                (*(m->body))[i]->accept(*this);

    if (m->body_of_else != nullptr){

        std::cout<<"else"<<std::endl;
            for(uint i=0; i< m->body_of_else->size(); i++)
                (*(m->body_of_else))[i]->accept(*this);

    }
}

void TableConstructor::visit(WhileNode *m) {
        if (m->condition != nullptr)
            m->condition->accept(*this);

        if (m->body != nullptr)
            for(uint i=0; i< m->body->size(); i++)
                (*(m->body))[i]->accept(*this);
}

void TableConstructor::visit(ForNode *m) {
        if (m->loop_variable_declaration != nullptr)
            m->loop_variable_declaration->accept(*this);
        
        if (m->initial_statement != nullptr)
            m->initial_statement->accept(*this);

        if (m->condition != nullptr)
            m->condition->accept(*this);

        if (m->body != nullptr)
            for(uint i=0; i< m->body->size(); i++)
                (*(m->body))[i]->accept(*this);
}

void TableConstructor::visit(ReturnNode *m) {
        if (m->return_value != nullptr)
            m->return_value->accept(*this);
}

void TableConstructor::visit(FunctionCallNode *m) {
        if (m->arguments != nullptr)
            for(uint i=0; i< m->arguments->size(); i++)
                (*(m->arguments))[i]->accept(*this);
}
