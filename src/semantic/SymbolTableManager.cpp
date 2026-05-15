#include "SymbolTableManager.hpp"

namespace Semantic {

//==============================================================================
// Constructor
//==============================================================================

SymbolTableManager::SymbolTableManager() 
    : level(0), dx(0) {
    // Initialize with space for reserved words (0-32)
    tab.reserve(100);
    tab.resize(RESERVED_WORDS_COUNT);
    
    // Initialize block table with global block at index 0
    btab.reserve(20);
    btab.emplace_back();  // Global block at index 0
    
    // Initialize display
    display.reserve(10);
    display.push_back(0);  // Level 0 points to btab[0]
}

//==============================================================================
// Initialization
//==============================================================================

void SymbolTableManager::init() {
    // Clear any existing data (except reserved word slots)
    tab.resize(RESERVED_WORDS_COUNT);
    btab.clear();
    btab.emplace_back();  // Recreate global block
    atab.clear();
    display.clear();
    display.push_back(0);
    level = 0;
    dx = 0;
    
    // Index 0 is reserved (undefined/error)
    // Index 1: Integer type
    tab[1] = TabEntry("Integer", 0, OBJ_TYPE, TYPE_INTEGER, 0, 1, 0, 4);
    
    // Index 2: Real type
    tab[2] = TabEntry("Real", 1, OBJ_TYPE, TYPE_REAL, 0, 1, 0, 8);
    
    // Index 3: Char type
    tab[3] = TabEntry("Char", 2, OBJ_TYPE, TYPE_CHAR, 0, 1, 0, 1);
    
    // Index 4: Boolean type
    tab[4] = TabEntry("Boolean", 3, OBJ_TYPE, TYPE_BOOLEAN, 0, 1, 0, 1);
    
    // Index 5: String type
    tab[5] = TabEntry("String", 4, OBJ_TYPE, TYPE_STRING, 0, 1, 0, 256);
    
    // Index 6: True constant
    tab[6] = TabEntry("True", 5, OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 1);
    
    // Index 7: False constant
    tab[7] = TabEntry("False", 6, OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 0);
    
    // Update global block's last pointer
    btab[0].last = 7;
    
    // Note: You can add more predefined identifiers here
    // e.g., writeln, readln, etc.
}

//==============================================================================
// Scope Management
//==============================================================================

void SymbolTableManager::enterBlock() {
    // Increment level
    level++;
    
    // Create new block in btab
    int blockIndex = static_cast<int>(btab.size());
    btab.emplace_back();
    
    // Update display
    if (level < static_cast<int>(display.size())) {
        display[level] = blockIndex;
    } else {
        display.push_back(blockIndex);
    }
    
    // Reset data allocation for this block
    dx = 0;
}

void SymbolTableManager::exitBlock() {
    if (level > 0) {
        level--;
        dx = 0;  // Reset dx when exiting block
    }
}

//==============================================================================
// Symbol Registration
//==============================================================================

int SymbolTableManager::enterSymbol(const std::string& id, int obj, int type) {
    // Check if already declared in current scope
    if (isDeclaredInCurrentScope(id)) {
        return -1;  // Error: redeclaration
    }
    
    // Get current block
    int blockIndex = getCurrentBlock();
    
    // Create new tab entry
    int newIndex = static_cast<int>(tab.size());
    TabEntry entry;
    entry.id = id;
    entry.link = btab[blockIndex].last;  // Link to previous entry in this block
    entry.obj = obj;
    entry.type = type;
    entry.ref = 0;
    entry.nrm = 1;
    entry.lev = level;
    entry.adr = 0;
    
    // Calculate address for variables
    if (obj == OBJ_VARIABLE || obj == OBJ_PARAMETER) {
        entry.adr = allocateVariable();
    }
    
    // Add to tab
    tab.push_back(entry);
    
    // Update block's last pointer
    btab[blockIndex].last = newIndex;
    
    return newIndex;
}

int SymbolTableManager::enterConstant(const std::string& id, int type, int value) {
    int index = enterSymbol(id, OBJ_CONSTANT, type);
    if (index >= 0) {
        tab[index].adr = value;  // Store constant value in adr field
    }
    return index;
}

int SymbolTableManager::enterVariable(const std::string& id, int type) {
    return enterSymbol(id, OBJ_VARIABLE, type);
}

int SymbolTableManager::enterType(const std::string& id, int type, int ref) {
    int index = enterSymbol(id, OBJ_TYPE, type);
    if (index >= 0) {
        tab[index].ref = ref;
        // For named types, store size in adr
        if (type == TYPE_INTEGER) tab[index].adr = 4;
        else if (type == TYPE_REAL) tab[index].adr = 8;
        else if (type == TYPE_CHAR) tab[index].adr = 1;
        else if (type == TYPE_BOOLEAN) tab[index].adr = 1;
    }
    return index;
}

int SymbolTableManager::enterProcedure(const std::string& id) {
    return enterSymbol(id, OBJ_PROCEDURE, TYPE_UNDEFINED);
}

int SymbolTableManager::enterFunction(const std::string& id, int returnType) {
    return enterSymbol(id, OBJ_FUNCTION, returnType);
}

int SymbolTableManager::enterParameter(const std::string& id, int type, bool isVar) {
    int index = enterSymbol(id, OBJ_PARAMETER, type);
    if (index >= 0) {
        tab[index].nrm = isVar ? 0 : 1;  // 0 for var parameter, 1 for value
        
        // Update block's parameter info
        int blockIndex = getCurrentBlock();
        btab[blockIndex].lpar = index;
        // Update parameter size
        int paramSize = (type == TYPE_REAL) ? 8 : 4;
        btab[blockIndex].psze += paramSize;
    }
    return index;
}

int SymbolTableManager::enterField(const std::string& id, int type, int recordBlock) {
    (void)recordBlock;  // Will be used when implementing record field handling
    
    int savedLevel = level;
    
    // Temporarily set to record's block
    level = 0;  // Fields are at level 0 relative to record
    
    int index = enterSymbol(id, OBJ_FIELD, type);
    
    // Restore
    level = savedLevel;
    
    return index;
}

int SymbolTableManager::enterArrayType(int xtyp, int etyp, int low, int high, int elsz) {
    int atabIndex = static_cast<int>(atab.size());
    
    int size = (high - low + 1) * elsz;
    
    atab.emplace_back(xtyp, etyp, 0, low, high, elsz, size);
    
    return atabIndex;
}

//==============================================================================
// Symbol Lookup
//==============================================================================

int SymbolTableManager::lookup(const std::string& id) const {
    // Search from current level down to level 0
    for (int l = level; l >= 0; l--) {
        int blockIndex = display[l];
        
        // Traverse linked list of this block
        int current = btab[blockIndex].last;
        while (current >= RESERVED_WORDS_COUNT) {
            if (tab[current].id == id) {
                return current;
            }
            current = tab[current].link;
        }
    }
    
    // Check predefined identifiers (index 1-7)
    for (int i = 1; i < RESERVED_WORDS_COUNT && i < static_cast<int>(tab.size()); i++) {
        if (tab[i].id == id) {
            return i;
        }
    }
    
    return -1;  // Not found
}

int SymbolTableManager::lookupInCurrentScope(const std::string& id) const {
    int blockIndex = getCurrentBlock();
    
    // Traverse linked list of current block only
    int current = btab[blockIndex].last;
    while (current >= RESERVED_WORDS_COUNT) {
        if (tab[current].id == id) {
            return current;
        }
        current = tab[current].link;
    }
    
    return -1;  // Not found in current scope
}

bool SymbolTableManager::isDeclared(const std::string& id) const {
    return lookup(id) >= 0;
}

bool SymbolTableManager::isDeclaredInCurrentScope(const std::string& id) const {
    return lookupInCurrentScope(id) >= 0;
}

//==============================================================================
// Accessors
//==============================================================================

TabEntry& SymbolTableManager::getTabEntry(int index) {
    return tab.at(index);
}

const TabEntry& SymbolTableManager::getTabEntry(int index) const {
    return tab.at(index);
}

BTabEntry& SymbolTableManager::getBTabEntry(int index) {
    return btab.at(index);
}

const BTabEntry& SymbolTableManager::getBTabEntry(int index) const {
    return btab.at(index);
}

ATabEntry& SymbolTableManager::getATabEntry(int index) {
    return atab.at(index);
}

const ATabEntry& SymbolTableManager::getATabEntry(int index) const {
    return atab.at(index);
}

int SymbolTableManager::getType(const std::string& id) const {
    int index = lookup(id);
    if (index >= 0) {
        return tab[index].type;
    }
    return TYPE_UNDEFINED;
}

int SymbolTableManager::getObjectClass(const std::string& id) const {
    int index = lookup(id);
    if (index >= 0) {
        return tab[index].obj;
    }
    return -1;
}

//==============================================================================
// Debug/Output
//==============================================================================

void SymbolTableManager::printSymbolTable(std::ostream& os) const {
    os << "\n=== Symbol Table (tab) ===\n";
    os << "idx  id          obj       type    ref  nrm  lev  adr  link\n";
    os << "------------------------------------------------------------\n";
    
    for (size_t i = 0; i < tab.size(); i++) {
        const auto& entry = tab[i];
        if (entry.id.empty() && i >= RESERVED_WORDS_COUNT) {
            continue;  // Skip empty entries past reserved words
        }
        
        os << i << " ";
        if (i < 10) os << " ";
        
        // ID
        os << (entry.id.empty() ? "-" : entry.id);
        for (size_t j = entry.id.length(); j < 12; j++) os << " ";
        
        // Object class
        std::string objStr = objToString(entry.obj);
        os << objStr;
        for (size_t j = objStr.length(); j < 10; j++) os << " ";
        
        // Type
        std::string typeStr = typeToString(entry.type);
        os << typeStr;
        for (size_t j = typeStr.length(); j < 8; j++) os << " ";
        
        // Ref, nrm, lev, adr, link
        os << entry.ref << "    ";
        os << entry.nrm << "    ";
        os << entry.lev << "    ";
        os << entry.adr << "    ";
        os << entry.link;
        
        os << "\n";
    }
}

void SymbolTableManager::printBlockTable(std::ostream& os) const {
    os << "\n=== Block Table (btab) ===\n";
    os << "idx  last  lpar  psze  vsze\n";
    os << "----------------------------\n";
    
    for (size_t i = 0; i < btab.size(); i++) {
        const auto& entry = btab[i];
        os << i << " ";
        if (i < 10) os << " ";
        
        os << entry.last << "     ";
        if (entry.last < 10) os << " ";
        
        os << entry.lpar << "     ";
        if (entry.lpar < 10) os << " ";
        
        os << entry.psze << "     ";
        if (entry.psze < 10) os << " ";
        
        os << entry.vsze << "\n";
    }
}

void SymbolTableManager::printArrayTable(std::ostream& os) const {
    if (atab.empty()) {
        os << "\n=== Array Table (atab) ===\n";
        os << "(empty)\n";
        return;
    }
    
    os << "\n=== Array Table (atab) ===\n";
    os << "idx  xtyp  etyp  eref  low   high  elsz  size\n";
    os << "---------------------------------------------\n";
    
    for (size_t i = 0; i < atab.size(); i++) {
        const auto& entry = atab[i];
        os << i << " ";
        if (i < 10) os << " ";
        
        os << entry.xtyp << "     ";
        os << entry.etyp << "     ";
        os << entry.eref << "     ";
        os << entry.low << "     ";
        os << entry.high << "    ";
        os << entry.elsz << "     ";
        os << entry.size << "\n";
    }
}

//==============================================================================
// Helper Methods
//==============================================================================

int SymbolTableManager::findLastInBlock() const {
    return btab[getCurrentBlock()].last;
}

void SymbolTableManager::updateBlockLast(int tabIndex) {
    btab[getCurrentBlock()].last = tabIndex;
}

int SymbolTableManager::allocateVariable(int size) {
    int addr = dx;
    dx += size;
    
    // Update variable size for current block
    int blockIndex = getCurrentBlock();
    btab[blockIndex].vsze = dx;
    
    return addr;
}

} // namespace Semantic
