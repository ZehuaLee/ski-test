/**
 * @file
 * Definitions of a DIP tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/dip.hh"
#include "mem/cache/base.hh"

DIP::DIP(const Params *p)
    : BIP(p), psel(name(), p->psel_bits)
{
    inform("%s: DIP automatically selects BIP or LRU", name());
    numDedicatedSets = p->ndsets;
    numAllDedicatedSets = p->ndsets * 2;
    numSetsInConstituency = numSets / numAllDedicatedSets;

    checkConstituency();
}

void
DIP::checkConstituency()
{
    int lru, bip;
    lru = 0;
    bip = 0;

    for(int i = 0; i < numSets; i++) {
        //inform("set: %d, constituency: %d, offset: %d", i, getConstituencyID(i), getConstituencyOffset(i));
        if (isDedicatedLRU(i) == true && isDedicatedBIP(i) == true) {
            fatal("[DIP] dedicated sets collide.");
        }
        if (isDedicatedLRU(i)) { lru++; inform("[DIP] set %d is LRU-dedicated", i); }
        if (isDedicatedBIP(i)) { bip++; inform("[DIP] set %d is BIP-dedicated", i); }
    }

    inform("[DIP] %d LRU-dedicated sets, and %d BIP-dedicated sets.", lru, bip);

    if ( lru != bip )
        fatal("[DIP] the number of dedicated sets per policy is not same between two policies.");

    if ( lru + bip != numAllDedicatedSets )
        fatal("[DIP] the number of all dedicated sets does not match.");
}

void
DIP::regStats(){

    BIP::regStats();

    num_dedicated_sets
        .name(name() + ".ndsets")
        .desc("DIP: the number of dedicated sets per policy")
        ;
    num_dedicated_sets = numDedicatedSets;

    num_all_dedicated_sets
        .name(name() + ".ndsets_all")
        .desc("DIP: the number of all dedicated sets")
        ;
    num_all_dedicated_sets = numAllDedicatedSets;

    psel.regStats(name());

}

unsigned int
DIP::getConstituencyID(int set)
{
    return set / numSetsInConstituency;
}

unsigned int
DIP::getConstituencyOffset(int set)
{
    return set % numSetsInConstituency;
}

bool
DIP::isDedicatedLRU(int set)
{
    unsigned int id = getConstituencyID(set);
    unsigned int offset = getConstituencyOffset(set);

    if (id % 2 == 0)
        // not constituency for LRU
        return false;

    if (id % numSetsInConstituency == offset)
        return true;
    return false;
}

bool
DIP::isDedicatedBIP(int set)
{
    unsigned int id = getConstituencyID(set);
    unsigned int offset = getConstituencyOffset(set);

    if (id % 2 == 1)
        // not constituency for BIP
        return false;

    if (id % numSetsInConstituency == (((unsigned int)~(offset)) % numSetsInConstituency))
        return true;
    return false;
}

void
DIP::setSampling(BlkType *blk, int set)
{
    assert(blk == NULL);
    // miss
    if (isDedicatedLRU(set))
        psel.increment();
    if (isDedicatedBIP(set))
        psel.decrement();
}

BaseSetAssoc::BlkType*
DIP::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id, bool is_read)
{
  BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id, is_read);

    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    } else {
        setSampling(blk, extractSet(addr));
    }

    return blk;
}

void
DIP::insertBlock_bip(PacketPtr pkt, BlkType *blk)
{
    int set = extractSet(pkt->getAddr());

    if (epsilon_counter < epsilon_numerator) {
        sets[set].moveToHead(blk);
        mru_insertions++;
    }

    epsilon_counter++;
    if ( epsilon_counter >= epsilon_denominator )
        epsilon_counter = 0;
}

void
DIP::insertBlock_lru(PacketPtr pkt, BlkType *blk)
{
    int set = extractSet(pkt->getAddr());
    sets[set].moveToHead(blk);
    mru_insertions++;
}

void
DIP::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);
    all_insertions++;

    int set = extractSet(pkt->getAddr());

    if (isDedicatedLRU(set)) {
        insertBlock_lru(pkt, blk);
        return;
    }
    if (isDedicatedBIP(set)) {
        insertBlock_bip(pkt, blk);
        return;
    }

    // follower sets
    if (psel.isOver()) {
        insertBlock_bip(pkt, blk);
        return;
    }
    insertBlock_lru(pkt, blk);
    return;
}

DIP*
DIPParams::create()
{
    return new DIP(this);
}
