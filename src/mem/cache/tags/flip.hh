/**
 * @file
 * Declaration of a FLIP (Flexible Insertion and Promotion) tag store.
 */

#ifndef __MEM_CACHE_TAGS_FLIP_HH__
#define __MEM_CACHE_TAGS_FLIP_HH__

#include "mem/cache/tags/lru.hh"
#include "mem/cache/tags/sat_counter.hh"
#include "mem/cache/tags/sdp_counter.hh"
#include "params/FLIP.hh"

class FLIP : public LRU
{
    StackDistanceCounter sdp;
    StackDistanceCounterArray sdparr;

    // position pointer
    std::vector<ConservativeSaturatingCounter> pos;
    // first reuse counter
    std::vector<uint64_t> frc;

    unsigned int blk_max_count;
    unsigned int detect_th;

    Cycles update_interval;

    class UpdatePriorityEvent : public Event {
      private:
        FLIP *flip;
      public:
        UpdatePriorityEvent(FLIP *f);

        void process();
        const char* description() const;
    };

    UpdatePriorityEvent updateEvent;

    void scheduleUpdatePriorityEvent(Cycles delay)
    {
        if (updateEvent.squashed())
            reschedule(updateEvent, clockEdge(delay));
        if (!updateEvent.scheduled())
            schedule(updateEvent, clockEdge(delay));
    }

  public:
    /** Convenience typedef. */
    typedef FLIPParams Params;

    /**
     * Construct and initialize this tag store.
     */
    FLIP(const Params *p);

    /**
     * Destructor
     */
    ~FLIP() {}

    virtual void startup();

    virtual BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                                 int context_src, bool is_read=true);
    virtual BlkType* findVictim(Addr addr);
    virtual void insertBlock(PacketPtr pkt, BlkType *blk);
    virtual void invalidate(BlkType *blk);

    virtual void updatePriorityPointer();
    void samplePriorityPointer();

    virtual void regStats();

    std::vector<Stats::Distribution*> posDistDuration;
    std::vector<Stats::Distribution*> posDistBlock;
};

#endif // __MEM_CACHE_TAGS_FLIP_HH__
