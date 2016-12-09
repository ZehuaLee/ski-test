#include "base/output.hh"
#include "mem/cache/tags/prof/addr_tracer.hh"

using namespace std;

AddressTracer::AddressTracer( string _name,
                              bool _set_sample,
                              unsigned int _nsets,
                              Addr _bsize,
                              std::string _order_by )
    : BaseTagsProfiler(_name), ssid(_nsets, _bsize), rdanalyze(_nsets, _bsize)
{
    inform("%s AddressTracer inspecting address stream", name());
    open_stream();
    addr_map.clear();
    reusehist.clear();
    order_by = _order_by;
    set_sample = _set_sample;
    usage_distance = 0;
    if(set_sample)
        inform("%s AddressTracer set_sample=%s, num_sets=%d, block_size=%d", name(), set_sample, _nsets, _bsize);
}

void AddressTracer::open_stream()
{
    insert_ostream = simout.create(name()+".insert.raw");
    access_ostream = simout.create(name()+".access.raw");
    reused_ostream = simout.create(name()+".reusedist.raw");
}

void AddressTracer::close_stream()
{
    simout.close(insert_ostream);
    simout.close(access_ostream);
    simout.close(reused_ostream);
}

void
AddressTracer::insert(Addr addr)
{
    if(set_sample && !is_samplesets(addr)) return;
    insert_stream.push_back(usage_distance, addr);
    addr_map.put_addr(addr);
    usage_distance++;
    reusehist[addr] = 0;
    rdanalyze.insert(addr);
}


void
AddressTracer::access(Addr addr)
{
    if(set_sample && !is_samplesets(addr)) return;
    access_stream.push_back(usage_distance, addr);
    addr_map.put_addr(addr);
    usage_distance++;
    reusehist[addr]++;
    rdanalyze.access(addr);
}


void
AddressTracer::victim(Addr addr)
{
    if(set_sample && !is_samplesets(addr)) return;
    reusehist.erase(addr);
    rdanalyze.victim(addr);
}

bool
AddressTracer::is_samplesets(Addr addr)
{
    if( !set_sample ) return true;

    if( ssid.isSampledAddr(addr) ) return true;
    return false;

    // Addr baddr = addr / bsize;
    // assert(addr % bsize == 0);

    // if( (uint64_t)baddr % 32 == 0 )
    //     return true;
    // return false;
}

void
AddressTracer::dump()
{
    if ( order_by == "spacefill" )
        addr_map.order_by_spacefill();
    else if ( order_by == "firsttouch" )
        addr_map.order_by_firsttouch();
    else
        assert(false);

    open_stream();

    *insert_ostream << "# usage distance\t ordered block address" << endl;
    for( auto it = insert_stream.begin(); it != insert_stream.end(); it++ ){
        *insert_ostream << it->first << " " << addr_map[it->second] << endl;
    }

    *access_ostream << "# usage distance\t ordered block address" << endl;
    for( auto it = access_stream.begin(); it != access_stream.end(); it++ ){
        *access_ostream << it->first << " " << addr_map[it->second] << endl;
    }

    rdanalyze.output(reused_ostream);

    close_stream();
}

void
AddressTracer::reset()
{
    insert_stream.clear();
    access_stream.clear();
    addr_map.clear();
    rdanalyze.reset();
    usage_distance = 0;
}
