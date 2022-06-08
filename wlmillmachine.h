#ifndef WLMILLMACHINE_H
#define WLMILLMACHINE_H

#include <QDebug>
#include "wlgmachine.h"

class WLMillMachine : public WLGMachine
{
Q_OBJECT

public:
    explicit WLMillMachine(WLGProgram *_Program,WLEVScript *_MScript,WLEVScript *_LScript,QObject *parent=nullptr);



};

#endif // WLMILLMACHINE_H
