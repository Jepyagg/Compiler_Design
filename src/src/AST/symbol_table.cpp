#include "AST/symbol_table.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>


SymbolTableNode::SymbolTableNode(vector<SymbolEntryNode*>* _entries) {
        this->entries = _entries;
    }

SymbolTableNode::~SymbolTableNode(){
}
