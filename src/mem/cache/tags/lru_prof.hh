#ifndef __MEM_CACHE_TAGS_LRU_PROF_HH__
#define __MEM_CACHE_TAGS_LRU_PROF_HH__

#include <cassert>
#include <cstring>
#include <list>

#include "mem/cache/tags/prof/base.hh"
#include "mem/cache/tags/base.hh"
#include "mem/cache/tags/lru.hh"
#include "params/LRUProfiler.hh"

class BaseCache;


class LRUProfiler : public LRU
{
  private:
    BaseTagsProfiler* profilers;
    Tick interval;

    class SamplingEvent : public Event
    {
      private:
        LRUProfiler* tags;

      public:
        SamplingEvent(LRUProfiler* _tags) {
            tags = _tags;
        };
        void process() { tags->sampling(); };
    };
    SamplingEvent* se;
  public:
    typedef LRUProfilerParams Params;

    LRUProfiler(const Params *p);

    virtual void startup();
    virtual void regStats();

  virtual BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat, int context_src, bool is_read=true);
    virtual CacheBlk* findVictim(Addr addr);
    virtual void insertBlock(PacketPtr pkt, BlkType *blk);
    virtual void invalidate(BlkType *blk);

    bool should_be_profiled(BlkType *blk) const;

    void sampling() { profilers->sampling(); schedule(se, curTick()+interval); };
};

#endif // __MEM_CACHE_TAGS_LRU_PROF_HH__
