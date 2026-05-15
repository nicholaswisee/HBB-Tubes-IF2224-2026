#include "semantic/SymbolTableManager.hpp"
#include "semantic/TypeSystem.hpp"
#include <iostream>
#include <cassert>

using namespace Semantic;

void testPredefinedIdentifiers() {
    std::cout << "\n=== Test 1: Predefined Identifiers ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Check that predefined types exist
    assert(symTable.isDeclared("Integer"));
    assert(symTable.isDeclared("Real"));
    assert(symTable.isDeclared("Char"));
    assert(symTable.isDeclared("Boolean"));
    assert(symTable.isDeclared("String"));
    assert(symTable.isDeclared("True"));
    assert(symTable.isDeclared("False"));
    
    // Check types
    assert(symTable.getType("Integer") == TYPE_INTEGER);
    assert(symTable.getType("Real") == TYPE_REAL);
    assert(symTable.getType("Boolean") == TYPE_BOOLEAN);
    assert(symTable.getType("True") == TYPE_BOOLEAN);
    assert(symTable.getType("False") == TYPE_BOOLEAN);
    
    std::cout << "✓ All predefined identifiers present and correct\n";
    symTable.printSymbolTable();
}

void testVariableDeclaration() {
    std::cout << "\n=== Test 2: Variable Declaration ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Enter variables
    int idx1 = symTable.enterVariable("x", TYPE_INTEGER);
    int idx2 = symTable.enterVariable("y", TYPE_REAL);
    int idx3 = symTable.enterVariable("flag", TYPE_BOOLEAN);
    
    assert(idx1 > 0);
    assert(idx2 > 0);
    assert(idx3 > 0);
    
    // Check they exist
    assert(symTable.isDeclared("x"));
    assert(symTable.isDeclared("y"));
    assert(symTable.isDeclared("flag"));
    
    // Check types
    assert(symTable.getType("x") == TYPE_INTEGER);
    assert(symTable.getType("y") == TYPE_REAL);
    assert(symTable.getType("flag") == TYPE_BOOLEAN);
    
    std::cout << "✓ Variables declared successfully\n";
    symTable.printSymbolTable();
    symTable.printBlockTable();
}

void testScopeManagement() {
    std::cout << "\n=== Test 3: Scope Management ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Global scope
    symTable.enterVariable("globalVar", TYPE_INTEGER);
    assert(symTable.getCurrentLevel() == 0);
    
    // Enter a procedure scope
    symTable.enterBlock();
    assert(symTable.getCurrentLevel() == 1);
    
    symTable.enterVariable("localVar", TYPE_REAL);
    
    // Check that local is found in current scope
    assert(symTable.isDeclaredInCurrentScope("localVar"));
    assert(!symTable.isDeclaredInCurrentScope("globalVar"));  // Not in current
    assert(symTable.isDeclared("globalVar"));  // But found when searching outer
    
    // Check types
    assert(symTable.getType("localVar") == TYPE_REAL);
    assert(symTable.getType("globalVar") == TYPE_INTEGER);
    
    // Exit block
    symTable.exitBlock();
    assert(symTable.getCurrentLevel() == 0);
    
    // localVar should not be found anymore
    assert(!symTable.isDeclared("localVar"));
    assert(symTable.isDeclared("globalVar"));
    
    std::cout << "✓ Scope management works correctly\n";
    symTable.printSymbolTable();
    symTable.printBlockTable();
}

void testLookup() {
    std::cout << "\n=== Test 4: Symbol Lookup ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Add some variables
    symTable.enterVariable("a", TYPE_INTEGER);
    symTable.enterVariable("b", TYPE_REAL);
    
    // Lookup should work
    int idxA = symTable.lookup("a");
    int idxB = symTable.lookup("b");
    int idxInt = symTable.lookup("Integer");
    
    assert(idxA >= 0);
    assert(idxB >= 0);
    assert(idxInt >= 0);
    
    // Not found
    int idxNotFound = symTable.lookup("nonexistent");
    assert(idxNotFound == -1);
    
    // Check entry details
    const TabEntry& entryA = symTable.getTabEntry(idxA);
    assert(entryA.id == "a");
    assert(entryA.type == TYPE_INTEGER);
    assert(entryA.obj == OBJ_VARIABLE);
    
    std::cout << "✓ Lookup works correctly\n";
}

void testRedeclaration() {
    std::cout << "\n=== Test 5: Redeclaration Detection ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // First declaration
    int idx1 = symTable.enterVariable("x", TYPE_INTEGER);
    assert(idx1 > 0);
    
    // Second declaration in same scope should fail
    int idx2 = symTable.enterVariable("x", TYPE_REAL);
    assert(idx2 == -1);  // Should return error
    
    // But redeclaration in inner scope is OK (shadowing)
    symTable.enterBlock();
    int idx3 = symTable.enterVariable("x", TYPE_BOOLEAN);
    assert(idx3 > 0);
    
    // Now lookup should find the inner x
    int foundIdx = symTable.lookup("x");
    assert(foundIdx == idx3);
    assert(symTable.getType("x") == TYPE_BOOLEAN);
    
    std::cout << "✓ Redeclaration handled correctly\n";
}

void testTypeSystem() {
    std::cout << "\n=== Test 6: Type System ===\n";
    
    // Test type compatibility
    assert(TypeSystem::isCompatible(TYPE_INTEGER, TYPE_INTEGER));
    assert(!TypeSystem::isCompatible(TYPE_REAL, TYPE_INTEGER));  // Different types
    assert(!TypeSystem::isCompatible(TYPE_INTEGER, TYPE_BOOLEAN));
    
    // Test assignment compatibility
    assert(TypeSystem::isAssignmentCompatible(TYPE_REAL, TYPE_INTEGER));
    assert(TypeSystem::isAssignmentCompatible(TYPE_INTEGER, TYPE_INTEGER));
    assert(!TypeSystem::isAssignmentCompatible(TYPE_INTEGER, TYPE_REAL));
    
    // Test binary operations
    assert(TypeSystem::getBinaryOpResultType("+", TYPE_INTEGER, TYPE_INTEGER) == TYPE_INTEGER);
    assert(TypeSystem::getBinaryOpResultType("+", TYPE_INTEGER, TYPE_REAL) == TYPE_REAL);
    assert(TypeSystem::getBinaryOpResultType("+", TYPE_REAL, TYPE_INTEGER) == TYPE_REAL);
    assert(TypeSystem::getBinaryOpResultType("/", TYPE_INTEGER, TYPE_INTEGER) == TYPE_REAL);
    assert(TypeSystem::getBinaryOpResultType("div", TYPE_INTEGER, TYPE_INTEGER) == TYPE_INTEGER);
    assert(TypeSystem::getBinaryOpResultType("=", TYPE_INTEGER, TYPE_INTEGER) == TYPE_BOOLEAN);
    assert(TypeSystem::getBinaryOpResultType("and", TYPE_BOOLEAN, TYPE_BOOLEAN) == TYPE_BOOLEAN);
    
    // Test unary operations
    assert(TypeSystem::getUnaryOpResultType("-", TYPE_INTEGER) == TYPE_INTEGER);
    assert(TypeSystem::getUnaryOpResultType("not", TYPE_BOOLEAN) == TYPE_BOOLEAN);
    assert(TypeSystem::getUnaryOpResultType("+", TYPE_REAL) == TYPE_REAL);
    
    std::cout << "✓ Type system works correctly\n";
}

void testArrayType() {
    std::cout << "\n=== Test 7: Array Type Entry ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Enter array type: array[1..10] of Integer
    int atabIdx = symTable.enterArrayType(TYPE_INTEGER, TYPE_INTEGER, 1, 10, 4);
    assert(atabIdx == 0);  // First array entry
    
    // Create a type that uses this array
    int typeIdx = symTable.enterType("IntArray", TYPE_UNDEFINED, atabIdx);
    
    // Enter a variable of this array type
    int varIdx = symTable.enterVariable("arr", TYPE_UNDEFINED);
    if (varIdx >= 0) {
        symTable.getTabEntry(varIdx).ref = atabIdx;
    }
    
    // Check array table
    const ATabEntry& arrEntry = symTable.getATabEntry(atabIdx);
    assert(arrEntry.low == 1);
    assert(arrEntry.high == 10);
    assert(arrEntry.etyp == TYPE_INTEGER);
    assert(arrEntry.size == 40);  // 10 elements * 4 bytes
    
    std::cout << "✓ Array type entry works correctly\n";
    symTable.printArrayTable();
}

void testConstants() {
    std::cout << "\n=== Test 8: Constants ===\n";
    
    SymbolTableManager symTable;
    symTable.init();
    
    // Enter user-defined constants
    int idx1 = symTable.enterConstant("MAX_SIZE", TYPE_INTEGER, 100);
    int idx2 = symTable.enterConstant("PI_APPROX", TYPE_INTEGER, 3);
    
    assert(idx1 > 0);
    assert(idx2 > 0);
    
    // Check values
    assert(symTable.getTabEntry(idx1).adr == 100);
    assert(symTable.getTabEntry(idx2).adr == 3);
    
    // Check predefined constants
    assert(symTable.getTabEntry(symTable.lookup("True")).adr == 1);
    assert(symTable.getTabEntry(symTable.lookup("False")).adr == 0);
    
    std::cout << "✓ Constants work correctly\n";
}

int main() {
    std::cout << "====================================\n";
    std::cout << "Symbol Table Manager Test Suite\n";
    std::cout << "====================================\n";
    
    try {
        testPredefinedIdentifiers();
        testVariableDeclaration();
        testScopeManagement();
        testLookup();
        testRedeclaration();
        testTypeSystem();
        testArrayType();
        testConstants();
        
        std::cout << "\n====================================\n";
        std::cout << "✓ All tests passed!\n";
        std::cout << "====================================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
