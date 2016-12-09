// LRU tags for inspection, made by inheriting LRU

#include <list>
#include <string>

#include "mem/cache/tags/prof/base.hh"
#include "mem/cache/tags/prof/insertion.hh"
#include "mem/cache/tags/prof/reusability.hh"
#include "mem/cache/tags/prof/addr_tracer.hh"
#include "mem/cache/tags/lru_prof.hh"
#include "mem/cache/base.hh"
#include "sim/core.hh"

using namespace std;

LRUProfiler::LRUProfiler(const Params *p)
    :LRU(p)
{
    inform("%s LRUProfiler for inspecting all accesses", name());
    se = new SamplingEvent(this);
    interval = clockEdge(p->interval);

    profilers = new ProfilerComposite(name());
    profilers->add(new InsertionProfiler(name()));
    profilers->add(new ReusabilityProfiler(name()));
    profilers->add(new AddressTracer(name(), true, numSets, getBlockSize()));
}

void
LRUProfiler::startup()
{
    schedule(se, curTick()+interval);
}

void
LRUProfiler::regStats()
{
    LRU::regStats();
    profilers->regStats();
}

BaseSetAssoc::BlkType*
LRUProfiler::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id, bool is_read)
{
    BlkType* blk;
    blk = LRU::accessBlock(addr, is_secure, lat, master_id, is_read);

    if ( should_be_profiled(blk) ) {
        profilers->access(regenerateBlkAddr(blk->tag, blk->set));
    }

    return blk;
}

BaseSetAssoc::BlkType*
LRUProfiler::findVictim(Addr addr)
{
    BlkType* blk;
    blk = LRU::findVictim(addr);

    if ( should_be_profiled(blk) ) {
        profilers->victim(regenerateBlkAddr(blk->tag, blk->set));
    }

    return blk;
}

void
LRUProfiler::insertBlock(PacketPtr pkt, BlkType *blk)
{
    LRU::insertBlock(pkt, blk);
    assert(blk);
    profilers->insert(regenerateBlkAddr(blk->tag, blk->set));
}

void
LRUProfiler::invalidate(BlkType *blk)
{
    // regenerateBlkAddr() should be called before invalidate()
    assert(blk);
    profilers->victim(regenerateBlkAddr(blk->tag, blk->set));
    LRU::invalidate(blk);
}

bool
LRUProfiler::should_be_profiled(BlkType *blk) const
{
    if ( blk == NULL )
        return false;
    if ( !blk->isValid() )
        return false;
    return true;
}

LRUProfiler*
LRUProfilerParams::create()
{
    return new LRUProfiler(this);
}
