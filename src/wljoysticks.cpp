#include "wljoysticks.h"

WLJoysticks::WLJoysticks(QObject *parent) : QObject(parent)
{
QTimer::singleShot(1000,this,&WLJoysticks::initJoystick);
}

bool WLJoysticks::getButton(int id, int button)
{
QJoysticks *joystick=QJoysticks::getInstance();

return joystick? joystick->getButton(id,button) :false;
}

int WLJoysticks::getPOV(int id, int pov)
{
QJoysticks *joystick=QJoysticks::getInstance();

return joystick? joystick->getPOV(id,pov) : -1;
}

double WLJoysticks::getAxis(int id, int axis)
{
QJoysticks *joystick=QJoysticks::getInstance();

return joystick? joystick->getAxis(id,axis) : 0.0;
}

QString WLJoysticks::getName(int id)
{
QJoysticks *joystick=QJoysticks::getInstance();

return joystick? joystick->getName(id) : "";
}

int WLJoysticks::getNum()
{
QJoysticks *joystick=QJoysticks::getInstance();

return joystick? joystick->inputDevices().size() : 0;
}

void WLJoysticks::setEnabled(bool enable)
{
if(m_enable==enable) return;

m_enable=enable;

if(m_enable){

}
else {

}

}

void WLJoysticks::initJoystick()
{
QStringList joystickNames = QJoysticks :: getInstance () -> deviceNames ();

QList <QJoystickDevice *> joysticks = QJoysticks :: getInstance () -> inputDevices ();

QJoysticks *joystick=QJoysticks::getInstance();

connect(joystick,&QJoysticks::buttonEvent,[=](const QJoystickButtonEvent &event)
{
qDebug()<<"changedButtonJoystick name:"<<event.joystick->name<<" id:"<<event.joystick->id<<event.button<<event.pressed;

emit changedJoystick(event.joystick->id);
emit changedButtonJoystick(event.joystick->id,event.button,event.pressed);
});

connect(joystick,&QJoysticks::axisEvent,[=](const QJoystickAxisEvent &event)
{
qDebug()<<"changedAxisJoystick name:"<<event.joystick->name<<" id:"<<event.joystick->id<<event.axis<<event.value;

emit changedJoystick(event.joystick->id);
emit changedAxisJoystick(event.joystick->id,event.axis,event.value);
});

connect(joystick,&QJoysticks::POVEvent,[=](const QJoystickPOVEvent &event)
{
qDebug()<<"changedPOVJoystick name:"<<event.joystick->name<<" id:"<<event.joystick->id<<event.pov<<event.angle;

emit changedJoystick(event.joystick->id);
emit changedPOVJoystick(event.joystick->id,event.pov,event.angle);
});

}

