#ifndef WLPIDWIDGET_H
#define WLPIDWIDGET_H

#include <QDialog>
#include "wlaxis.h"

#include "ui_wlpidwidget.h"

namespace Ui {
class WLPidWidget;
}

class WLPidWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WLPidWidget(QString name,WLPidData data,QWidget *parent = nullptr);
    ~WLPidWidget();

    WLPidData getPidData();

private:
    Ui::WLPidWidget *ui;

    QString m_name;

signals:
    void changedPidData(WLPidData);
};

#endif // WLPIDWIDGET_H
