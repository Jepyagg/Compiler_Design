#pragma once
#include "AST/ast.hpp"
#include "visitor/visitor.hpp"

class SymbolEntryNode {
    public:
        string sym_name;
        enumKind sym_kind;
        int sym_level;
        VariableInfo* sym_type;
        vector<VariableInfo*> sym_attribute;
        int decl_check = 0;
        int stack_idx = 0;
        SymbolEntryNode(string _name, enumKind _kind, int _level, VariableInfo* _type, vector<VariableInfo*> _attribute);
        ~SymbolEntryNode();
};