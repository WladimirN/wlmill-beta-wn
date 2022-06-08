#ifndef WLJOYSTICKS_H
#define WLJOYSTICKS_H

#include <QObject>
#include <QTimer>
#include <QDebug>

#include "QJoysticks.h"

class WLJoysticks : public QObject
{
    Q_OBJECT
public:
    explicit WLJoysticks(QObject *parent = nullptr);

private:
   int m_id=-1;
  bool m_enable=false;

public:
Q_INVOKABLE     bool getButton(int id,int number);
Q_INVOKABLE      int getPOV(int id,int number);
Q_INVOKABLE   double getAxis(int id,int number);
Q_INVOKABLE  QString getName(int id);
Q_INVOKABLE      int getNum();

signals:
    void changedJoystick(int id);
    void changedButtonJoystick(int id,int button, bool press);
    void changedPOVJoystick(int id,int pov,int angle);
    void changedAxisJoystick(int id,int axis,double value);

public slots:
    void setEnabled(bool enable);

private slots:
    void initJoystick();
};

#endif // WLJOYSTICKS_H
