#ifndef __MEM_CACHE_TAGS_PROF_ADDR_TABLES_HH__
#define __MEM_CACHE_TAGS_PROF_ADDR_TABLES_HH__

#include <set>
#include <unordered_map>
#include <vector>

#include "base/misc.hh" //for panic() fatal() infrom()
#include "base/types.hh" //for type Addr
#include "sim/core.hh"

class AddressTable {

  private:
    std::set<Addr> entries;

  public:
    AddressTable() { entries.clear(); };

    void insert(Addr addr) { entries.insert(addr); };
    void erase(Addr addr) {
        if (!exists(addr))
            panic("There is no entry for this address. Please insert() beforehand.");
        entries.erase(addr);
    };

    bool exists(Addr addr) { return (entries.find(addr) != entries.end()); };
    uint64_t size() { return entries.size(); };
};

class AddressCountTable {

  private:
    std::unordered_map<Addr, uint64_t> ct;

  public:
    AddressCountTable(){
        clearAll();
    };

    void increment(Addr addr, uint64_t n = 1) {
        auto ite = ct.find(addr);
        if (ite == ct.end())
            panic("There is no entry for this address. Please insert() at first");
        ite->second += n;
    };

    void insert(Addr addr, uint64_t n = 0) {
        if (isExist(addr))
            panic("There already exists an entry for this address.");
        ct.insert(std::pair<Addr, uint64_t>(addr, n));
    };

    void remove(Addr addr) {
        auto ite = ct.find(addr);
        if (ite == ct.end())
            panic("There is no entry for this address.");
        ct.erase(ite);
    };

    bool isExist(const Addr addr) const {
        auto ite = ct.find(addr);
        if (ite == ct.end())
            return false;
        return true;
    };

    const uint64_t getCount(Addr addr) const {
        auto ite = ct.find(addr);
        if (ite == ct.end())
            panic("There is no entry for this address.");
        return ite->second;
    };

    std::vector<Addr> allAddress() {
        std::vector<Addr> addvec;
        addvec.clear();
        for(auto ite = ct.begin();
            ite != ct.end(); ++ite)
            {
                addvec.push_back(ite->first);
            }
        return addvec;
    };

    void clearAll() { ct.clear(); };

    uint64_t& operator[](const Addr addr) { // for write
        return ct[addr];
    }

    friend class AddressCountTableVisitor;
};

#endif
