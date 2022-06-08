#ifndef WLAXISWIDGET_H
#define WLAXISWIDGET_H

#include <QDialog>
#include "wlaxis.h"

namespace Ui {
class WLAxisWidget;
}

class WLAxisWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WLAxisWidget(WLAxis *_axis,bool main,double _offset,QWidget *parent = nullptr);
    ~WLAxisWidget();

public:
    void saveDataAxis();
    double getOffset();

    bool isUniqueInputs();
    bool isUniqueOutputs();

    double geDelaytSCurveMs();

    WLAxis *getAxis() {return m_axis;}

    quint8 getIndexInPEL();
    quint8 getIndexInMEL();
    quint8 getIndexInORG();
    quint8 getIndexInALM();

    quint8 getIndexOutENB();
    quint8 getIndexOutRALM();

    typeActionInput getActInPEL();
    typeActionInput getActInMEL();
    typeActionInput getActInALM();
private:
    Ui::WLAxisWidget *ui;

    WLAxis *m_axis;
    bool m_slave=false;
    double m_stepsize=1.0;

public slots:
    void accept() {saveDataAxis();}
    void setUnit(QString);

    void setStepSize(double stepsize);
private slots:

    void onEditParSMPlus();
    void onEditPid();

    void updateTypeMotor(int);

};

#endif // WLAXISWIDGET_H
