#include "codegen/codegen.hpp"
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
#include <fstream>
#include <iomanip>
#include <cstdio>

extern string file_name2;                                               // open file to write
extern string output_dir;                                               // output directory

SymbolTableNode* cur_table = nullptr;                                   // point to current node symbol table
SymbolTableNode* cur_func_table = nullptr;                              // if in func, point to func symbol table
vector<SymbolTableNode*> code_table_list;                               // symbol table list
ofstream fout;                                                          // open file
bool tempuse[15];                                                       // check which tem can use t0~t6, s2~s11
int a_tmp[8];                                                           // store argument's stack index 
int curr_idx = -20;                                                     // know current stack index
int expr_check = 0;                                                     // whether in stack or not
int funccall_check = 0;
int local_val = 0, assign_check = 0;
int local_idx = -1;
int global_idx = -1;
int label_use = 1;
int while_cond = 0, while_label = -1;
int for_cond = 0, for_idx = 0;

string check_and_change(int idx) {
    if(idx > 6) {
        string tmp = "s" + to_string(idx - 5);
        return tmp;
    } else {
        string tmp = "t" + to_string(idx);
        return tmp;
    }
}

// get temporary register number
int get_stack_idx() {
    for(int i = 0; i < 15; ++i) {
        if(!tempuse[i]) {
            return i;
        }
    }
    return -1;
}

void codegen::visit(ProgramNode *m) {

    // open file
    FILE *pFile;
    string tmp_name = output_dir + "/" + file_name2 + ".s";
    pFile = fopen(tmp_name.c_str(), "w");
    fclose(pFile);
    fout.open(output_dir + "/" + file_name2 + ".s");
    fout << "    .file \"" + file_name2 + ".p\"\n";
    fout << "    .option nopic\n";

    // point to current node table and push table
    cur_table = m->symbol_table_node;
    code_table_list.push_back(m->symbol_table_node);
    
    // use symbol table information, so no need visit
    // visit declaration
    // if (m->declaration_node_list != nullptr) {
    //     for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
    //         (*(m->declaration_node_list))[i]->accept(*this);
    //     }
    // }

    // search table to find uninitialized global variable
    fout << ".bss\n";
    for(uint i = 0; i < cur_table->entries->size(); ++i) {
        string tmp = (*cur_table->entries)[i]->sym_name;
        if((*cur_table->entries)[i]->sym_kind == KIND_VAR) {
            fout << tmp + ":\n";
            fout << "    .word 0\n";
        }
    }

    // search table to find constant
    fout << ".text\n";
    for(uint i = 0; i < cur_table->entries->size(); ++i) {
        string tmp = (*cur_table->entries)[i]->sym_name;
        if((*cur_table->entries)[i]->sym_kind == KIND_CONST) {
            fout << tmp + ":\n";
            fout << "    .word ";
            fout << (*cur_table->entries)[i]->sym_type->int_literal << "\n";
        }
    }

    // initialize all temporary register
    for(int i = 0; i < 8; ++i) {
        tempuse[i] = false;
    }

    // visit function
    if(m->function_node_list != nullptr) {
        for(uint i = 0; i < m->function_node_list->size(); ++i) {
            (*(m->function_node_list))[i]->accept(*this);

            // reset stack index
            curr_idx = - 20;
        }

        // reset all temporary register
        for(int i = 0; i < 8; ++i) {
            tempuse[i] = false;
        }
    }

    // gen main code
    fout << ".text\n"; 
    fout << "    .align 2\n"; 
    fout << "    .global main\n"; 
    fout << "    .type main, @function\n"; 
    fout << "main:\n";
    fout << "    addi sp, sp, -64\n";
    fout << "    sd ra, 56(sp)\n";
    fout << "    sd s0, 48(sp)\n";
    fout << "    addi s0, sp, 64\n";

    // visit compound
    if (m->compound_statement_node != nullptr) {
        m->compound_statement_node->accept(*this);
    }

    // gen main end code
    fout << "    ld ra, 56(sp)\n";
    fout << "    ld s0, 48(sp)\n";
    fout << "    addi sp, sp, 64\n";
    fout << "    jr ra\n";
    fout << "    .size main, .-main\n";
    fout.close();
}

void codegen::visit(DeclarationNode *m) {

    // visit variable node
    if (m->variables_node_list != nullptr) {
        for(uint i = 0; i < m->variables_node_list->size(); ++i) {
            (*(m->variables_node_list))[i]->accept(*this);
        }
    }
}

void codegen::visit(VariableNode *m) {

    int can_use_temp = get_stack_idx();

    // visit constant value
    if (m->constant_value_node != nullptr) {
        m->constant_value_node->accept(*this);
    }

    // search table to find idx
    for(uint i = 0; i < cur_table->entries->size(); ++i) {
        string tmp = (*cur_table->entries)[i]->sym_name;
        if(tmp == m->variable_name && (*cur_table->entries)[i]->sym_kind == KIND_CONST) {
            
            // store value to stack
            string stack_idx = to_string((*cur_table->entries)[i]->stack_idx);
            string reg_tmp = check_and_change(can_use_temp);
            fout << "    sw " + reg_tmp + ", " + stack_idx + "(s0)\n";
            tempuse[can_use_temp] = false;
            break;
        }
        if(for_cond == 1 && tmp == m->variable_name) {
            (*cur_table->entries)[i]->stack_idx = curr_idx;
            curr_idx -= 4;
            break;
        }
    }
}

void codegen::visit(ConstantValueNode *m) {

    // use temporary register to store value
    int can_use_temp = get_stack_idx();
    string intvalue = to_string(m->constant_value->int_literal);
    string reg_tmp = check_and_change(can_use_temp);
    fout << "    li " + reg_tmp + ", " + intvalue + "\n";
    tempuse[can_use_temp] = true;
}

void codegen::visit(FunctionNode *m) {

    // initialize function stack index
    int stack_index = -20;
    cur_table = m->symbol_table_node; // current table is function(first compound) table
    cur_func_table = m->symbol_table_node; // point to function(first compound) table

    // generate text code and function prologue code
    fout << ".text\n"; 
    fout << "    .align 2\n"; 
    fout << "    .global " + m->function_name + "\n"; 
    fout << "    .type " + m->function_name + ", @function\n"; 
    fout << m->function_name + ":\n";
    fout << "    addi sp, sp, -64\n";
    fout << "    sd ra, 56(sp)\n";
    fout << "    sd s0, 48(sp)\n";
    fout << "    addi s0, sp, 64\n";
    
    // search table to find parameters
    int tmp_stack_index = stack_index;
    for(uint i = 0; i < cur_table->entries->size(); ++i) {
        string tmp = (*cur_table->entries)[i]->sym_name;
        if((*cur_table->entries)[i]->sym_kind == KIND_PARAM) {
            
            // store parameter to stack
            if(i < 8) {
                (*cur_table->entries)[i]->stack_idx = tmp_stack_index;
                string idx = to_string(i);
                string stack_idx = to_string(tmp_stack_index);
                fout << "    sw a" + idx + ", " + stack_idx + "(s0)\n";
                tmp_stack_index -= 4;
            }
        }
    }
    curr_idx = tmp_stack_index;

    // use symbol table information, so no need visit
    // visit function parameters
    // if (m->parameters != nullptr) {
    //     for(uint i = 0; i < m->parameters->size(); ++i) {
    //         (*(m->parameters))[i]->node->accept(*this);
    //     }
    // }

    // visit function body
    if (m->body != nullptr) {
        m->body->accept(*this);
    }

    // gen function epilogue code
    fout << "    ld ra, 56(sp)\n";
    fout << "    ld s0, 48(sp)\n";
    fout << "    addi sp, sp, 64\n";
    fout << "    jr ra\n";
    fout << "    .size " + m->function_name + ", .-" + m->function_name + "\n";
}

void codegen::visit(CompoundStatementNode *m) {

    // point to current node table and push table
    cur_table = m->symbol_table_node;
    code_table_list.push_back(m->symbol_table_node);
     
    // search table to find variable
    int tmp_stack_index = curr_idx;
    for(uint i = 0; i < cur_table->entries->size(); ++i) {
        if((*cur_table->entries)[i]->sym_kind == KIND_VAR || (*cur_table->entries)[i]->sym_kind == KIND_CONST) {
            (*cur_table->entries)[i]->stack_idx = tmp_stack_index;
            tmp_stack_index -= 4;
        }
    }
    curr_idx = tmp_stack_index;

    // visit declaration
    if (m->declaration_node_list != nullptr) {
        for(uint i = 0; i < m->declaration_node_list->size(); ++i) {
            (*(m->declaration_node_list))[i]->accept(*this);
        }
    }

    // visit statement
    if (m->statement_node_list != nullptr) {
        for(uint i = 0; i < m->statement_node_list->size(); ++i) {
            cur_table = m->symbol_table_node;
            (*(m->statement_node_list))[i]->accept(*this);
        }
    }

    // pop table in list;
    code_table_list.pop_back();
}

void codegen::visit(AssignmentNode *m) {

    int use_idx = get_stack_idx(), local_check = 0, local_idx_check = 0;

    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);  
    }
    assign_check = 1;
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
        local_idx_check = local_idx;
        local_check = local_val;
        local_val = 0;
    }
    local_idx = 0;
    assign_check = 0;

    // gen assignment code
    if(local_check == 0) {
        string reg_tmp = check_and_change(use_idx);
        string reg_tmp2 = check_and_change(use_idx + 1);
        fout << "    sw " + reg_tmp + ", 0(" + reg_tmp2 + ")\n";
        tempuse[use_idx] = false;
        tempuse[use_idx + 1] = false;
    } else {
        string idx = to_string(local_idx_check);
        string reg_tmp = check_and_change(use_idx);
        fout << "    sw " + reg_tmp + ", " + idx + "(s0)\n";
        local_val = 0;
    }
    if(for_cond == 0) {
        tempuse[use_idx] = false;
    } else {
        for_idx = local_idx_check;
    }
}

void codegen::visit(PrintNode *m) {

    int use_idx = get_stack_idx();
    
    // visit expression node
    if (m->expression_node != nullptr) {
        m->expression_node->accept(*this);
    }
    
    // generate print code
    int local_check = local_val, local_idx_check = local_idx;
    if(local_check == 0) {
        string reg_tmp = check_and_change(use_idx);
        fout << "    lw a0, 0(" + reg_tmp + ")\n";
    } else {
        string idx = to_string(local_idx_check);
        fout << "    lw a0, " + idx + "(s0)\n";
        local_val = 0;
    }
    fout << "    jal ra, print\n";
    tempuse[use_idx] = false;
}

void codegen::visit(ReadNode *m) {

    // generate read code
    int use_idx = get_stack_idx();
    fout << "    jal ra, read\n";
    
    // visit variable reference node
    if (m->variable_reference_node != nullptr) {
        m->variable_reference_node->accept(*this);
    }

    // generate read code
    int local_check = local_val, local_idx_check = local_idx;
    if(local_check == 0) {
        string reg_tmp = check_and_change(use_idx);
        fout << "    sw a0, 0(" + reg_tmp + ")\n";
    } else {
        string idx = to_string(local_idx_check);
        fout << "    sw a0, " + idx + "(s0)\n";
        local_val = 0;
    }
    tempuse[use_idx] = false;
}

void codegen::visit(VariableReferenceNode *m) {

    // search table list to check declare
    int can_use_temp = get_stack_idx();
    int find = 0;

    // search local table
    for(int i = code_table_list.size() - 1; i > 0; --i) {
        for(uint j = 0; j < code_table_list[i]->entries->size(); ++j) {
            string tmp = (*code_table_list[i]->entries)[j]->sym_name;
            if((*code_table_list[i]->entries)[j]->sym_kind != KIND_FUNC) {
                if(m->variable_name == tmp) {
                    find = 1;
                    local_val = 1;
                    local_idx = (*code_table_list[i]->entries)[j]->stack_idx;
                    if(expr_check == 1) {
                        string idx = to_string(local_idx);
                        string reg_tmp = check_and_change(can_use_temp);
                        fout << "    lw " + reg_tmp + ", " + idx + "(s0)\n";
                    }
                }
            }
        }
        if(find == 1) {
            break;
        }
    }

    // search global table
    for(uint j = 0; j < code_table_list[0]->entries->size(); ++j) {
        string tmp = (*code_table_list[0]->entries)[j]->sym_name;
        if((*code_table_list[0]->entries)[j]->sym_kind == KIND_VAR || (*code_table_list[0]->entries)[j]->sym_kind == KIND_CONST) {
            if(m->variable_name == tmp && find == 0) {
                find = 1;

                // generate variable reference code
                string reg_tmp = check_and_change(can_use_temp);
                string reg_tmp2 = check_and_change(can_use_temp + 1);
                global_idx = can_use_temp;
                tempuse[can_use_temp] = true;
                fout << "    la " + reg_tmp + ", " + tmp + "\n";
                if(expr_check == 1) {
                    fout << "    lw " + reg_tmp2 + ", 0(" + reg_tmp + ")\n";
                    fout << "    mv " + reg_tmp + ", " + reg_tmp2 + "\n";
                }
            }
        }
        if(find == 1) {
            break;
        }
    }

    // not have array
    // if (m->expression_node_list != nullptr) {
    //     for(uint i = 0; i < m->expression_node_list->size(); ++i) {
    //         (*(m->expression_node_list))[i]->accept(*this);
    //     }
    // }
}

void codegen::visit(BinaryOperatorNode *m) {

    int use_opd = 0, idx = get_stack_idx();
    string opt = "";
    switch(m->op) {
        case OP_PLUS: use_opd = 1; opt = "+"; break;
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

    string left_idx = check_and_change(idx);
    string right_idx = check_and_change(idx + 1);
    string tmp_idx = check_and_change(idx + 2);

    string command = tmp_idx + ", " + left_idx + ", " + right_idx + "\n";
    string branch_label = "L" + to_string(label_use);
    string while_str = "L" + to_string(while_label);
    string branch_command = left_idx + ", " + right_idx + ", " + branch_label + "\n";
    string while_command = left_idx + ", " + right_idx + ", " + while_str + "\n";

    expr_check = 1;
    if (m->left_operand != nullptr) {
        m->left_operand->accept(*this);
        local_val = 0;
    }

    tempuse[idx] = true;
    expr_check = 1;
    if (m->right_operand != nullptr) {
        m->right_operand->accept(*this);
        local_val = 0;
    }

    if(opt == "+") {
        fout << "    addw " + command;
    } else if(opt == "-") {
        fout << "    subw " + command;
    } else if(opt == "*") {
        fout << "    mulw " + command;
    } else if(opt == "/") {
        fout << "    divw " + command;
    } else if(opt == "mod") {
        fout << "    remw " + command;
    } else if(opt == "<") {
        if(while_cond == 1) {
            fout << "    blt " + while_command;
        } else {
            fout << "    bge " + branch_command;
        }
    } else if(opt == "<=") {
        if(while_cond == 1) {
            fout << "    ble " + while_command;
        } else {
            fout << "    bgt " + branch_command;
        }
    } else if(opt == "=") {
        if(while_cond == 1) {
            fout << "    beq " + while_command;
        } else {
            fout << "    bne " + branch_command;
        }
    } else if(opt == ">") {
        if(while_cond == 1) {
            fout << "    bgt " + while_command;
        } else {
            fout << "    ble " + branch_command;
        }
    } else if(opt == ">=") {
        if(while_cond == 1) {
            fout << "    bge " + while_command;
        } else {
            fout << "    blt " + branch_command;
        }
    } else if(opt == "<>") {
        if(while_cond == 1) {
            fout << "    bne " + while_command;
        } else {
            fout << "    beq " + branch_command;
        }
    }

    if(use_opd != 4) {
        fout << "    mv " + left_idx + ", " + tmp_idx + "\n";
    }
    tempuse[idx + 1] = false;
    expr_check = 0;
}

void codegen::visit(UnaryOperatorNode *m) {

    string opt = "";
    switch(m->op) {
        case OP_MINUS: opt += "-"; break;
        default: break;
    }
       
    if (m->operand != nullptr) {
        m->operand->accept(*this);
    }

    if(opt == "-") {
        fout << "    li t0, 0\n";
        fout << "    subw t2, t0, t1\n";
        fout << "    mv t0, t2\n";
    }
}

void codegen::visit(IfNode *m) {

    int use_idx = get_stack_idx(), true_label, false_label, finish_label;
    true_label = label_use++;
    false_label = label_use;
    
    if (m->condition != nullptr) {
        m->condition->accept(*this);
        tempuse[use_idx] = false;
    }
    label_use++;
    finish_label = label_use++;
    string fin_label = "L" + to_string(finish_label);
    string if_label = "L" + to_string(true_label);
    fout << if_label + ":\n";

    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
        fout << "    j " + fin_label + "\n";
    }

    string else_label = "L" + to_string(false_label);
    fout << else_label + ":\n";

    if (m->body_of_else != nullptr) {
        for(uint i = 0; i < m->body_of_else->size(); ++i) {
            (*(m->body_of_else))[i]->accept(*this);
        }
        // fout << "    j " + fin_label + "\n";
    }
    fout << fin_label + ":\n";
}

void codegen::visit(WhileNode *m) {

    int use_idx = get_stack_idx(), condition_label, body_label;
    body_label = label_use;
    condition_label = label_use + 1;
    string cond_label = "L" + to_string(condition_label);
    string bd_label = "L" + to_string(body_label);
    fout << "    j " + cond_label + "\n";
    label_use += 2;
    
    // visit while body
    if (m->body != nullptr) {
        fout << bd_label + ":\n";
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    // generate while condition code
    fout << cond_label + ":\n";
    if (m->condition != nullptr) {
        while_cond = 1;
        while_label = body_label;
        m->condition->accept(*this);
        while_label = -1;
        while_cond = 0;
        tempuse[use_idx] = false;
    }
}

void codegen::visit(ForNode *m) {

    // point to current node table and push table
    cur_table = m->symbol_table_node;
    code_table_list.push_back(m->symbol_table_node);

    int use_idx = get_stack_idx(), condition_label, finish_label, forvar_idx = 0, constant_idx;
    condition_label = label_use++;
    finish_label = label_use++;
    string cond_label = "L" + to_string(condition_label);
    string fin_label = "L" + to_string(finish_label);
    string iter = check_and_change(use_idx);
    string iter2 = check_and_change(use_idx + 1);
    string iter3 = check_and_change(use_idx + 2);
    string tmp_reg = check_and_change(use_idx + 3);
    tempuse[use_idx + 2] = true; // add one to iter
    
    if (m->loop_variable_declaration != nullptr) {
        for_cond = 1;
        m->loop_variable_declaration->accept(*this);
    }
    
    if (m->initial_statement != nullptr) {
        m->initial_statement->accept(*this);
        forvar_idx = for_idx;
        for_idx = 0;
        for_cond = 0;
    }
    constant_idx = curr_idx;
    curr_idx -= 4;
    string const_idx = to_string(constant_idx);
    // visit second integer
    if (m->condition != nullptr) {
        m->condition->accept(*this);
        fout << "    sw " + iter2 + ", " + const_idx + "(s0)\n";
    }

    string forrr_idx = to_string(forvar_idx);
    fout << cond_label + ":\n";
    fout << "    lw " + iter + ", " + forrr_idx + "(s0)\n";
    fout << "    lw " + iter2 + ", " + const_idx + "(s0)\n";
    fout << "    bge " + iter + ", " + iter2 + ", " + fin_label + "\n";

    if (m->body != nullptr) {
        for(uint i = 0; i < m->body->size(); ++i) {
            (*(m->body))[i]->accept(*this);
        }
    }

    fout << "    lw " + iter + ", " + forrr_idx + "(s0)\n";
    fout << "    li " + iter3 + ", 1\n";
    fout << "    addw " + tmp_reg + ", " + iter + ", " + iter3 + "\n";
    fout << "    mv " + iter + ", " + tmp_reg + "\n";
    fout << "    sw " + iter + ", " + forrr_idx + "(s0)\n";
    fout << "    j " + cond_label + "\n";
    fout << fin_label + ":\n";
    tempuse[use_idx] = false;
    tempuse[use_idx + 1] = false;
    tempuse[use_idx + 2] = false;
    curr_idx += 8;
    code_table_list.pop_back();
}

void codegen::visit(ReturnNode *m) {
    if (m->return_value != nullptr) {
        m->return_value->accept(*this);
    }
    string idx = to_string(local_idx);
    fout << "    lw a0, " + idx + "(s0)\n";
    local_val = 0;
}

void codegen::visit(FunctionCallNode *m) {

    // gen function_call code
    int use_idx = get_stack_idx(), tmp_expr = expr_check, cnt = 0;
    expr_check = 0;
    tempuse[use_idx] = true;
    if (m->arguments != nullptr) {
        for(uint i = 0; i < m->arguments->size(); ++i) {
            cnt++;
            funccall_check = 0;
            (*(m->arguments))[i]->accept(*this);
            int local_check = local_val, local_idx_check = local_idx;
            string reg_tmp = "a" + to_string(i);
            string reg_idx = to_string(curr_idx);
            curr_idx -= 4;
            if(local_check == 0) {
                string reg_tmp2 = check_and_change(global_idx);
                tempuse[global_idx] = false;
                global_idx = -1;
                if(funccall_check == 1) {
                    fout << "    sw " + reg_tmp2 + ", " + reg_idx + "(s0)\n";
                    fout << "    lw " + reg_tmp + ", " + reg_idx + "(s0)\n";
                } else {
                    fout << "    lw " + reg_tmp + ", 0(" + reg_tmp2 + ")\n";
                }
            } else {
                string idx = to_string(local_idx_check);
                fout << "    lw " + reg_tmp + ", " + idx + "(s0)\n";
                local_val = 0;
            }
            fout << "    sw " + reg_tmp + ", " + reg_idx + "(s0)\n";
        }
    }
    for(int i = cnt - 1; i >= 0; --i) {
        curr_idx += 4;
        string idx = to_string(curr_idx);
        string reg_tmp = "a" + to_string(i);
        fout << "    lw " + reg_tmp + ", " + idx + "(s0)\n";
    }
    string reg_tmp = check_and_change(use_idx);
    fout << "    jal ra, " + m->function_name + "\n";
    fout << "    mv " + reg_tmp + ", a0\n";
    global_idx = use_idx;
    expr_check = tmp_expr;
    funccall_check = 1;
}
