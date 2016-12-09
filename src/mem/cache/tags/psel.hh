#ifndef __MEM_CACHE_TAGS_PSEL_HH__
#define __MEM_CACHE_TAGS_PSEL_HH__

#include <cmath>
#include <map>
#include <string>

#include "base/statistics.hh"
#include "base/types.hh"

class Averager {
    Tick start;
    Tick end;
    Tick duration;
    double average;

    bool first_add;

  public:

    Averager() { clear(); first_add = true;}

    Tick getStart() { return start; }

    double getAverage() { return average; }

    void add(Tick _start, Tick _end, unsigned int _counter) {
        assert(_start <= _end);

        Tick all, _duration;

        if (first_add) {
            average = _counter;
            start = curTick();
            end = curTick();
            duration = 0;
            first_add = false;
            return;
        }

        _duration = _end - _start;
        if (_duration == 0)
            return;
        all = duration + _duration;
        average = _counter * (double)_duration / all + average * (double)duration / all;
        end = _end;
        duration = all;
    }

    void add(double _counter) {
        add(end, curTick(), _counter);
    }

    void clear() { duration = 0; end = curTick(); start = curTick(); }
};


class PolicySelectorPlotter {

    Tick prev_sampled;
    Tick prev_reset;
    unsigned int prev_counter;
    double threshold;

    unsigned int counter_bit;
    unsigned int counter_max;
    unsigned int counter_min;

    std::string plot_filename;
    std::ostream* psel_out;
    std::map<Tick, unsigned int> psel_hist;

    Averager a;

  public:
    PolicySelectorPlotter(std::string name, unsigned int _nbit);

    void sample(unsigned int counter, bool final);

    void resetStats();
};


class PolicySelector {

    unsigned int counter;
    unsigned int max;
    unsigned int nbit;

    PolicySelectorPlotter plotter;

    Tick sampled;

  public:

    PolicySelector(std::string name, unsigned int _nbit);

    void increment();
    void decrement();

    bool isOver() { return counter > max / 2 ? true : false; }
    bool isUnder() { return isOver() ? false : true; }

    void sampleCounter();
    void regStats(std::string name);

    void resetStats(){};
    void dumpStats(){ plotter.sample(counter, true); };

    Stats::Distribution valueDist;
};

#endif
