#ifndef WLDELAY_H
#define WLDELAY_H

#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

#define sizeTimers 32

class WLTimerScript : public QObject
{
	Q_OBJECT

public:
    WLTimerScript(QObject *parent);
    ~WLTimerScript();

private:
qint64 m_point[sizeTimers];
  bool m_activ[sizeTimers];

QElapsedTimer ElapsedTimer;

public:

    Q_INVOKABLE void start(int index)     {if(index<sizeTimers)    {m_activ[index]=true; m_point[index]=ElapsedTimer.elapsed();}}
    Q_INVOKABLE void restart(int index)   {if(index<sizeTimers)    {start(index);}}
    Q_INVOKABLE void stop(int index)      {if(index<sizeTimers)    {m_activ[index]=false;}}
    Q_INVOKABLE qint64 getCount(int index){if(index<sizeTimers)    return ElapsedTimer.elapsed()-m_point[index]; else return -1;}

public slots:

private slots:

	//void timeEnd();
};

#endif // WLDELAY_H
