#ifndef __MEM_CACHE_TAGS_SDP_COUNTER_HH__
#define __MEM_CACHE_TAGS_SDP_COUNTER_HH__

#include <iostream>

#include "base/callback.hh"
#include "base/output.hh"
#include "sim/core.hh"

class StackDistanceCounter {

    std::vector<uint64_t> counter;
    bool enable_reset;
    bool enable_dump;

    unsigned int assoc;
    std::string file_prefix;
    std::ostream* fout;

  public:
    StackDistanceCounter( std::string _name,
                          unsigned int _assoc,
                          bool autoreset = true,
                          bool autodump = true
                         )
        : counter(_assoc, 0)
    {
        file_prefix  = _name;
        assoc = _assoc;
        enable_reset = autoreset;
        enable_dump = autodump;

        if(enable_reset) {
            Stats::registerResetCallback(new MakeCallback<StackDistanceCounter, &StackDistanceCounter::reset>(this));
        }

        if(enable_dump) {
            Stats::registerDumpCallback(new MakeCallback<StackDistanceCounter, &StackDistanceCounter::dump>(this));
        }
    }

    void sample(unsigned int lrustate) {
        assert(lrustate < assoc);
        counter[lrustate]++;
    }

    uint64_t get(unsigned int lrustate) {
        assert(lrustate < assoc);
        return counter[lrustate];
    }

    void reset() {
        dump();
        for(int i = 0; i < assoc; i++)
            counter[i] = 0;
    }
    void dump() {
        if(!enable_dump) return;
        fout = simout.create(file_prefix+".sdp.txt");
        *fout << "# lrustate\tcount" << std::endl;
        for(int i = 0; i < assoc; i++)
            *fout << i << "\t" << counter[i] << std::endl;
        simout.close(fout);
    }

};


class StackDistanceCounterArray {
    std::vector<StackDistanceCounter*> array;
    bool enable_reset;
    bool enable_dump;

    unsigned int assoc;
    unsigned int array_size;
    std::string fprefix;
    std::ostream* fout;

  public:
    StackDistanceCounterArray( std::string _name,
                               unsigned int _assoc,
                               unsigned int _array_size,
                               bool autoreset = true,
                               bool autodump = true
                               )
    {
        fprefix  = _name;
        assoc = _assoc;
        array_size = _array_size;
        enable_reset = autoreset;
        enable_dump = autodump;

        array.resize(array_size);
        for(unsigned int i = 0; i < array_size; i++){
            array[i] = new StackDistanceCounter(fprefix+"."+std::to_string(i), assoc, enable_reset, enable_dump);
        }
    }

    void sample(unsigned int index, unsigned int lrustate) {
        unsigned int _index;
        if      (index < 0)              _index = 0;
        else if (index > array_size - 1) _index = array_size - 1;
        else                             _index = index;
        assert(_index < array_size);
        array[_index]->sample(lrustate);
    }

    void dump() {
        for(auto i = array.begin(); i != array.end(); i++)
            (*i)->dump();
    }
};

#endif
