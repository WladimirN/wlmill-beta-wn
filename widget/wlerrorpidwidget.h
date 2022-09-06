#ifndef WLERRORPIDWIDGET_H
#define WLERRORPIDWIDGET_H

#include <QDialog>
#include "wlaxis.h"

namespace Ui {
class WLErrorPidWidget;
}

class WLErrorPidWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WLErrorPidWidget(QString name,WLErrorPidData error,double _stepSize,QWidget *parent = nullptr);
    ~WLErrorPidWidget();

  WLErrorPidData getErrorPidData();

private:
    Ui::WLErrorPidWidget *ui;

    QString m_name;

    double m_stepSize=1;

    WLErrorPidData curData;

    QString m_unit="1";

signals:
    void changedErrorPidData(WLErrorPidData);

private slots:
    void updateFminutes();

public slots:

    void setUnit(QString);
    void setFminutes(bool enable);
};

#endif // WLERRORPIDWIDGET_H
