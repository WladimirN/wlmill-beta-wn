#include <qglobal.h>
#include "wlmachine.h"


WLMachine::WLMachine(QObject *parent)
{

}

WLMachine::~WLMachine()
{
quit();
wait();
}

void WLMachine::addDrive(WLDrive *_drive)
{
m_Drives<<_drive;
}

void WLMachine::removeDrive(WLDrive *_drive)
{
m_Drives.removeOne(_drive);
}

void WLMachine::SDStop()
{
WLDrive::startStopDrives();
}

bool WLMachine::goDriveManual(QString nameDrive,double IncDec,float step)
{
if(!isPossiblyManual()
 ||!verifyReadyMotion()) return false;

qDebug()<<nameDrive<<"goDriveManual"<<IncDec<<step;

WLDrive *drive=WLDrive::getDrive(nameDrive);

IncDec=qBound((double)-1.0,IncDec,(double)1.0);

if(isAuto())
   {
   SDStop();
   return false;
   }
else
if(drive
 &&drive->getAxis())
{
if(IncDec==0.0)
    drive->startStop();
 else
 {
 drive->setVmov(0);
 drive->startMovManual(IncDec,step);
 }
}

return true;
}

void WLMachine::updateAuto()
{
qDebug()<<"WLMachine::updateAuto()";

QMutexLocker locker(&MutexAuto);

if(isAuto())
switch(m_typeAutoMachine)
 {
 case AUTO_DrivesFind: updateDrivesFindPos();break;
 }
}

bool WLMachine::updateDrivesFindPos()
{
if(!isActivDrives())
{
 if(m_listFindDrivePos.isEmpty())
 {
 resetAuto();
 QTimer::singleShot(10,this,SLOT(setFinished()));
 return 0;
 }
 else
 {
 QString curAxisFind=(m_listFindDrivePos.takeFirst()).toUpper();
 bool ok=false;

 for(int i=0;i<curAxisFind.size();i++){
   ok|=goDriveFind(curAxisFind.at(i));
   }

 if(!ok)
     return updateDrivesFindPos();
 }
}

return 1;
}

bool WLMachine::goDrivesFind()
{
if(!isPossiblyManual()
 ||!verifyReadyMotion()) return false;

if(isAuto()) {
 SDStop();
 }
else
 {
 qDebug()<<"WLMachine::goFindDrivePos"<<m_strFindDrivePos;

 WLDrive::resetDrives();

 m_listFindDrivePos=m_strFindDrivePos.split(",");

 if(!m_listFindDrivePos.isEmpty()){
   m_typeAutoMachine=AUTO_DrivesFind;
   setAuto();
   updateAuto();

   return true;
   }
 }

return false;
}

void WLMachine::setDrivesFinded()
{

foreach(QString curAxisFind,m_strFindDrivePos.split(","))   {
   for(int i=0;i<curAxisFind.size();i++){
      if(WLDrive::getDrive(curAxisFind[i].toUpper()))
         WLDrive::getDrive(curAxisFind[i].toUpper())->setTruPosition(true);
      }
   }

}

void WLMachine::setEnable(bool enable)
{
m_enable=enable;
emit changedEnable(m_enable);
}

void WLMachine::setPercentManual(double per)
{
m_percentManual=qBound(0.01,per,100.0);

emit changedPercentManual(m_percentManual);

if(isPossiblyManual()){
foreach(WLDrive *Drive,WLDrive::getDriveList()) {
    Drive->setPercentManual(m_percentManual);
    }
}
}

void WLMachine::setReady(bool ready)
{
if(m_ready!=ready){
     m_ready=ready;
     emit changedReady(m_ready);
     }
}

bool WLMachine::goDriveFind(QString nameDrive)
{
if(!isPossiblyManual()
 ||!verifyReadyMotion()) {return false;}

qDebug()<<"goDriveFind"<<nameDrive;

WLDrive *drive=WLDrive::getDrive(nameDrive);

if(drive==nullptr) return false;

if(drive->getName().isEmpty())
{
qDebug()<<"emptyName";
}
else
 {
 if(drive->isAuto())
    {
    qDebug()<<"already set Auto";
    drive->reset();
    }
 else if(drive->getAxis())
        {
        drive->setMovFind();
        drive->startTask();
        }
 }

return true;
}

bool WLMachine::goDriveTeach(QString nameDrive)
{
if(!isPossiblyManual()) {return false;}

qDebug()<<"goDriveTeach"<<nameDrive;

WLDrive *drive=WLDrive::getDrive(nameDrive);

if(drive)
 {
 drive->reset();
 drive->setMovTeach();
 drive->startTask();

 return true;
 }

return false;
}

bool WLMachine::goDriveVerify(QString nameDrive)
{
if(!isPossiblyManual()
 ||!verifyReadyMotion()) {return false;}

qDebug()<<"goDriveVerify"<<nameDrive;

WLDrive *drive=WLDrive::getDrive(nameDrive);

if(drive)
 {
 drive->reset();
 drive->setMovVerify();
 drive->startTask();

 return true;
 }

return false;
}
