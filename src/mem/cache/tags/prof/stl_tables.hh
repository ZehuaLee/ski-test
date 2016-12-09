#ifndef __MEM_CACHE_TAGS_PROF_STLTABLES_HH__
#define __MEM_CACHE_TAGS_PROF_STLTABLES_HH__

#include <set>
#include <unordered_map>

#include "base/misc.hh" //for panic() fatal() infrom()
#include "base/types.hh"

class AddressSet : public std::set<Addr> {

  public:
    AddressSet() { clear(); };

    void erase(Addr addr) {
        if (!exists(addr))
            panic("There is no entry for this address. Please insert() beforehand.");
        std::set<Addr>::erase(addr);
    };

    bool exists(Addr addr) { return (find(addr) != end()); };
};

class AddressCounterTable : public std::unordered_map<Addr, uint64_t> {

  public:
    AddressCounterTable() { clear(); };

    bool exists(const Addr addr) const {
        return find(addr) == end() ? false : true;
    };
    void insert(Addr addr, uint64_t n = 0) {
        if (exists(addr)) panic("There already exists an entry for this address.");
        std::unordered_map<Addr, uint64_t>::insert(std::pair<Addr, uint64_t>(addr, n));
    };
    void erase(Addr addr) {
        auto ite = find(addr);
        if (ite == end()) panic("There is no entry for this address.");
        std::unordered_map<Addr, uint64_t>::erase(ite);
    };
    const uint64_t getCount(Addr addr) const {
        auto ite = find(addr);
        if (ite == end()) panic("There is no entry for this address.");
        return ite->second;
    };
};

#endif
