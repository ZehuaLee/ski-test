#include "sim/core.hh"
#include "mem/cache/tags/prof/reusability.hh"

ReusabilityProfiler::ReusabilityProfiler(std::string _name)
    : BaseTagsProfiler(_name)
{
    inform("%s ReusabilityProfiler inspecting block reusability", name());

    dist_size = 64;
    previous_reset_tick = 0;
    previous_finalize_tick = 0;

    distReusableTick.resize(dist_size, 0);
    distDeadTick.resize(dist_size, 0);
    distAllTick.resize(dist_size, 0);
}

void
ReusabilityProfiler::regStats()
{
    using namespace Stats;

    distReusableBlocks
        .init(0, dist_size, 1)
        .name(name()+".blk_reusable")
        .desc("reuse to reusable block distribution")
        .flags(total);
    distDeadBlocks
        .init(0, dist_size, 1)
        .name(name()+".blk_dead")
        .desc("reuse to dead block distribution")
        .flags(total);
    distAllBlocks
        .init(0, dist_size, 1)
        .name(name()+".blk_all")
        .desc("reuse to all block distribution")
        .flags(total);
}

void
ReusabilityProfiler::access(Addr addr)
{
    assert(blocks.count(addr));
    blocks[addr]->sample_as_reusable();
}

void
ReusabilityProfiler::victim(Addr addr)
{
    assert(blocks.count(addr));
    blocks[addr]->sample_as_dead();
    delete blocks[addr];
    blocks.erase(addr);
}

void
ReusabilityProfiler::insert(Addr addr)
{
    assert(!blocks.count(addr));
    blocks[addr] = new BlockSample(addr, this);
}

void
ReusabilityProfiler::reset()
{
    inform("%s ReusabilityProfiler reset", name());

    for(auto& block : blocks) block.second->reset();
    previous_reset_tick = curTick();
    distReusableTick.reset();
    distDeadTick.reset();
    distAllTick.reset();
}

void
ReusabilityProfiler::finalize()
{
    inform("%s ReusabilityProfiler finalizes %d blocks", name(), blocks.size());

    for(auto& block : blocks) block.second->finalize();

    tick_to_block(&distReusableTick, &distReusableBlocks);
    tick_to_block(&distDeadTick, &distDeadBlocks);
    tick_to_block(&distAllTick, &distAllBlocks);
}

void
ReusabilityProfiler::dump()
{
    finalize();
}

void
ReusabilityProfiler::tick_to_block( OverflowVector<Tick>* tickDist,
                                    Stats::Distribution* blockDist
                                    )
{
    // need precision of double type for accuracy
    double sampled_tick = curTick() - previous_reset_tick;
    for (unsigned i = 0; i < dist_size; i++)
        blockDist->sample(i, (*tickDist)[i]/sampled_tick);
    blockDist->sample(tickDist->get_maxid(), tickDist->get_overflow_value()/sampled_tick);
    blockDist->sample(tickDist->get_minid(), tickDist->get_underflow_value()/sampled_tick);
}
