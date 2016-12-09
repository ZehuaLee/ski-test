#include "params/BaseCache.hh"
#include "sim/core.hh"
#include "base/statistics.hh"
#include "base/callback.hh"

#include "mem/cache/cacti.hh"

#include "McPAT/cacti/cacti_interface.h"


Cacti::Cacti(const Params *p)
{
    ip.parse_cfg(p->cacti_config);
    check_consistency(p);

    inform("Cacti initialized by %s", p->cacti_config);
    uca = CACTI::cacti_interface(p->cacti_config);

    dy_read_energy_per_access = uca.power.readOp.dynamic;
    dy_write_energy_per_access = uca.power.writeOp.dynamic;

    double long_channel_leakage_reduction
        = 0.1
        + 0.9 * (0.8 * uca.data_array2->long_channel_leakage_reduction_memcell)
        + 0.2 * uca.data_array2->long_channel_leakage_reduction_periperal;

    le_bank_power = ip.long_channel_device ? uca.power.readOp.leakage*long_channel_leakage_reduction : uca.power.readOp.leakage;

    previous_sampled_tick = 0;

    Callback* dcb = new MakeCallback<Cacti, &Cacti::sampleLeak>(this);
    Stats::registerDumpCallback(dcb);
    Callback* rcb = new MakeCallback<Cacti, &Cacti::sampleLeak>(this);
    Stats::registerResetCallback(rcb);
}

void Cacti::check_consistency(const Params *p)
{
    if (ip.cache_sz != p->size) {
        fatal("CACTI: cache size does not match, in %s", p->name);
    }

    if (ip.assoc != p->assoc) {
        fatal("CACTI: associativity does not match, in %s", p->name);
    }

    if (ip.fast_access == true && p->sequential_access == true) {
        inform("CACTI: ip.fast_access=%s", ip.fast_access);
        inform("CACTI: p->sequential_access=%s", p->sequential_access);
        fatal("CACTI: sequential access does not match, in %s", p->name);
    }

    if (!ip.dvs_voltage.empty()) {
        fatal("CACTI: cannot process multiple dvs levels, in %s", p->name);
    }

    // Todo: check cache line size
    // if (ip.block_sz != _system->cacheLineSize()) {
    //     fatal("CACTI: cache line size does not match in %s", cache_name);
    // }
}

void Cacti::access(PacketPtr pkt)
{
    if (pkt->isWrite() ) {
        writeCount++;
        totalDynamicEnergy += dy_read_energy_per_access;
    } else if (pkt->isRead()) {
        readCount++;
        totalDynamicEnergy += dy_write_energy_per_access;
    } else {
        fatal("Oh. The cache access is not read and not write...");
    }
    sampleLeak();
}

void Cacti::read()
{
    readCount++;
    totalDynamicEnergy += dy_write_energy_per_access;
    sampleLeak();
}

void Cacti::write()
{
    writeCount++;
    totalDynamicEnergy += dy_read_energy_per_access;
    sampleLeak();
}

void Cacti::fill()
{
    read();
    write();
    sampleLeak();
}

void Cacti::sampleLeak()
{
    double sampled_seconds;
    Tick sampled_tick = curTick() - previous_sampled_tick;

    sampled_seconds = (double)sampled_tick / SimClock::Int::s;
    sampledSeconds += sampled_seconds;

    totalLeakageEnergy += sampled_seconds * le_bank_power;

    previous_sampled_tick = curTick();
}

void Cacti::regStats(const string name)
{
    // CACTI values
    dynamicReadEnergyPerAccess
        .name(name + ".cacti.dy_read_energy_per_access")
        .desc("Read energy per access (nJ)");
    dynamicReadEnergyPerAccess = dy_read_energy_per_access * 1e9;
    dynamicWriteEnergyPerAccess
        .name(name + ".cacti.dy_write_energy_per_access")
        .desc("Write energy per access (nJ)");
    dynamicWriteEnergyPerAccess = dy_write_energy_per_access * 1e9;
    leakageBankPower
        .name(name + ".cacti.le_bank_power")
        .desc("Leakage power par bank (mW)");
    leakageBankPower = le_bank_power * 1e3;

    // Counter
    readCount
        .name(name + ".cacti.read_count")
        .desc("Read count");
    writeCount
        .name(name + ".cacti.write_count")
        .desc("Write count");
    sampledSeconds
        .name(name + ".cacti.seconds")
        .desc("Sampled time by CACTI in seconds ");

    // Total Energies
    totalDynamicEnergy
        .name(name + ".cacti.total_energy_dynamic")
        .desc("Dynamic energy (J)");
    totalLeakageEnergy
        .name(name + ".cacti.total_energy_leakage")
        .desc("Leakage energy (J)");
    totalEnergy
        .name(name + ".cacti.total_energy")
        .desc("Total energy (J)");
    totalEnergy = totalDynamicEnergy + totalLeakageEnergy;
}
