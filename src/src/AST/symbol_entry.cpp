#include "AST/symbol_entry.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>

SymbolEntryNode::SymbolEntryNode(string _name, enumKind _kind, int _level, enumSYMType _type, vector<VariableInfo> _attribute) {
        this->sym_name = _name;
        this->sym_kind = _kind;
        this->sym_level = _level;
        this->sym_type = _type;
        this->sym_attribute = _attribute;
}
SymbolEntryNode::~SymbolEntryNode(){
}