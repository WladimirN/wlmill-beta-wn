#include <QTimer>
#include <QFile>
#include <QCoreApplication>
#include <QXmlStreamWriter>
#include "wlmill.h"

double SGProbe::FProbe1=50;
double SGProbe::FProbe2=0;
double SGProbe::backDist=3;
double SGProbe::headDiam=2;
bool SGProbe::enDoubleProbe=false;
WLIOPut::typeActionInput SGProbe::typeStop=WLIOPut::INPUT_actSdStop;


double WLGMachine::getDistG1StartAt() const
{
return m_distG1StartAt;
}

void WLGMachine::setDistG1StartAt(double distG1StartAt)
{
m_distG1StartAt = distG1StartAt >=0 ? distG1StartAt : 0;
}

double WLGMachine::getFeedG1StartAt() const
{
return m_feedG1StartAt;
}

void WLGMachine::setFeedG1StartAt(double feedG1StartAt)
{
m_feedG1StartAt = feedG1StartAt>=0 ? feedG1StartAt : 0;
}

WLGMachine::WLGMachine(WLGProgram *_Program,WLEVScript *_MScript,WLEVScript *_LScript,QObject *parent)
    :WLMachine(parent)
    ,m_HeightMap(this)
    ,m_Joysticks(this)
{
m_Program=_Program;

m_MScript=_MScript;
m_LScript=_LScript;

motDevice=nullptr;

QDir().mkdir(_scriptPath);
QDir().mkdir(_iconsGMPath);

setContinueMov(true);
setBLNextMov(true);

loadMScript();
loadLScript(); //загружаем старые или дефолтные

QFile::remove(mScriptFile);
QFile::remove(lScriptFile); //удаляем старые

m_busy=false;
m_pause=false;

setPercentManualStr("0.1,0.25,1,5,10,25,50,75,100");
}

WLGMachine::~WLGMachine()
{
saveConfig();
motDevice->deleteLater();
qDebug()<<"end GMachine";
}

void WLGMachine::setStateSpindle(int state, int index)
{
stateSpindle=(statesSpindle)state;

motDevice->getModulePlanner()->setEnableSOut(state!=statesSpindle::Stop);

setSOut(getGCode()->getValue('S') * getStateSpindle());
}

void WLGMachine::plusPercentManual()
{
int index=m_percentManualList.indexOf(getPercentManual());

if(index!=-1)
 {
 index++;
 if(index<m_percentManualList.size())  setPercentManual(m_percentManualList[index]);
 }
else {
 for(int i=0;i<m_percentManualList.size();i++)
  {
  if(getPercentManual()<m_percentManualList[i])
      {
      setPercentManual(m_percentManualList[i]);
      break;
      }
  }
}

}

void WLGMachine::minusPercentManual()
{
int index=m_percentManualList.indexOf(getPercentManual());

if(index!=-1)
  {
  index--;
  if(index>=0)  setPercentManual(m_percentManualList[index]);
  }
else {
 for(int i=m_percentManualList.size()-1;i>=0;i--)
  {
  if(getPercentManual()>m_percentManualList[i])
      {
      setPercentManual(m_percentManualList[i]);
      break;
      }
  }
}
}

QList<WLGDrive *> WLGMachine::getGDrives()
{
QList<WLGDrive *> ret;

foreach(WLDrive *drive,getDrives())
{
if(strcmp(drive->metaObject()->className(),"WLGDrive")==0)
  {
  ret<<static_cast<WLGDrive*>(drive);
  }
}

return ret;
}

WLMPG *WLGMachine::getMPG()
{
WLModuleMPG *MMPG=motDevice->getModuleMPG();

if(MMPG)
  return MMPG->getMPG(0);
else
  return nullptr;
}

void WLGMachine::addCurrentSCor()
{
 /*
SCorrectSOut addScor;

addScor.Sadj=tarSOut;
addScor.Scor=calcCorrectSOut(curSOut);

m_correctSList.prepend(addScor);

sortCorrectSList();

setPercentSOut(100.0);
setSOut(getSTar());
*/
WLModulePlanner *Planner=getMotionDevice()->getModulePlanner();

QList <WLSpindleData> List=Planner->getSpindle()->getDataList();

WLSpindleData corrS;

if(List.size()<2){
  sendMessage("Error set correct s","no set basic point",0);
  return;
  }

double gS=getGCode()->getValue('S');

if(gS>List.last().inValue){
  sendMessage("Error set correct s","current S very high",0);
  return;
  }

if(gS<List.first().inValue){
  sendMessage("Error set correct s","current S very low",0);
  return;
  }

corrS.inValue=gS;
corrS.outValue=Planner->getSpindle()->getCurData().outValue;

bool edited=false;

for(uint8_t i=0;i<List.size();i++){
 if(List[i].inValue==corrS.inValue){
   List[i].outValue=corrS.outValue;
   edited=true;
   break;
   }
 }

if(!edited) {
 List+=corrS;
 }

getMotionDevice()->getModulePlanner()->getSpindle()->setDataList(List);

setPercentS(100.0);
setSOut(getSTar());
}

void WLGMachine::clearSCorList()
{
//m_correctSList.clear();
WLModulePlanner *Planner=getMotionDevice()->getModulePlanner();

QList<WLSpindleData> list=Planner->getSpindle()->getDataList();
QList<WLSpindleData> newList;

if(!list.isEmpty()) {
  newList+=list.first();
  newList+=list.last();
  }

getMotionDevice()->getModulePlanner()->getSpindle()->setDataList(newList);
}


WLGDrive *WLGMachine::getDrive(QString nameDrive,bool send)
{
WLDrive *drive=WLDrive::getDrive(nameDrive);

if(drive!=nullptr)
{
if(strcmp(drive->metaObject()->className(),"WLGDrive")==0)
             return static_cast<WLGDrive*>(drive);
}

if(!drive&&send)
    setMessage(nameClass(),tr("error name drive"),-1);

return nullptr;
}

void WLGMachine::setEnable(bool on)
{
qDebug()<<"WLGMachine::setOn"<<on;

if(!on) //OFF
 {
 reset();

 m_MScript->runFunction("OFF()",true);
 m_LScript->runFunction("OFF()",true);

 m_MScript->setEnable(false);
 m_LScript->setEnable(false);

 if(getMPG())
     getMPG()->setEnable(false);

 foreach(WLDrive *drive,getDrives()){
    drive->setEnable(false);
    }

 stopMov();
 }
 else{ //ON

  foreach(WLDrive *drive,getDrives()){
     drive->setEnable(true);
     }

  m_LScript->setBaseCode(loadLScript());
  m_MScript->setBaseCode(loadMScript());

  m_LScript->setEnable(true);
  m_MScript->setEnable(true);

  m_MScript->runFunction("ON()",true);
  m_LScript->runFunction("ON()",true);

  WLDrive::setMainPadDrives();

  QVector <quint8> indexsAxis;

  QStringList List=QString("X,Y,Z,A,B,C").split(","); //only this name

  foreach(WLGDrive *mD, getGDrives()){
      if(List.indexOf(mD->getName()) != -1 && mD->getAxis())
          indexsAxis+=mD->getAxis()->getIndex();
      }

  WLModulePlanner *ModulePlanner=getMotionDevice()->getModulePlanner();

  qDebug()<<"WLModulePlanner indexs size"<<indexsAxis.size();
  if(ModulePlanner) ModulePlanner->setIAxisSlave(indexsAxis.data(),indexsAxis.size());

  if(getMPG())
  {
   if(isUseMPG())  {
    auto GDrives=getGDrives();
    for(quint8 i=0;i<GDrives.size();i++) {
       if(GDrives[i]->getAxis())
           getMPG()->setDataAxis(i,GDrives[i]->getAxis()->getIndex(),1.0f/getMPG()->getPulses()/GDrives[i]->dimension());
       }

    getMPG()->setEnable(true);
    }
   else {
    getMPG()->setEnable(false);
    }
  }
 }

updatePosible();

WLMachine::setEnable(on);
}


bool WLGMachine::isPossiblyManual()
{
WLModulePlanner *MPlanner=getMotionDevice()->getModulePlanner();

if(MPlanner)
{
return   (MPlanner->getStatus()==PLANNER_stop
        ||MPlanner->getStatus()==PLANNER_pause)
         &&WLMachine::isPossiblyManual();
}

return WLMachine::isPossiblyManual();
}

bool WLGMachine::isPossiblyEditModeRun()
{
WLModulePlanner *MPlanner=getMotionDevice()->getModulePlanner();

if(MPlanner)
{
return  (MPlanner->getStatus()==PLANNER_stop
       ||MPlanner->getStatus()==PLANNER_pause
       ||MPlanner->getStatus()==PLANNER_waitStart);
}

return true;
}

long WLGMachine::getTrajIProgramSize()
{
QMutexLocker locker(&MutexMillTraj);

return MillTraj.isEmpty() ? 0 : (MillTraj.last().index-MillTraj.first().index);
}

void WLGMachine::updateGModel()
{
SAxisGModel GA=m_GModel.getA();
SAxisGModel GB=m_GModel.getB();
SAxisGModel GC=m_GModel.getC();

if(getDrive("A")){
 GA.rotary = (getDrive("A")->getType()==WLDrive::Rotary);
 GA.inf= getDrive("A")->isInfinity();
 }else{
 GA.rotary = true;
 GA.inf = false;
 }

if(getDrive("B")){
 GB.rotary = (getDrive("B")->getType()==WLDrive::Rotary);
 GB.inf= getDrive("B")->isInfinity();
 }else{
 GB.rotary = true;
 GB.inf = false;
 }

if(getDrive("C")){
 GC.rotary = (getDrive("C")->getType()==WLDrive::Rotary);
 GC.inf= getDrive("C")->isInfinity();
 }else{
 GC.rotary = true;
 GC.inf = false;
 }

m_GModel.setA(GA);
m_GModel.setB(GB);
m_GModel.setC(GC);
}

void WLGMachine::init(void)
{
qDebug()<<"Init GMachine"<<thread();

if(!QDir(configGMPath).exists())
    QDir().mkdir(configGMPath);

motDevice=new WLMotion;

connect(motDevice,SIGNAL(sendMessage(QString,QString,int)),this,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);

loadConfig();

updateMainDimXYZ();

updateGModel();

initMScript();
initLScript();
initJoystick();
initHeightMap();

if(m_MScript&&m_LScript)
 {
 m_MScript->addObject(m_LScript,"LSCRIPT");
 m_LScript->addObject(m_MScript,"MSCRIPT");
 }

initValuesScript();

QTimer *autoSaveTimer = new QTimer;
connect(autoSaveTimer,SIGNAL(timeout()),SLOT(saveConfig()));
autoSaveTimer->start(5*60*1000);

connect(&m_GCode,SIGNAL(changedSC(int)),SLOT(saveConfig()));

if(motDevice->isValidProtocol()
  ||(!motDevice->isReady())){ //no device
   setReady(true);
   }else{
   motDevice->closeConnect();   
   };

}

void WLGMachine::updateBusy() {

bool last=m_busy;

m_busy=isRunGProgram()||isActiv()||isActivDrives()||isRunMScript();

qDebug()<<"WLGMachine::updateBusy()"<<"runProgram"<<isRunGProgram()<<"isActiv"<<isActiv()<<"activDrive"<<isActivDrives()<<"runScript"<<isRunMScript();

if(last!=m_busy)
    emit changedBusy(m_busy);
}

bool WLGMachine::isEmptyMotion()
{
    WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

    if(ModulePlanner)
        return     MillTraj.isEmpty()
                && ModulePlanner->isEmpty()
                &&!ModulePlanner->isMoving()
                &&!(isRunGProgram()&&!isCompleteGProgram());
else
    return true;
}

float WLGMachine::getCurFxyz()
{
float sum=0;

foreach(WLDrive *drive,getDrives())
  {
  if(drive->getName()=="X"
   ||drive->getName()=="Y"
   ||drive->getName()=="Z")  sum+=pow(drive->getVnow(),2);

  }

return sqrt(sum);
}

float WLGMachine::getCurSOut()
{
return getMotionDevice()->getModulePlanner()->getSpindle()->getCurData().inValue;
}


bool WLGMachine::isProbe()
{
    if(getMotionDevice()->getModulePlanner()){
        return getMotionDevice()->getModulePlanner()->isProbe2()
                ||getMotionDevice()->getModulePlanner()->isProbe3();
    }
    else {
        return false;
    }
}

double WLGMachine::getProbe(QString name)
{
if(getMotionDevice()->getModulePlanner()){
        WLDrive *drive=getDrive(name);
        if(drive){
            return getProbeGPoint().get(drive->getName());
        }
    }
return 0;
}


void WLGMachine::updateMainDimXYZ()
{
double minDim;

QList <WLDrive*> drivesXYZ;

if(WLDrive::getDrive("X")) drivesXYZ+=WLDrive::getDrive("X");
if(WLDrive::getDrive("Y")) drivesXYZ+=WLDrive::getDrive("Y");
if(WLDrive::getDrive("Z")) drivesXYZ+=WLDrive::getDrive("Z");

if(drivesXYZ.isEmpty()) return;

if(drivesXYZ.size()==1)
{
m_mainDim=drivesXYZ.first()->getDriveDim().valueReal;
}
else
{
minDim=drivesXYZ.first()->getDriveDim().valueReal;

for(int i=1;i<drivesXYZ.size();i++)
    if(drivesXYZ[i]->getDriveDim().valueReal<minDim)	minDim=drivesXYZ[i]->getDriveDim().valueReal;

m_mainDim=minDim;

for(int i=0;i<drivesXYZ.size();i++)
    {
    drivesXYZ[i]->setKGear(m_mainDim/(drivesXYZ[i]->getDriveDim().valueReal));
    }

m_mainDim/=1<<xPD;
}

//WLModuleWhell *ModuleWhell=motDevice->getModuleWhell();
qDebug()<<"MainDimm"<<m_mainDim;
}

void WLGMachine::setSafeProbe(bool enable)
{
m_safeProbe=enable;

if(isRunGProbe())
  getMotionDevice()->getModulePlanner()->setActSafeProbe(WLIOPut::INPUT_actNo);
else
  getMotionDevice()->getModulePlanner()->setActSafeProbe(m_safeProbe ? getActSafeProbe() : WLIOPut::INPUT_actNo);
}

void WLGMachine::initMScript()
{
qDebug()<<"WLGMachine::initMScript()";

m_MScript->addObject(this,"MACHINE");
m_MScript->addObject(getGCode(),"GCODE");

m_MScript->addObject(getMPG(),"MPG");

connect(m_MScript,SIGNAL(sendMessage(QString,QString,int)),this,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);
connect(m_MScript,SIGNAL(complete(QString)),SLOT(setCompleteScript(QString)),Qt::QueuedConnection);

connect(m_MScript,&WLEVScript::changedBusy,this,&WLGMachine::updatePosible);

m_MScript->setIncludePath(_scriptPath);
m_MScript->setBaseCode(loadMScript(),true);

connect(m_Program,&WLGProgram::changedProgram,this
        ,[=](){m_MScript->runFunction(QString("changedGProgram()"),true);});

}

void WLGMachine::initLScript()
{
qDebug()<<"WLGMachine::initLScript()";

m_LScript->addObject(this,"MACHINE");
m_LScript->addObject(getGCode(),"GCODE");

m_LScript->addObject(getMPG(),"MPG");

connect(m_LScript,SIGNAL(sendMessage(QString,QString,int)),this,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);
connect(m_LScript,&WLEVScript::error,this,[=](QString txt){
              m_LScript->reset();
              m_LScript->setEnable(false);
              sendMessage("LScript","set disable: "+txt,-1);
              });

m_LScript->setIncludePath(_scriptPath);
m_LScript->setBaseCode(loadLScript(),true);

m_LScript->setEnableTimerTask();

WLModuleIOPut *MIOPut=getMotionDevice()->getModuleIOPut();

if(MIOPut)
  {
  connect(MIOPut,&WLModuleIOPut::changedInput,this
          ,[=](int index){m_LScript->runFunction(QString("changedInput(%1,%2)").arg(index).arg(MIOPut->getInput(index)->getNow()),true);});

  connect(MIOPut,&WLModuleIOPut::changedOutput,this
          ,[=](int index){m_LScript->runFunction(QString("changedOutput(%1,%2)").arg(index).arg(MIOPut->getOutput(index)->getNow()),true);});

  }

WLModuleUART *MUART=getMotionDevice()->getModuleUART();

if(MUART)
 {
 m_LScript->addObject(MUART,"MUART");
 }

connect(m_Program,&WLGProgram::changedProgram,this
        ,[=](){m_LScript->runFunction(QString("changedGProgram()"),true);});


WLModuleAxis *MAxis=getMotionDevice()->getModuleAxis();

if(MAxis)
 {
 m_LScript->addObject(MAxis,"MAXIS");
 }
}

void WLGMachine::initJoystick()
{
connect(&m_Joysticks,&WLJoysticks::changedPOVJoystick,[=](int id,int pov,int angle)
{
if(m_LScript!=nullptr
 &&this->isEnable()){
  m_LScript->runScript(QString("changedPOVJoystick(%1,%2,%3)").arg(id).arg(pov).arg(angle),true);
  }
});

connect(&m_Joysticks,&WLJoysticks::changedButtonJoystick,[=](int id,int button,bool press)
{
if(m_LScript!=nullptr
 &&this->isEnable()){
  m_LScript->runScript(QString("changedButtonJoystick(%1,%2,%3)").arg(id).arg(button).arg(press),true);
  }
});

connect(&m_Joysticks,&WLJoysticks::changedAxisJoystick,[=](int id,int axis,double value)
{
if(m_LScript!=nullptr
 &&this->isEnable()){
  m_LScript->runScript(QString("changedAxisJoystick(%1,%2,%3)").arg(id).arg(axis).arg(value),true);
  }
});

m_LScript->addObject(&m_Joysticks,"JOYSTICK");
m_MScript->addObject(&m_Joysticks,"JOYSTICK");
}

void WLGMachine::initHeightMap()
{
m_MScript->addObject(&m_HeightMap,"HMAP");
m_LScript->addObject(&m_HeightMap,"HMAP");
}

void WLGMachine::initValuesScript()
{

}

void WLGMachine::stopMov() //полная остановка
{
qDebug()<<"WLGMachine::stopMov() MillTraj.size()="<<MillTraj.size();

QMutexLocker locker(&MutexMillTraj);

if(isRunGProgram()&&!isEmptyMotion())
    m_sStop=true;

MillTraj.clear();

MutexShowTraj.lock();
showMillTraj.clear(); 
MutexShowTraj.unlock();

emit changedTrajSize(MillTraj.size());

resetAuto();

if(!isRunMScript()
 &&isEnable())
 {
 setIgnoreInPause(false);
 setIgnoreInStop(false);
 }

m_pause=
m_runGProgram=false;

WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

if(ModulePlanner)
     ModulePlanner->stopMov();

SDStop();

QTimer::singleShot(0,this,SLOT(setFinished()));//???
}



void WLGMachine::reset()
{
qDebug()<<"WLGMachine::reset()";

if(isRunGProgram()) {
   sendMessage("WLGMachine::reset() program: \""+m_Program->getName()+"\"",QString(" element: %1").arg(m_Program->getLastMovElement()),1);
   }

MillTraj.clear();
m_elementsTime=0;

MutexShowTraj.lock();
showMillTraj.clear();
MutexShowTraj.unlock();

baseTraj.clear();

m_runList=
m_continue=
m_waitMScript=
m_waitPause=
m_sStop=
m_runGProgram=false;

stopMov();

m_iProgram=0;

if(m_MScript)
  {
  m_MScript->reset();

  if(m_MScript->isEnable())
    {
    m_MScript->runFunction("RESET()",false);
    }
  //  else {
  //  m_MScript->setEnable(true);
  //  m_MScript->runFunction("RESET()",false);
  //  m_MScript->setEnable(false);
  //  }
  }

emit changedPause(m_pause=false);
emit changedReadyRunList(m_readyRunList=false);

updateBusy();
updatePosible();
}

void WLGMachine::startMov()
{
qDebug()<<"WLGMachine::startMov";

if(isPause())  {
  setPause(false);
  return;
  }
  else {
  if(isEmptyMotion())  return;
  }

QMutexLocker locker(&MutexStart);

WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

if(getInPause()&&!isIngnoreInPause()) //нажата ли пауза
    {
    sendMessage("WLGMachine","wrong state inPause",0);
    //reset();
    return;
    }

if(getInStop()&&!isIngnoreInStop())
    {
    sendMessage("WLGMachine","wrong state inStop",0);
    //reset();
    return;
    }

if(!verifyReadyMotion())
    {
    //reset();
    return;
    }

if(m_waitMScript&&isRunMScript()) return;

saveConfig();

if((ModulePlanner->getStatus()==PLANNER_pause
  ||ModulePlanner->getStatus()==PLANNER_stop)
 &&!m_runList)
{    
 if(!isEmptyMotion()
   ||isRunMScript())
   {
   //Flag.set(ma_runlist,1);

   if(m_continue)
    {
    //if(isRunScript())
    //     continueMovList();
    //  else
         runMScript("CONTINUE()");
    }
    else
    {
    m_programTime.start();
    m_elementsTime=m_iProgram;

    foreach(WLDrive *mD,getDrives())
       {
       mD->setMainPad();

       if(mD->getAxis()) {

        mD->getAxis()->clearMParList();

        QList <dataPad> padList=mD->pad()->getDataList();

        foreach(dataPad pad,padList){
            if(pad.name=="fast"
             ||pad.name=="minusFast"){
              mD->getAxis()->addMParList(pad.Aac/mD->getDriveDim().value
                                        ,pad.Ade/mD->getDriveDim().value
                                        ,pad.Vst/mD->getDriveDim().value
                                        ,pad.Vma/mD->getDriveDim().value
                                        ,pad.name);
             }

            }
        }
       }

    if(getDrive("Z"))
       ModulePlanner->setHPause(isUseHPause(),m_hPause/getDrive("Z")->dimension());

    ModulePlanner->startMov();
    updateMovPlanner();
    }
   }
}
else
 ModulePlanner->startMov();

m_sStop=false;

ModulePlanner->update();//по нему срабатывает ложно если нет номеров элемента
}


QString WLGMachine::correctSOut()
{
QString ret;
for(int i=0;i<m_correctSList.size();i++)
    {
	if(i!=0) ret+=",";

    ret+=QString::number(m_correctSList[i].Sadj)+","+QString::number(m_correctSList[i].Scor);
    }

return ret;
}

void WLGMachine::setStringCorrectSOut(QString str)
{
QStringList List=str.split(",");
SCorrectSOut Scorr;

m_correctSList.clear();

for(int i=1;i<List.size();i+=2)
  {
  Scorr.Sadj=List[i-1].toDouble();
  Scorr.Scor=List[i].toDouble();

  m_correctSList+=Scorr;
  }

//sortCorrectSList();
}


void WLGMachine::saveConfig()
{
QMutexLocker locker(&MutexSaveConfig);

QFile FileXML(configGMFile);

QByteArray Data;

QXmlStreamWriter stream(&Data);

if(!isReady()) return;

stream.setAutoFormatting(true);

stream.setCodec(QTextCodec::codecForName("Windows-1251"));
stream.writeStartDocument("1.0");

stream.writeStartElement("WhiteLineMillConfig");

//stream.writeAttribute("usePWMS",QString::number(isUsePWMS()));
//stream.writeAttribute("useCorrectSOut",QString::number(isUseCorrectSOut()));
stream.writeAttribute("strFindDrivePos",getStrFindDrivePos());

//stream.writeAttribute("correctSOut",correctSOut());

//stream.writeAttribute("MinMaxS",QString::number(m_minS)+","+QString::number(m_maxS));
//stream.writeAttribute("MinMaxOutS",QString::number(m_minSOut)+","+QString::number(m_maxSOut));
//stream.writeAttribute("iCurTool",QString::number(m_curTool));
stream.writeAttribute("autoStartGCode",QString::number(isAutoStartGCode()));
stream.writeAttribute("autoSetSafeProbe",QString::number(isAutoSetSafeProbe()));

stream.writeAttribute("actSafeProbe",QString::number(getActSafeProbe()));

stream.writeAttribute("useGModel",QString::number(isUseGModel()));

if(isUseHPause())
stream.writeAttribute("hPause",QString::number(HPause()));

stream.writeAttribute("useMPG",QString::number(isUseMPG()));

stream.writeAttribute("feedVBacklash",  QString::number(VBacklash()));
//stream.writeAttribute("feedVProbe",     QString::number(VProbe()));

stream.writeAttribute("ContinueMov",QString::number(isContinueMov()));
stream.writeAttribute("BacklashNextMov",QString::number(isBLNextMov()));

stream.writeAttribute("percentManualStr",getPercentManualStr());

stream.writeAttribute("G1StartAt",QString("%1,%2").arg(getDistG1StartAt()).arg(getFeedG1StartAt()));

 stream.writeStartElement("Device");
     stream.writeAttribute("WLMotion",motDevice->getNameDevice());

 stream.writeEndElement();

 stream.writeStartElement("Drive");

 foreach(WLGDrive *drive,getGDrives())
 {
 stream.writeStartElement(drive->metaObject()->className());
   drive->writeXMLData(stream);
 stream.writeEndElement();
 }

 stream.writeEndElement();

 stream.writeStartElement("Position");
 stream.writeAttribute("G28",m_GCode.getG28Position().toString());
 stream.writeAttribute("G43",m_GCode.getG43Position().toString());
 stream.writeAttribute("offsetHTool",QString::number(getOffsetHTool()));
 stream.writeEndElement();

 stream.writeStartElement("GModel");
  m_GModel.writeXMLData(stream);
 stream.writeEndElement();
/*
 for(int i=0;i<sizeSC;i++)
 {
 stream.writeStartElement("SC");
 stream.writeAttribute("i",QString::number(i));

 stream.writeAttribute("GPoint",m_GCode.getOffsetSC(i).toString());
 stream.writeAttribute("refPoint0",m_GCode.getRefPoint0SC(i).toString());
 stream.writeAttribute("refPoint1",m_GCode.getRefPoint1SC(i).toString());
 stream.writeEndElement();
 }
*/

 stream.writeStartElement("GCode");
 getGCode()->writeXMLData(stream);
 stream.writeEndElement();

stream.writeEndElement();

stream.writeEndDocument();

if(FileXML.open(QIODevice::WriteOnly))
{
FileXML.write(Data.constData());
FileXML.close();
}

motDevice->writeToFile(configGMPath+motDevice->getNameDevice()+".xml");

getGCode()->writeToolFile(toolsFile);
getGCode()->writeSCFile(_scFile);
}

void WLGMachine::saveMScript(QString txt)
{
QFile FileMJS(_mScriptFile);
qDebug()<<"WLGMachine::saveMScript";

QDir().mkdir(_scriptPath);

if(FileMJS.open(QIODevice::WriteOnly))            {
      FileMJS.write(QTextCodec::codecForName("Windows-1251")->fromUnicode(txt));
      FileMJS.close();
      }
}

void WLGMachine::saveLScript(QString txt)
{
QFile FileLJS(_lScriptFile);
qDebug()<<"WLGMachine::saveLScript()";

QDir().mkdir(_scriptPath);

if(FileLJS.open(QIODevice::WriteOnly))            {
      FileLJS.write(QTextCodec::codecForName("Windows-1251")->fromUnicode(txt));
      FileLJS.close();
}
}

QString WLGMachine::loadMScript()
{
QFile FileMJS(_mScriptFile);
QString codeMScript;
bool saveF=false;

if(!FileMJS.open(QIODevice::ReadOnly|QIODevice::Text))  {
    FileMJS.setFileName(mScriptFile);

    saveF=true;

    if(!FileMJS.open(QIODevice::ReadOnly|QIODevice::Text))  {
       FileMJS.setFileName(":/data/wlmillconfig/script/mscript.js");
       FileMJS.open(QIODevice::ReadOnly|QIODevice::Text);
       }
    }

if(FileMJS.isOpen()){
    codeMScript=QTextCodec::codecForName("Windows-1251")->toUnicode(FileMJS.readAll());

    if(codeMScript.isEmpty()){
      FileMJS.close();
      FileMJS.setFileName(":/data/wlmillconfig/script/mscript.js");
      FileMJS.open(QIODevice::ReadOnly|QIODevice::Text);
      codeMScript=QTextCodec::codecForName("Windows-1251")->toUnicode(FileMJS.readAll());

      saveF=true;
      }

    FileMJS.close();
    }


if(saveF)
   saveMScript(codeMScript);

return codeMScript;
}


QString WLGMachine::loadLScript()
{
QFile FileLJS(_lScriptFile);
QString codeLScript;
bool saveF=false;

if(!FileLJS.open(QIODevice::ReadOnly|QIODevice::Text))  {
    FileLJS.setFileName(lScriptFile);

    saveF=true;

    if(!FileLJS.open(QIODevice::ReadOnly|QIODevice::Text))  {
       FileLJS.setFileName(":/data/wlmillconfig/script/lscript.js");
       FileLJS.open(QIODevice::ReadOnly|QIODevice::Text);
       }
    }

if(FileLJS.isOpen()){
    codeLScript=QTextCodec::codecForName("Windows-1251")->toUnicode(FileLJS.readAll());

    if(codeLScript.isEmpty()){
      FileLJS.close();
      FileLJS.setFileName(":/data/wlmillconfig/script/lscript.js");
      FileLJS.open(QIODevice::ReadOnly|QIODevice::Text);
      codeLScript=QTextCodec::codecForName("Windows-1251")->toUnicode(FileLJS.readAll());

      saveF=true;
      }

    FileLJS.close();
    }


if(saveF)
   saveLScript(codeLScript);

return codeLScript;
}

void WLGMachine::setMessage(QString name, QString data, int code)
{
qDebug()<<"WLGMachine::setMessage"<<code;

if(code<0) {
 emit error();
 reset();
 }
 else if(code==0)
      setPause(1);


 emit sendMessage(name,data,code);
}


void WLGMachine::addDrive(WLGDrive *millDrive)
{
qDebug()<<"WLGMachine::addDrive"<<millDrive->getName();

millDrive->moveToThread(this->thread());

connect(millDrive,SIGNAL(finished()),this,SLOT(setFinished()),Qt::QueuedConnection);
connect(millDrive,SIGNAL(paused()),this,SLOT(setFinished()),Qt::QueuedConnection);

connect(millDrive,SIGNAL(sendMessage(QString,QString,int)),this,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);

connect(millDrive,&WLDrive::changedAuto,this,[=](){updateBusy();},Qt::QueuedConnection);

WLMachine::addDrive(millDrive);
}

void WLGMachine::removeDrive(WLGDrive *millDrive)
{
disconnect(millDrive,nullptr,nullptr,nullptr);
WLMachine::removeDrive(millDrive);
}

bool WLGMachine::loadConfig()
{
QMutexLocker locker(&MutexSaveConfig);
QFile FileXML(configGMFile);
QXmlStreamReader stream;
QStringList List;
int i=0;

qDebug()<<"load config GMachine";

if(!FileXML.open(QIODevice::ReadOnly))    {
           FileXML.setFileName(":/data/wlmillconfig/mmconfig.xml");
           FileXML.open(QIODevice::ReadOnly);
           }

if(FileXML.isOpen())
  {
  stream.setDevice(&FileXML);

  while(!stream.atEnd())
   {
   stream.readNextStartElement();

   if(stream.name()=="WhiteLineMillConfig")
     {

	 if(!stream.attributes().value("MinMaxOutS").isEmpty()) 
	     { 
		 List=stream.attributes().value("MinMaxOutS").toString().split(",");
		 setRangeSOut(List[0].toFloat(),List[1].toFloat());
	     }

	 if(!stream.attributes().value("MinMaxS").isEmpty()) 
	     { 
		 List=stream.attributes().value("MinMaxS").toString().split(",");
		 setRangeS(List[0].toFloat(),List[1].toFloat());
	     }


     if(!stream.attributes().value("G1StartAt").isEmpty())
         {
         List=stream.attributes().value("G1StartAt").toString().split(",");

         if(List.size()==2){
           setDistG1StartAt(List[0].toDouble());
           setFeedG1StartAt(List[1].toDouble());
         }
         }


     if(!stream.attributes().value("AutoStart").isEmpty())  //OLD 18/12/21
         setAutoStartGCode(stream.attributes().value("AutoStart").toInt());

     if(!stream.attributes().value("autoStartGCode").isEmpty())
         setAutoStartGCode(stream.attributes().value("autoStartGCode").toInt());

     if(!stream.attributes().value("autoSetSafeProbe").isEmpty())
         setAutoSetSafeProbe(stream.attributes().value("autoSetSafeProbe").toInt());     

     if(!stream.attributes().value("actSafeProbe").isEmpty())
         setActSafeProbe(static_cast<WLIOPut::typeActionInput>(stream.attributes().value("actSafeProbe").toString().toInt()));

     if(!stream.attributes().value("hPause").isEmpty())
        setHPause(stream.attributes().value("hPause").toString().toFloat());
	 

	 if(!stream.attributes().value("ContinueMov").isEmpty()) 
	     setContinueMov(stream.attributes().value("ContinueMov").toString().toInt());

	 if(!stream.attributes().value("BacklashNextMov").isEmpty()) 
		 setBLNextMov(stream.attributes().value("BacklashNextMov").toString().toInt());

     if(!stream.attributes().value("useGModel").isEmpty())
         setEnableGModel(stream.attributes().value("useGModel").toInt());

     if(!stream.attributes().value("hPause").isEmpty())
         {
         setEnableHPause(true);
         setHPause(stream.attributes().value("hPause").toString().toFloat());
         }

     if(!stream.attributes().value("useMPG").isEmpty())
         {
         setUseMPG(stream.attributes().value("useMPG").toInt());
         }

     if(!stream.attributes().value("strFindDrivePos").isEmpty())
         setStrFindDrivePos(stream.attributes().value("strFindDrivePos").toString());

	 if(!stream.attributes().value("feedVBacklash").isEmpty()) 
		 setFeedVBacklash(stream.attributes().value("feedVBacklash").toString().toFloat());

      if(!stream.attributes().value("correctSOut").isEmpty())
	    setStringCorrectSOut(stream.attributes().value("correctSOut").toString()); 

      if(!stream.attributes().value("percentManualStr").isEmpty())
        setPercentManualStr(stream.attributes().value("percentManualStr").toString());

/*
     if(!stream.attributes().value("SmoothDist").isEmpty())
		 {
         m_smoothDist=stream.attributes().value("SmoothDist").toString().toFloat();
         if(m_smoothDist<0) m_smoothDist=0;
		   else
             if(m_smoothDist>5) m_smoothDist=5;
         }

	 if(!stream.attributes().value("SimpliDist").isEmpty()) 
		 {
         setSimpliDist(stream.attributes().value("SimpliDist").toString().toFloat());
         }
*/
	 while(!stream.atEnd())
	 { 
	  stream.readNextStartElement();

	  if(stream.name()=="WhiteLineMillConfig") break;
	  if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

	  if(stream.name()=="Device"
	  &&stream.tokenType()==QXmlStreamReader::StartElement)
	   {


       if(!stream.attributes().value("WLMotion").isEmpty())
            {
            QString initFile=configGMPath+stream.attributes().value("WLMotion").toString()+".xml";

            WLMotion WLMDev;

            if(!WLMDev.initFromFile(initFile))
              {
              initFile=":/data/wlmillconfig/WLM35A.xml";
              WLMDev.initFromFile(initFile);
              }

            auto infoList=WLDevice::availableDevices();

            bool findOk=false;

            foreach(WLDeviceInfo info,infoList)
            {
            if(info.isValid(WLMDev.getInfo()))
               {
               findOk=true;

               motDevice->setInfo(info);
               motDevice->openConnect();

               if(!motDevice->waitForReady())
                   {
                   motDevice->closeConnect();
                   emit sendMessage("WLGMachine",tr("device %1 not ready.").arg(motDevice->getNameDevice())+" ("+motDevice->getUID96()+")",0);
                   }
               else
                   {
                   findOk=true;                   
                   }

               break;
               }
            }

            qDebug()<<"find Device"<<findOk<<"open"<<motDevice->isOpenConnect();

            if(!motDevice->initFromFile(initFile))
                emit sendMessage("WLGMachine",tr("error read file device: ")+initFile,0);

            if(motDevice->getModuleConnect())
                             motDevice->getModuleConnect()->setEnableHeart(true);

            if(!findOk
             &&!motDevice->getUID96().isEmpty())
                {
                emit sendMessage("WLGMachine",tr("device %1 not found.").arg(motDevice->getNameDevice())+" ("+motDevice->getUID96()+")",0);
                }
            }

       }


	  if(stream.name()=="Outputs"
	  &&stream.tokenType()==QXmlStreamReader::StartElement)
	   {
	   QStringList List;

	     while(!stream.atEnd())
	      {
          stream.readNextStartElement();
	  	
	  	  if(stream.name()=="Outputs") break;
	  	  if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;
	   
	  	///if(stream.name()=="WLMotion") motDevice->readXMLData(stream);
	     }
	   }	

      if(stream.name()=="Drive")
       {
       if(motDevice->getModuleAxis())
           while(!stream.atEnd())
            {
            stream.readNextStartElement();

            if(stream.name()=="Drive") break;
            if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;


            if(stream.name()=="WLMillDrive"
             ||stream.name()=="WLGDrive")
             {
             WLGDrive *GDrive = new WLGDrive("",motDevice->getModuleAxis());

             GDrive->readXMLData(stream);
             addDrive(GDrive);
             }

            }
      }

//--old Style
    if(stream.name()=="HomePos"){
         WLGPoint GP;
         GP.fromString(stream.attributes().value("GPoint").toString());
         m_GCode.setG28Position(GP);
         continue;
         }
//---
    if(stream.name()=="Position")
         {
         WLGPoint GP;

         if(!stream.attributes().value("G28").isEmpty()){
           GP.fromString(stream.attributes().value("G28").toString());
           m_GCode.setG28Position(GP);
           }

         if(!stream.attributes().value("G43").isEmpty()){
           GP.fromString(stream.attributes().value("G43").toString());
           m_GCode.setG43Position(GP);
           }

         if(!stream.attributes().value("offsetHTool").isEmpty()){
           setOffsetHTool(stream.attributes().value("offsetHTool").toDouble());
           }

         continue;
         }

    if(stream.name()=="GModel"){
         m_GModel.readXMLData(stream);
         continue;
         }

    //--old
    if(stream.name()=="SC"){
		 qDebug()<<"loadSC";
		 WL3DPoint SC;

		 i=stream.attributes().value("i").toString().toInt();

         WLGPoint GP;

         if(!stream.attributes().value("Frame").isEmpty())
           {
           WLFrame Fr;
           Fr.fromString(stream.attributes().value("Frame").toString());

           GP.x=Fr.x;
           GP.y=Fr.y;
           GP.z=Fr.z;
           m_GCode.setOffsetSC(i,GP,false);

           Fr.fromString(stream.attributes().value("refPoint0").toString());
           GP.x=Fr.x;
           GP.y=Fr.y;
           GP.z=Fr.z;
           m_GCode.setRotPoint0SC(i,GP);

           Fr.fromString(stream.attributes().value("refPoint1").toString());
           GP.x=Fr.x;
           GP.y=Fr.y;
           GP.z=Fr.z;
           m_GCode.setRotPoint1SC(i,GP);
           }
           else
           {
           GP.fromString(stream.attributes().value("GPoint").toString());
           m_GCode.setOffsetSC(i,GP,false);

           GP.fromString(stream.attributes().value("refPoint0").toString());
           m_GCode.setRotPoint0SC(i,GP);

           GP.fromString(stream.attributes().value("refPoint1").toString());
           m_GCode.setRotPoint1SC(i,GP);
           continue;
           }
	     }
      //---
        if(stream.name()=="Tool")
         {
         qDebug()<<"loadTool";

         WLEData Tool;

         if(!stream.attributes().value("Data").isEmpty())
           {
           QStringList list =  stream.attributes().value("Data").toString().split(",");

           Tool.insert("index",stream.attributes().value("i").toString());

           if(list.size()==3)
               {
               Tool.insert("Diam",list[0]);
               Tool.insert("D",list[1]);
               Tool.insert("H",list[2]);
               }
              else if(list.size()==3)
                    {
                    Tool.insert("D",list[0]);
                    Tool.insert("H",list[1]);
                    }
           }
           else {
           foreach(QXmlStreamAttribute attr,stream.attributes()){
             Tool.insert(attr.name().toString(),attr.value().toString());
             }
           }

         getGCode()->setTool(Tool.value("index",getGCode()->getDataTool()->count()).toInt(),Tool);
         continue;
         }

        if(stream.name()=="GCode")
         {
         qDebug()<<"loadGCode";
         getGCode()->readXMLData(stream);
         continue;
         }
     
     }
	}
   }

  if(motDevice->getModuleAxis())
    {
    connect(motDevice->getModuleAxis(),SIGNAL(changedInEMGStop()),SLOT(updateInput()));
    connect(motDevice->getModuleAxis(),SIGNAL(changedInSDStop()),SLOT(updateInput()));
    }


  WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

  if(ModulePlanner)
  {
  connect(ModulePlanner,&WLModulePlanner::changedStatus,this,&WLGMachine::updateStatusMPlanner,Qt::DirectConnection);//Qt::QueuedConnection);
  connect(ModulePlanner,&WLModulePlanner::changedProbe,this,&WLGMachine::updateInProbe,Qt::DirectConnection);//Qt::QueuedConnection);

  connect(ModulePlanner,&WLModulePlanner::changedEmpty,this,&WLGMachine::setFinished,Qt::QueuedConnection);

  connect(ModulePlanner,&WLModulePlanner::reset,this,&WLGMachine::reset,Qt::QueuedConnection);
  connect(ModulePlanner,&WLModulePlanner::changedFree,this,&WLGMachine::setFinished,Qt::QueuedConnection);

  connect(motDevice->getModulePlanner(),SIGNAL(changedSOut(float)),SLOT(setTarSOut(float)));

  if(ModulePlanner->getSpindle()->getDataList().isEmpty()){ //переход на новый формат с 18,12,21
    QList <WLSpindleData> spindleDataList;
    WLSpindleData SD;

    foreach(SCorrectSOut scor,m_correctSList){
      SD.inValue=scor.Sadj;
      SD.outValue=((scor.Scor-m_minS)*(m_maxSOut-m_minSOut)/(m_maxS-m_minS)+m_minSOut)/100.0f;

      spindleDataList+=SD;
      }

    ModulePlanner->getSpindle()->setDataList(spindleDataList);

    ModulePlanner->getSpindle()->setAcc((m_maxS-m_minS)/((m_maxSOut-m_minSOut)/100/1000/ModulePlanner->getSpindle()->getAcc()));
    ModulePlanner->getSpindle()->setDec((m_maxS-m_minS)/((m_maxSOut-m_minSOut)/100/1000/ModulePlanner->getSpindle()->getDec()));
    }

  setSOut(m_GCode.getValue('S'));

  ModulePlanner->setModeRun(PLANNER_normal);  

  ModulePlanner->setEnableSOut(false);  
  ModulePlanner->stopMov();
  }

  getGCode()->readToolFile(toolsFile);

  if(QFile(scFile).exists()){
     getGCode()->readSCFile(scFile);
     QFile(scFile).remove();
     }
    else {
     getGCode()->readSCFile(_scFile);
     }

  updatePosible();

  return true;
  }



return false;
}

void WLGMachine::continueMovList()
{
m_continue=false;
//Flag.set(ma_runlist);

motDevice->getModulePlanner()->startMov();

updateMovPlanner();
}


void WLGMachine::setFinished()
{
qDebug()<<"WLGMachine::setFinished() actDrive"<<isActivDrives()
                                  <<"emptyMotion"<<isEmptyMotion()
                                  <<"runProgram"<<isRunGProgram()
                                  <<"auto"<<isAuto()
                                  <<"ready"<<isReady();

if(!isReady()) return;

QMutexLocker locker(&MutexFinished);

updateMovPlanner();

if(isAuto()){
 updateAuto();
 }
 else{
 if(isEmptyMotion()
    &&m_readyRunList
    &&(!isRunMScript())
    &&(!isActivDrives()))//if no mov data and last mov
     {
     if(m_runGProgram)
         {
         long h,m;
         float ms;
         long time_s=m_programTime.elapsed();

         h=time_s/3600000;
         time_s-=h*3600000;

         m=time_s/60000;
         time_s-=m*60000;

         ms=time_s;

         m_Program->setLastMovElement(0);
         sendMessage("WLGMachine",QString("\"%1\" time: %2:%3:%4 ").arg(m_Program->getName()).arg(h).arg(m).arg(ms/1000.0,1),1);
         }

     stopMov();
     emit changedReadyRunList(m_readyRunList=false);
     }

  updatePosible();
  }

updateBusy();
}

void WLGMachine::updateAuto()
{
if(isAuto()&&(m_typeAutoMMachine!=AUTO_no))
 {
 updateGMachineAuto();
 }
else
 {
 WLMachine::updateAuto();
 }
}

void WLGMachine::updateGMachineAuto()
{    
qDebug()<<"WLGMachine::updateGMachineAuto()";

QMutexLocker locker(&MutexMillAuto);

if(isAuto()&&!isPause())
switch(m_typeAutoMMachine)
 {
 case AUTO_DProbe:   /* updateProbe();  */  break;
 case AUTO_HProbe:   /* updateHProbe();    */break;
 case AUTO_HTProbe:  /* updateHToolProbe();*/break;
 case AUTO_GProbe:    updateGProbe();    break;
 default: break;
    //case AUTO_Program: return updateProgram();
 }

}

bool WLGMachine::updateGProbe()
{
WLModulePlanner *Planner=getMotionDevice()->getModulePlanner();

qDebug()<<"WLGMachine::updateGProbe() ioper="<<iOperation<<"size="<<GProbeList.size()<<Planner->isBusy()<<MillTraj.isEmpty();

SGProbe   *GProbe=nullptr;
int index;

for(index=0;index<GProbeList.size();index++)
    if(!GProbeList.at(index).ex)
    {
    GProbe=&GProbeList[index];
    break;
    }

if(!GProbe)
  {
  resetAuto();
  QTimer::singleShot(10,this,SLOT(setFinished()));
  return false;
  }

if(!Planner->isBusy()&&MillTraj.isEmpty())
switch(GProbe->type)
{
case probeXY:
              switch(iOperation)
              {
               case 0: if(getInProbe())
                          {
                          setMessage(tr("error state sensor signal"),"inProbe",-1);
                          resetAuto();
                          return false;
                          }

                        Planner->clear(); //едем к точке и опускаемся
                        Planner->resetProbe();
                        Planner->setActInProbe(SGProbe::typeStop);


                        runGCode(QString("G53G0G90X%1Y%2").arg(GProbe->awaitPoint.x-GProbe->dist*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                          .arg(GProbe->awaitPoint.y-GProbe->dist*sin(M_PI/180.0*GProbe->angle),0,'f',5));

                        runGCode(QString("G53G1G90Z%1F%2").arg(GProbe->awaitPoint.z,0,'f',5)
                                                          .arg(GProbe->F1,0,'f',3));

                        iOperation=5;
                        break;

              case 5: if(Planner->isProbe2()||getInProbe()) {
                        setMessage(tr("error sensor signal"),"inProbe",-5);//если наткнулись на припятствие при подходе то ошибка
                        }else{
                        Planner->resetProbe();                              //едем на касание
                        Planner->setActInProbe(SGProbe::typeStop);
                        runGCode(QString("G53G1G90X%1Y%2%F%3").arg(GProbe->awaitPoint.x+GProbe->sumDist()*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(GProbe->awaitPoint.y+GProbe->sumDist()*sin(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(GProbe->F1,0,'f',3));
                        }
                       iOperation=10;
                       break;

              case 10: if(Planner->isProbe2()){  //если было касание
                         if(GProbe->doubleProbe){//если нужно повторное,то отезжаем
                         iOperation=15;

                         if(SGProbe::backDist<0){

                            Planner->resetProbe();
                            Planner->setActInProbe(SGProbe::typeStop); //остановимся при отскоке

                            runGCode(QString("G53G0G90X%1Y%2").arg(getProbeGPoint().x+SGProbe::backDist*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(getProbeGPoint().y+SGProbe::backDist*sin(M_PI/180.0*GProbe->angle),0,'f',5));
                            }else{
                            runGCode(QString("G53G0G90X%1Y%2").arg(getProbeGPoint().x-SGProbe::backDist*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(getProbeGPoint().y-SGProbe::backDist*sin(M_PI/180.0*GProbe->angle),0,'f',5));
                            }
                         }else{
                         goto endeProbeXY;
                         }
                       }else{
                       GProbe->error=true;
                       iOperation=55;
                       setMessage(tr("no sensor signal"),"inProbe",-10);
                       }
                       break;

              case 15: if(Planner->isProbe3()
                       ||!getInProbe()) {//едем на касание повторно
                        iOperation=50;
                        Planner->resetProbe();
                        Planner->setActInProbe(SGProbe::typeStop);

                        runGCode(QString("G53G1G90X%1Y%2%F%3").arg(GProbe->awaitPoint.x+GProbe->sumDist()*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(GProbe->awaitPoint.y+GProbe->sumDist()*sin(M_PI/180.0*GProbe->angle),0,'f',5)
                                                              .arg(GProbe->F2,0,'f',3));
                        //runGCode(QString("G53G1G90X%1Y%2F%3").arg(getProbeGPoint().x+qAbs(SGProbe::backDist)*1.5*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                        //                                     .arg(getProbeGPoint().y+qAbs(SGProbe::backDist)*1.5*sin(M_PI/180.0*GProbe->angle),0,'f',5)
                        //                                     .arg(GProbe->F2,0,'f',5));;
                        }else{
                        GProbe->error=true;
                        iOperation=55;
                        setMessage(tr("no sensor signal"),"inProbe",-15);
                        }


                       break;


             case 50: if(Planner->isProbe2()){  //если было касание
                        endeProbeXY:

                        iOperation=0;

                        GProbe->probePoint=getProbeGPoint();

                        GProbe->probePoint.x+=SGProbe::headDiam/2.0*(cos(M_PI/180.0*GProbe->angle));
                        GProbe->probePoint.y+=SGProbe::headDiam/2.0*(sin(M_PI/180.0*GProbe->angle));

                        GProbe->ex=true;

                        runGCode(QString("G53G0G90X%1Y%2").arg(GProbe->awaitPoint.x-GProbe->dist*cos(M_PI/180.0*GProbe->angle),0,'f',5)
                                                          .arg(GProbe->awaitPoint.y-GProbe->dist*sin(M_PI/180.0*GProbe->angle),0,'f',5));



                        if((index==(GProbeList.size()-1)) //если последний
                         ||(GProbe->getStartXYStr()!=GProbeList[index+1].getStartXYStr())){ //или если разные точки начала
                          runGCode(QString("G53G0G90Z%1").arg(GProbe->Zback,0,'f',5));
                          }
                        }else{
                        GProbe->error=true;
                        iOperation=55;
                        setMessage(tr("no sensor signal"),"inProbe",-50);
                        }
                        break;

               case 55: resetAuto();
                        QTimer::singleShot(10,this,SLOT(setFinished()));
                        break;
               }

              break;
case probeZ:
              switch(iOperation)
              {
               case 0: if(getInProbe())
                          {
                          setMessage(tr("error state sensor signal"),"inProbe",-1);
                          resetAuto();
                          return false;
                          }

                       Planner->clear();
                       Planner->resetProbe();
                       Planner->setActInProbe(SGProbe::typeStop);


                       runGCode(QString("G53G0G90X%1Y%2").arg(GProbe->awaitPoint.x,0,'f',5)
                                                         .arg(GProbe->awaitPoint.y,0,'f',5));

                       runGCode(QString("G53G0G90Z%1F%2").arg(GProbe->awaitPoint.z+GProbe->dist,0,'f',5)
                                                          .arg(GProbe->F1,0,'f',5));

                       iOperation=5;
                       break;

              case 5: if(Planner->isProbe2()||getInProbe()){
                        setMessage(tr("error sensor signal"),"inProbe",-5);
                        }else{
                        Planner->resetProbe();
                        Planner->setActInProbe(SGProbe::typeStop);
                        runGCode(QString("G53G1G91Z%1F%2").arg(-GProbe->sumDist(),0,'f',5)
                                                          .arg( GProbe->F1,0,'f',3));
                        }

                        iOperation=10;
                        break;

              case 10: if(Planner->isProbe2()){
                          if(GProbe->doubleProbe){
                          iOperation=15;

                          if(SGProbe::backDist<0) {

                           Planner->resetProbe();
                           Planner->setActInProbe(SGProbe::typeStop); //остановимся при отскоке

                           runGCode(QString("G53G0G90Z%1").arg(getProbeGPoint().z-SGProbe::backDist,0,'f',5));
                           }else{
                           runGCode(QString("G53G0G90Z%1").arg(getProbeGPoint().z+SGProbe::backDist,0,'f',5));
                           }
                          }else{
                          goto endeProbeZ;
                          }
                         }else{
                         GProbe->error=true;
                         iOperation=55;
                         qDebug()<<Planner->isProbe2();
                         setMessage(tr("no sensor signal"),"inProbe",-10);
                         }
                       break;

              case 15: if(Planner->isProbe3()
                        ||!getInProbe()) {//едем на касание повторно
                         iOperation=50;
                         Planner->resetProbe();
                         Planner->setActInProbe(SGProbe::typeStop);

                         runGCode(QString("G53G1G91Z%1F%2").arg(-GProbe->sumDist(),0,'f',5)
                                                           .arg( GProbe->F2,0,'f',3));

                         //runGCode(QString("G53G1G90Z%1F%2").arg(getProbeGPoint().z-qAbs(SGProbe::backDist)*1.5,0,'f',5)
                         //                                  .arg(GProbe->F2,0,'f',3));
                         }else{
                         GProbe->error=true;
                         iOperation=55;
                         setMessage(tr("no sensor signal"),"inProbe",-15);
                         }
                        break;

              case 50:  if(Planner->isProbe2()){  //если было касание
                         endeProbeZ:
                         iOperation=0;

                         GProbe->probePoint=getProbeGPoint();
                         //GProbe->probePointSC.z-=SGProbe::headDiam/2;
                         GProbe->ex=true;

                         runGCode(QString("G53G0G90Z%1").arg(GProbe->Zback,0,'f',5));
                         }else{
                         GProbe->error=true;
                         iOperation=55;
                         setMessage(tr("no sensor signal"),"inProbe",-50);
                         }
                        break;

               case 55: resetAuto();
                        QTimer::singleShot(10,this,SLOT(setFinished()));
                        break;
               }


}

return true;
}


bool WLGMachine::isActivDrive(QString name)
{
WLDrive *Drive=WLDrive::getDrive(name);

if(Drive) return Drive->isActiv();
return 0;
}

void WLGMachine::setCurPosition(QString nameCoord,double pos)
{
WLDrive *Drive=getDrive(nameCoord);

if(Drive){
  Drive->setPosition(pos);
  }
}

void WLGMachine::setTruPositionDrive(QString nameCoord,bool tru)
{
WLDrive *Drive=getDrive(nameCoord);

if(Drive){
  Drive->setTruPosition(tru);
  }
}

void WLGMachine::setCurPositionSC(QString nameCoord,double pos)
{
WLGPoint curPos  =getCurrentPosition();
WLGPoint curPosSC=getCurrentPositionActivSC();
WLGPoint newPos=curPosSC;
WLGPoint SCG;

int iSC=m_GCode.getActivSC(&SCG);

//qDebug()<<"curPos"<<curPos.toString();
//newPos.fromM(getGCode()->getSC(getGCode()->getActivSC()).to6D().toM().inverted()*curPos.to6D().toM());

if(nameCoord=="X") newPos.x=pos / (getGCode()->isXDiam() ? 2.0:1.0);
else
if(nameCoord=="Y") newPos.y=pos;
else
if(nameCoord=="Z") newPos.z=pos+getGCode()->getHToolOfst()+m_HeightMap.getValue(curPos.x,curPos.y);
else
if(nameCoord=="A") newPos.a=pos;
else
if(nameCoord=="B") newPos.b=pos;
else
if(nameCoord=="C") newPos.c=pos;
else
if(nameCoord=="U") newPos.u=pos;
else
if(nameCoord=="V") newPos.v=pos;
else
if(nameCoord=="W") newPos.w=pos;


WLFrame    OffsetSC;
WLFrame newOffsetSC;
WLFrame    newPosFr;
WLFrame    curPosFr;
double rSC=m_GCode.getRotCurSC();

curPosFr.x=curPosSC.x;
curPosFr.y=curPosSC.y;
curPosFr.z=curPosSC.z;
curPosFr.a=-rSC;

newPosFr.x=newPos.x;
newPosFr.y=newPos.y;
newPosFr.z=newPos.z;
newPosFr.a=-rSC;

OffsetSC.x=SCG.x;
OffsetSC.y=SCG.y;
OffsetSC.z=SCG.z;
OffsetSC.a=rSC;

newOffsetSC.fromM(newPosFr.toM().inverted()*curPosFr.toM()*OffsetSC.toM());

WLGPoint newSCG=SCG-(newPos-curPosSC);

newSCG.x=newOffsetSC.x;
newSCG.y=newOffsetSC.y;
newSCG.z=newOffsetSC.z;

m_GCode.setOffsetSC(iSC,newSCG);
m_GCode.setDataSC(iSC,"GProgram",m_Program->getName());
}

void WLGMachine::setCurPositionSCT(QString nameCoord,double pos)
{
WLGPoint curPosSC=getCurrentPositionActivSC();
WLGPoint SCG;

int iOT=getGCode()->getOfstTool();

if(nameCoord=="X") getGCode()->setDataTool(iOT,"Xo",curPosSC.x-pos/(getGCode()->isXDiam()? 2.0:1.0)
                  +getGCode()->getDataToolNum(iOT,"Xo",0));
else
if(nameCoord=="Y") getGCode()->setDataTool(iOT,"Yo",curPosSC.y-pos+getGCode()->getDataCurToolNum("Yo",0));
else
if(nameCoord=="Z") getGCode()->setDataTool(iOT,"Zo",curPosSC.z-pos+getGCode()->getDataCurToolNum("Zo",0)
                   +getGCode()->getHToolOfst());

}

double WLGMachine::getCurPositionSCT(QString name)
{
double ret=getCurPositionSC(name);

name=name.toUpper();

if(name=="X") ret-=getGCode()->getDataCurToolNum("Xo",0);
else
if(name=="Y") ret-=getGCode()->getDataCurToolNum("Yo",0);
else
if(name=="Z") ret-=getGCode()->getDataCurToolNum("Zo",0);

return ret;
}

double WLGMachine::getCurPositionSC(QString name)
{
WLGPoint GP=getCurrentPositionActivSC();

name=name.toUpper();

if(name=="X") return GP.x;
else
if(name=="Y") return GP.y;
else
if(name=="Z") return GP.z-getGCode()->getHToolOfst();
else
if(name=="A") return GP.a;
else
if(name=="B") return GP.b;
else
if(name=="C") return GP.c;
else
if(name=="U") return GP.u;
else
if(name=="V") return GP.v;
else
if(name=="W") return GP.w;


return 0;
}

double WLGMachine::getCurPosition(QString name)
{
WLGPoint GP=getCurrentPosition();

name=name.toUpper();

if(name=="X") return GP.x;
else
if(name=="Y") return GP.y;
else
if(name=="Z") return GP.z;
else
if(name=="A") return GP.a;
else
if(name=="B") return GP.b;
else
if(name=="C") return GP.c;
else
if(name=="U") return GP.u;
else
if(name=="V") return GP.v;
else
if(name=="W") return GP.w;

return 0;
}


QString WLGMachine::getCurPositionStr()
{
QString ret;

WLGPoint GP=getCurrentPosition();

ret+="G53 ";

if(getDrive("X")) ret+="X"+QString::number(GP.x);
if(getDrive("Y")) ret+="Y"+QString::number(GP.y);
if(getDrive("Z")) ret+="Z"+QString::number(GP.z);
if(getDrive("A")) ret+="A"+QString::number(GP.a);
if(getDrive("B")) ret+="B"+QString::number(GP.b);
if(getDrive("C")) ret+="C"+QString::number(GP.c);
if(getDrive("U")) ret+="U"+QString::number(GP.u);
if(getDrive("V")) ret+="V"+QString::number(GP.v);
if(getDrive("W")) ret+="W"+QString::number(GP.w);

return ret;
}

void WLGMachine::setDriveManualWhell(QString nameDrive,quint8 X1,bool vmode)
{
WLModuleMPG *ModuleMPG=motDevice->getModuleMPG();

WLGDrive *Drive=getDrive(nameDrive,false);

if(ModuleMPG)
{
 if(nameDrive.isEmpty()
  ||Drive==nullptr
  ||Drive->getName().isEmpty())
 {
 ModuleMPG->getMPG(0)->setIndexAxis(0);
 }
 else
  if(isEnable())
  {
   Drive->setMainPad();

   Drive->setKSpeed(1);
   Drive->setVmov(0);

   ModuleMPG->getMPG(0)->setVmode(vmode);
   ModuleMPG->getMPG(0)->setIndexAxis(Drive->getAxis()->getIndex()+1);
   ModuleMPG->getMPG(0)->setIndexX(X1);
  }

}

}

Q_INVOKABLE void WLGMachine::clearGProbe()
{
qDebug()<<"WLGMachine::clearGProbe";

if(!verifyReadyAutoMotion()) {return;}
GProbeList.clear();
}


void WLGMachine::runGProbe()
{
qDebug()<<"WLGMachine::goGProbe()";

if(!verifyReadyAutoMotion()) {return;}

if(getInProbe()) {
    setMessage("WLGMachine::goGProbe","wrong state inProbe",-1);
    return;
    }

m_typeAutoMMachine=AUTO_GProbe;

iOperation=0;

setAuto();

setSafeProbe(isSafeProbe());

updateAuto();
}

bool WLGMachine::isCompleteGProbe()
{
foreach(SGProbe GProbe,GProbeList){
if(!GProbe.ex)  return false;
}

return true;
}

double WLGMachine::getGProbe(int index, QString name)
{
if(index>=GProbeList.size()
 ||!GProbeList[index].ex) {
 setMessage("WLGMachine::getProbe","error data",-1);
 return 0;
 }else { 
 //WLGPoint GPoint=GProbeList[index].probePointSC;
 //
 //GPoint.z+=getGCode()->getHofst();
 //return m_GCode.getPointActivSC(GPoint,false).get(name);
  return GProbeList[index].probePoint.get(name);
 }
}

double WLGMachine::getGProbeSC(int index, QString name)
{
if(index>=GProbeList.size()
 ||!GProbeList[index].ex) {
 setMessage("WLGMachine::getProbeSC","error data",-1);
 return 0;
 }else {
 //return GProbeList[index].probePointSC.get(name);
  WLGPoint GPoint=GProbeList[index].probePoint;

  GPoint=m_GCode.getPointActivSC(GPoint,true);
  GPoint.z-=getGCode()->getHToolOfst();

  return GPoint.get(name);
 }
}

void WLGMachine::addGProbeXY(double x,double y,double z,double angle,double _dist,double _distA)
{
qDebug()<<"WLGMachine::addGProbeXY"<<x<<y<<z<<angle<<_dist<<_distA<<"SD"<<(SGProbe::typeStop==WLIOPut::INPUT_actSdStop);;

if(!verifyReadyAutoMotion()) {return;}

SGProbe GProbe;

GProbe.type=probeXY;

GProbe.angle=angle;

_distA=qAbs(_distA);

GProbe.dist =qBound(0.1,_dist ,500.0);
GProbe.distA=qBound(0.0,_distA,500.0);

GProbe.awaitPoint.x=x;
GProbe.awaitPoint.y=y;
GProbe.awaitPoint.z=z;

GProbe.Zback=getCurPosition("Z");

GProbe.F1= SGProbe::FProbe1<=0.0 ? 50 : SGProbe::FProbe1;
GProbe.F2= SGProbe::FProbe2<=0.0 ? GProbe.F1/3 : SGProbe::FProbe2;

if(GProbe.F2>GProbe.F1)
     GProbe.F2=GProbe.F1;

GProbe.doubleProbe=SGProbe::enDoubleProbe;

GProbeList.append(GProbe);
}

void WLGMachine::addGProbeZ(double x, double y, double z,double _dist,double _distA)
{
qDebug()<<"WLGMachine::addGProbeZ"<<x<<y<<z<<_dist<<_distA<<"SD"<<(SGProbe::typeStop==WLIOPut::INPUT_actSdStop);

if(!verifyReadyAutoMotion()) {return;}

SGProbe GProbe;

GProbe.type=probeZ;

_distA=qAbs(_distA);

GProbe.dist =qBound(0.1,_dist ,500.0);
GProbe.distA=qBound(0.0,_distA,500.0);

GProbe.awaitPoint.x=x;
GProbe.awaitPoint.y=y;
GProbe.awaitPoint.z=z;

GProbe.Zback=getCurPosition("Z");

GProbe.F1= SGProbe::FProbe1<=0.0 ? 50 : SGProbe::FProbe1;
GProbe.F2= SGProbe::FProbe2<=0.0 ? GProbe.F1/3 : SGProbe::FProbe2;

if(GProbe.F2>GProbe.F1)
     GProbe.F2=GProbe.F1;

GProbe.doubleProbe=SGProbe::enDoubleProbe;

GProbeList.append(GProbe);
}


void WLGMachine::goDriveTouch(QString nameDrive,int dir,float F)
{
qDebug()<<"WLGMachine::goDriveTouch"<<nameDrive<<dir<<F;

if(!verifyReadyAutoMotion()) {return;}

WLGDrive *Drive=getDrive(nameDrive,true);

if(Drive)
{
Drive->reset();

Drive->setMovTouch(dir,F);
Drive->startTask();
}

}


void WLGMachine::setPause(bool pause) //остановка в любой момент
{
qDebug()<<"WLGMachine::setPause()"<<pause<<" runList"<<isRunList()<<" wait"<<m_waitPause;

if(!m_busy&&pause) return;

if(m_waitPause)
   {
   emit changedPause(m_pause);
   return;
   }

if(getInPause() //нажата ли пауза
 &&!pause
 &&!isIngnoreInPause())
    {
    sendMessage("WLGMachine","wrong state inPause",1);
    emit changedPause(m_pause);
    return;
    }

if(getInStop()
 &&!pause
 &&!isIngnoreInStop())
    {
    sendMessage("WLGMachine","wrong state inStop",1);
    emit changedPause(m_pause);
    return;
    }

if(getInSDStop()
 &&!pause)
    {
    sendMessage("WLGMachine","wrong state inSDStop",1);
    emit changedPause(m_pause);
    return;
    }

if(getInEMGStop()
 &&!pause)
    {
    sendMessage("WLGMachine","wrong state inEMGStop",1);
    emit changedPause(m_pause);
    return;
    }

MutexStart.lock();

WLModulePlanner *Planner=motDevice->getModulePlanner();

if(pause!=m_pause)
{
if(pause)
     {
     if(Planner->isBusy())
       {
       if(!isRunMScript())
          {
          m_waitPause=
          m_continue=true;
          }
       Planner->pauseMov();
       }
     else
       WLDrive::pauseDrives();
     }
     else
       {
       if(isRunGProgram()
       &&!isRunMScript())  {

        //verify M3 M4 M5           
        m_waitMScript=false;

        qDebug()<<">>>m_GCodePause M3"<<pauseStateSpindle<<stateSpindle;

        if(pauseStateSpindle!=stateSpindle){
            switch (pauseStateSpindle) {
            case Stop: m_waitMScript=true;
                       runMCode(5);
                       break;
            case FW:   m_waitMScript=true;
                       runMCode(3);
                       break;
            case RE:  m_waitMScript=true;
                       runMCode(4);
                       break;
            }
          }

        if(!m_waitMScript)
            QTimer::singleShot(0,this,&WLGMachine::startMov);
        }
        else
        {
        if(isRunMScript())
         {
         continueMovList();
         }
        else
        if(m_continue)
         {
         QTimer::singleShot(0,this,&WLGMachine::startMov);
         }
        else
         {
         WLDrive::accelDrives();
         }


        }
       }
}

MutexStart.unlock();

emit changedPause(m_pause=pause);

QTimer::singleShot(0,this,&WLGMachine::setFinished);
}

void WLGMachine::updateStatusMPlanner(int status)
{
qDebug()<<"WLGMachine::updateStatusMPlanner"<<status;

switch(status)
 {
 case PLANNER_paused:   m_runList=false;
                        break;

 case PLANNER_pause:    m_runList=false;;

                         emit changedPause(m_pause=true);

                         if(isRunGProgram()){
                             sendMessage("program pause: \""+m_Program->getName()+"\"",QString(" element: %1").arg(m_Program->getLastMovElement()),1);
                             pauseStateSpindle=stateSpindle;
                             m_GCodePause=getGCode()->getData();
                             //qDebug()<<"<<<m_GCodePause M3"<<m_GCodePause.MCode[3]<<" M5"<<m_GCodePause.MCode[5];
                             }

                         if(!isRunMScript())
                             runMScript("PAUSE()");

                         break;

 case PLANNER_stopped:  m_runList=false;
                        break;

 case PLANNER_stop:      m_runList=false;

                        if(!isRunMScript()){
                          if(isRunGProgram()
                           &&!isEmptyMotion()){
                               stopMov();
                               }

                          if(m_sStop)
                               {
                               m_sStop=false;
                               runMScript("STOP()");
                               }
                           }
                        break;

 case  PLANNER_run:     m_runList=true;
                       break;
 }

updateBusy();
updatePosible();
QTimer::singleShot(0,this,SLOT(setFinished()));
}

void WLGMachine::updateInProbe(bool state)
{
if(state){
 if(m_safeProbe
  &&isRunGProgram()
  &&!isRunGProbe())
  sendMessage(metaObject()->className(),"detect safe Probe",0);
 }
}

void WLGMachine::updatePosible()
{
bool posManual=isPossiblyManual();

if(posManual){
  setSafeProbe(isSafeProbe()||isAutoSetSafeProbe());
  }

emit changedPossibleManual(posManual);
emit changedPossibleEditModeRun(isPossiblyEditModeRun());
}


WLGPoint WLGMachine::getAxisPosition()
{
WLGPoint GP;

if(getDrive("X")) GP.x=getDrive("X")->position();
if(getDrive("Y")) GP.y=getDrive("Y")->position();
if(getDrive("Z")) GP.z=getDrive("Z")->position();
if(getDrive("A")) GP.a=getDrive("A")->position();
if(getDrive("B")) GP.b=getDrive("B")->position();
if(getDrive("C")) GP.c=getDrive("C")->position();
if(getDrive("U")) GP.u=getDrive("U")->position();
if(getDrive("V")) GP.v=getDrive("V")->position();
if(getDrive("W")) GP.w=getDrive("W")->position();

return GP;
}

WLGPoint WLGMachine::getAxisErrorPosition()
{
WLGPoint GP;

if(getDrive("X")) GP.x=getDrive("X")->errPosition();
if(getDrive("Y")) GP.y=getDrive("Y")->errPosition();
if(getDrive("Z")) GP.z=getDrive("Z")->errPosition();
if(getDrive("A")) GP.a=getDrive("A")->errPosition();
if(getDrive("B")) GP.b=getDrive("B")->errPosition();
if(getDrive("C")) GP.c=getDrive("C")->errPosition();
if(getDrive("U")) GP.u=getDrive("U")->errPosition();
if(getDrive("V")) GP.v=getDrive("V")->errPosition();
if(getDrive("W")) GP.w=getDrive("W")->errPosition();

return GP;
}

WLGPoint WLGMachine::getCurrentPosition(bool real)
{
WLGPoint GP;

if(real)
 {
 if(getDrive("X")) GP.x=getDrive("X")->getRealPosition();
 if(getDrive("Y")) GP.y=getDrive("Y")->getRealPosition();
 if(getDrive("Z")) GP.z=getDrive("Z")->getRealPosition();
 if(getDrive("A")) GP.a=getDrive("A")->getRealPosition();
 if(getDrive("B")) GP.b=getDrive("B")->getRealPosition();
 if(getDrive("C")) GP.c=getDrive("C")->getRealPosition();
 if(getDrive("U")) GP.u=getDrive("U")->getRealPosition();
 if(getDrive("V")) GP.v=getDrive("V")->getRealPosition();
 if(getDrive("W")) GP.w=getDrive("W")->getRealPosition();
 }
else
 {
 if(getDrive("X")) GP.x=getDrive("X")->getViewPosition();
 if(getDrive("Y")) GP.y=getDrive("Y")->getViewPosition();
 if(getDrive("Z")) GP.z=getDrive("Z")->getViewPosition();
 if(getDrive("A")) GP.a=getDrive("A")->getViewPosition();
 if(getDrive("B")) GP.b=getDrive("B")->getViewPosition();
 if(getDrive("C")) GP.c=getDrive("C")->getViewPosition();
 if(getDrive("U")) GP.u=getDrive("U")->getViewPosition();
 if(getDrive("V")) GP.v=getDrive("V")->getViewPosition();
 if(getDrive("W")) GP.w=getDrive("W")->getViewPosition();
 }

return GP;
}

WLGPoint WLGMachine::getCurrentPositionActivSC()
{
return m_GCode.getPointActivSC(getCurrentPosition(),true);
}

void WLGMachine::setAuto()
{
if(getMPG()&&isUseMPG())
   getMPG()->setEnable(false);

WLMachine::setAuto();

updateBusy();
}

void WLGMachine::resetAuto()
{
if(getMPG()
 &&isUseMPG()
 &&isEnable())
   getMPG()->setEnable(true);

m_typeAutoMMachine=AUTO_no;

WLMachine::resetAuto();

updateBusy();
updatePosible();
}

void WLGMachine::setSOut(float S)
{
motDevice->getModulePlanner()->setSOut(S);
}

WLGPoint WLGMachine::getProbeGPoint()
{
WLGPoint ret;
WLModulePlanner *planner=getMotionDevice()->getModulePlanner();

if(planner)
foreach(WLDrive *drive,getDrives()){
 WLDrivePosition DPos;

 if(drive->getAxis()
  &&planner->getIAxis(drive->getAxis()->getIndex()!=-1)){
   DPos.step=planner->getProbe2(planner->getIAxis(drive->getAxis()->getIndex()));
   ret.set(drive->getName(),DPos.get(drive->getDriveDim()));
   }
 }

return ret;
}

WLGPoint WLGMachine::getProbeGPointSC()
{
WLGPoint ret=m_GCode.getPointActivSC(getProbeGPoint(),true);

ret.z-=getGCode()->getHToolOfst();

return ret;
}


bool WLGMachine::runGProgram(int istart)
{
qDebug()<<"WLGMachine::runGProgram"<<istart<<m_Program->getName();

if(isRunGProgram()) return false;

QList <int> iSCList=m_Program->getSCList();
foreach(int iSC,iSCList) {
  if(m_GCode.getSC(iSC).isEmpty()) {
    sendMessage(metaObject()->className(),tr("no inicial")+": "+m_GCode.getSCGStr(iSC),-1);
    return 0;
    }
  }

QList <int> toolList=m_Program->getToolList();
foreach(int tool,toolList) {
  if(m_GCode.getTool(tool).isEmpty()) {
    sendMessage(metaObject()->className(),tr("no inicial")+": "+m_GCode.getTGStr(tool),-1);
    return 0;
    }
  }

if(isPause())  {
  sendMessage(metaObject()->className(),tr("pause activ"),-1);
  return 0;
  }

foreach(WLGDrive *MDrive,getGDrives())
  {
  if(!MDrive->isEnable()) {
    sendMessage(metaObject()->className(),QString(tr("drive %1 not enable!")).arg(MDrive->getName()),-1);
    return 0;
    }

  }

QList <WLElementTraj> curTraj;
QList <QPair <int,QStringList>> detectMList;
QString txt;

MillTraj.clear();
baseTraj.clear();

preRunProgramList.clear();

MutexShowTraj.lock();
showMillTraj.clear();
MutexShowTraj.unlock();

if(isEnable())
 {
 setIgnoreInPause(false);
 setIgnoreInStop(false);
 }

if(MillTraj.isEmpty())
   {
   WLGPoint curPos=getCurrentPosition();

   m_GCode.data()->lastGPoint=m_GCode.getPointActivSC(curPos,true);
   m_GCode.data()->lastGPoint.z-=m_HeightMap.getValue(curPos.x
                                                     ,curPos.y);

   lastMillGPoint=curPos;

   qDebug()<<"WLGMachine::runGProgram updateLastGPoint"<<m_GCode.data()->lastGPoint.toString(0);

   getGCode()->loadStr(getGCode()->getStrRunProgram());
   }

WLGPoint curGPoint=getGCode()->getPointActivSC(getGCode()->getCurPoint(),true);

if(getGCode()->isGCode(90))
    curGPoint.z-=getGCode()->getHToolOfst();

if(istart>0)
 { 
 for(m_iProgram=0;m_iProgram<istart;m_iProgram++)
   {   
   txt=m_Program->getTextElement(m_iProgram);

   if(!WLGProgram::translate(txt,curTraj,&m_GCode,m_iProgram,m_iProgram<(istart-1)))
       {
       sendMessage(metaObject()->className(),QString("error G code in line %1: %2").arg(m_iProgram).arg(txt),-1);
       return 0;
       }

   if(getGCode()->isValidValue('Z'))
     {
     curGPoint=getGCode()->getPointActivSC(getGCode()->getCurPoint(),true);

     if(getGCode()->isGCode(90))
         curGPoint.z-=getGCode()->getHToolOfst();
     }

   foreach(WLElementTraj et,curTraj){
     if(et.isScript())
       {                
       int Mcode=et.escript.script.remove(QRegExp("[M()]")).toInt();

       if(Mcode<0) continue;

       QPair <int,QStringList> MPair(Mcode,QStringList()<<m_GCode.getContextGCodeList());

       for(int i=0;i<detectMList.size();i++){ //ищем чтобы был всегда один
           if(detectMList[i].first==MPair.first)  {
              detectMList.removeAt(i);
              break;
              }
         }

       detectMList.append(MPair);

       for (int i=0;i<detectMList.size();i++) {
             if(MPair.first==3)  {if(detectMList.at(i).first==4||detectMList.at(i).first==5) detectMList.removeAt(i--);}
        else if(MPair.first==4)  {if(detectMList.at(i).first==3||detectMList.at(i).first==5) detectMList.removeAt(i--);}
        else if(MPair.first==5)  {if(detectMList.at(i).first==3||detectMList.at(i).first==4) detectMList.removeAt(i--);}
        else if(MPair.first==7)  {if(detectMList.at(i).first==8||detectMList.at(i).first==9) detectMList.removeAt(i--);}
        else if(MPair.first==8)  {if(detectMList.at(i).first==7||detectMList.at(i).first==9) detectMList.removeAt(i--);}
        else if(MPair.first==9)  {if(detectMList.at(i).first==7||detectMList.at(i).first==8) detectMList.removeAt(i--);}
        }
       }
      }

   if(!curTraj.isEmpty())
     {
     WLElementTraj lastElement = curTraj.last();
     curTraj.clear();
     curTraj+=lastElement;
     }
   }
 }
else
 m_iProgram=0;

m_runGProgram=true;

QStringList GDrive=QString(GPointNames).split(",");
foreach(QString name,GDrive)
 {
 if(getDrive(name))
  m_nowBL.set(name,getDrive(name)->rot() ? getDrive(name)->getHalfBacklash()
                                         :-getDrive(name)->getHalfBacklash());
 }

if(istart!=0)
{
QList <int> filtertMList;
filtertMList<<6<<3<<4<<5<<7<<8<<9;

for(int i=0;i<detectMList.size();i++)
  for(int j=0;j<filtertMList.size();j++)
        if(detectMList.at(i).first==filtertMList.at(j)){

           qDebug()<<"detect MCode M"<<filtertMList.at(i)<<detectMList.at(i).second.join(" ");

           preRunProgramList+=detectMList.at(i).second;
           preRunProgramList+=QString("M%1").arg(detectMList.at(i).first);
           }

WLGPoint endPoint=getGCode()->getCurPoint();

preRunProgramList+=getGCode()->getActivGCodeString();

//if(getCurrentPosition().z<planeZ)
//  preRunProgramList+=QString("G53 G0 Z%1").arg(planeZ);

if(getCurrentPosition().z<getGCode()->getG28Position().z)
  {
  double safeZ=getGCode()->getG28Position().z;

  if(getDrive("Z")
    &&safeZ>getDrive("Z")->maxPosition())  safeZ=getDrive("Z")->maxPosition();

  preRunProgramList+=QString("G53 G0 Z%1 //add wlmill -up z").arg(safeZ);
  }

preRunProgramList+=QString("G53 G0 X%1 Y%2 //add wlmill -mov xy").arg(endPoint.x).arg(endPoint.y);
preRunProgramList+=QString("G53 G0 A%1 B%2 C%3 //add wlmill -mov abc").arg(endPoint.a).arg(endPoint.b).arg(endPoint.c);

if(getGCode()->isGCode(0)||getDistG1StartAt()==0.0){
   preRunProgramList+=QString("G0 Z%1 //add wlmill -mov z down G0 only").arg(curGPoint.z);
   }
   else{
   preRunProgramList+=QString("G0 Z%1 //add wlmill -mov z down G0").arg(curGPoint.z+getDistG1StartAt());

   double F = getFeedG1StartAt() > 0 ? getFeedG1StartAt() : getGCode()->getValue('F');

   preRunProgramList+=QString("G1 Z%1 F%2 //add wlmill -mov z down G1").arg(curGPoint.z).arg(F);
   }

preRunProgramList+=getGCode()->getContextGCodeList();

m_GCode.data()->lastGPoint=getCurrentPositionActivSC();

}

//updateMovProgram();

emit changedTrajSize(MillTraj.size());
//emit changedReadyRunList(Flag.set(ma_readyRunList,!MillTraj.isEmpty()));
emit changedReadyRunList(m_readyRunList=true);

//if(Flag.get(ma_autostart))   startMovList();
if(isRunMScript()){
   startMov();
   }

QTimer::singleShot(0,this,&WLGMachine::setFinished);

return true;
}

bool WLGMachine::loadGProgram(QString file,bool build)
{
bool ret=false;

qDebug()<<"WLGMachine::loadGProgram"<<file<<build;

if(!isRunGProgram())
 {
 ret=m_Program->loadFile(file,build);
 }

if(!ret)
    {
    qDebug()<<"no load GProgram";
    sendMessage(metaObject()->className(),tr("eror load")+" GProgram: "+file,0);
    }

return ret;
}


bool WLGMachine::runGCode(QString gtxt)
{
//WL6DPoint lastGPoint=getCurrentPositionActivSC();
WLFrame Fr;	
QList <WLElementTraj> curListTraj;
QList <WLElementTraj>    ListTraj;

qDebug()<<"WLGMachine::runGCode"<<gtxt;

if(isRunMScript()
 &&m_GCode.detectMCode(gtxt))
 {
 qDebug()<<"error execute M in runGCode:"<<"("<<gtxt<<")";
 setMessage(metaObject()->className(),tr("error execute M in runGCode:")+"("+gtxt+")",-1);
 return false;
 }

const float simpliD=m_mainDim*(1<<xPD);

WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

if(MillTraj.isEmpty()
&&!ModulePlanner->isBusy())
    {    
    WLGPoint curPos=getCurrentPosition();

    m_GCode.data()->lastGPoint=m_GCode.getPointActivSC(curPos,true);

    lastMillGPoint=curPos;

    qDebug()<<"WLGMachine::runGCode updateLastGPoint"<<m_GCode.data()->lastGPoint.toString(0);
    }

if(WLGProgram::translate(gtxt,curListTraj,&m_GCode))
    {		
    WLElementTraj::simpliTrajectory(ListTraj,curListTraj,simpliD);

	addElementTraj(curListTraj);
	
    emit changedTrajSize(MillTraj.size());
    emit changedReadyRunList(m_readyRunList=true);
	  

     if(!isPause())
       {
       if(isRunList())
         {
         updateMovPlanner();
         }
       else  if(m_autoStartGCode||isRunMScript())
               {
               startMov();
               }
        }

    updateBusy();
	return true;
    }

return false;
}

int WLGMachine::updateMovProgram()
{
QString txt;
QList<WLElementTraj> simpliTraj;
QList<WLElementTraj> curTraj;
WLElementTraj lastEG4142;

WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();
//WLElementTraj ETraj;

float simpliD=(float)m_mainDim*(1<<xPD);

int isimpli;
bool txtProgram=true;

qDebug()<<"WLGMachine::updateMovProgram() m_iProgram"<<m_iProgram
        <<"runProgram"<<isRunGProgram()
        <<"runScript"<<isRunMScript();

if(isRunGProgram()
&&!isRunMScript())
 {
 while(MillTraj.size()<100
     &&(!baseTraj.isEmpty()||m_iProgram<m_Program->getElementCount())
     &&(MillTraj.isEmpty()||(!MillTraj.last().isScript())))
   {

   if(!baseTraj.isEmpty()
     &&baseTraj.first().isScript()) //если первый элемент Script то отправляем его на исполнение
     {
     addElementTraj(QList<WLElementTraj>()<<baseTraj.takeFirst());
     continue;
     }

   if(m_iProgram<m_Program->getElementCount()) //читаем программу
    {
    if(preRunProgramList.isEmpty()){             //предварительный список
      txt=m_Program->getTextElement(m_iProgram);
      txtProgram=true;
      }
      else {
      txt=preRunProgramList.takeFirst();
      txtProgram=false;
      }

    curTraj.clear();

    if(!baseTraj.isEmpty())
        curTraj.prepend(baseTraj.takeLast());

    if(!WLGProgram::translate(txt,curTraj,&m_GCode,m_iProgram)) {
     QTimer::singleShot(0,this,[=](){setMessage(getNameGProgram(false),"error in GProgram",-1);});
     return 0;
     }

    if(txtProgram)
         m_iProgram++;

    WLElementTraj::removeEmpty(curTraj);

    if(curTraj.isEmpty()) continue;

    baseTraj+=curTraj;
    }  

    isimpli=WLElementTraj::simpliTrajectory(simpliTraj
                                           ,baseTraj
                                           ,simpliD
                                           ,true);

   if((isimpli+1)<baseTraj.size()) //если сгладили и дошли до точки, но не до конца
    {
    addElementTraj(simpliTraj);
    baseTraj=baseTraj.mid(isimpli+1); //оставляем один элемент на будущее, может и Script
    }   
    else if(m_iProgram==(m_Program->getElementCount())      //до конца
          ||WLElementTraj::detectScript(baseTraj)) //до конца
          {
          while(!baseTraj.isEmpty())  //перемещаем всё + один Script
           {
           addElementTraj(QList<WLElementTraj>()<<baseTraj.first());
           if(baseTraj.takeFirst().isScript())
               break;
           }
          }

   }  

 }
return 1;
}

bool WLGMachine::verifyReadyMotion()
{
if(!isEnable())
   {sendMessage(metaObject()->className(),tr("is off!"),0); return false;}

if(getInEMGStop())
   {sendMessage(nameClass(),tr("wrong state")+"(inEMGStop=0)",0);return false;}

if(getInSDStop())
   {sendMessage(nameClass(),tr("wrong state")+"(inSDStop=0)",0);return false;}

if(getInPause()&&!isIngnoreInPause())
   {sendMessage(nameClass(),tr("wrong state")+"(inPause=0)",0);return false;}

if(getInStop()&&!isIngnoreInStop())
   {sendMessage(nameClass(),tr("wrong state")+"(inStop=0)",0);return false;}


if(isSafeProbe()
 &&!isRunGProbe()){
   if(getInProbe()){
       sendMessage(nameClass(),tr("wrong state")+"(inProbe=0)",0);return false;
       }      
   }

return true;
}

bool WLGMachine::verifyReadyAutoMotion()
{
if(!verifyReadyMotion()) return false;

if(isPause())
   {sendMessage(nameClass(),tr("activ pause"),0);return false;}

return true;
}

bool verifyLineHeightMap (QList<WLElementTraj> &Traj)
{
/*
QList<WLElementTraj> newTraj;

for(int i=0;i<Traj.size();i++)
 {
 if(!Traj[i].isLine()) return false;
   {
   Traj[i]

   }
 }
*/
return true;
}

void WLGMachine::addHeightMap(QList<WLElementTraj> &Traj)
{
QList<WLElementTraj> newTraj;

if(!m_HeightMap.isValid()
 ||!m_HeightMap.isEnable()) return;

while(!Traj.isEmpty())
{
WLElementTraj ET=Traj.takeFirst();

switch(ET.type)
{
case WLElementTraj::line: {
                          double interpolationStepX = m_HeightMap.getInterpStepX();
                          double interpolationStepY = m_HeightMap.getInterpStepY();

                          QVector3D stXY(ET.data.line.startPoint.x
                                        ,ET.data.line.startPoint.y
                                        ,ET.data.line.startPoint.z);

                          QVector3D enXY(ET.data.line.endPoint.x
                                        ,ET.data.line.endPoint.y
                                        ,ET.data.line.endPoint.z);

                          QVector3D vec=enXY-stXY;
                          double length;

                          if(qIsNaN(vec.length()))
                             {
                             ET.data.line.startPoint.z+=m_HeightMap.getValue(ET.data.line.startPoint.x
                                                                            ,ET.data.line.startPoint.y);

                             ET.data.line.endPoint.z+=m_HeightMap.getValue(ET.data.line.endPoint.x
                                                                           ,ET.data.line.endPoint.y);
                             newTraj+=ET;
                             break;
                             }
                             else {
                             if (fabs(vec.x()) / fabs(vec.y()) < interpolationStepX / interpolationStepY)
                                 length = interpolationStepY / (vec.y() / vec.length());
                             else
                                 length = interpolationStepX / (vec.x() / vec.length());

                             length = fabs(length);

                             QVector3D seg = vec.normalized() * length;
                             int count = trunc(vec.length() / length);
                             seg = vec / count;

                             if (count == 0)
                                {
                                ET.data.line.startPoint.z+=m_HeightMap.getValue(ET.data.line.startPoint.x
                                                                               ,ET.data.line.startPoint.y);

                                ET.data.line.endPoint.z+=m_HeightMap.getValue(ET.data.line.endPoint.x
                                                                              ,ET.data.line.endPoint.y);
                                newTraj+=ET;
                                break;
                                }

                             for (int i = 0; i < count; i++) {

                                 WLElementTraj segLine=ET;

                                 segLine.data.line.startPoint.x+=seg.x()*i;
                                 segLine.data.line.startPoint.y+=seg.y()*i;
                                 segLine.data.line.startPoint.z+=seg.z()*i;

                                 segLine.data.line.endPoint=segLine.data.line.startPoint;

                                 segLine.data.line.startPoint.z+=m_HeightMap.getValue(segLine.data.line.startPoint.x
                                                                                     ,segLine.data.line.startPoint.y);

                                 if(i==count-1)
                                 {
                                 segLine.data.line.endPoint.x=ET.data.line.endPoint.x;
                                 segLine.data.line.endPoint.y=ET.data.line.endPoint.y;
                                 segLine.data.line.endPoint.z=ET.data.line.endPoint.z;
                                 }
                                 else{
                                 segLine.data.line.endPoint.x+=seg.x();
                                 segLine.data.line.endPoint.y+=seg.y();
                                 segLine.data.line.endPoint.z+=seg.z();
                                 }

                                 segLine.data.line.endPoint.z+=m_HeightMap.getValue(segLine.data.line.endPoint.x
                                                                                   ,segLine.data.line.endPoint.y);


                                 newTraj+=segLine;
                                 }
                             }

                          }
                          break;


case WLElementTraj::arc:
                        {
                        double interpolationStep = 5;//qMin(m_HeightMap.getInterpStepX(),m_HeightMap.getInterpStepY());

                        if(ET.data.arc.R/2.0<interpolationStep
                         ||ET.data.arc.plane!=17)
                          {
                          ET.data.arc.startPoint.z+=m_HeightMap.getValue(ET.data.arc.startPoint.x
                                                                        ,ET.data.arc.startPoint.y);

                          ET.data.arc.endPoint.z+=m_HeightMap.getValue(ET.data.arc.endPoint.x
                                                                      ,ET.data.arc.endPoint.y);
                          newTraj+=ET;
                          }
                          else {
                           WLElementTraj segArc=ET;

                           double A_st=ET.data.arc.startPoint.to3D().getAxy(ET.data.arc.centerPoint.to3D());
                           double A_en=ET.data.arc.endPoint.to3D().getAxy(ET.data.arc.centerPoint.to3D());

                           if((ET.data.arc.CCW)&&(A_en<=A_st))  A_en+=2.0*M_PI;
                           if((!ET.data.arc.CCW)&&(A_en>=A_st)) A_en-=2.0*M_PI;

                           double lenght=qAbs((A_en-A_st)*ET.data.arc.R);

                           int count=trunc(lenght/interpolationStep)+1;

                           double dA=(A_en-A_st)/count;
                           double A=A_st;

                           for (int i = 0;i<count; i++) {

                               if(i!=0){
                               segArc.data.arc.startPoint.x=segArc.data.arc.endPoint.x;
                               segArc.data.arc.startPoint.y=segArc.data.arc.endPoint.y;
                               segArc.data.arc.startPoint.z=segArc.data.arc.endPoint.z;
                               }
                               else{
                               segArc.data.arc.startPoint.z+=m_HeightMap.getValue(segArc.data.arc.startPoint.x
                                                                                 ,segArc.data.arc.startPoint.y);
                               }

                               A+=dA;

                               if(i==count-1)
                                {
                                A=A_en;

                                segArc.data.arc.endPoint.x=ET.data.arc.endPoint.x;
                                segArc.data.arc.endPoint.y=ET.data.arc.endPoint.y;
                                }
                                else{
                                segArc.data.arc.endPoint.x=segArc.data.arc.centerPoint.x;
                                segArc.data.arc.endPoint.y=segArc.data.arc.centerPoint.y;

                                segArc.data.arc.endPoint.x+=segArc.data.arc.R*cos(A);
                                segArc.data.arc.endPoint.y+=segArc.data.arc.R*sin(A);
                                }

                               segArc.data.arc.endPoint.z=ET.data.arc.endPoint.z;
                               segArc.data.arc.endPoint.z+=m_HeightMap.getValue(segArc.data.arc.endPoint.x
                                                                               ,segArc.data.arc.endPoint.y);

                               newTraj+=segArc;

                               if((ET.data.arc.CCW&&A>=A_en)
                               ||(!ET.data.arc.CCW&&A<=A_en)) break;
                               }

                          }
                         }
                        break;
case WLElementTraj::delay:
                            ET.data.delay.point.z+=m_HeightMap.getValue(ET.data.delay.point.x
                                                                       ,ET.data.delay.point.y);
                            newTraj+=ET;
                            break;

default: newTraj+=ET;
}

}


Traj=newTraj;
}

#define useULine
void WLGMachine::addSmooth(QList<WLElementTraj> &addTraj)
{
WLElementTraj ET;
WL3DPoint PS,PE,PO,Pm,P;
WLGPoint GPm;
float B;
float V=0,L=0;
bool ok;
bool add=false;

if(!MillTraj.isEmpty()){
  add=true;

  addTraj.prepend(MillTraj.takeLast());

  MutexShowTraj.lock();
  showMillTraj.removeLast();
  MutexShowTraj.unlock();
  }


#ifndef useULine
QMatrix3x3 T;
T(0,0)= 1; T(0,1)= 0; T(0,2)= 0;
T(1,0)=-3; T(1,1)= 4; T(1,2)=-1;
T(2,0)= 2; T(2,1)=-4; T(2,2)= 2;

double ax[3],ay[3],az[3],dt;

dt=1.0f/20;
#endif


for(int i=1;i<addTraj.size();i++)
  {
  //qDebug()<<"addTraj.smooth"<<addTraj[i].getG64P();
  if(!addTraj[i-1].isLine()
   ||!addTraj[i].isLine()   
   ||addTraj[i-1].isFast()
   ||addTraj[i].isFast()
   ||addTraj[i].isStopMode()
   ||addTraj[i-1].isStopMode()
   ||addTraj[i].getSmoothP()==0.0
   ) continue;

  ET=addTraj[i];

  WLGPoint V0,V1;

  V0=addTraj[i-1].data.line.endPoint
    -addTraj[i-1].data.line.startPoint;

  V1=addTraj[i].data.line.endPoint
    -addTraj[i].data.line.startPoint;

  B=calcAngleGrd(V0.to3D().normalize(),V1.to3D().normalize())/2.0;

  if(B>0.1)
//  if(qAbs(B) <(90-smoothAng)
//   &&qAbs(B*2)>(smoothAng+0.1))
   {
   V=2.0f*addTraj[i].getSmoothP()/sin(B/180.0f*M_PI);
   //V=2.0*smooth/sin(B/180.0f*M_PI);

   //qDebug()<<addTraj[i-1].startV.to3D().toString()<<" "<<addTraj[i-1].startV.to3D().toString();

   V=qMin((float)V,qMin(addTraj[i-1].movDistanceXYZ*0.95f,addTraj[i].movDistanceXYZ)/2.0f);

   L=V*sin(B/180.0*M_PI)/2.0;

   PO=addTraj[i-1].data.line.endPoint.to3D();

   PS=addTraj[i-1].data.line.endPoint.to3D();
   PE=addTraj[i].data.line.startPoint.to3D();

   PS-=(addTraj[i-1].data.line.endPoint-addTraj[i-1].data.line.startPoint).to3D().normalize()*V;
   PE+=(addTraj[i].data.line.endPoint-addTraj[i].data.line.startPoint).to3D().normalize()*V;

   addTraj[i-1].data.line.endPoint.x=PS.x;
   addTraj[i-1].data.line.endPoint.y=PS.y;
   addTraj[i-1].data.line.endPoint.z=PS.z;

   addTraj[i].data.line.startPoint.x=PE.x;
   addTraj[i].data.line.startPoint.y=PE.y;
   addTraj[i].data.line.startPoint.z=PE.z;

   GPm=addTraj[i].data.line.startPoint;

   Pm=((PS+PE)/2-PO).normalize()*L+PO;

#ifndef useULine
   ax[0]=PS.x*T(0,0)+Pm.x*T(0,1)+PE.x*T(0,2);
   ax[1]=PS.x*T(1,0)+Pm.x*T(1,1)+PE.x*T(1,2);
   ax[2]=PS.x*T(2,0)+Pm.x*T(2,1)+PE.x*T(2,2);

   ay[0]=PS.y*T(0,0)+Pm.y*T(0,1)+PE.y*T(0,2);
   ay[1]=PS.y*T(1,0)+Pm.y*T(1,1)+PE.y*T(1,2);
   ay[2]=PS.y*T(2,0)+Pm.y*T(2,1)+PE.y*T(2,2);

   az[0]=PS.z*T(0,0)+Pm.z*T(0,1)+PE.z*T(0,2);
   az[1]=PS.z*T(1,0)+Pm.z*T(1,1)+PE.z*T(1,2);
   az[2]=PS.z*T(2,0)+Pm.z*T(2,1)+PE.z*T(2,2);

   ET.setF(addTraj[i-1].getF());

   WLGPoint SGP;
   WLGPoint  GP;

   SGP=addTraj[i-1].endPoint;
   GP=SGP;

   for(float d=dt;d<1;d+=dt)
    {
    GP.x=ax[0]+ax[1]*d+ax[2]*d*d;
    GP.y=ay[0]+ay[1]*d+ay[2]*d*d;
    GP.z=az[0]+az[1]*d+az[2]*d*d;

    ET.setLineXYZ(SGP,GP);
    ET.calcPoints(&ok,getGModel());
    addTraj.insert(i++,ET);

    SGP=GP;
    }
#else
   GPm.x=Pm.x;
   GPm.y=Pm.y;
   GPm.z=Pm.z;

   ET.setULine(addTraj[i-1].data.line.endPoint,GPm,addTraj[i].data.line.startPoint);
   //ET.setLineXYZ(addTraj[i-1].endPoint,addTraj[i].startPoint);
   ET.F=addTraj[i-1].F;
   ET.calcPoints(&ok,getGModel());

   addTraj.insert(i++,ET);
#endif

   }
  }

if(add){
  MutexShowTraj.lock();
  showMillTraj+=addTraj.first();
  MutexShowTraj.unlock();

  MillTraj.append(addTraj.takeFirst());
  }
}

void WLGMachine::addCalcGModel(QList<WLElementTraj> &addTraj)
{
QList<WLElementTraj> addModelTraj;
bool ok;

qDebug()<<"WLGMachine::addCalcGModel";

for(int i=0;i<addTraj.size();i++)
   {
   getGModel()->setOffsetFrame(getGCode()->getOffsetSC(getGCode()->getActivSC()).to3D());

   addModelTraj+=addTraj[i].calcModelPoints(&ok,getGModel(),2);
   }

addTraj=addModelTraj;
}

void WLGMachine::addRotaryPosition(WLGPoint &lastPoint,QList<WLElementTraj> &addTraj)
{
QList<WLElementTraj> addModelTraj;

qDebug()<<"WLGMachine::addRotaryPosition";

for(int i=0;i<addTraj.size();i++)
   {
   if(i>0){
      addTraj[i].setStartPoint(addTraj[i-1].getEndPoint());
      }
      else {
      addTraj[i].setStartPoint(lastPoint);
      }

   WLGPoint   endPoint=addTraj[i].getEndPoint();

   if(getDrive("A")
    &&getDrive("A")->getType()==WLDrive::Rotary
    &&getDrive("A")->isInfinity())
      endPoint.a=getDrive("A")->calcRotaryInfEndPosition(lastPoint.a,endPoint.a);

   if(getDrive("B")
    &&getDrive("B")->getType()==WLDrive::Rotary
    &&getDrive("B")->isInfinity())
      endPoint.b=getDrive("B")->calcRotaryInfEndPosition(lastPoint.b,endPoint.b);

   if(getDrive("C")
    &&getDrive("C")->getType()==WLDrive::Rotary
    &&getDrive("C")->isInfinity())
     endPoint.c=getDrive("C")->calcRotaryInfEndPosition(lastPoint.c,endPoint.c);

   lastPoint=addTraj[i].getStartPoint();

   addTraj[i].setEndPoint(endPoint);
   }


if(!addTraj.isEmpty())
    lastMillGPoint=addTraj.last().getEndPoint();
}

void WLGMachine::addBacklash(QList<WLElementTraj> &Traj)
{
//перебор и добавление отработки люфтов
//можно сделать в реальном времени
WLElementTraj ETraj;
//WL3DPoint nowBL;
WLGPoint nextBL;
WLGPoint nextNBL;

WLGPoint deltaBL;

WLGPoint lastBL;
WLGPoint movV;
WLGPoint movNV;
WLGPoint lastMBL;

QStringList GDrive=QString(GPointNames).split(",");

bool add=false;

if(!MillTraj.isEmpty()){
  add=true;

  Traj.prepend(MillTraj.takeLast());

  MutexShowTraj.lock();
  showMillTraj.removeLast();
  MutexShowTraj.unlock();
  }


bool ok;

for(int i= add ? 1 : 0;i<Traj.size();i++)
{
//qDebug()<<"Traj:"<<Traj[i].str;

if(Traj[i].getType()!=WLElementTraj::line
 &&Traj[i].getType()!=WLElementTraj::arc)  continue;

movV=Traj[i].isArc() ? Traj[i].startV+Traj[i].endV : Traj[i].startV;

if((i+1)<Traj.size())
 movNV=Traj[i+1].isArc() ? Traj[i+1].startV+Traj[i+1].endV : Traj[i+1].startV;
else
 movNV=movV;

nextNBL=WLGPoint();
nextBL=m_nowBL;

foreach(QString name,GDrive)
{
 if(getDrive(name))
 {
  if(movV.get(name)!=0.0)
     {  
     nextBL.set(name,movV.get(name) > 0 ?  getDrive(name)->getHalfBacklash():-getDrive(name)->getHalfBacklash());
     }
 }
}

deltaBL=nextBL-m_nowBL;

//qDebug()<<"movV"<<movV.toString()<<" deltaBL"<<deltaBL.toString()<<" nextBL"<<nextBL.toString()<<" nowBL"<<m_nowBL.toString();;

if(Traj[i].isFast()
 &&Traj[i].isLine())//if fast set backlash in element
  {
  foreach(QString name,GDrive)
   {
   if(deltaBL.get(name)==0.0
    &&nextNBL.get(name)!=0.0) nextBL.set(name,nextNBL.get(name));
   }
  //if(deltaBL.x==0.0&&nextNBL.x!=0.0) nextBL.x=nextNBL.x;
  Traj[i].setLine(Traj[i].data.line.startPoint+m_nowBL,Traj[i].data.line.endPoint+nextBL);
  }
else  if(!deltaBL.isNull()) { //add line backlash
         if(i>0
         &&Traj[i-1].isLine()
         &&Traj[i-1].isPreBacklash()) {

            bool ok;

            Traj[i-1].calcPoints(&ok,getGModel());

            WLGPoint lastV=Traj[i-1].startV;
            WLGPoint lastBL;

            foreach(QString name,GDrive)
             {
             if(lastV.get(name)==0.0  //если нет движения у предыдущего
               &&deltaBL.get(name)!=0.0) { //и мы можем перенести выборку на предыдущий элемент
                lastBL.set(name,deltaBL.get(name)); //переносим
                deltaBL.set(name,0);
               }
             }

            if(!lastBL.isNull()) //что то перенесли
            {
            qDebug()<<"release preview backlash";
            double k=((Traj[i-1].getEndPoint()+lastBL)-Traj[i-1].getStartPoint()).getR()
                             /(Traj[i-1].getEndPoint()-Traj[i-1].getStartPoint()).getR();

            Traj[i-1].setEndPoint(Traj[i-1].getEndPoint()+lastBL);
            Traj[i-1].setF(k*Traj[i-1].F);
            Traj[i-1].calcPoints(&ok,getGModel());
            }
          }

         if(!deltaBL.isNull()){ //добавляем линию если не всё перенесли
         ETraj=Traj[i];
         ETraj.setStopMode(true);
         ETraj.setLine(Traj[i].getStartPoint()+m_nowBL,Traj[i].getStartPoint()+nextBL);

         if(m_Fbacklash!=0.0f)    ETraj.setF(m_Fbacklash);

         ETraj.calcPoints(&ok,getGModel());
         Traj.insert(i,ETraj);
         }
         else
           i--; //так как мы не добавляем элемент а всё перенесли впредыдущий
        }
        else if(Traj[i].isArc()){ //circ corr
               Traj[i].setArc(Traj[i].data.arc.startPoint+m_nowBL
                             ,Traj[i].data.arc.centerPoint+m_nowBL
                             ,Traj[i].data.arc.endPoint+m_nowBL
                             ,Traj[i].data.arc.CCW
                             ,Traj[i].data.arc.plane);
               }
               else  { //line corr
               Traj[i].setLine(Traj[i].data.line.startPoint+m_nowBL
                              ,Traj[i].data.line.endPoint+nextBL);
              }


m_nowBL=nextBL;
}


if(add){
  MutexShowTraj.lock();
  showMillTraj+=Traj.first();
  MutexShowTraj.unlock();

  MillTraj.append(Traj.takeFirst());
  }

}

int WLGMachine::updateMovPlanner()
{
WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

qDebug()<<"WLGMachine::updateMovPlanner() MillTraj.size()="<<MillTraj.size()<<"MPlanner.free()="<<ModulePlanner->getFree();

QMutexLocker locker(&MutexMillTraj);
bool ok=true;
//qDebug()<<"update Pause";
if(!ModulePlanner
  ||isPause()) {
   qDebug()<<"updateMovPlanner >> pause";
  return 0;
  }

if(m_runGProgram) // if run program
  {
  updateMovProgram();  

  if(!ModulePlanner->isEmpty()
   &&!isRunMScript())
            {
            m_Program->setLastMovElement(ModulePlanner->getCurIdElement());
            }
            else
            {
            m_Program->setLastMovElement(m_iProgram-1);
            }
  }

//qDebug()<<"update runList"<<Flag.get(ma_runlist);
if(!m_runList) {
    qDebug()<<">>noRunList";
    return 0;
    }

//qDebug()<<"update Empty";
if(isEmptyMotion())   {
    qDebug()<<">>Empty Motion";
    return 1;
    }

while((ModulePlanner->getFree()>0)
   &&(!MillTraj.isEmpty())
   &&(!isRunGProgram()
     ||isRunMScript()
     ||MillTraj.size()>1
     ||MillTraj.first().isScript()
     ||m_iProgram==(m_Program->getElementCount()))
   &&m_runList
   &&ok) //если можно отправлять
{
WLElementTraj ME=MillTraj.takeFirst();
qDebug()<<"take"<<ME.type<<ME.str<<" MillTraj.size()="<<MillTraj.size();
switch(ME.type)
{
case WLElementTraj::script: qDebug()<<"Script"<<isActiv()<<isRunMScript()<<(ME.index==ModulePlanner->getCurIdElement())<<ME.escript.singleRun;

                            if(!isActiv()  //стоит
                           ||(!isRunMScript()
                            &&ME.index==ModulePlanner->getCurIdElement())  //или порождён текущим элементом
                           &&!ME.escript.singleRun){                      //и возможен запуск паралельный
                               runMScript(ME.escript.script);
                               ok=true;
                               }
                               else{
                               ok=false;
                               }

                           break;

case WLElementTraj::delay: ok=ModulePlanner->addDelay(ME.data.delay.time
                                                     ,ME.S * getStateSpindle()
                                                     ,ME.index);
                           break;

case WLElementTraj::line:  {
                           double dFxyz=0;
                           double dF=0;
                           double dFscale=0;
                           double kF=1;
                           double dL;

                           if(ME.data.line.G93)
                           {
                           //qDebug()<<"G93";
                           foreach(WLGDrive *mD,getGDrives()) {

                            dL=(ME.data.line.endPoint.get(mD->getName())
                               -ME.data.line.startPoint.get(mD->getName())
                                )/mD->dimension();

                            dF+=dL*dL;
                            }
                           //(m_mainDim*60.0/ME.speedF) обратный коэф для будущей подстановки
                           //kF=(m_mainDim*60.0/ME.speedF)*sqrt(dF)/(60.0/ME.speedF);

                           kF=(m_mainDim)*sqrt(dF);
                           }
                           else if(!isUseGModel()){
                           foreach(WLGDrive *mD,getGDrives())  {
                            dL=(ME.data.line.endPoint.get(mD->getName())
                               -ME.data.line.startPoint.get(mD->getName())
                                )/mD->dimension();

                            dF+=dL*dL;

                            if(mD->getName()=="X"
                             ||mD->getName()=="Y"
                             ||mD->getName()=="Z") dFxyz+=dL*dL;
                            }

                           kF= dFxyz!=0.0 ? sqrt(dF/dFxyz) : 1;
                           //qDebug()<<"!isUseGModel()"<<dF<<dFxyz;
                           }
                           else
                           {
                           foreach(WLGDrive *mD,getGDrives())  {
                            dL=(ME.data.line.endPoint.get(mD->getName())
                               -ME.data.line.startPoint.get(mD->getName())
                                )/mD->dimension();
                            dF+=dL*dL;

                            dL*=mD->dimension()/m_mainDim;
                            dFscale+=dL*dL;
                            }

                           kF= sqrt(dF/dFscale);
                           }


                           if(kF!=0.0)
                              {
                              QVector <quint8> indexs;
                              QVector <qint32> ePos;
                              quint8 i=0;

                              foreach(WLGDrive *mD,getGDrives()){
                              if(!mD->getAxis()) continue;
                              ePos+=(qint32)((qint64)round(ME.data.line.endPoint.get(mD->getName())/mD->dimension()));
                              indexs+=i;
                              i++;
                              }

                              ok=ModulePlanner->addLine (MASK_abs
                                                       |(ME.isFast()  ? MASK_fline:0)
                                                       |(ME.isSmooth()? MASK_ensmooth:0)
                                                       ,indexs.size()
                                                       ,indexs.data()
                                                       ,ePos.data()
                                                       ,ME.S * getStateSpindle()
                                                       ,ME.isFast() ? -1 :(kF*ME.F/60.0)/m_mainDim
                                                       ,ME.index);
                              }
                           }
                           break;

case WLElementTraj::uline:   {
                             QVector <quint8> indexs;
                             QVector <qint32> ePos;
                             QVector <qint32> mPos;

                             quint8 i=0;

                             foreach(WLGDrive *mD,getGDrives()){
                              if(!mD->getAxis()) continue;
                              ePos+=(qint32)((qint64)round(ME.data.uline.endPoint.get(mD->getName())/mD->dimension()));
                              mPos+=(qint32)((qint64)round(ME.data.uline.midPoint.get(mD->getName())/mD->dimension()));
                              indexs+=i;
                              i++;
                              }

                              ok=ModulePlanner->addULine(MASK_abs
                                                       |(ME.isFast()  ? MASK_fline:0)
                                                       |(ME.isSmooth()? MASK_ensmooth:0)
                                                       ,indexs.size()
                                                       ,indexs.data()
                                                       ,ePos.data()
                                                       ,mPos.data()
                                                       ,ME.S * getStateSpindle()
                                                       ,ME.isFast() ? -1 : (ME.F/60)/m_mainDim
                                                       ,ME.index);
                           }
                           break;

case WLElementTraj::arc: {
                          QVector <quint8>  indexs;
                          QVector <qint32>  ePos;
                          QVector <qint32>  cPos;

                          quint8 Ib=255;
                          quint8 Jb=255;
                          quint8 Kb=255;

                          quint8 i=0;

                          foreach(WLGDrive *mD,getGDrives()){
                           if(!mD->getAxis()) continue;
                           ePos+=(qint32)((qint64)round(ME.data.arc.endPoint.get(mD->getName())/mD->dimension()));
                           cPos+=(qint32)((qint64)round(ME.data.arc.centerPoint.get(mD->getName())/mD->dimension()));
                           indexs+=i;

                           if(mD->getName()=="X") Ib=i;
                           else if(mD->getName()=="Y") Jb=i;
                           else if(mD->getName()=="Z") Kb=i;

                           i++;
                           }

                          qint32 ePosIJK[3];
                          qint32 cPosIJ[2];

                          quint8 I,J,K;

                          switch(ME.data.arc.plane)
                          {
                          case 18: I=Kb;J=Ib;K=Jb; break;
                          case 19: I=Jb;J=Kb;K=Ib; break;
                          default: I=Ib;J=Jb;K=Kb; break;
                          }

                          if(I==255||J==255)
                            {
                            sendMessage("WLGMachine",QString("error set arcmotion iI=%1 iJ%2").arg(I).arg(J),-1);
                            return 0;
                            }

                          cPosIJ[0]=cPos[I];
                          cPosIJ[1]=cPos[J];

                          ePosIJK[0]=ePos[I];
                          ePosIJK[1]=ePos[J];

                          if(K!=255){
                           ePosIJK[2]=ePos[K];
                           ePos[2]=ePosIJK[2];
                           indexs[2]=K;
                           }

                          ePos[0]=ePosIJK[0];
                          ePos[1]=ePosIJK[1];

                          indexs[0]=I;
                          indexs[1]=J;

                         ok=ModulePlanner->addCirc(MASK_abs
                                                 |(ME.data.arc.CCW  ? MASK_ccw:0)
                                                 |(ME.isFast()      ? MASK_fline:0)
                                                 |(K!=255           ? MASK_circxyz:0)
                                                 |(ME.isSmooth()    ? MASK_ensmooth:0)
                                                 ,indexs.size()
                                                 ,indexs.data()
                                                 ,ePos.data()
                                                 ,cPosIJ
                                                 ,ME.S * getStateSpindle()
                                                 ,ME.isFast() ? -1 : (ME.F/60)/m_mainDim
                                                 ,ME.index);
                          }
                          break;
}

if(!ok){
    MillTraj.prepend(ME);
    }

}


emit changedTrajSize(MillTraj.size());

qDebug()<<"WLGMachine::updateMovBuf() "<<ok<<">> MillTraj.size()="<<MillTraj.size()<<"MPlanner.free()="<<ModulePlanner->getFree();

return (ModulePlanner->getFree()>0)&&(!MillTraj.isEmpty())&&(!m_MScript->isBusy());
}


void WLGMachine::addElementTraj(QList<WLElementTraj>  ListTraj)
{
WLElementTraj         ETraj;
QList<WLElementTraj>  addTraj;
QList<WLElementTraj>  addModelTraj;

if(!ListTraj.isEmpty())
{
#ifdef DEF_HMAP
if(isRunMScript()
||!isRunGProgram())
    for(int i=0;i<ListTraj.size();i++){
    ListTraj[i].useHMap=false;
    }

getHeightMap()->addHeighMapPoints(ListTraj);
#endif

while(!ListTraj.isEmpty()){
    ETraj=ListTraj.takeFirst();

    if(ETraj.isArc())
        addTraj+=addCirclePoints(ETraj);
      else
        addTraj+=ETraj;
    }

WLElementTraj::removeEmpty(addTraj);
WLElementTraj::calcPoints(addTraj,getGModel());

addRotaryPosition(lastMillGPoint,addTraj);

if(isUseGModel()){
 addCalcGModel(addTraj);

 //Backlash
 WLElementTraj::calcPoints(addTraj,getGModel());
 addBacklash(addTraj);
 }
 else {
 //Backlash
 WLElementTraj::calcPoints(addTraj,getGModel());
 addBacklash(addTraj);

 addSmooth(addTraj); //G64 P - ULine
 }

WLElementTraj::calcPoints(addTraj,getGModel());

MillTraj+=addTraj;

MutexShowTraj.lock();
showMillTraj+=addTraj;
MutexShowTraj.unlock();
}

}



void WLGMachine::updateInput()
{
//QMutexLocker locker(&MutexInput);
WLModuleAxis *ModuleAxis=motDevice->getModuleAxis();


if(ModuleAxis->getInput(MAXIS_inEMGStop)->getCond()>1) emit changedEMG(ModuleAxis->getInput(MAXIS_inEMGStop));

if(ModuleAxis->getInput(MAXIS_inSDStop)->getCond()==2) //если была нажата
    {
  //emit sendPause(tr("the stop button")+" SD-stop "+tr("is pressed"));
   // setMessage("WLGMachine",tr("the stop button")+" SD-stop "+tr("is pressed"),0);
  //saveLog("Machine",("the stop button is pressed"));
    }

if(ModuleAxis->getInput(MAXIS_inEMGStop)->getCond()==2) //если была нажата
	{
    //setMessage("WLGMachine",tr("emg stop button")+" EMG-stop "+tr("is pressed"),-1);
  //saveLog("Machine",("the stop button is pressed"));
	}

if(ModuleAxis->getInput(MAXIS_inEMGStop)->getCond()==3) //если была отжата
    {
  //saveLog("Machine",("the stop button is pressed"));		
	}

}

void WLGMachine::setTarSOut(float val)
{
tarSOut=val;
}

QList<WLElementTraj>  WLGMachine::addCirclePoints(WLElementTraj  _ETraj)
{
//qDebug()<<"addCirclePonts";
QList<WLElementTraj> ListTraj;

///ETraj.Points.clear();
WLElementTraj  ETraj=_ETraj;

bool addPointXAxis;
bool addPointYAxis;

ETraj.data.arc.startPoint =WLGCode::convertPlane(_ETraj.data.arc.startPoint ,_ETraj.data.arc.plane,true);
ETraj.data.arc.centerPoint=WLGCode::convertPlane(_ETraj.data.arc.centerPoint,_ETraj.data.arc.plane,true);
ETraj.data.arc.endPoint   =WLGCode::convertPlane(_ETraj.data.arc.endPoint   ,_ETraj.data.arc.plane,true);

WLGDrive *Xd=getDrive("X");
WLGDrive *Yd=getDrive("Y");
WLGDrive *Zd=getDrive("Z");

switch(ETraj.data.arc.plane)
{
case 17: addPointXAxis=Xd->getBacklash()!=0;
         addPointYAxis=Yd->getBacklash()!=0;
         break;
case 18: addPointXAxis=Zd->getBacklash()!=0;
         addPointYAxis=Xd->getBacklash()!=0;
         break;
case 19: addPointXAxis=Yd->getBacklash()!=0;
         addPointYAxis=Zd->getBacklash()!=0;
         break;

default: addPointXAxis=addPointYAxis=false;
}

if(ETraj.isArc())
   {
   double Ast=ETraj.data.arc.startPoint.to3D().getAxy(ETraj.data.arc.centerPoint.to3D());
   double Aen=ETraj.data.arc.endPoint.to3D()  .getAxy(ETraj.data.arc.centerPoint.to3D());
   double   R=ETraj.data.arc.endPoint.to3D()  .getRxy(ETraj.data.arc.centerPoint.to3D());

   if(( ETraj.data.arc.CCW) &&(Aen<=Ast)) Aen+=2*M_PI;
   if((!ETraj.data.arc.CCW)&&(Aen>=Ast)) Aen-=2*M_PI;

   Ast*=180.0/M_PI;
   Aen*=180.0/M_PI;

   if(qAbs(Aen-Ast)>300) addPointXAxis=true;


   float zA=(ETraj.data.arc.endPoint.z-ETraj.data.arc.startPoint.z)
            /qAbs(Aen-Ast);

  // qDebug("Ast/en= %d / %d",iAst,iAen);

   int idA=ETraj.data.arc.CCW ? 1:-1;

   //qDebug()<<"addPoinXYAxis"<<addPointXAxis;
   //qDebug()<<"addPointYAxis"<<addPointYAxis;

   if(addPointXAxis
    ||addPointYAxis)
   {
   double Acur;


   float cdX,cdY; //cur direction
   float ldX,ldY; //last direction

   Acur=Ast;

   cdX = qRound(cos(Acur*M_PI/180.0)*100.0)/100.0;
   cdY = qRound(sin(Acur*M_PI/180.0)*100.0)/100.0;

   while(Acur!=Aen)
   {       
   ldX=cdX;
   ldY=cdY;

   if(ETraj.data.arc.CCW){
     Acur+=90.0;

     if(Acur>Aen)
          Acur=Aen;
     }
     else{
     Acur-=90.0;

     if(Acur<Aen)
          Acur=Aen;
     }

   //qDebug()<<"Acur="<<Acur;

   cdX = qRound(cos(Acur*M_PI/180.0)*100.0)/100.0;
   cdY = qRound(sin(Acur*M_PI/180.0)*100.0)/100.0;

   //qDebug()<<"cdX"<<cdX<<"ldX"<<ldX;

   if(addPointXAxis
  &&((cdX>=0.0&&ldX<0.0)||(cdX<=0.0&&ldX>0.0))
  &&(Acur!=Aen)){
     //qDebug()<<"addy";
     ListTraj+=ETraj;

     ListTraj.last().data.arc.endPoint=ListTraj.last().data.arc.centerPoint;    

     if(ETraj.data.arc.CCW){
      ListTraj.last().data.arc.endPoint.y+=  ldX < 0 ? -R : R;
      }
      else{
      ListTraj.last().data.arc.endPoint.y+=  ldX < 0 ? R : -R;
      }

     ListTraj.last().data.arc.endPoint.z=ListTraj.last().data.arc.startPoint.z+zA*qAbs((Acur-Ast));

     ListTraj.last().setArc(ListTraj.last().data.arc.startPoint
                           ,ListTraj.last().data.arc.centerPoint
                           ,ListTraj.last().data.arc.endPoint
                           ,ListTraj.last().data.arc.CCW,ListTraj.last().data.arc.plane);
     }

   if(addPointYAxis
   &&((cdY>=0.0&&ldY<0.0)||(cdY<=0.0&&ldY>0.0))
   &&(Acur!=Aen)){
     //qDebug()<<"addx";

     ListTraj+=ETraj;

     ListTraj.last().data.arc.endPoint=ListTraj.last().data.arc.centerPoint;

     if(ETraj.data.arc.CCW){
      ListTraj.last().data.arc.endPoint.x+= ldY < 0 ? R : -R;
      }
      else{
      ListTraj.last().data.arc.endPoint.x+= ldY < 0 ? -R : R;
      }

     ListTraj.last().data.arc.endPoint.z=ListTraj.last().data.arc.startPoint.z+zA*qAbs(Acur-Ast);

     ListTraj.last().setArc(ListTraj.last().data.arc.startPoint
                           ,ListTraj.last().data.arc.centerPoint
                           ,ListTraj.last().data.arc.endPoint
                           ,ListTraj.last().data.arc.CCW,ListTraj.last().data.arc.plane);
     }
   }
  }

ListTraj+=ETraj;

for(int i=1;i<ListTraj.size();i++)
    {
    ListTraj[i].data.arc.startPoint=ListTraj[i-1].data.arc.endPoint;
    }

for(int i=0;i<ListTraj.size();i++)
    {
    ListTraj[i].data.arc.startPoint =WLGCode::convertPlane(ListTraj[i].data.arc.startPoint ,_ETraj.data.arc.plane,false);
    ListTraj[i].data.arc.centerPoint=WLGCode::convertPlane(ListTraj[i].data.arc.centerPoint,_ETraj.data.arc.plane,false);
    ListTraj[i].data.arc.endPoint   =WLGCode::convertPlane(ListTraj[i].data.arc.endPoint   ,_ETraj.data.arc.plane,false);
    }
}

return ListTraj;
}

void WLGMachine::setPercentManualStr(QString str)
{
QStringList list=str.split(",");

m_percentManualList.clear();

foreach(QString vstr,list){
  m_percentManualList+=vstr.toDouble();
}
}

QString WLGMachine::getPercentManualStr()
{
QString ret;

foreach(float val,m_percentManualList)
    ret+=QString::number(val)+",";

ret.chop(1);

return ret;
}

void WLGMachine::runMCode(int iM)
{
runMScript("M"+QString::number(iM)+"()");
}

void WLGMachine::runMScript(QString txt)
{
if(m_MScript)
    {
    qDebug()<<"WLGMachine::runMScript"<<txt<<isActiv();

        //detect MCode
    bool ok;
    QString M=txt;
    int iM;

    if(M.contains(QRegExp("^M\\d[(][)]$")))
      {
      M.remove("()");
      M.remove("M");

      iM=M.toInt(&ok);

      qDebug()<<"detect MCode:"<<iM<<ok;

      switch (iM) {
          case 3: stateSpindle=FW;   break;
          case 4: stateSpindle=RE;  break;
          case 5: stateSpindle=Stop; break;
          }
      }

    m_MScript->runFunction(txt);
    }

QTimer::singleShot(0,this,&WLGMachine::updateBusy);
}

void WLGMachine::setCompleteScript(QString func)
{
qDebug()<<"WLGMachine::setCompleteScript"<<func;

if(func.isNull()) return;
else
if(func=="CONTINUE()") {
   continueMovList();
   }
else
if(func=="PAUSE()") {
   m_waitPause=false;
   }
/*else
if(func=="STOP()") {
        Flag.reset(ma_stop);
        setFinished();
       }*/
else
   setFinished();

if(!isRunMScript())
{
if(m_waitMScript)
  {
  m_waitMScript=false;
  QTimer::singleShot(0,this,SLOT(startMov()));
  }

setSafeProbe(isSafeProbe());

updateBusy();
updatePosible();
}

}



