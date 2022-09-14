#include <QButtonGroup>
#include "wleditmillwidget.h"

WLEditMillWidget::WLEditMillWidget(WLGMachine *_MillMachine,QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	MillMachine=_MillMachine;

    WLModuleAxis *ModuleAxis=MillMachine->getMotionDevice()->getModuleAxis();
    WLModulePlanner *ModulePlanner=MillMachine->getMotionDevice()->getModulePlanner();

    ui.cbActSafeProbe->addItems(QString("no,SDstop,EMGStop").split(","));
    ui.cbActSafeProbe->setCurrentIndex(MillMachine->getActSafeProbe());

    ui.sbPerFpause->setValue(MillMachine->getMotionDevice()->getModulePlanner()->getKFpause()*100);

    ui.leStrFindDrivePos->setText(MillMachine->getStrFindDrivePos());

    ui.editInEMG->  setModule(MillMachine->getMotionDevice()->getModuleIOPut());
    ui.editInSD->   setModule(MillMachine->getMotionDevice()->getModuleIOPut());

    ui.editInProbe->setModule(MillMachine->getMotionDevice()->getModuleIOPut());
    ui.editInPause->setModule(MillMachine->getMotionDevice()->getModuleIOPut());
    ui.editInStop-> setModule(MillMachine->getMotionDevice()->getModuleIOPut());

    ui.editInEMG->  setLabel("InEMGStop");
    ui.editInSD->   setLabel("InSDStop");

    ui.editInProbe->setLabel("InProbe");
    ui.editInPause->setLabel("InPause");
    ui.editInStop-> setLabel("InStop");
		
    ui.editInEMG->  setValue(ModuleAxis->getInput(MAXIS_inEMGStop)->getIndex());
    ui.editInSD->   setValue(ModuleAxis->getInput(MAXIS_inSDStop)->getIndex());

    ui.editInProbe->setValue(ModulePlanner->getInput(PLANNER_inProbe)->getIndex());
    ui.editInPause->setValue(ModulePlanner->getInput(PLANNER_inPause)->getIndex());
    ui.editInStop-> setValue(ModulePlanner->getInput(PLANNER_inStop)->getIndex());

    ui.editInEMG->  setCheckable(true);
    ui.editInSD->   setCheckable(true);
    ui.editInProbe->setCheckable(true);
    ui.editInPause->setCheckable(true);
    ui.editInStop-> setCheckable(true);

    ui.editInEMG->  setChecked(ui.editInEMG->  value()!=0);
    ui.editInSD->   setChecked(ui.editInSD->   value()!=0);
    ui.editInProbe->setChecked(ui.editInProbe->value()!=0);
    ui.editInPause->setChecked(ui.editInPause->value()!=0);

    ui.sbSmoothAng->setValue(MillMachine->getMotionDevice()->getModulePlanner()->getSmoothAng());
    ui.sbHeartMs->setValue(MillMachine->getMotionDevice()->getModuleConnect()->getTimeHeartVal());
    ui.sbTimeoutMs->setValue(MillMachine->getMotionDevice()->getModuleConnect()->getTimeoutConnectVal());

    ui.cbHPause->setChecked(MillMachine->isUseHPause());
    ui.sbHPause->setValue(MillMachine->HPause());
    ui.sbHPause->setEnabled(ui.cbHPause->isChecked());

	ui.sbFbls->setValue(MillMachine->VBacklash());

    ui.cbAutoStart->setChecked(MillMachine->isAutoStartGCode());
    ui.cbAutoSetSafeProbe->setChecked(MillMachine->isAutoSetSafeProbe());

    ui.cbUseMPG->setChecked(MillMachine->isUseMPG());

    ui.cbUseDriveA->setChecked(MillMachine->getDrive("A")!=nullptr);
    ui.cbUseDriveB->setChecked(MillMachine->getDrive("B")!=nullptr);
    ui.cbUseGModel->setChecked(MillMachine->isUseGModel());

   // ui.sbOffsetHToolProbe->setValue(MillMachine->getOffsetHTool());

    connect(ui.pbVerError,SIGNAL(clicked()),SLOT(onVerifyError()));

    ui.lePercentManual->setText(MillMachine->getPercentManualStr());

    ui.sbFeedZG1->setValue(MillMachine->getFeedG1StartAt());
    ui.sbDistZG1->setValue(MillMachine->getDistG1StartAt());

    setModal(true);

}

WLEditMillWidget::~WLEditMillWidget()
{

}

QString WLEditMillWidget::verifyError()
{
QString str;

WLGDrive *ZD=MillMachine->getDrive("Z");

 if(ui.cbHPause->isChecked()
 &&!ZD->isInfinity()
&&((ZD->maxPosition()<ui.sbHPause->value())||(ZD->minPosition()>ui.sbHPause->value())))
    str+=tr("Pause height off-axis position")+"\n";


return str;
}

bool WLEditMillWidget::getNeedClose() const
{
return m_needClose;
}

void WLEditMillWidget::onVerifyError()
{
    QString str=verifyError();

if(str.isEmpty()) str=tr("No error!!!");

QMessageBox::information(this, tr("Verify error"),str,QMessageBox::Ok);
}

void WLEditMillWidget::accept()
{
m_needClose=saveDataMill();

foreach(QDialog *dialog,dialogList)
                dialog->accept();

QDialog::accept();
}


bool WLEditMillWidget::saveDataMill()
{
bool ret=false;

//MillMachine->setRangeS(ui.sbSmin->value(),ui.sbSmax->value());
//MillMachine->setRangeSOut(ui.sbSminOut->value(),ui.sbSmaxOut->value());

WLModuleAxis    *ModuleAxis   =MillMachine->getMotionDevice()->getModuleAxis();
WLModulePlanner *ModulePlanner=MillMachine->getMotionDevice()->getModulePlanner();

MillMachine->setActSafeProbe(static_cast<typeActionInput>(ui.cbActSafeProbe->currentIndex()));

MillMachine->setHPause(ui.sbHPause->value());

ModuleAxis->setInEMGStop(ui.editInEMG->isChecked()  ? ui.editInEMG->value()  :0);
ModuleAxis->setInSDStop (ui.editInSD->isChecked()   ? ui.editInSD->value()   :0);

ModulePlanner->setInProbe(ui.editInProbe->isChecked()? ui.editInProbe->value():0);
ModulePlanner->setInPause(ui.editInPause->isChecked()? ui.editInPause->value():0);
ModulePlanner->setInStop(ui.editInStop->isChecked()? ui.editInStop->value():0);

MillMachine->getMotionDevice()->getModulePlanner()->setSmoothAng(ui.sbSmoothAng->value());

MillMachine->getMotionDevice()->getModuleConnect()->setTimersConnect(ui.sbTimeoutMs->value(),ui.sbHeartMs->value());

MillMachine->setFeedVBacklash(ui.sbFbls->value());

MillMachine->setEnableHPause(ui.cbHPause->isChecked());

MillMachine->setHPause(ui.sbHPause->value());
MillMachine->setUseMPG(ui.cbUseMPG->isChecked());

MillMachine->setStrFindDrivePos(ui.leStrFindDrivePos->text());

MillMachine->setAutoStartGCode(ui.cbAutoStart->isChecked());
MillMachine->setAutoSetSafeProbe(ui.cbAutoSetSafeProbe->isChecked());

MillMachine->getMotionDevice()->getModulePlanner()->setKFpause(ui.sbPerFpause->value()/100.0f);

if(ui.cbUseDriveA->isChecked())
{
if(MillMachine->getDrive("A")==nullptr)
    {
    WLGDrive *MD = new WLGDrive("A",MillMachine->getMotionDevice()->getModuleAxis());

    MillMachine->addDrive(MD);
    ret=true;
    }
}
else if(MillMachine->getDrive("A")!=nullptr)
       {
       MillMachine->removeDrive(MillMachine->getDrive("A"));
       ret=true;
       }

if(ui.cbUseDriveB->isChecked())
{
if(MillMachine->getDrive("B")==nullptr)
    {
    WLGDrive *MD = new WLGDrive("B",MillMachine->getMotionDevice()->getModuleAxis());

    MillMachine->addDrive(MD);
    ret=true;
    }
}
else if(MillMachine->getDrive("B")!=nullptr)
       {
       MillMachine->removeDrive(MillMachine->getDrive("B"));
       ret=true;
       }

//MillMachine->setOffsetHTool(ui.sbOffsetHToolProbe->value());
MillMachine->setEnableGModel(ui.cbUseGModel->isChecked());

MillMachine->setPercentManualStr(ui.lePercentManual->text());

MillMachine->setFeedG1StartAt(ui.sbFeedZG1->value());
MillMachine->setDistG1StartAt(ui.sbDistZG1->value());


return ret;
}

void WLEditMillWidget::keyPressEvent(QKeyEvent *event)
{
event->accept();
}
