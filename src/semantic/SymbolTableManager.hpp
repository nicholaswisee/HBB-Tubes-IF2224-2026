#pragma once

#include "SymbolTable.hpp"
#include "TypeSystem.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

namespace Semantic {

/**
 * SemanticError - Exception for semantic analysis errors
 */
class SemanticError : public std::runtime_error {
public:
    int line;
    SemanticError(int lineNum, const std::string& msg) 
        : std::runtime_error(msg), line(lineNum) {}
};

/**
 * SymbolTableManager - Manages symbol tables (tab, btab, atab)
 * Provides interface for entering symbols, looking them up, and managing scopes
 */
class SymbolTableManager {
public:
    // Constructor - initializes symbol tables
    SymbolTableManager();
    
    // Initialize predefined identifiers (types, constants, etc.)
    void init();
    
    //===========================================================================
    // Scope Management
    //===========================================================================
    
    // Enter a new block (procedure, function, record, or compound statement)
    void enterBlock();
    
    // Exit current block
    void exitBlock();
    
    // Get current lexical level
    int getCurrentLevel() const { return level; }
    
    // Get current block index
    int getCurrentBlock() const { return display[level]; }
    
    //===========================================================================
    // Symbol Registration
    //===========================================================================
    
    // Enter a symbol into the current scope
    // Returns the index in tab where the symbol was entered
    int enterSymbol(const std::string& id, int obj, int type);
    
    // Enter a constant
    int enterConstant(const std::string& id, int type, int value);
    
    // Enter a variable
    int enterVariable(const std::string& id, int type);
    
    // Enter a type definition
    int enterType(const std::string& id, int type, int ref = 0);
    
    // Enter a procedure
    int enterProcedure(const std::string& id);
    
    // Enter a function with return type
    int enterFunction(const std::string& id, int returnType);
    
    // Enter a formal parameter
    int enterParameter(const std::string& id, int type, bool isVar = false);
    
    // Enter a record field
    int enterField(const std::string& id, int type, int recordBlock);
    
    // Enter an array type
    // Returns index in atab
    int enterArrayType(int xtyp, int etyp, int low, int high, int elsz = 1);
    
    //===========================================================================
    // Symbol Lookup
    //===========================================================================
    
    // Lookup identifier starting from current scope and going outward
    // Returns index in tab, or -1 if not found
    int lookup(const std::string& id) const;
    
    // Lookup identifier in current scope only
    // Returns index in tab, or -1 if not found
    int lookupInCurrentScope(const std::string& id) const;
    
    // Check if identifier is declared (in any accessible scope)
    bool isDeclared(const std::string& id) const;
    
    // Check if identifier is declared in current scope
    bool isDeclaredInCurrentScope(const std::string& id) const;
    
    //===========================================================================
    // Accessors
    //===========================================================================
    
    // Get tab entry by index
    TabEntry& getTabEntry(int index);
    const TabEntry& getTabEntry(int index) const;
    
    // Get btab entry by index
    BTabEntry& getBTabEntry(int index);
    const BTabEntry& getBTabEntry(int index) const;
    
    // Get atab entry by index
    ATabEntry& getATabEntry(int index);
    const ATabEntry& getATabEntry(int index) const;
    
    // Get the type of an identifier
    int getType(const std::string& id) const;
    
    // Get the object class of an identifier
    int getObjectClass(const std::string& id) const;
    
    //===========================================================================
    // Debug/Output
    //===========================================================================
    
    // Print symbol table contents
    void printSymbolTable(std::ostream& os = std::cout) const;
    
    // Print block table
    void printBlockTable(std::ostream& os = std::cout) const;
    
    // Print array table
    void printArrayTable(std::ostream& os = std::cout) const;
    
    // Get number of entries in tab
    int getTabSize() const { return static_cast<int>(tab.size()); }
    
    // Get number of entries in btab
    int getBTabSize() const { return static_cast<int>(btab.size()); }
    
    // Get number of entries in atab
    int getATabSize() const { return static_cast<int>(atab.size()); }
    
private:
    //===========================================================================
    // Data Members
    //===========================================================================
    
    std::vector<TabEntry> tab;      // Identifier table (index 0-32 reserved)
    std::vector<BTabEntry> btab;    // Block table (index 0 = global)
    std::vector<ATabEntry> atab;    // Array table
    
    std::vector<int> display;       // display[level] = index in btab for that level
    int level;                      // Current lexical level (0 = global)
    int dx;                         // Data allocation index (for calculating addresses)
    
    //===========================================================================
    // Helper Methods
    //===========================================================================
    
    // Find last entry in current block's linked list
    int findLastInBlock() const;
    
    // Update btab's last pointer to point to new entry
    void updateBlockLast(int tabIndex);
    
    // Allocate memory for a variable and return its address
    int allocateVariable(int size = 1);
};

} // namespace Semantic
