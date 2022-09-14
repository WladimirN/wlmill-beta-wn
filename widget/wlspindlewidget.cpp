#include <QButtonGroup>
#include "wlmotion.h"
#include "wlspindlewidget.h"
#include "ui_wlspindlewidget.h"


WLSpindleWidget::WLSpindleWidget(WLSpindle *_Spindle,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLSpindleWidget)
{
    ui->setupUi(this);

    Spindle=_Spindle;

    WLMotion *MDevice=static_cast<WLMotion*>(Spindle->getModule()->getDevice());

    ui->editOutSPWM ->setModule(MDevice->getModulePWM(),false);
    ui->editOutSAOUT->setModule(MDevice->getModuleAIOPut(),false);
    ui->editOutSOUT ->setModule(MDevice->getModuleIOPut(),false);
    ui->editOutSAxis->setModule(MDevice->getModuleAxis());

    QButtonGroup *grOutput=new QButtonGroup(this);

    if(ui->editOutSPWM->isEnable())
        grOutput->addButton(ui->editOutSPWM->getButton());

    if(ui->editOutSAOUT->isEnable())
        grOutput->addButton(ui->editOutSAOUT->getButton());

    if(ui->editOutSOUT->isEnable())
        grOutput->addButton(ui->editOutSOUT->getButton());

    if(ui->editOutSAxis->isEnable())
        grOutput->addButton(ui->editOutSAxis->getButton());

    grOutput->setExclusive(true);

    ui->editOutSPWM  ->setCheckable(true);
    ui->editOutSAOUT ->setCheckable(true);
    ui->editOutSOUT  ->setCheckable(true);
    ui->editOutSAxis ->setCheckable(true);

    switch(Spindle->getTypeSOut())
    {
    case WLModule::typeEAxis:
                        ui->editOutSAxis->setChecked(true);
                        ui->editOutSAxis->setValue(Spindle->getISOut());
                        break;

    case WLModule::typeEOutPWM:
                        ui->editOutSPWM->setChecked(true);
                        ui->editOutSPWM->setValue(Spindle->getISOut());
                        break;

    case WLModule::typeEAOutput:
                        ui->editOutSAOUT->setChecked(true);
                        ui->editOutSAOUT->setValue(Spindle->getISOut());
                        break;

    case WLModule::typeEOutput:
                        ui->editOutSOUT->setChecked(true);
                        ui->editOutSOUT->setValue(Spindle->getISOut());
                        break;

    default: ui->gbSOut->setChecked(false);
             ui->editOutSPWM->setChecked(true);
             break;
    }

    ui->sbAccSOut->setValue( Spindle->getAcc());
    ui->sbDecSOut->setValue(-Spindle->getDec());

    ui->cbFastChangeSOut->setChecked(Spindle->isFastChangeSOut());

    ui->editOutENBSpindle->setModule(MDevice->getModuleIOPut(),false);
    ui->editOutENBSpindle->setValue(Spindle->getOutput(SPINDLE_outENB)->getIndex());

    ui->editOutENBSpindle->setCheckable(true);
    ui->editOutENBSpindle->setChecked(ui->editOutENBSpindle->value()!=0);
    ui->editOutENBSpindle->setLabel("outENB");

    ui->editOutRUNSpindle->setModule(MDevice->getModuleIOPut(),false);
    ui->editOutRUNSpindle->setValue(Spindle->getOutput(SPINDLE_outRUN)->getIndex());

    ui->editOutRUNSpindle->setCheckable(true);
    ui->editOutRUNSpindle->setChecked(ui->editOutRUNSpindle->value()!=0);
    ui->editOutRUNSpindle->setLabel("outRUN");

    initTableCalcSout();

}

WLSpindleWidget::~WLSpindleWidget()
{
    delete ui;
}

QList <WLSpindleData> WLSpindleWidget::getSpindleDataList()
{
QList <WLSpindleData> retList;

for(int i=0;i<ui->twCalcSout->rowCount();i++)
 {
 WLSpindleData sdata;

 if(ui->twCalcSout->item(i,0)!=nullptr
  &&ui->twCalcSout->item(i,1)!=nullptr
 &&!ui->twCalcSout->item(i,0)->data(0).toString().isEmpty()
 &&!ui->twCalcSout->item(i,1)->data(0).toString().isEmpty())
  {
  sdata.inValue =ui->twCalcSout->item(i,0)->data(0).toString().replace(",",".").toDouble();
  sdata.outValue=ui->twCalcSout->item(i,1)->data(0).toString().replace(",",".").toDouble();

  qDebug()<<"spindleData"<<sdata.inValue<<sdata.outValue;

  retList+=sdata;
  }
 }

return retList;
}

void WLSpindleWidget::accept()
{
Spindle->setDataList(getSpindleDataList());

Spindle->setAcc( ui->sbAccSOut->value());
Spindle->setDec(-ui->sbDecSOut->value());


if(!ui->gbSOut->isChecked()) {
Spindle->resetElementSpindle();
}
else if(ui->editOutSPWM->isChecked()){
Spindle->setElementSOut(WLDevice::typeEOutPWM,ui->editOutSPWM->value());
}
else if(ui->editOutSAOUT->isChecked()){
Spindle->setElementSOut(WLDevice::typeEAOutput,ui->editOutSAOUT->value());
}
else if(ui->editOutSOUT->isChecked()){
Spindle->setElementSOut(WLDevice::typeEOutput,ui->editOutSOUT->value());
}
else if(ui->editOutSAxis->isChecked()){
Spindle->setElementSOut(WLDevice::typeEAxis,ui->editOutSAxis->value());
}

Spindle->setFastSOut(ui->cbFastChangeSOut->isChecked());
Spindle->setOutENB(ui->editOutENBSpindle->isChecked() ? ui->editOutENBSpindle->value():0);
Spindle->setOutRUN(ui->editOutRUNSpindle->isChecked() ? ui->editOutRUNSpindle->value():0);

}

void WLSpindleWidget::initTableCalcSout()
{
QList <WLSpindleData> sList=Spindle->getDataList();

ui->twCalcSout->setColumnCount(2);
ui->twCalcSout->setRowCount(10);

ui->twCalcSout->setHorizontalHeaderLabels (QString(tr("S,outValue")).split(","));

for(int i=0;i<ui->twCalcSout->rowCount()&&i<sList.size();i++)
 {
 ui->twCalcSout->setItem (i,0,new QTableWidgetItem(QString::number(sList[i].inValue)));
 ui->twCalcSout->setItem (i,1,new QTableWidgetItem(QString::number(sList[i].outValue)));
 }
}
