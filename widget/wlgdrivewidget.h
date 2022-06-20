#ifndef WLMILLDRIVEDIALOG_H
#define WLMILLDRIVEDIALOG_H

#include <QDialog>
#include "wlmilldrive.h"
#include "ui_wlgdrivewidget.h"

namespace Ui {
class WLMillDriveWidget;
}

class WLGDriveWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WLGDriveWidget(WLGDrive *_drive,QWidget *parent = nullptr);
    ~WLGDriveWidget();

private:
    Ui::WLMillDriveWidget *ui;

    WLGDrive *m_MDrive;

    QString m_unit;
    // QDialog interface
public slots:
    void accept() {m_MDrive->setBacklash(ui->sbBacklash->value());}

    void setUnit(QString);
};

#endif // WLMILLDRIVEDIALOG_H
