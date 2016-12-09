
#include "base/statistics.hh"
#include "mem/cache/tags/prof/insertion.hh"
#include "base/callback.hh"

InsertionProfiler::InsertionProfiler(const std::string _name)
    : BaseTagsProfiler(_name)
{
    inform("%s InsertionProfiler inspecting insertions and their reusability", name());
    Callback *cb = new MakeCallback<InsertionProfiler, &InsertionProfiler::finalize>(this);
    registerExitCallback(cb);
}

void InsertionProfiler::regStats()
{
    using namespace Stats;

    insertionDeadVerified
        .name(name()+".dead_verified")
        .desc("insertions verified as dead-on-fill")
        .flags(total);

    insertionDeadLeft
        .name(name()+".dead_left")
        .desc("insertions left as dead-on-fill at the end of the simulation")
        .flags(total);

    insertionDead
        .name(name()+".dead_all")
        .desc("insertions judged as dead-on-fill (dead_verified + dead_left)")
        .flags(total);
    insertionDead = insertionDeadVerified + insertionDeadLeft;

    insertionReusable
        .name(name()+".reusable")
        .desc("insertions verfied as reusable")
        .flags(total);
}

void InsertionProfiler::insert(Addr addr)
{
    assert(!ins_table.exists(addr));
    ins_table.insert(addr);
}

void InsertionProfiler::access(Addr addr)
{
    if ( !ins_table.exists(addr) )
        return;
    insertionReusable++;
    ins_table.erase(addr);
}

void InsertionProfiler::victim(Addr addr)
{
    if ( !ins_table.exists(addr) )
        return;
    insertionDeadVerified++;
    ins_table.erase(addr);
}

void InsertionProfiler::finalize()
{
    insertionDeadLeft = ins_table.size();
}
