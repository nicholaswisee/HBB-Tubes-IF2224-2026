#pragma once

#include <string>
#include <vector>
#include <iostream>

// Symbol table entry for identifiers (tab)
struct TabEntry {
    std::string id;    // identifier name
    int link = 0;      // index of previous identifier in same block
    int obj = 0;       // object class (constant, variable, type, proc, func)
    int type = 0;      // type code
    int ref = 0;       // reference to atab/btab for composite types
    int nrm = 1;       // 1 = normal variable, 0 = var parameter
    int lev = 0;       // lexical level
    int adr = 0;       // address / offset / constant value
};

// Block table entry (btab)
struct BTabEntry {
    int last = 0;      // index of last identifier in this block
    int lpar = 0;      // index of last parameter
    int psze = 0;      // parameter size
    int vsze = 0;      // variable size
};

// Array table entry (atab)
struct ATabEntry {
    int xtyp = 0;      // index type
    int etyp = 0;      // element type
    int eref = 0;      // reference to element detail (atab/btab)
    int low = 0;       // lower bound
    int high = 0;      // upper bound
    int elsz = 0;      // element size
    int size = 0;      // total size
};

class SymbolTableManager {
public:
    SymbolTableManager();

    // --- tab management ---
    int enter(const std::string& id, int obj, int type, int ref = 0, int nrm = 1, int lev = -1, int adr = 0);
    int lookup(const std::string& id) const;
    int lookupLocal(const std::string& id, int block) const;
    const TabEntry& getTab(int idx) const;
    TabEntry& getTab(int idx);
    int tabSize() const;

    // --- btab management ---
    int enterBlock();
    void exitBlock();
    int currentBlock() const;
    int currentLevel() const;
    void finalizeParameters();
    const BTabEntry& getBTab(int idx) const;
    BTabEntry& getBTab(int idx);
    int btabSize() const;

    // --- atab management ---
    int enterArray(int xtyp, int etyp, int eref, int low, int high, int elsz, int size);
    const ATabEntry& getATab(int idx) const;
    ATabEntry& getATab(int idx);
    int atabSize() const;

    // --- display ---
    int getDisplay(int level) const;

    // --- helpers ---
    void printTab() const;
    void printBTab() const;
    void printATab() const;

private:
    std::vector<TabEntry> tab;
    std::vector<BTabEntry> btab;
    std::vector<ATabEntry> atab;
    std::vector<int> display;
    int level = 0;

    void initPredefined();
};
