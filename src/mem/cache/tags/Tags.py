# Copyright (c) 2012-2013 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Prakash Ramrakhyani

from m5.params import *
from m5.proxy import *
from ClockedObject import ClockedObject

class BaseTags(ClockedObject):
    type = 'BaseTags'
    abstract = True
    cxx_header = "mem/cache/tags/base.hh"
    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")

    # Get the block size from the parent (system)
    block_size = Param.Int(Parent.cache_line_size, "block size in bytes")

    # Get the hit latency from the parent (cache)
    hit_latency = Param.Cycles(Parent.hit_latency,
                               "The hit latency for this cache")
    write_hit_latency = Param.Cycles(Parent.write_hit_latency,"write_hit_latency")
class BaseSetAssoc(BaseTags):
    type = 'BaseSetAssoc'
    abstract = True
    cxx_header = "mem/cache/tags/base_set_assoc.hh"
    assoc = Param.Int(Parent.assoc, "associativity")
    sequential_access = Param.Bool(Parent.sequential_access,
        "Whether to access tags and data sequentially")

class LRU(BaseSetAssoc):
    type = 'LRU'
    cxx_class = 'LRU'
    cxx_header = "mem/cache/tags/lru.hh"

class RandomRepl(BaseSetAssoc):
    type = 'RandomRepl'
    cxx_class = 'RandomRepl'
    cxx_header = "mem/cache/tags/random_repl.hh"

class FALRU(BaseTags):
    type = 'FALRU'
    cxx_class = 'FALRU'
    cxx_header = "mem/cache/tags/fa_lru.hh"

class LRUProfiler(LRU):
    """Act as LRU while inspecting reuse profiler"""
    type = 'LRUProfiler'
    cxx_class = 'LRUProfiler'
    cxx_header = "mem/cache/tags/lru_prof.hh"
    interval = Param.Cycles(100000, "sampling interval")

class BIP(LRU):
    type = 'BIP'
    cxx_class = 'BIP'
    cxx_header = 'mem/cache/tags/bip.hh'
    epsilon_denominator = Param.Int(32, "denominator of Epsilon")
    epsilon_numerator = Param.Int(1, "numerator of Epsilon")

class DIP(BIP):
    type = 'DIP'
    cxx_class = 'DIP'
    cxx_header = 'mem/cache/tags/dip.hh'
    ndsets = Param.Int(32, "the number of dedicated sets per policy")
    psel_bits = Param.Int(11, "the number of bits of the policy selector counter")

class FLIP(LRU):
    type = 'FLIP'
    cxx_class = 'FLIP'
    cxx_header = 'mem/cache/tags/flip.hh'
    detect_th = Param.Int(1, "the threshold to detect reuses for least-recently-used blocks")
    blk_max_count = Param.Int(3, "maximum access count of each block")
    update_interval = Param.Cycles(5000000, "interval updating inst/prmt positions")
    start_prior = Param.Int(0, "promotion priority at start")

class FLIPProfiler(FLIP):
    """Act as FLIP while inspecting reuse profiler"""
    type = 'FLIPProfiler'
    cxx_class = 'FLIPProfiler'
    cxx_header = "mem/cache/tags/flip_prof.hh"
    interval = Param.Cycles(100000, "sampling interval")
