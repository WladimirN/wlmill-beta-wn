#include "wlaxiswidget.h"
#include "ui_wlaxiswidget.h"
#include "wlpamwidget.h"
#include "wlpidwidget.h"
#include "wlerrorpidwidget.h"
#include "wldevice.h"

WLAxisWidget::WLAxisWidget(WLAxis *_axis,bool _slave,double _offset, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WLAxisWidget)
{
    ui->setupUi(this);

    m_slave=_slave;
    m_axis=_axis;

    ui->gbDynamic->setVisible(!m_slave);
    ui->gbOffset ->setVisible(m_slave);

    ui->sbOffset->setValue(m_slave ?_offset : 0);

    ui->cbActInALM->addItems(QString("no,SDstop,EMGStop").split(","));
    ui->cbActInPEL->addItems(QString("no,SDstop,EMGStop").split(","));
    ui->cbActInMEL->addItems(QString("no,SDstop,EMGStop").split(","));

    ui->cbActInALM->setCurrentIndex(m_axis->getActIn(AXIS_inALM));
    ui->cbActInPEL->setCurrentIndex(m_axis->getActIn(AXIS_inPEL));
    ui->cbActInMEL->setCurrentIndex(m_axis->getActIn(AXIS_inMEL));

    connect(ui->editInALM,&WLEditIOWidget::toggle,ui->cbActInALM,&QComboBox::setEnabled);
    connect(ui->editInPEL,&WLEditIOWidget::toggle,ui->cbActInPEL,&QComboBox::setEnabled);
    connect(ui->editInMEL,&WLEditIOWidget::toggle,ui->cbActInMEL,&QComboBox::setEnabled);

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(m_axis->getModule()->getDevice()->getModule(WLModule::typeMIOPut));

    ui->editInALM->setModule(ModuleIOPut);
    ui->editInORG->setModule(ModuleIOPut);
    ui->editInPEL->setModule(ModuleIOPut);
    ui->editInMEL->setModule(ModuleIOPut);

    ui->editInALM->setLabel("inALM");
    ui->editInORG->setLabel("inORG");
    ui->editInPEL->setLabel("inPEL");
    ui->editInMEL->setLabel("inMEL");

    ui->editInALM->setValue(m_axis->getInput(AXIS_inALM)->getIndex());
    ui->editInORG->setValue(m_axis->getInput(AXIS_inORG)->getIndex());
    ui->editInPEL->setValue(m_axis->getInput(AXIS_inPEL)->getIndex());
    ui->editInMEL->setValue(m_axis->getInput(AXIS_inMEL)->getIndex());

    ui->editInALM->setCheckable(true);
    ui->editInORG->setCheckable(true);
    ui->editInPEL->setCheckable(true);
    ui->editInMEL->setCheckable(true);

    ui->editInALM->setChecked(ui->editInALM->value()!=0);
    ui->editInORG->setChecked(ui->editInORG->value()!=0);
    ui->editInPEL->setChecked(ui->editInPEL->value()!=0);
    ui->editInMEL->setChecked(ui->editInMEL->value()!=0);

    ui->editOutRALM->setModule(ModuleIOPut,false);
    ui->editOutENB ->setModule(ModuleIOPut,false);

    ui->editOutRALM->setValue(m_axis->getOutput(AXIS_outRALM)->getIndex());
    ui->editOutENB ->setValue(m_axis->getOutput(AXIS_outENB)->getIndex());

    ui->editOutRALM->setCheckable(true);
    ui->editOutENB->setCheckable(true);

    ui->editOutRALM->setChecked(ui->editOutRALM->value()!=0);
    ui->editOutENB->setChecked(ui->editOutENB->value()!=0);

    ui->editOutRALM->setLabel("outRALM");
    ui->editOutENB ->setLabel("outENB");

    ui->cbTypePulse->addItems(QString("SD,CW/CCW,AB,ABx2,ABx4,SDx2,CW/CCWx2").split(","));
    connect(ui->cbTypePulse,SIGNAL(activated(int)),SLOT(updateCBTypePulse(int)));

    ui->cbTypePulse->setCurrentIndex(m_axis->getTypePulse());

    ui->cbInvStep->setChecked(m_axis->getOutSDInv()&MAF_invStep);
    ui->cbInvDir ->setChecked(m_axis->getOutSDInv()&MAF_invDir);

    ui->sbDelaySCurveMs->setValue(m_axis->getDelaySCurve()*1000.0);

   connect(ui->pbEditParSMPlus,&QPushButton::clicked,this,&WLAxisWidget::onEditParSMPlus);
   connect(ui->pbEditPid,&QPushButton::clicked,this,&WLAxisWidget::onEditPid);
   connect(ui->pbEditErrorPid,&QPushButton::clicked,this,&WLAxisWidget::onEditErrorPid);

    //ui->gbMParMinus->setChecked(m_axis->getStepMotorMParPlus()!=m_axis->getStepMotorMParMinus());

    ui->gbInput->setToolTip(
                 tr(
                "<b>Inputs number</font></b>"
                "<ol>"
                  "<li>inPEL - plus  end limit</li>"
                  "<li>inMEL - minus end limit</li>"
                  "<li>inALM - alarm input from driver controller</li>"
                "</ol>"
                "<b>Type Action</font></b>"
                "<ol>"
                  "<li>no - no action</li>"
                  "<li>EMGStop - emergency stop</li>"
                  "<li>SDStop - slow down stop</li>"
                "</ol>"
                  )
                );

    ui->gbOutput->setToolTip(
                tr(
                "<b>Outputs number</font></b>"
                "<ol>"
                  "<li>outENB -  enable to driver controller</li>"
                  "<li>outRALM - reset alarm to driver controller</li>"
                "</ol>"
                  )
                );

     ui->gbDynamic->setToolTip("<img src='/image/scurve.png'/> Book");

     connect(ui->cbTypeMotor,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WLAxisWidget::updateTypeMotor);

     ui->buttonBox->setVisible(false);

     setEditSpindle(false);
}

WLAxisWidget::~WLAxisWidget()
{
    delete ui;
}

void WLAxisWidget::setShowButtonDialog(bool en)
{
ui->buttonBox->setVisible(en);
}

void WLAxisWidget::saveDataAxis()
{
m_axis->setDelaySCurve(ui->sbDelaySCurveMs->value()/1000.0);

m_axis->setInALM(getIndexInALM());
m_axis->setInORG(getIndexInORG());
m_axis->setInPEL(getIndexInPEL());
m_axis->setInMEL(getIndexInMEL());

m_axis->setOutRALM(getIndexOutRALM());
m_axis->setOutENB(getIndexOutENB());

m_axis->setActIn(AXIS_inALM,getActInALM());
m_axis->setActIn(AXIS_inPEL,getActInPEL());
m_axis->setActIn(AXIS_inMEL,getActInMEL());

m_axis->setTypePulse((typePulseAxis)ui->cbTypePulse->currentIndex()
                    ,(ui->cbInvStep->isChecked()?MAF_invStep:0)
                    |(ui->cbInvDir->isChecked() ?MAF_invDir:0));

if(ui->cbTypeMotor->currentData()==WLAxis::AXIS_encoderStepMotor
 ||ui->cbTypeMotor->currentData()==WLAxis::AXIS_errEncoderStepMotor){
  m_axis->setEncoder(ui->encoder->value());
  }
  else {
  m_axis->setEncoder(-1);
  }

m_axis->setTypeMotor(ui->cbTypeMotor->currentData().value<WLAxis::typeMotorAxis>());
}

double WLAxisWidget::getOffset()
{
qDebug()<<"getOffset";
return ui->sbOffset->value();
}

bool WLAxisWidget::isUniqueInputs()
{
QList <quint8> listInput;

listInput+=getIndexInALM();
listInput+=getIndexInORG();
listInput+=getIndexInPEL();
listInput+=getIndexInMEL();

for(int i=0;i<listInput.size();i++)
    for(int j=i;j<listInput.size();j++)
       {
       if((listInput[i]==listInput[j])
        &&(listInput[i]!=0)
        &&(listInput[i]!=1)
        &&(i!=j)) return false;
       }

return true;
}

bool WLAxisWidget::isUniqueOutputs()
{
QList <quint8> listOutput;

if(ui->editOutENB->isChecked())  listOutput+=ui->editOutENB->value();
if(ui->editOutRALM->isChecked()) listOutput+=ui->editOutRALM->value();

for(int i=0;i<listOutput.size();i++)
    for(int j=i;j<listOutput.size();j++)
       {
       if((listOutput[i]==listOutput[j])
        &&(listOutput[i]!=0)
        &&(i!=j)) return false;
       }

return true;
}

double WLAxisWidget::geDelaytSCurveMs()
{
return  ui->sbDelaySCurveMs->value();
}

quint8 WLAxisWidget::getIndexInPEL()
{
return ui->editInPEL->isChecked() ? ui->editInPEL->value():0;
}

quint8 WLAxisWidget::getIndexInMEL()
{
return ui->editInMEL->isChecked() ? ui->editInMEL->value():0;
}

quint8 WLAxisWidget::getIndexInORG()
{
return ui->editInORG->isChecked() ? ui->editInORG->value():0;
}

quint8 WLAxisWidget::getIndexInALM()
{
return ui->editInALM->isChecked() ? ui->editInALM->value():0;
}

quint8 WLAxisWidget::getIndexOutENB()
{
return ui->editOutENB->isChecked() ? ui->editOutENB->value():0;
}

quint8 WLAxisWidget::getIndexOutRALM()
{
return ui->editOutRALM->isChecked() ? ui->editOutRALM->value():0;
}

typeActionInput WLAxisWidget::getActInPEL()
{
return static_cast<typeActionInput>(ui->cbActInPEL->currentIndex());
}

typeActionInput WLAxisWidget::getActInMEL()
{
return static_cast<typeActionInput>(ui->cbActInMEL->currentIndex());
}

typeActionInput WLAxisWidget::getActInALM()
{
    return static_cast<typeActionInput>(ui->cbActInALM->currentIndex());
}

void WLAxisWidget::setEditSpindle(bool en)
{
 ui->cbTypeMotor->clear();

 ui->cbTypeMotor->addItem("noMotor",(WLAxis::AXIS_noMotor));

 if(en){
 ui->cbTypeMotor->addItem("spindleStepMotor",(WLAxis::AXIS_spindleStepMotor));
 }
 else{
 ui->cbTypeMotor->addItem("stepMotor",(WLAxis::AXIS_stepMotor));

 WLModuleEncoder *ModuleEncoder=static_cast<WLModuleEncoder*>(m_axis->getModule()->getDevice()->getModule(WLModule::typeMEncoder));

 if(ModuleEncoder)  {
   ui->cbTypeMotor->addItem("encoderStepMotor",(WLAxis::AXIS_encoderStepMotor));
   ui->cbTypeMotor->addItem("errEncoderStepMotor",(WLAxis::AXIS_errEncoderStepMotor));

   ui->encoder->setModule(ModuleEncoder);
   ui->encoder->setValue(m_axis->getEncoder());
   }
 }

for(int i=0;i<ui->cbTypeMotor->count();i++) {
    if(ui->cbTypeMotor->itemData(i).value<WLAxis::typeMotorAxis>()==m_axis->getTypeMotor())   {
    ui->cbTypeMotor->setCurrentIndex(i);
    break;
    }
 }
}

void WLAxisWidget::accept()
{
saveDataAxis();

if(sender()==ui->buttonBox)
            close();
}

void WLAxisWidget::setUnit(QString txt)
{
m_unit=txt;

ui->sbOffset->setSuffix(txt);
}

void WLAxisWidget::setStepSize(double stepsize)
{
if(stepsize>0){

double k=stepsize/m_stepsize;

m_stepsize=stepsize;
}
else {
qDebug()<<"WLAxisWidget::setStepSize error stepsize";
}

}

void WLAxisWidget::onEditParSMPlus()
{
WLMParAxis parA=m_axis->getStepMotorMParPlus();

dataPad pad;

pad.Aac=parA.Aacc/(1<<xPD);
pad.Ade=parA.Adec/(1<<xPD);
pad.Vma=parA.Fmax/(1<<xPD);

pad.Vst=0;
pad.name="stepMototPlus";

WLPamWidget PW(pad,10000000,this);

PW.setEnabledEditName(false);

PW.setEnabledEditVst(false);
PW.setUnit("1");
PW.setFminutes(false);

PW.show();

if(PW.exec()) {
 pad=PW.getPad();

 parA.Aacc=pad.Aac*(1<<xPD);
 parA.Adec=pad.Ade*(1<<xPD);
 parA.Fmax=pad.Vma*(1<<xPD);

 m_axis->setMPar(parA,AXIS_StepMotorMParPlus);
 }

//if(true)
//    m_axis->setMPar(parA,AXIS_StepMotorMParAll);
//else

}

void WLAxisWidget::onEditPid()
{
WLPidWidget PW(QString("Axis %1").arg(m_axis->getIndex()),m_axis->getPidData(),this);

PW.setModal(true);
PW.show();

if(PW.exec()) {
  m_axis->setPidData(PW.getPidData());
  }
}

void WLAxisWidget::onEditErrorPid()
{
WLErrorPidWidget EPW(QString("Axis %1").arg(m_axis->getIndex()),m_axis->getErrorPidData(),m_stepsize,this);

EPW.setUnit(m_unit);
EPW.setFminutes(m_Fminutess);

EPW.setModal(true);
EPW.show();

if(EPW.exec()) {
  m_axis->setErrorPidData(EPW.getErrorPidData());
  }
}

void WLAxisWidget::updateTypeMotor(int index)
{
WLAxis::typeMotorAxis type=static_cast<WLAxis::typeMotorAxis>(ui->cbTypeMotor->currentData().toInt());

switch(type){
case WLAxis::AXIS_noMotor:
                   ui->gbTypePulse->setVisible(false);
                     ui->gbEncoder->setVisible(false);
                         ui->gbPID->setVisible(false);
                    ui->gbMParPlus->setVisible(false);                   
                    ui->gbErrorPos->setVisible(false);
                       ui->gbInput->setVisible(false);
                      ui->gbOutput->setVisible(false);                      
                     ui->gbDynamic->setVisible(false);
                   break;

case WLAxis::AXIS_stepMotor:
                   ui->gbTypePulse->setVisible(true);
                     ui->gbEncoder->setVisible(false);
                         ui->gbPID->setVisible(false);
                    ui->gbMParPlus->setVisible(false);
                    ui->gbErrorPos->setVisible(false);
                       ui->gbInput->setVisible(true);
                      ui->gbOutput->setVisible(true);
                     ui->gbDynamic->setVisible(!m_slave);
                   break;

case WLAxis::AXIS_encoderStepMotor:
                   ui->gbTypePulse->setVisible(true);
                     ui->gbEncoder->setVisible(true);
                         ui->gbPID->setVisible(true);
                    ui->gbMParPlus->setVisible(true);
                    ui->gbErrorPos->setVisible(true);
                    ui->gbInput->setVisible(true);
                   ui->gbOutput->setVisible(true);
                  ui->gbDynamic->setVisible(false);

case WLAxis::AXIS_errEncoderStepMotor:
                   ui->gbTypePulse->setVisible(true);
                     ui->gbEncoder->setVisible(true);
                         ui->gbPID->setVisible(true);
                    ui->gbMParPlus->setVisible(false);
                    ui->gbErrorPos->setVisible(true);
                       ui->gbInput->setVisible(true);
                      ui->gbOutput->setVisible(true);
                     ui->gbDynamic->setVisible(!m_slave);
                    break;

case WLAxis::AXIS_spindleStepMotor:
                   ui->gbTypePulse->setVisible(true);
                     ui->gbEncoder->setVisible(false);
                         ui->gbPID->setVisible(false);
                    ui->gbMParPlus->setVisible(false);
                    ui->gbErrorPos->setVisible(false);
                       ui->gbInput->setVisible(false);
                      ui->gbOutput->setVisible(false);
                     ui->gbDynamic->setVisible(false);

                   break;
}

}

void WLAxisWidget::setFminutes(bool fmin)
{
m_Fminutess=fmin;
}



