#ifndef __MEM_CACHE_TAGS_PROF_BASE_HH__
#define __MEM_CACHE_TAGS_PROF_BASE_HH__

#include <list>
#include <string>

#include "base/callback.hh"
#include "base/statistics.hh"
#include "base/types.hh"
#include "sim/core.hh"
#include "sim/eventq.hh"

class BaseTagsProfiler
{
  private:
    std::string parent_name;

  public:
    BaseTagsProfiler(std::string parent) { parent_name = parent; };
    virtual ~BaseTagsProfiler() {};

    std::string name() { return parent_name + ".prof"; };

    virtual void regStats() {};

    virtual void insert(Addr addr) = 0;
    virtual void access(Addr addr) = 0;
    virtual void victim(Addr addr) = 0;

    virtual void sampling() = 0;

    virtual void dump() {};
    virtual void reset() {};

    virtual void add(BaseTagsProfiler* prof) {
        fatal("This function should not be used if this object is a leaf.");
    };
};

class ProfilerComposite : public BaseTagsProfiler
{
  private:
    std::list<BaseTagsProfiler*> p;

  public:
    ProfilerComposite(std::string _parent) : BaseTagsProfiler(_parent) {
        Stats::registerDumpCallback(new MakeCallback<ProfilerComposite, &ProfilerComposite::dump>(this));
        Stats::registerResetCallback(new MakeCallback<ProfilerComposite, &ProfilerComposite::reset>(this));
    };
    ~ProfilerComposite() { for(auto i : p) delete i; };

    std::string name() { return BaseTagsProfiler::name() + ".composite"; };

    void regStats() { for(auto i : p) i->regStats(); }

    void insert(Addr addr) { for(auto i : p) i->insert(addr); }
    void access(Addr addr) { for(auto i : p) i->access(addr); }
    void victim(Addr addr) { for(auto i : p) i->victim(addr); }

    void sampling() { for(auto i : p) i->sampling(); };

    void dump() { for(auto i : p) i->dump(); }
    void reset() { for(auto i : p) i->reset(); }

    void add(BaseTagsProfiler* prof) { p.push_back(prof); };
};

#endif // __MEM_CACHE_TAGS_PROF_BASE_HH__
