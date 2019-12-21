#pragma once
#include "AST/ast.hpp"
#include "visitor/visitor.hpp"

class SymbolTableNode {
    public:
        vector<SymbolEntryNode*>* entries;
        SymbolTableNode(vector<SymbolEntryNode*>* _entries);
        ~SymbolTableNode();
        // void accept(ASTVisitorBase &v) { v.visit(this); }
};