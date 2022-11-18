#include "wltimerscript.h"

WLTimerScript::WLTimerScript(QObject *parent)
    : QObject(parent)
{
m_elapsedTimer.start();
}

WLTimerScript::~WLTimerScript()
{

}

void WLTimerScript::start(QString name)
{
STimerScript timer;

timer.m_activ=true;
timer.m_point=m_elapsedTimer.elapsed();

m_Timers.insert(name,timer);
}

void WLTimerScript::restart(QString name)
{
start(name);
}

void WLTimerScript::stop(QString name)
{
if(m_Timers.keys().indexOf(name)!=-1)
  {
  STimerScript timer=m_Timers.value(name);
  timer.m_stopCount=getCount(name);
  timer.m_activ=false;

  m_Timers.insert(name,timer);
 }
}

qint64 WLTimerScript::getCount(QString name)
{
if(m_Timers.keys().indexOf(name)!=-1)
  {
  if(m_Timers.value(name).m_activ)
    return m_elapsedTimer.elapsed()-m_Timers.value(name).m_point;
  else
    return m_Timers.value(name).m_stopCount;
  }

return 0;
}

