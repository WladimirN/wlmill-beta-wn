#ifndef WLDELAY_H
#define WLDELAY_H

#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

#define sizeTimers 32

struct STimerScript{
qint64 m_point;
qint64 m_stopCount;
  bool m_activ;

STimerScript() {m_activ=false; m_stopCount=0;}
};

class WLTimerScript : public QObject
{
	Q_OBJECT

public:
    WLTimerScript(QObject *parent);
    ~WLTimerScript();

private:
QMap<QString,STimerScript> m_Timers;

QElapsedTimer m_elapsedTimer;

public:

    Q_INVOKABLE void start(QString name);
    Q_INVOKABLE void restart(QString name);
    Q_INVOKABLE void stop(QString name);
    Q_INVOKABLE qint64 getCount(QString name);
    Q_INVOKABLE qint64 getCountMS(QString name) {return getCountMS(name);}

public slots:

private slots:

	//void timeEnd();
};

#endif // WLDELAY_H
