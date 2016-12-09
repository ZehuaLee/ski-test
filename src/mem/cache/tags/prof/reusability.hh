#ifndef __MEM_CACHE_TAGS_PROF_BLOCKSAMPLER_HH__
#define __MEM_CACHE_TAGS_PROF_BLOCKSAMPLER_HH__

#include <unordered_map>
#include <vector>

#include "base/callback.hh"
#include "base/statistics.hh"

#include "mem/cache/tags/prof/base.hh"

template<typename T> class OverflowVector{
  private:
    unsigned underflow_minid;
    T underflow_value;
    unsigned  overflow_maxid;
    T overflow_value;
    std::vector<T> vec;

  public:

    OverflowVector () { reset(); };

    T& operator[](size_t i) {
        if ( i < 0 ) {
            if ( i < underflow_minid ) underflow_minid = i;
            return underflow_value;
        }
        if ( i < vec.size() ) {
            return vec[i];
        }
        if ( i > overflow_maxid ) overflow_maxid = i;
        return overflow_value;
    };

    void resize(size_t size, T val = 0) { vec.resize(size, val); };

    size_t get_maxid () { return overflow_maxid; };
    size_t get_minid () { return underflow_minid; };

    T get_overflow_value () { return overflow_value; };
    T get_underflow_value () { return underflow_value; };

    void reset() {
        underflow_value = 0;
        overflow_value = 0;
        underflow_minid = 0;
        overflow_maxid = 0;
        for(size_t i = 0; i < vec.size(); i++ ){ vec[i] = 0; }
    };
};

class ReusabilityProfiler : public BaseTagsProfiler {
  private:

    class BlockSample {
      public:
        Addr addr;          //address
        Tick insert_tick;   // tick when this block is inserted
        Tick previous_tick; // previously sampled tick
        unsigned reuse_count;   // reused count
        ReusabilityProfiler *bs;

        BlockSample(Addr _addr, ReusabilityProfiler *_bs){
            addr = _addr; insert_tick = curTick(); previous_tick = curTick(); reuse_count = 0;
            bs = _bs;
        };

        void sample_as_reusable() {
            Tick sampled_tick = curTick() - previous_tick;
            bs->distReusableTick[reuse_count] += sampled_tick;
            bs->distAllTick[reuse_count] += sampled_tick;
            reuse_count++;
            previous_tick = curTick();
        };

        void sample_as_dead() {
            Tick sampled_tick = curTick() - previous_tick;
            bs->distDeadTick[reuse_count] += sampled_tick;
            bs->distAllTick[reuse_count] += sampled_tick;
            previous_tick = curTick();
        };

        void reset() {
            previous_tick = curTick();
        };

        void finalize() {
            sample_as_dead();
        };
    };

    Tick previous_reset_tick;
    Tick previous_finalize_tick;

    std::unordered_map<Addr, BlockSample*> blocks;
    std::unordered_map<Addr, BlockSample*>::iterator it;

    OverflowVector<Tick> distReusableTick;
    OverflowVector<Tick> distDeadTick;
    OverflowVector<Tick> distAllTick;

    Stats::Distribution distReusableBlocks;
    Stats::Distribution distDeadBlocks;
    Stats::Distribution distAllBlocks;

    unsigned dist_size;

  public:
    ReusabilityProfiler(std::string name);

    std::string name() { return BaseTagsProfiler::name() + ".reuse"; };

    virtual void access(Addr addr);
    virtual void victim(Addr addr);
    virtual void insert(Addr addr);

    virtual void sampling() {};
    virtual void regStats();

    virtual void reset();
    virtual void dump();

    void finalize();
    void tick_to_block(OverflowVector<Tick>* tickDist, Stats::Distribution* blockDist);

};

#endif //__MEM_CACHE_TAGS_PROF_BLOCKSAMPLER_HH__
