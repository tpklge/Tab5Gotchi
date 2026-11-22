#include "core/engine.hpp"
#include <cstdio>
#include <cstdlib>
using namespace gotchi;
int main(int argc,char**argv){unsigned seed=argc>1?std::strtoul(argv[1],nullptr,10):42;auto s=newborn(0,seed,1);tick(s,s.incubationTarget);for(unsigned day=0;day<15;day++){for(int hour=0;hour<24;hour++){s.stats[Full]=75;s.stats[Hygiene]=90;s.stats[Energy]=90;s.stats[Health]=95;auto a=Action(hour%3==0?Action::Pet:hour%3==1?Action::Explore:Action::Sleep);act(s,a);s.sleeping=0;tick(s,3600000);}printf("dia=%u família=%u etapa=%s forma=%u tendências=%u/%u/%u\n",day+1,s.family,stageName(s.stage),s.form,unsigned(s.tendencies[0]),unsigned(s.tendencies[1]),unsigned(s.tendencies[2]));}return 0;}
