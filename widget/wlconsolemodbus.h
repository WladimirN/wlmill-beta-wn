#ifndef WLCONSOLEMODBUS_H
#define WLCONSOLEMODBUS_H

#include <QDialog>
#include <QTime>
#include <QRegExpValidator>

#include "wlmoduledmodbus.h"

namespace Ui {
class WLConsoleModbus;
}

class WLConsoleModbus : public QDialog
{
    Q_OBJECT

public:
    explicit WLConsoleModbus(QWidget *parent = nullptr);
    ~WLConsoleModbus();

    void setModuleModbus(WLModuleDModbus *_Modbus);

private:
    Ui::WLConsoleModbus *ui;

    WLModuleDModbus *m_Modbus=nullptr;

private slots:
    void sendData();
    void reciveData(QByteArray);
    void timeoutData(QByteArray);
};

#endif // WLCONSOLEMODBUS_H
