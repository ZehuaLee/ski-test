#ifndef __MEM_CACHE_TAGS_SAMPLEDSETIDENTIFIER_HH__
#define __MEM_CACHE_TAGS_SAMPLEDSETIDENTIFIER_HH__

#include "base/types.hh"

class SampledSetIdentifier {

    unsigned int nsets;
    unsigned int numSetsInConstituency;
    unsigned int bsize;

  public:
    SampledSetIdentifier(unsigned int _nsets, unsigned int _bsize) {
        nsets = _nsets;
        bsize = _bsize;
        numSetsInConstituency = 32;
    }

    unsigned int getSet(Addr addr) { return (addr / bsize) % nsets; }
    unsigned int getConstituencyID(unsigned int set) { return set / numSetsInConstituency; }
    unsigned int getConstituencyOffset(unsigned int set) { return set % numSetsInConstituency; }

    // exmpale of tid ... 0:LRU 1:BIP or 0:1st-thread 1:2nd-thread
    bool isSampledSet(unsigned int set, unsigned int tid = 0) {
        unsigned int id = getConstituencyID(set);
        unsigned int offset = getConstituencyOffset(set);
        assert(offset + tid < numSetsInConstituency);

        if (id == offset + tid) return true;
        return false;
    }

    bool isSampledSetInverted(unsigned int set, unsigned int tid = 0) {
        unsigned int id = getConstituencyID(set);
        unsigned int offset = getConstituencyOffset(set);
        unsigned int invert_offset = numSetsInConstituency - 1 - offset;
        assert(invert_offset - tid < numSetsInConstituency);

        if (id == invert_offset - tid) return true;
        return false;
    }

    bool isSampledAddr(Addr addr, unsigned int tid = 0) { return isSampledSet( getSet(addr), tid ); }
    bool isSampledAddrInverted(Addr addr, unsigned int tid = 0) { return isSampledSetInverted( getSet(addr), tid ); }
};

#endif
