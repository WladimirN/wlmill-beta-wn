#include "wltimerscript.h"

WLTimerScript::WLTimerScript(QObject *parent)
    : QObject(parent)
{
for(int i=0;i<sizeTimers;i++) {
  m_activ[i]=0;
  }

ElapsedTimer.start();
}

WLTimerScript::~WLTimerScript()
{

}
