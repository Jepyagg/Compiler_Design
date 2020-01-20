#include "AST/symbol_table.hpp"
#include "AST/symbol_entry.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>


SymbolTableNode::SymbolTableNode(vector<SymbolEntryNode*>* _entries) {
    this->entries = _entries;
}

SymbolTableNode::~SymbolTableNode(){
    if(this->entries != nullptr) {
        for(uint i = 0; i < this->entries->size(); ++i) {
            delete (*(this->entries))[i];
            (*(this->entries))[i] = nullptr;
        }
        delete this->entries;
        this->entries = nullptr;
    }
}
