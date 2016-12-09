#ifndef __MEM_CACHE_CACTI_HH__
#define __MEM_CACHE_CACTI_HH__

#include "McPAT/cacti/cacti_interface.h"
#include "base/statistics.hh"
#include "mem/packet.hh"
#include "params/BaseCache.hh"

class Cacti {

private:

    // CACTI variables
    CACTI::InputParameter ip;
    CACTI::uca_org_t uca;

    // Statistics
    Stats::Formula dynamicReadEnergyPerAccess;
    Stats::Formula dynamicWriteEnergyPerAccess;
    Stats::Formula leakageBankPower;

    Stats::Scalar readCount;
    Stats::Scalar writeCount;
    Stats::Scalar sampledSeconds;

    Stats::Scalar totalDynamicEnergy;
    Stats::Scalar totalLeakageEnergy;
    Stats::Formula totalEnergy;

    // others
    double dy_read_energy_per_access;
    double dy_write_energy_per_access;
    double le_bank_power;

    double total_energy;

    typedef BaseCacheParams Params;
    void check_consistency(const Params *p);

    Tick previous_sampled_tick;

public:

    Cacti(const Params *p);

    void access(PacketPtr pkt);
    void sampleLeak();

    void read();
    void write();
    void fill();

    void regStats(const string name);
};

#endif //__MEM_CACHE_CACTI_HH__
