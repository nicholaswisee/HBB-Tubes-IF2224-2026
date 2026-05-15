#pragma once

#include <string>

namespace Semantic {

/**
 * TabEntry - Identifier Table Entry
 * Stores information about identifiers (variables, constants, procedures, etc.)
 * Indexes 0-32 are reserved for predefined words
 * User-defined identifiers start from index 33
 */
struct TabEntry {
    std::string id;    // Name of the identifier
    int link;          // Pointer to previous identifier in same scope (linked list)
    int obj;           // Object class: constant, variable, type, procedure, function
    int type;          // Type code (from TypeSystem)
    int ref;           // Reference to atab (arrays) or btab (records/procedures)
    int nrm;           // Normal variable = 1, var parameter = 0
    int lev;           // Lexical level (0 = global, 1 = procedure, etc.)
    int adr;           // Address/offset meaning depends on obj:
                       //   - variable: offset in stack frame
                       //   - constant: constant value (for integer constants)
                       //   - field: offset in record
                       //   - procedure/function: entry point address
                       //   - type: size of the type
    
    TabEntry() : id(""), link(0), obj(0), type(0), ref(0), nrm(1), lev(0), adr(0) {}
    
    TabEntry(const std::string& name, int l, int o, int t, int r, int n, int lv, int a)
        : id(name), link(l), obj(o), type(t), ref(r), nrm(n), lev(lv), adr(a) {}
};

/**
 * BTabEntry - Block Table Entry
 * Stores information about blocks (procedures, functions, records)
 * Index 0 is the global block (main program)
 */
struct BTabEntry {
    int last;          // Index of last identifier in this block (in tab)
    int lpar;          // Index of last parameter (in tab), 0 if no parameters
    int psze;          // Total size of parameters (in memory units)
    int vsze;          // Total size of local variables (in memory units)
    
    BTabEntry() : last(0), lpar(0), psze(0), vsze(0) {}
    
    BTabEntry(int l, int lp, int p, int v)
        : last(l), lpar(lp), psze(p), vsze(v) {}
};

/**
 * ATabEntry - Array Table Entry
 * Stores information about array types
 */
struct ATabEntry {
    int xtyp;          // Index type (type of array indices)
    int etyp;          // Element type (type of array elements)
    int eref;          // Reference to element type details (if element is array/record)
    int low;           // Lower bound of index
    int high;          // Upper bound of index
    int elsz;          // Size of one element (in memory units)
    int size;          // Total size of array = (high - low + 1) * elsz
    
    ATabEntry() : xtyp(0), etyp(0), eref(0), low(0), high(0), elsz(0), size(0) {}
    
    ATabEntry(int xt, int et, int er, int l, int h, int es, int s)
        : xtyp(xt), etyp(et), eref(er), low(l), high(h), elsz(es), size(s) {}
};

} // namespace Semantic
