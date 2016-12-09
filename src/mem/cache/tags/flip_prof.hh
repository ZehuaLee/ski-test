#ifndef __MEM_CACHE_TAGS_FLIP_PROF_HH__
#define __MEM_CACHE_TAGS_FLIP_PROF_HH__

#include <cassert>
#include <cstring>
#include <list>

#include "mem/cache/tags/prof/base.hh"
#include "mem/cache/tags/base.hh"
#include "mem/cache/tags/flip.hh"
#include "params/FLIPProfiler.hh"

class BaseCache;

class FLIPProfiler : public FLIP
{
  private:
    BaseTagsProfiler* profilers;
    Tick interval;

    class SamplingEvent : public Event
    {
      private:
        FLIPProfiler* tags;

      public:
        SamplingEvent(FLIPProfiler* _tags) {
            tags = _tags;
        };
        void process() { tags->sampling(); };
    };
    SamplingEvent* se;
  public:
    typedef FLIPProfilerParams Params;

    FLIPProfiler(const Params *p);

    virtual void startup();
    virtual void regStats();

  virtual BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat, int context_src, bool is_read=true);
    virtual BlkType* findVictim(Addr addr);
    virtual void insertBlock(PacketPtr pkt, BlkType *blk);
    virtual void invalidate(BlkType *blk);

    bool should_be_profiled(BlkType *blk) const;

    void sampling() { profilers->sampling(); schedule(se, curTick()+interval); };
};

#endif // __MEM_CACHE_TAGS_FLIP_PROF_HH__
