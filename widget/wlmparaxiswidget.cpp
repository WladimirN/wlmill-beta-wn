#include "wlmparaxiswidget.h"
#include "ui_wlmparaxiswidget.h"

WLMParAxisWidget::WLMParAxisWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLMParAxisWidget)
{
    ui->setupUi(this);
}

WLMParAxisWidget::~WLMParAxisWidget()
{
    delete ui;
}

void WLMParAxisWidget::setMPar(WLMParAxis _MPar)
{
MPar=_MPar;

ui->sbAacc->setValue(( MPar.Aacc)/(1<<xPD));
ui->sbFmax->setValue(( MPar.Fmax)/(1<<xPD));
ui->sbAdec->setValue((-MPar.Adec)/(1<<xPD));
}

void WLMParAxisWidget::setName(QString name)
{
ui->name->setText(name);
}

WLMParAxis WLMParAxisWidget::getMPar()
{
WLMParAxis ret=MPar;

ret.Aacc= (ui->sbAacc->value()*(1<<xPD));
ret.Fmax= (ui->sbFmax->value()*(1<<xPD));
ret.Adec=-(ui->sbAdec->value()*(1<<xPD));

return ret;
}

