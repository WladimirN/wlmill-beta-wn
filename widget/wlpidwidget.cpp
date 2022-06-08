#include "wlpidwidget.h"
#include "ui_wlpidwidget.h"

WLPidWidget::WLPidWidget(QString name,WLPidData data,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLPidWidget)
{
    ui->setupUi(this);

    ui->sbP->setValue(data.p);
    ui->sbI->setValue(data.i);
    ui->sbD->setValue(data.d);

    setWindowTitle("Pid: "+name);

    m_name=name;

    ui->pbApply->setDisabled(true);

    connect(ui->sbP,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});
    connect(ui->sbI,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});
    connect(ui->sbD,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});

    connect(ui->pbApply,&QPushButton::clicked,[=](){ui->pbApply->setDisabled(true);
                                                    emit pidChanged(getPidData());});

}

WLPidWidget::~WLPidWidget()
{
delete ui;
}

WLPidData WLPidWidget::getPidData()
{
WLPidData ret;

ret.p=ui->sbP->value();
ret.i=ui->sbI->value();
ret.d=ui->sbD->value();

return ret;
}
