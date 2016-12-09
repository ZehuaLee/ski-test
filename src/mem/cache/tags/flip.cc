/**
 * @file
 * Definitions of a FLIP tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/flip.hh"
#include "mem/cache/tags/sdp_counter.hh"
#include "mem/cache/base.hh"

FLIP::UpdatePriorityEvent::UpdatePriorityEvent(FLIP* f)
{
    flip = f;
}

void
FLIP::UpdatePriorityEvent::process()
{
    flip->updatePriorityPointer();
}

const char*
FLIP::UpdatePriorityEvent::description() const
{
    return "FLIP update event";
}

FLIP::FLIP(const Params *p)
    : LRU(p),
      sdp(name()+".raw", assoc), sdparr(name()+".raw", assoc, p->blk_max_count+1),
      pos(p->blk_max_count, ConservativeSaturatingCounter(assoc-1, 7, p->start_prior, 0)),
      frc(p->blk_max_count, 0),
      blk_max_count(p->blk_max_count), detect_th(p->detect_th),
      update_interval(p->update_interval), updateEvent(this)
{
}

void
FLIP::regStats()
{
    LRU::regStats();

    for(int i = 0; i <= blk_max_count; i++) {
        Stats::Distribution* dist = new Stats::Distribution;
        (*dist)
            .init(0, assoc-1, 1)
            .name(name()+".flip_pos_dur_"+std::to_string(i))
            .desc("priority position for each promotion (duration)");
        posDistDuration.push_back(dist);
    }
    for(int i = 0; i <= blk_max_count; i++) {
        Stats::Distribution* dist = new Stats::Distribution;
        (*dist)
            .init(0, assoc-1, 1)
            .name(name()+".flip_pos_blk_"+std::to_string(i))
            .desc("priority position for each promotion (block)");
        posDistBlock.push_back(dist);
    }
}

void
FLIP::startup()
{
    inform("FLIP: flexibly promoted until 0-th to %d-th promotions for each block", blk_max_count);
    inform("FLIP: access-detecting threshold is %d", detect_th);

    scheduleUpdatePriorityEvent(update_interval);
    inform("FLIP: priority update scheduled every %d cycles", update_interval);
}

BaseSetAssoc::BlkType*
FLIP::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id, bool is_read)
{
  BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id, is_read);
  unsigned int lrustate;

    if (blk != NULL) {
        // for getting stack distance profiling
        lrustate = sets[blk->set].getLRUState(blk);
        sdp.sample(lrustate);
        sdparr.sample(blk->tag_refcount, lrustate);
        // for FLIP
        unsigned int cur_index = blk->tag_refcount;
        if(lrustate == assoc - 1)
            //if(index < blk_max_count)
            frc[cur_index]++;
        if(blk->tag_refcount < blk_max_count)
            blk->tag_refcount++;
        unsigned int new_index = blk->tag_refcount;
        if (new_index < blk_max_count) {
            sets[blk->set].moveTo(blk, pos[new_index]);
            posDistBlock[new_index]->sample((unsigned int)pos[new_index], 1);
        } else if (new_index == blk_max_count) {
            sets[blk->set].moveToHead(blk);
            posDistBlock[new_index]->sample(0, 1);
        } else {
            fatal("block new_index is larger than blk_max_count");
        }

    }

    return blk;
}

BaseSetAssoc::BlkType*
FLIP::findVictim(Addr addr)
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc - 1];

    if (blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
FLIP::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());

    // for FLIP
    blk->tag_refcount = 0;
    sets[set].moveTo(blk, pos[0]);
    posDistBlock[0]->sample((unsigned int)pos[0], 1);
}

void
FLIP::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

void
FLIP::updatePriorityPointer()
{
    samplePriorityPointer();
    inform("FLIP: %s priority updated @ tick %d", name(), curTick());
    for(unsigned int i = 0; i < blk_max_count; i++) {
        if (frc[i] >= detect_th) {
            pos[i]--;
            inform("FLIP: %d-th promotion to %d (observed %d first reuses)",
                   i, (unsigned int)pos[i], frc[i]);
        } else {
            pos[i]++;
            inform("FLIP: %d-th promotion to %d (observed %d first reuses)",
                   i, (unsigned int)pos[i], frc[i]);
        }
        frc[i] = 0;
    }

    sdp.dump();
    sdparr.dump();
    scheduleUpdatePriorityEvent(update_interval);
}

void
FLIP::samplePriorityPointer()
{
    static Tick prev_sampled = curTick();

    for(int i = 0; i < blk_max_count; i++) {
        posDistDuration[i]->sample((unsigned int)pos[i], curTick() - prev_sampled);
    }

    prev_sampled = curTick();
}

FLIP*
FLIPParams::create()
{
    return new FLIP(this);
}
