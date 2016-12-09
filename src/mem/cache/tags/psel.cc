#include "mem/cache/tags/psel.hh"
#include "base/output.hh"
#include "base/callback.hh"
#include "sim/core.hh"

using namespace std;

PolicySelectorPlotter::PolicySelectorPlotter(string name, unsigned int _nbit)
    : counter_bit(_nbit)
{
    plot_filename = name + ".psel_plot.txt";
    psel_out = simout.create(plot_filename);

    if (counter_bit - 7 > 0) {
        threshold = pow(2, counter_bit - 7);
    } else {
        threshold = 1;
    }
    inform("%s: PolicySelectorPlotter: ignoring displacement of counters within %d", name, threshold);

    a.clear();

    Stats::registerResetCallback(new MakeCallback<PolicySelectorPlotter, &PolicySelectorPlotter::resetStats>(this));
}

void
PolicySelectorPlotter::sample(unsigned int counter, bool final = false)
{
    a.add(counter);

    if (!final)
        if ( abs(counter - a.getAverage()) < threshold )
            return;

    *psel_out << a.getStart() << "\t" << a.getAverage() << endl;
    a.clear();
}

void
PolicySelectorPlotter::resetStats()
{
    a.clear();
    simout.close(psel_out);
    simout.remove(plot_filename);
    psel_out = simout.create(plot_filename);
}


PolicySelector::PolicySelector(string name, unsigned int _nbit)
    : nbit(_nbit), plotter(name, _nbit)
{
    max = pow(2, nbit) - 1;
    counter = max / 2.0;

    Stats::registerResetCallback(new MakeCallback<PolicySelector, &PolicySelector::resetStats>(this));
    Stats::registerDumpCallback(new MakeCallback<PolicySelector, &PolicySelector::dumpStats>(this));
}

void
PolicySelector::regStats(std::string name)
{
    valueDist
        .init(0, pow(2, nbit) - 1, pow(2, nbit - 4))
        .name(name + ".psel_dist")
        .desc("psel counter-tick distribution")
        .flags(Stats::pdf)
        ;
}

void
PolicySelector::increment()
{
    sampleCounter();
    counter < max ? counter++ : counter = max;
    plotter.sample(counter);
}

void
PolicySelector::decrement()
{
    sampleCounter();
    counter > 0 ? counter-- : counter = 0;
    plotter.sample(counter);
}

void
PolicySelector::sampleCounter()
{
    valueDist.sample(counter, curTick() - sampled);
    sampled = curTick();
}
