#include "mem/snapmem/snap_mem_ctrl.hh"

// SnapMemCtrlMasterPort

SnapMemCtrl::SnapMemCtrlMasterPort::SnapMemCtrlMasterPort(const std::string& _name, SnapMemCtrl* ctrl)
    : QueuedMasterPort(_name, ctrl, queue), queue(*ctrl, *this), ctrl(ctrl)
{

}

AddrRangeList SnapMemCtrl::SnapMemCtrlMasterPort::getAddrRanges() const
{
    return ctrl->getAddrRanges();
}

bool SnapMemCtrl::SnapMemCtrlMasterPort::recvTimingResp(PacketPtr pkt)
{
    return ctrl->recvTimingResp(pkt);
}

void SnapMemCtrl::SnapMemCtrlMasterPort::recvFunctionalSnoop(PacketPtr pkt)
{
    ctrl->recvFunctionalSnoop(pkt);
}

void SnapMemCtrl::SnapMemCtrlMasterPort::recvTimingSnoopReq(PacketPtr pkt)
{
    ctrl->recvTimingSnoopReq(pkt);
}

Tick SnapMemCtrl::SnapMemCtrlMasterPort::recvAtomicSnoop(PacketPtr pkt)
{
    return ctrl->recvAtomicSnoop(pkt);
}

// SnapMemCtrlMasterSnapPort

bool SnapMemCtrl::SnapMemCtrlMasterSnapPort::recvTimingResp(PacketPtr pkt)
{
    return ctrl->recvTimingSnapperPort(pkt);
}

// SnapMemCtrlSlavePort

SnapMemCtrl::SnapMemCtrlSlavePort::SnapMemCtrlSlavePort(const std::string& _name, SnapMemCtrl* ctrl)
    : QueuedSlavePort(_name, ctrl, queue), queue(*ctrl, *this), ctrl(ctrl)
{

}

AddrRangeList SnapMemCtrl::SnapMemCtrlSlavePort::getAddrRanges() const
{
    return ctrl->getAddrRanges();
}

bool SnapMemCtrl::SnapMemCtrlSlavePort::recvTimingReq(PacketPtr pkt)
{
    return ctrl->recvTimingReq(pkt);
}

Tick SnapMemCtrl::SnapMemCtrlSlavePort::recvAtomic(PacketPtr pkt)
{
    return ctrl->recvAtomic(pkt);
}

void SnapMemCtrl::SnapMemCtrlSlavePort::recvFunctional(PacketPtr pkt)
{
    ctrl->recvFunctional(pkt);
}

// SnapMemCtrl

SnapMemCtrl::SnapMemCtrl(const Params *params)
    : MemObject(params),
      snapshotEvent(this),
      snapshotCycleEvent(this),
      finalizeSnapshotEvent(this),
      masterPort(params->name+".master_port", this),
      masterSnapPort(params->name+".master_snap_port", this),
      addrRanges(params->addr_ranges.begin(), params->addr_ranges.end()),
      snapInterval(params->snap_interval),
      unitSize(params->unit_size),
      scat(params->queue_size),
      agu(this),
      system(params->system),
      basecpus(params->basecpus)
{
    inform("SnapMemCtrl %s: Snapshot Memory Controller, Masayuki Sato @ 2014", name());
    inform("SnapMemCtrl %s: schedules first snapshot event at tick %d", name(), snapInterval+curTick());
    cacheLineSize = system->cacheLineSize();
    schedule(&snapshotEvent, snapInterval + curTick());
}

void
SnapMemCtrl::init()
{
    //assert(slavePort.isConnected());
    //slavePort.sendRangeChange();
    mid = system->getMasterId(this->name());
    inform("SnapMemCtrl %s: mid is %d", name(), mid);
}

void SnapMemCtrl::regStats()
{
    countSnapshots
        .name(name()+".count_snapshot")
        .desc("The number of snapshot");
    snapshotCycles
        .name(name()+".snapshot_cycles")
        .desc("cycles elapsed for snapshooting");
    snapshotSeconds
        .name(name()+".snapshot_seconds")
        .desc("seconds elpased for snapshooting");
    snapshotSeconds = snapshotCycles * clockPeriod() / SimClock::Int::s;

    writtenAddressTableSize
        .name(name()+".num_written_block")
        .desc("The number of all sampled address entries");

    writtenAddressTableSizePerSnapshots
        .name(name()+".num_written_block_avg")
        .desc("The average number of sampled address entries per snapshot");
    writtenAddressTableSizePerSnapshots = writtenAddressTableSize / countSnapshots;

    snoopedRequests
        .name(name()+".snooped_requests")
        .desc("The number of snooped requests");
    snoopedWriteRequests
        .name(name()+".snooped_write_requests")
        .desc("The number of snooped write requests.");
    snoopedUpgradeRequests
        .name(name()+".snooped_upgrade_requests")
        .desc("The number of snooped upgrade requests.");
}

AddrRangeList SnapMemCtrl::getAddrRanges() const
{
    return addrRanges;
}

Addr SnapMemCtrl::translateUnitAddr(Addr addr)
{
    return addr & ~(unitSize - 1);
}

bool SnapMemCtrl::recvTimingReq(PacketPtr pkt)
{
    return true;
}

bool SnapMemCtrl::recvTimingResp(PacketPtr pkt)
{
    catchSnapReadResponsePacket(pkt);
    return true;
}

bool SnapMemCtrl::recvTimingSnoopReq(PacketPtr pkt)
{
    catchWriteRequestPacket(pkt);
    return true;
}

Tick SnapMemCtrl::recvAtomicSnoop(PacketPtr pkt)
{
    return curTick();
}

Tick SnapMemCtrl::recvAtomic(PacketPtr pkt)
{
    return curTick();
}

void SnapMemCtrl::recvFunctional(PacketPtr pkt)
{

}

void SnapMemCtrl::recvFunctionalSnoop(PacketPtr pkt)
{

}

void SnapMemCtrl::catchSnapReadResponsePacket(PacketPtr pkt)
{
    //inform("SnapMemCtrl: %d from main", pkt->getAddr());
    assert(scat.isExist(pkt->getAddr()));
    RequestPtr req = new Request(pkt->getAddr(), cacheLineSize, 0, mid);
    PacketPtr req_pkt = new Packet(req, MemCmd::WriteReq, cacheLineSize);
    req_pkt->allocate();
    delete pkt->req;
    delete pkt;
    masterSnapPort.schedTimingReq(req_pkt, nextCycle());
}

void SnapMemCtrl::catchWriteRequestPacket(PacketPtr pkt)
{
    if (! (pkt->isWrite() || pkt->isUpgrade()))
        return;
    assert(pkt->isRequest());
    Addr addr = pkt->getAddr();
    snoopedRequests++;
    if (pkt->isWrite()) snoopedWriteRequests++;
    if (pkt->isUpgrade()) snoopedUpgradeRequests++;
    //inform("SnapMemCtrl: Write request to %d snooped", translateUnitAddr(addr));
    wat.push(translateUnitAddr(addr));
}

bool SnapMemCtrl::recvTimingSnapperPort(PacketPtr pkt)
{
    // completion of snapshot of this block
    scat.erase(pkt->getAddr());
    delete pkt->req;
    delete pkt;
    return true;
}

void SnapMemCtrl::snapshot()
{
    if (!system->isTimingMode()) {
        inform("SnapMemCtrl %s: not invoked because of atomic mode at %d, next %d",
               name(), curTick(), snapInterval + curTick());
        schedule(&snapshotEvent, snapInterval + curTick());
        return;
    }
    inform("SnapMemCtrl %s: gets in snapshot mode at tick %d",
           name(), curTick());
    inform("SnapMemCtrl %s: %d memory regions are updated (%d bytes / region)",
           name(), wat.getSize(), unitSize);
    writtenAddressTableSize += wat.getSize();
    countSnapshots++;
    suspendCPUs();
    schedule(&snapshotCycleEvent, nextCycle());
}

void SnapMemCtrl::suspendCPUs()
{
    int num_cpus;
    int num_contexts;

    num_cpus = basecpus.size();
    for (int i = 0; i < num_cpus; i++) {
        num_contexts = basecpus[i]->numContexts();
        for (int j = 0; j < num_contexts; j++) {
            basecpus[i]->suspendContext(j);
            inform("SnapMemCtrl %s: suspend cpu %d context %d", name(), i, j);
        }
    }
}

void SnapMemCtrl::resumeCPUs()
{
    int num_cpus;
    int num_contexts;

    num_cpus = basecpus.size();
    for (int i = 0; i < num_cpus; i++) {
        num_contexts = basecpus[i]->numContexts();
        for (int j = 0; j < num_contexts; j++) {
            basecpus[i]->activateContext(j);
            inform("SnapMemCtrl %s: activate cpu %d context %d", name(), i, j);
        }
    }
}

void SnapMemCtrl::snapshotCycle()
{
    snapshotCycles++;

    sendReadPacketToNormalMemory();

    if ( wat.isEmpty() && scat.isEmpty() && agu.isComplete() ) {
        inform("SnapMemCtrl %s: All written blocks go into the snapshot memory at tick %d",
               name(), curTick());
        schedule(&finalizeSnapshotEvent, curTick());
        return;
    }

    schedule(&snapshotCycleEvent, nextCycle());
}

void SnapMemCtrl::sendReadPacketToNormalMemory()
{
    Addr uaddr;

    if (scat.isFull())
        return;
    if (agu.isComplete()) {
        if (wat.isEmpty())
            return;
        uaddr = wat.pop();
        agu.init(uaddr, unitSize, cacheLineSize);
        //inform("AddressGenerationUnit init by %d", uaddr);
    }
    uaddr = agu.pop();
    //inform("AddressGenerationUnit generate %d", uaddr);
    RequestPtr req = new Request(uaddr, cacheLineSize, 0, mid);
    PacketPtr req_pkt = new Packet(req, MemCmd::ReadReq);
    //inform("SnapMemCtrl: Send read packet to addr %d", req_pkt->getAddr());
    req_pkt->allocate();
    assert(!scat.isExist(uaddr));
    scat.push(uaddr);
    //inform("SnapMemCtrl: SnapshotCurrentReadAddressTable: %d pushed", addr);
    masterPort.schedTimingReq(req_pkt, clockEdge());
}

void SnapMemCtrl::finalizeSnapshot()
{
    inform("SnapMemCtrl %s: finalize snapshot and schedule next snapshot event at tick %d",
           name(), snapInterval+curTick());
    schedule(&snapshotEvent, snapInterval + curTick());
    resumeCPUs();
}

BaseMasterPort& SnapMemCtrl::getMasterPort(const std::string& if_name, PortID idx)
{
    if (if_name == "master_port")
        return masterPort;
    if (if_name == "master_snap_port")
        return masterSnapPort;
    return MemObject::getMasterPort(if_name, idx);
}

SnapMemCtrl*
SnapMemCtrlParams::create()
{
    return new SnapMemCtrl(this);
}
