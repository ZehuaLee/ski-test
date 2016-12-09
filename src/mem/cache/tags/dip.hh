/**
 * @file
 * Declaration of a BIP tag store.
 */

#ifndef __MEM_CACHE_TAGS_DIP_HH__
#define __MEM_CACHE_TAGS_DIP_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "mem/cache/tags/bip.hh"
#include "mem/cache/tags/psel.hh"
#include "params/DIP.hh"


class DIP : public BIP
{
  private:

    PolicySelector psel;
    unsigned int numDedicatedSets;
    unsigned int numAllDedicatedSets;
    unsigned int numSetsInConstituency;

  public:
    /** Convenience typedef. */
    typedef DIPParams Params;

    // Stats::Formula epsilon_given;
    // Stats::Formula epsilon_real;

    Stats::Formula num_dedicated_sets;
    Stats::Formula num_all_dedicated_sets;

    /**
     * Construct and initialize this tag store.
     */
    DIP(const Params *p);

    /**
     * Destructor
     */
    ~DIP() {}

    virtual void checkConstituency();
    virtual unsigned int getConstituencyID(int set);
    virtual unsigned int getConstituencyOffset(int set);
    virtual bool isDedicatedLRU(int set);
    virtual bool isDedicatedBIP(int set);
    virtual void setSampling(BlkType *blk, int set);

    virtual BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                                 int context_src, bool is_read=true);
    virtual void insertBlock(PacketPtr pkt, BlkType *blk);
    virtual void insertBlock_bip(PacketPtr pkt, BlkType *blk);
    virtual void insertBlock_lru(PacketPtr pkt, BlkType *blk);

    virtual void regStats();
};

#endif // __MEM_CACHE_TAGS_LRU_HH__
