#ifndef __MEM_CACHE_TAGS_PROF_ADDRMAPPER_HH__
#define __MEM_CACHE_TAGS_PROF_ADDRMAPPER_HH__

#include "mem/cache/tags/prof/stl_tables.hh"

class AddressMapper : public AddressCounterTable {

    AddressSet addr_set;
    int ordered_addr_max;

  public:

    AddressMapper() {
        clear();
        addr_set.clear();
        ordered_addr_max = 0;
    }

    void put_addr(Addr addr) {
        addr_set.insert(addr);
        if(exists(addr)) return;
        insert(addr, ordered_addr_max);
        ordered_addr_max++;
    }

    void order_by_firsttouch() {}

    void order_by_spacefill() {
        uint64_t oa = 0;
        for( auto ite = addr_set.begin(); ite != addr_set.end(); ite++ ){
            (*this)[*ite] = oa; oa++;
        }
    }

    void erase(Addr addr) {
        AddressCounterTable::erase(addr);
        addr_set.erase(addr);
    }

    void clear() { addr_set.clear(); AddressCounterTable::clear(); ordered_addr_max = 0; }
};

#endif
