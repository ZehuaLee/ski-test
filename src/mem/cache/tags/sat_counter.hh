#ifndef __MEM_CACHE_TAGS_SATURATING_COUNTER__
#define __MEM_CACHE_TAGS_SATURATING_COUNTER__

class SaturatingCounter {

    unsigned int max;
    unsigned int counter;
    unsigned int start;

    void check_set(unsigned int n) {
        if (counter > max) counter = max;
        else               counter = n;
    }

  public:
    SaturatingCounter(unsigned int _max, unsigned int _start = 0)
        : max(_max), start(_start)
    { check_set(_start); }

    virtual void increment(unsigned int n = 1) {
        if   (max - n >= counter) counter += n;
        else                      counter = max;
    }
    virtual void decrement(unsigned int n = 1) {
        if   (counter >= n) counter -= n;
        else                counter = 0;
    }

    virtual void reset() { counter = start; }

    SaturatingCounter operator++(int) {
        SaturatingCounter sc(max, counter);
        increment();
        return sc;
    };
    SaturatingCounter operator--(int) {
        SaturatingCounter sc(max, counter);
        decrement();
        return sc;
    };
    operator unsigned int() { return counter; }
    operator int() { return (int)counter; }
    operator double() { return (double)counter; }
    operator long() { return (long)counter; }
    operator std::string() { return std::to_string(counter); }

    bool isMax() { return counter == max; }
    bool isMin() { return counter == 0; }
};


class ConservativeStateMachine : public SaturatingCounter {

  public:
    ConservativeStateMachine(unsigned int _max, unsigned int _start = 0)
        : SaturatingCounter(_max, _start)
    {}

    virtual void decrement(unsigned int n = 0) { reset(); }
};

class ConservativeSaturatingCounter : public SaturatingCounter {

    ConservativeStateMachine sm;

  public:

    ConservativeSaturatingCounter( unsigned int _max,
                                   unsigned int _state_max,
                                   unsigned int _start = 0,
                                   unsigned int _state_start = 0
                                    )
        : SaturatingCounter(_max, _start), sm(_state_max, _state_start)
    {}

    virtual void increment(unsigned int n = 1) {
        sm++;
        if(sm.isMax()) SaturatingCounter::increment(n);
    }

    virtual void decrement(unsigned int n = 1) {
        sm--;
        if(sm.isMin()) SaturatingCounter::decrement(n);
    }
};

#endif










