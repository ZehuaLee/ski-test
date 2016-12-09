#ifndef __MEM_CACHE_TAGS_PROF_REUSEDIST_HH__
#define __MEM_CACHE_TAGS_PROF_REUSEDIST_HH__

#include <algorithm>
#include <list>
#include <unordered_map>

#include "mem/cache/tags/prof/stl_tables.hh"

class LRUCacheSetTracer : public std::list<Addr> {
  public:
    LRUCacheSetTracer () {}
    void insert(Addr addr) { push_front(addr); }
    bool exists(Addr addr) { return !(std::find(begin(), end(), addr) == end()); }
    void victim(Addr addr) { remove(addr); }
    void access(Addr addr) { victim(addr); insert(addr); }
    unsigned int lrustate(Addr addr) { return distance(begin(), std::find(begin(), end(), addr)); }
};

typedef AddressCounterTable DistanceDistribution;

class ReuseDistanceAnalyzer : public std::map<Addr, DistanceDistribution>
{
    unsigned int nsets;
    unsigned int bsize;
    unsigned int max_distance;
    AddressMapper addr_map;
    bool only_cache_range;
    std::unordered_map<unsigned int, LRUCacheSetTracer> sets;

  public:
    ReuseDistanceAnalyzer(unsigned int _nsets, unsigned int _bsize){
        nsets = _nsets; bsize = _bsize; max_distance = 0;
    }
    bool exists(const Addr addr) const {
        if ( find(addr) == end() ) return false; return true;
    };
    unsigned int setid(Addr addr) { return (addr / bsize) % nsets; }
    LRUCacheSetTracer& set(Addr addr) {
        if( sets.find(setid(addr)) == sets.end() )
            sets[setid(addr)] = LRUCacheSetTracer();
        return sets[setid(addr)];
    }
    void insert(Addr addr) {
        if ( !set(addr).exists(addr) )
            set(addr).insert(addr);
        else
            access(addr);
    }
    void victim(Addr addr) {
        //set(addr).victim(addr);
    }
    void access(Addr addr) {
        unsigned int reuse_dist = set(addr).lrustate(addr);
        max_distance = std::max(max_distance, reuse_dist);
        if( !exists(addr) )
            (*this)[addr] = DistanceDistribution();
        if( !(*this)[addr].exists(reuse_dist) )
            (*this)[addr][reuse_dist] = 0;
        (*this)[addr][reuse_dist]++;
        addr_map.put_addr(addr);
        set(addr).access(addr);
    }
    void output(std::ostream* stream) {
        addr_map.order_by_spacefill();
        ReuseDistanceAnalyzer::iterator it;
        DistanceDistribution::iterator dit;
        for( it = begin(); it != end(); it++) {
            for( dit = (it->second).begin() ; dit != (it->second).end() ; dit++ )
                *stream << dit->first << " "
                        << addr_map[it->first] << " "
                        << dit->second << std::endl;
            *stream << std::endl;
        }
    }
    void reset() { clear(); addr_map.clear(); }
};

#endif
