#ifndef __MEM_CACHE_TAGS_PROF_ADDRESS_TRACER_HH__
#define __MEM_CACHE_TAGS_PROF_ADDRESS_TRACER_HH__

#include <iostream>

#include "mem/cache/tags/prof/addr_mapper.hh"
#include "mem/cache/tags/prof/base.hh"
#include "mem/cache/tags/prof/reuse_dist.hh"
#include "mem/cache/tags/set_sample.hh"

class DistanceAddressStream : public std::list<std::pair<uint64_t, Addr> >
{
    typedef std::list<std::pair<uint64_t, Addr>> BaseType;
    typedef std::pair<uint64_t, Addr> ElementType;

  public:

    void push_back(uint64_t dist, Addr addr) {
        BaseType::push_back(ElementType(dist, addr));
    }
};

class AddressTracer : public BaseTagsProfiler {

    SampledSetIdentifier ssid;

    AddressMapper addr_map;

    AddressCounterTable reusehist;

    ReuseDistanceAnalyzer rdanalyze;

    std::ostream* insert_ostream;
    std::ostream* access_ostream;
    std::ostream* reused_ostream;

    DistanceAddressStream insert_stream;
    DistanceAddressStream access_stream;

    Tick usage_distance;

    Addr bsize;
    bool set_sample;
    std::string order_by;

  public:
    AddressTracer( std::string _name,
                   bool _set_sample = true,
                   unsigned int _nsets = 128,
                   Addr _bsize = 64,
                   std::string _order_by = "spacefill" );

    std::string name() { return BaseTagsProfiler::name() + ".trace"; };

    virtual void regStats() {};

    virtual void insert(Addr addr);
    virtual void access(Addr addr);
    virtual void victim(Addr addr);

    virtual void sampling() {};

    virtual void dump();
    virtual void reset();

    bool is_samplesets(Addr addr);
    void open_stream();
    void close_stream();

    void finalize() { dump(); };
};

#endif //__MEM_CACHE_TAGS_PROF_ADDRESS_TRACER_HH__
