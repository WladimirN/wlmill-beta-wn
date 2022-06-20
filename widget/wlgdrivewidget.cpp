#include "wlgdrivewidget.h"
#include "ui_wlgdrivewidget.h"

WLGDriveWidget::WLGDriveWidget(WLGDrive *_drive,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLMillDriveWidget)
{
    m_MDrive=_drive;

    ui->setupUi(this);

    ui->sbBacklash->setValue(m_MDrive->getBacklash());

    setUnit("unit");
}

WLGDriveWidget::~WLGDriveWidget()
{
    delete ui;
}

void WLGDriveWidget::setUnit(QString unit)
{
    m_unit=unit;

    ui->sbBacklash->setSuffix(" "+m_unit);
}
