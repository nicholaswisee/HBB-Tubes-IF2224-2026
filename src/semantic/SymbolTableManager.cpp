#include "semantic/SymbolTableManager.hpp"
#include "semantic/TypeSystem.hpp"

using namespace TypeSystem;

SymbolTableManager::SymbolTableManager() {
    tab.push_back(TabEntry{"", 0, 0, 0, 0, 1, 0, 0});
    btab.push_back(BTabEntry{0, 0, 0, 0});
    display.push_back(0);
    level = 0;
    initPredefined();
}

void SymbolTableManager::initPredefined() {
    enter("integer", OBJ_TYPE, TYPE_INTEGER);
    enter("real", OBJ_TYPE, TYPE_REAL);
    enter("char", OBJ_TYPE, TYPE_CHAR);
    enter("boolean", OBJ_TYPE, TYPE_BOOLEAN);
    enter("string", OBJ_TYPE, TYPE_STRING);

    enter("true", OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 1);
    enter("false", OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 0);

    enter("writeln", OBJ_PROCEDURE, TYPE_UNKNOWN, 0, 1, 0, 0);
    enter("readln", OBJ_PROCEDURE, TYPE_UNKNOWN, 0, 1, 0, 0);
    enter("write", OBJ_PROCEDURE, TYPE_UNKNOWN, 0, 1, 0, 0);
    enter("read", OBJ_PROCEDURE, TYPE_UNKNOWN, 0, 1, 0, 0);
}

int SymbolTableManager::enter(const std::string& id, int obj, int type, int ref, int nrm, int lev, int adr) {
    int actualLev = (lev < 0) ? level : lev;
    int block = display[actualLev];

    int existing = lookupLocal(id, block);
    if (existing != -1) {
        return -1; 
    }

    TabEntry entry;
    entry.id = id;
    entry.link = btab[block].last;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.nrm = nrm;
    entry.lev = actualLev;
    entry.adr = adr;

    int idx = static_cast<int>(tab.size());
    tab.push_back(entry);
    btab[block].last = idx;

    if (obj == TypeSystem::OBJ_VARIABLE && actualLev > 0) {
        if (nrm == 0) {
            btab[block].psze += 1;
        } else {
            btab[block].vsze += 1;
        }
    }

    return idx;
}

static bool ciEqual(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(a[i]) != std::tolower(b[i])) return false;
    }
    return true;
}

int SymbolTableManager::lookup(const std::string& id) const {
    for (int l = level; l >= 0; --l) {
        int block = display[l];
        int idx = btab[block].last;
        while (idx > 0) {
            if (ciEqual(tab[idx].id, id)) {
                return idx;
            }
            idx = tab[idx].link;
        }
    }
    return -1;
}

int SymbolTableManager::lookupLocal(const std::string& id, int block) const {
    int idx = btab[block].last;
    while (idx > 0) {
        if (ciEqual(tab[idx].id, id)) {
            return idx;
        }
        idx = tab[idx].link;
    }
    return -1;
}

const TabEntry& SymbolTableManager::getTab(int idx) const {
    return tab[idx];
}

TabEntry& SymbolTableManager::getTab(int idx) {
    return tab[idx];
}

int SymbolTableManager::tabSize() const {
    return static_cast<int>(tab.size());
}

int SymbolTableManager::enterBlock() {
    level++;
    int idx = static_cast<int>(btab.size());
    btab.push_back(BTabEntry{0, 0, 0, 0});
    if (static_cast<int>(display.size()) <= level) {
        display.push_back(idx);
    } else {
        display[level] = idx;
    }
    return idx;
}

void SymbolTableManager::exitBlock() {
    if (level > 0) {
        level--;
    }
}

void SymbolTableManager::finalizeParameters() {
    int block = display[level];
    btab[block].lpar = btab[block].last;
}

int SymbolTableManager::currentBlock() const {
    return display[level];
}

int SymbolTableManager::currentLevel() const {
    return level;
}

const BTabEntry& SymbolTableManager::getBTab(int idx) const {
    return btab[idx];
}

BTabEntry& SymbolTableManager::getBTab(int idx) {
    return btab[idx];
}

int SymbolTableManager::btabSize() const {
    return static_cast<int>(btab.size());
}

int SymbolTableManager::enterArray(int xtyp, int etyp, int eref, int low, int high, int elsz, int size) {
    ATabEntry entry;
    entry.xtyp = xtyp;
    entry.etyp = etyp;
    entry.eref = eref;
    entry.low = low;
    entry.high = high;
    entry.elsz = elsz;
    entry.size = size;
    int idx = static_cast<int>(atab.size());
    atab.push_back(entry);
    return idx;
}

const ATabEntry& SymbolTableManager::getATab(int idx) const {
    return atab[idx];
}

ATabEntry& SymbolTableManager::getATab(int idx) {
    return atab[idx];
}

int SymbolTableManager::atabSize() const {
    return static_cast<int>(atab.size());
}

int SymbolTableManager::getDisplay(int l) const {
    return display[l];
}

void SymbolTableManager::printTab() const {
    std::cout << "\n=== tab ===\n";
    std::cout << "idx  id        obj  type  ref  nrm  lev  adr  link\n";
    std::cout << "---------------------------------------------------\n";
    for (size_t i = 0; i < tab.size(); ++i) {
        const auto& e = tab[i];
        std::cout << i << "    "
                  << e.id << "  "
                  << e.obj << "    "
                  << e.type << "    "
                  << e.ref << "    "
                  << e.nrm << "    "
                  << e.lev << "    "
                  << e.adr << "    "
                  << e.link << "\n";
    }
}

void SymbolTableManager::printBTab() const {
    std::cout << "\n=== btab ===\n";
    std::cout << "idx  last  lpar  psze  vsze\n";
    std::cout << "-----------------------------\n";
    for (size_t i = 0; i < btab.size(); ++i) {
        const auto& e = btab[i];
        std::cout << i << "    "
                  << e.last << "    "
                  << e.lpar << "    "
                  << e.psze << "    "
                  << e.vsze << "\n";
    }
}

void SymbolTableManager::printATab() const {
    std::cout << "\n=== atab ===\n";
    std::cout << "idx  xtyp  etyp  eref  low  high  elsz  size\n";
    std::cout << "-----------------------------------------------\n";
    for (size_t i = 0; i < atab.size(); ++i) {
        const auto& e = atab[i];
        std::cout << i << "    "
                  << e.xtyp << "    "
                  << e.etyp << "    "
                  << e.eref << "    "
                  << e.low << "    "
                  << e.high << "    "
                  << e.elsz << "    "
                  << e.size << "\n";
    }
}
