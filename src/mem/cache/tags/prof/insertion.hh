#ifndef __MEM_CACHE_TAGS_PROF_INSERTION_HH__
#define __MEM_CACHE_TAGS_PROF_INSERTION_HH__

#include <fstream>
#include <set>
#include <string>

#include <gzstream.hh>

#include "base/statistics.hh"
#include "base/types.hh" // for type Addr
#include "mem/cache/tags/prof/base.hh"
#include "sim/core.hh"
#include "mem/cache/tags/prof/addr_tables.hh"

class InsertionProfiler : public BaseTagsProfiler {

  private:
    AddressTable ins_table;

    Stats::Scalar insertionDeadVerified;
    Stats::Scalar insertionDeadLeft;
    Stats::Scalar insertionReusable;
    Stats::Formula insertionDead;

  public:
    InsertionProfiler(std::string name);

    std::string name() { return BaseTagsProfiler::name() + ".insert"; };

    virtual void regStats();

    virtual void insert(Addr addr);
    virtual void access(Addr addr);
    virtual void victim(Addr addr);

    virtual void sampling() {};

    void finalize();
};

#endif // __MEM_CACHE_TAGS_PROF_INSERTION_HH__
