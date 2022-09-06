#include "wlerrorpidwidget.h"
#include "ui_wlerrorpidwidget.h"

WLErrorPidWidget::WLErrorPidWidget(QString name,WLErrorPidData data,double _stepSize,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLErrorPidWidget)
{
    ui->setupUi(this);

    curData=data;

    m_stepSize=_stepSize;

    ui->sbEmin->setValue(curData.Emin*m_stepSize);
    ui->sbEmax->setValue(curData.Emax*m_stepSize);

    setWindowTitle("Error Pid: "+name);

    m_name=name;

    ui->pbApply->setDisabled(true);

    connect(ui->sbFmax,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});
    connect(ui->sbFmin,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});

    connect(ui->sbEmax,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});
    connect(ui->sbEmin,QOverload<double>::of(&QDoubleSpinBox::valueChanged),[=](){ui->pbApply->setEnabled(true);});

    connect(ui->cbFMinutes,&QCheckBox::toggled,this,&WLErrorPidWidget::updateFminutes);

    connect(ui->pbApply,&QPushButton::clicked,[=](){ui->pbApply->setDisabled(true);
                                                    emit changedErrorPidData(getErrorPidData());});

    setFminutes(false);
    setUnit("1");

    updateFminutes();
}

WLErrorPidWidget::~WLErrorPidWidget()
{
    delete ui;
}

WLErrorPidData WLErrorPidWidget::getErrorPidData()
{
WLErrorPidData ret;

ret.Fmax=ui->sbFmax->value()/m_stepSize/(ui->cbFMinutes->isChecked() ? 60:1);
ret.Fmin=ui->sbFmin->value()/m_stepSize/(ui->cbFMinutes->isChecked() ? 60:1);

ret.Emax=ui->sbEmax->value()/m_stepSize;
ret.Emin=ui->sbEmin->value()/m_stepSize;

return ret;
}

void WLErrorPidWidget::updateFminutes()
{
if(ui->cbFMinutes->isChecked()){
 ui->sbFmax->setValue(curData.Fmax*m_stepSize*60.0);
 ui->sbFmin->setValue(curData.Fmin*m_stepSize*60.0);
 }
 else{
 ui->sbFmax->setValue(curData.Fmax*m_stepSize);
 ui->sbFmin->setValue(curData.Fmin*m_stepSize);
 }

setUnit(m_unit);
}

void WLErrorPidWidget::setUnit(QString unit)
{
m_unit=unit;

ui->sbFmax->setSuffix(" "+m_unit+"/"+(ui->cbFMinutes->isChecked() ? tr("min"):tr("sec")));
ui->sbFmin->setSuffix(" "+m_unit+"/"+(ui->cbFMinutes->isChecked() ? tr("min"):tr("sec")));
}

void WLErrorPidWidget::setFminutes(bool enable)
{
ui->cbFMinutes->setChecked(enable);
}
