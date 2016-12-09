/**
 * @file
 * Declaration of a BIP tag store.
 */

#ifndef __MEM_CACHE_TAGS_BIP_HH__
#define __MEM_CACHE_TAGS_BIP_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "mem/cache/tags/lru.hh"
#include "params/BIP.hh"

class BIP : public LRU
{
  public:
    /** Convenience typedef. */
    typedef BIPParams Params;

    /** epsilon parameter for bimodality **/
    unsigned int epsilon_denominator;
    unsigned int epsilon_numerator;

    /** counter for checking match between # insertions and epsilon **/
    unsigned int epsilon_counter;

    /** BIP-related statistics **/
    Stats::Scalar mru_insertions;
    Stats::Scalar all_insertions;

    Stats::Formula epsilon_given;
    Stats::Formula epsilon_real;

    /**
     * Construct and initialize this tag store.
     */
    BIP(const Params *p);

    /**
     * Destructor
     */
    ~BIP() {}

    virtual void insertBlock(PacketPtr pkt, BlkType *blk);

    virtual void regStats();
};

#endif // __MEM_CACHE_TAGS_LRU_HH__
