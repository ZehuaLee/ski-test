// FLIP tags for inspection, made by inheriting FLIP

#include <list>
#include <string>

#include "mem/cache/tags/prof/base.hh"
#include "mem/cache/tags/prof/insertion.hh"
#include "mem/cache/tags/prof/reusability.hh"
#include "mem/cache/tags/prof/addr_tracer.hh"
#include "mem/cache/tags/flip_prof.hh"
#include "mem/cache/base.hh"
#include "sim/core.hh"

using namespace std;

FLIPProfiler::FLIPProfiler(const Params *p)
    :FLIP(p)
{
    inform("%s FLIPProfiler for inspecting all accesses", name());
    se = new SamplingEvent(this);
    interval = clockEdge(p->interval);

    profilers = new ProfilerComposite(name());
    profilers->add(new InsertionProfiler(name()));
    profilers->add(new ReusabilityProfiler(name()));
    profilers->add(new AddressTracer(name(), true, numSets, getBlockSize()));
}

void
FLIPProfiler::startup()
{
    schedule(se, curTick()+interval);
}

void
FLIPProfiler::regStats()
{
    FLIP::regStats();
    profilers->regStats();
}

BaseSetAssoc::BlkType*
FLIPProfiler::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id, bool is_read)
{
    BlkType* blk;
    blk = FLIP::accessBlock(addr, is_secure, lat, master_id, is_read);

    if ( should_be_profiled(blk) ) {
        profilers->access(regenerateBlkAddr(blk->tag, blk->set));
    }

    return blk;
}

BaseSetAssoc::BlkType*
FLIPProfiler::findVictim(Addr addr)
{
    BlkType* blk;
    blk = FLIP::findVictim(addr);

    if ( should_be_profiled(blk) ) {
        profilers->victim(regenerateBlkAddr(blk->tag, blk->set));
    }

    return blk;
}

void
FLIPProfiler::insertBlock(PacketPtr pkt, BlkType *blk)
{
    FLIP::insertBlock(pkt, blk);
    assert(blk);
    profilers->insert(regenerateBlkAddr(blk->tag, blk->set));
}

void
FLIPProfiler::invalidate(BlkType *blk)
{
    // regenerateBlkAddr() should be called before invalidate()
    assert(blk);
    profilers->victim(regenerateBlkAddr(blk->tag, blk->set));
    FLIP::invalidate(blk);
}

bool
FLIPProfiler::should_be_profiled(BlkType *blk) const
{
    if ( blk == NULL )
        return false;
    if ( !blk->isValid() )
        return false;
    return true;
}

FLIPProfiler*
FLIPProfilerParams::create()
{
    return new FLIPProfiler(this);
}
