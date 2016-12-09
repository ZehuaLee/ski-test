#ifndef __MEM_SNAPMEM_SNAPMEMCTRL_HH__
#define __MEM_SNAPMEM_SNAPMEMCTRL_HH__

#include <set>

#include "cpu/base.hh"
#include "mem/mem_object.hh"
#include "mem/packet.hh"
#include "mem/packet_queue.hh"
#include "mem/qport.hh"
#include "params/SnapMemCtrl.hh"
#include "sim/eventq.hh"
#include "sim/system.hh"

class SnapMemCtrl : public MemObject {

public:

    typedef SnapMemCtrlParams Params;
    SnapMemCtrl(const Params *params);

    void regStats();

    virtual void init();

    //virtual BaseSlavePort& getSlavePort(const std::string& if_name, PortID idx = InvalidPortID);
    virtual BaseMasterPort& getMasterPort(const std::string& if_name, PortID idx = InvalidPortID);

    void snapshot();
    void snapshotCycle();
    void finalizeSnapshot();

    EventWrapper<SnapMemCtrl, &SnapMemCtrl::snapshot> snapshotEvent;
    EventWrapper<SnapMemCtrl, &SnapMemCtrl::snapshotCycle> snapshotCycleEvent;
    EventWrapper<SnapMemCtrl, &SnapMemCtrl::finalizeSnapshot> finalizeSnapshotEvent;

protected:

    class SnapMemCtrlMasterPort : public QueuedMasterPort {
        MasterPacketQueue queue;
    protected:
        SnapMemCtrl* ctrl;
        virtual AddrRangeList getAddrRanges() const;
        virtual bool recvTimingResp(PacketPtr pkt);
        virtual void recvFunctionalSnoop(PacketPtr pkt);
        virtual void recvTimingSnoopReq(PacketPtr pkt);
        virtual Tick recvAtomicSnoop(PacketPtr pkt);
    public:
        SnapMemCtrlMasterPort(const std::string& _name, SnapMemCtrl* ctrl);
        virtual bool isSnooping() const { return true; };
    };

    class SnapMemCtrlMasterSnapPort : public SnapMemCtrlMasterPort {
    public:
        SnapMemCtrlMasterSnapPort(const std::string& _name, SnapMemCtrl* ctrl)
            : SnapMemCtrlMasterPort(_name, ctrl)
        {}
        virtual bool isSnooping() const { return false; };
        virtual bool recvTimingResp(PacketPtr pkt);
    };

    class SnapMemCtrlSlavePort : public QueuedSlavePort {
        SlavePacketQueue queue;
        SnapMemCtrl* ctrl;
    protected:
        virtual AddrRangeList getAddrRanges() const;
        virtual bool recvTimingReq(PacketPtr pkt);
        virtual Tick recvAtomic(PacketPtr pkt);
        virtual void recvFunctional(PacketPtr pkt);
    public:
        SnapMemCtrlSlavePort(const std::string& _name, SnapMemCtrl* ctrl);
        virtual bool isSnooping() const { return true; };
    };

    //SnapMemCtrlSlavePort slavePort;
    SnapMemCtrlMasterPort masterPort;
    SnapMemCtrlMasterSnapPort masterSnapPort;

private:

    const AddrRangeList addrRanges;
    const Tick snapInterval;
    const uint64_t unitSize;
    unsigned int cacheLineSize;

    virtual bool recvTimingResp(PacketPtr pkt);
    virtual AddrRangeList getAddrRanges() const;
    virtual bool recvTimingReq(PacketPtr pkt);
    virtual Tick recvAtomic(PacketPtr pkt);
    virtual Tick recvAtomicSnoop(PacketPtr pkt);
    virtual void recvFunctional(PacketPtr pkt);
    virtual void recvFunctionalSnoop(PacketPtr pkt);
    virtual bool recvTimingSnoopReq(PacketPtr pkt);
    virtual bool recvTimingSnapperPort(PacketPtr pkt);

    Addr translateUnitAddr(Addr addr);

    bool filterSnapReadResponsePacket(PacketPtr pkt);
    void catchSnapReadResponsePacket(PacketPtr pkt);

    void filterWriteRequestPacket(PacketPtr pkt);
    void catchWriteRequestPacket(PacketPtr pkt);

    void sendReadPacketToNormalMemory();

    class AddressTable {
    private:
        int size;
        std::set<Addr> table;
    public:
        AddressTable(int _size) { size = _size; table.clear(); };
        AddressTable()          { size = 0; table.clear(); };
        bool isFull()           { if(size == 0) return false; return table.size() >= size; };
        void push(Addr addr)    { assert(!isFull()); table.insert(addr); };
        Addr pop()              { Addr addr = *table.begin(); erase(addr); return addr; };
        void erase(Addr addr)   { table.erase(addr); };
        bool isExist(Addr addr) { return table.find(addr) != table.end(); };
        bool isEmpty()          { return table.empty(); };
        uint64_t getSize()      { return table.size(); };
    };

    // WrittenAddressTable: memorize written address
    AddressTable wat;
    // Snapshot Current Address Table: memorize address that waits for backup
    AddressTable scat;

    // block offset processing currently
    class AddressGenerationUnit
    {
        SnapMemCtrl* ctrl;
        Addr unitAddr;
        Addr currentBlkAddr;
        unsigned int counter;
        unsigned int blkInUnit;
        unsigned int blkSize;
        unsigned int unitSize;
        bool complete;
    public:
        AddressGenerationUnit(SnapMemCtrl* _ctrl)
            : ctrl(_ctrl), complete(true) { };
        void init(Addr addr, unsigned int _unitSize, unsigned int _blkSize) {
            blkSize = _blkSize; unitSize = _unitSize;
            assert(addr % unitSize == 0);
            unitAddr = addr; currentBlkAddr = addr; blkInUnit = unitSize / blkSize;
            counter = 0; complete = false;
        };
        Addr pop() {
            assert(!complete);
            currentBlkAddr = unitAddr + blkSize * counter; counter++;
            if ( counter >= blkInUnit ) complete = true;
            return currentBlkAddr;
        };
        bool isComplete() { return complete; };
    };

    AddressGenerationUnit agu;

    System* system;
    MasterID mid;
    std::vector<BaseCPU*> basecpus;

    void suspendCPUs();
    void resumeCPUs();

    Stats::Scalar snapshotCycles;
    Stats::Formula snapshotSeconds;
    Stats::Scalar countSnapshots;
    Stats::Scalar writtenAddressTableSize;
    Stats::Formula writtenAddressTableSizePerSnapshots;
    Stats::Scalar snoopedRequests;
    Stats::Scalar snoopedWriteRequests;
    Stats::Scalar snoopedUpgradeRequests;

};

#endif
