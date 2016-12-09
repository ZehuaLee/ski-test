/**
 * @file
 * Definitions of a BIP tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/bip.hh"
#include "mem/cache/base.hh"

BIP::BIP(const Params *p)
    : LRU(p)
{
    epsilon_denominator = p->epsilon_denominator;
    epsilon_numerator = p->epsilon_numerator;
    epsilon_counter = 0;

    inform("%s: BIP Epsilon %d/%d", name(), epsilon_numerator, epsilon_denominator);
}

void
BIP::regStats(){

    LRU::regStats();

    mru_insertions
        .name(name() + ".mru_insert")
        .desc("BIP: the number of MRU insertions")
        ;

    all_insertions
        .name(name() + ".all_insert")
        .desc("BIP: the number of all insertions")
        ;

    epsilon_given
        .name(name() + ".epsilon_given")
        .desc("BIP: epsilon given as a simulation parameter")
        ;
    epsilon_given = (double)epsilon_numerator / epsilon_denominator;

    epsilon_real
        .name(name() + ".epsilon_real")
        .desc("BIP: epsilon observed by insertions")
        ;
    epsilon_real = mru_insertions / all_insertions;

}

void
BIP::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);
    all_insertions++;

    int set = extractSet(pkt->getAddr());

    if (epsilon_counter < epsilon_numerator) {
        sets[set].moveToHead(blk);
        mru_insertions++;
    }

    epsilon_counter++;
    if ( epsilon_counter >= epsilon_denominator )
        epsilon_counter = 0;
}

BIP*
BIPParams::create()
{
    return new BIP(this);
}
