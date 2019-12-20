#pragma once
#include "AST/ast.hpp"
#include "visitor/visitor.hpp"

class SymbolEntryNode : public ASTNodeBase {
    public:
        string sym_name;
        enumKind sym_kind;
        int sym_level;
        enumSYMType sym_type;
        vector<VariableInfo> sym_attribute;
        SymbolEntryNode(string _name, enumKind _kind, int _level, enumSYMType _type, vector<VariableInfo> _attribute);
        ~SymbolEntryNode();
        // void accept(ASTVisitorBase &v) { v.visit(this); }
};