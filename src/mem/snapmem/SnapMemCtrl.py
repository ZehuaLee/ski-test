from System import System
from m5.params import *
from MemObject import MemObject
from BaseCPU import BaseCPU
from m5.proxy import *

class SnapMemCtrl(MemObject):
    type = "SnapMemCtrl"
    cxx_header = "mem/snapmem/snap_mem_ctrl.hh"
    #slave_port = SlavePort("CPU-side port")
    master_port =  MasterPort("Memory-side port")
    master_snap_port = MasterPort("Memory-side port for snapshot memory")
    addr_ranges = VectorParam.AddrRange([AllMemory], "The address range for the CPU-side port")
    snap_interval = Param.Latency("500us", "Interval between two snapshot")
    unit_size = Param.MemorySize("64B", "Block size to be recognized for snapshots")
    queue_size = Param.Int("4", "Outstanding memory accesses for snapshot")
    system = Param.System(Parent.any, "System that the bus belongs to.")
    basecpus = VectorParam.BaseCPU("Connected CPU")
