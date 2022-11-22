#include "wldrive.h"
#include <math.h>

QList<WLDrive*> WLDrive::m_driveList;

WLDrive::WLDrive(QString _nameDrive,WLModuleAxis *_MAxis)
{
m_name = _nameDrive;
m_ModuleAxis=_MAxis;

for(int i=0;i<MAXSLAVEAXIS; i++)  m_ofstSlaveAxis[i]=0;

setLogicFindPos(WLDrive::noFind);

curKSpeed=1;

m_homePosition=0;
setOrgPosition(0);

setDimension(WLDriveDim::oneStepSize,0.01);

setPercentManual(100);

pad()->SaveAD(0,100,100,-100,"main");

setObjectName("Drive");

m_driveList+=this;
}


WLDrive::~WLDrive()
{
qDebug()<<"~WLDrive()";
m_driveList.removeOne(this);
}


int WLDrive::setPosition(double pos)
{
QMutexLocker locker(&MutexDrive);

if(isMotion())
     {sendMessage(getFullName(),tr("error setting axis position"),-34);return -1;}

m_nowPosition.set(pos,dim);

emit changedPosition(pos);

qDebug()<<getName()<<"setPosition"<<pos<<m_nowPosition.step<<" ofst:"<<m_nowPosition.offset;

setTruPosition(false);

if(getAxis()) getAxis()->setPos(m_nowPosition.getStepI32L());

m_posLast=m_nowPosition.getStepI32L();

Flag.set(fl_setpos,true);

return 0;
}

bool WLDrive::setDimension(WLDriveDim::typeDim _type,double A, double B)
{
const double lastPosition=position();

if(dim.set(_type,A,B))
{
if(getAxis())
 {
 getAxis()->setMinMaxPos(minPosition()/dim.value
                        ,maxPosition()/dim.value);

 setPosition(lastPosition);
 }
return true;
}

return false;
}

bool WLDrive::setPad(dataPad pad,typeMParAxis type)
{
bool ret=true;

qDebug()<<getName()<<"setPad"<<pad.toString()<<type;

foreach(WLAxis *Axis,m_AxisList)
{
ret=Axis->setMPar(pad.Aac/dim.value
                 ,pad.Ade/dim.value
                 ,pad.Vst/dim.value
                 ,pad.Vma/dim.value,type);

if(!ret) break;
}

return ret;
}

bool WLDrive::setMainPad()
{
if(pad()->findData("main"))
 { 
 if(pad()->findData("mainMinus"))
     {
     setPad(pad()->getData("main"),typeMParAxis::AXIS_MParPlus);
     setPad(pad()->getData("mainMinus"),typeMParAxis::AXIS_MParMinus);
     }
     else
     {
     setPad(pad()->getData("main"),typeMParAxis::AXIS_MParAll);
     }
 return true;
 }

return false;
}


void WLDrive::updatePosition()
{
QMutexLocker locker(&MutexDrive);
}

bool WLDrive::startMotion(float _Vmov)
{
qDebug()<<getName()<<"WLDrive::startMotionDrive"<<_Vmov<<position()<<">>"<<nextPosition();

QMutexLocker locker(&MutexDrive);

if(!Flag.get(fl_activ)
  ||isMotion()
  ||!getAxis()) {
 qDebug()<<!getAxis()<<isMotion()<<!getAxis();
 return false;
 }

getAxis()->resetLatch();

Flag.reset(fl_resetastop);

if(isAlarm()) {sendMessage(getFullName(),tr("drive controller has an error."),-31);return -31;}

if(_Vmov>=0.0f)   m_Vmov=_Vmov;

if(Flag.get(fl_velmode))
{
if(waitForStartMotion()<0) {sendMessage(getFullName(),tr("error when preparing for movement"),-32);return -1;}

m_startPosition=position();

if(getAxis()->movVel(Flag.get(fl_rot) ? MASK_dir:0,m_Vmov/dim.value))
  {
  //qDebug()<<getName()<<"velMode"<<" F="<<m_Vmov/dim.value;
  Flag.reset(fl_velmode);
  waitAfterStartMotion();
  }
}
else if(!isDone())
      {
      if(waitForStartMotion()<0) {sendMessage(getFullName(),tr("error when preparing for movement"),-32);return -1;}

      m_startPosition=position();

      if(getAxis()->movPos(MASK_abs,nextPositionDrive().step,m_Vmov/dim.value))
        {
        //qDebug()<<getName()<<"posMode"<<positionDrive().step<<">>"<<nextPositionDrive().step<<" F="<<m_Vmov/dim.value;
        m_nowPosition.offset=m_nextPosition.offset;
        waitAfterStartMotion();
        }

      }
     else
      {
      qDebug()<<getName()<<"Error Drive start noActiv"<<m_name;
      emit finished();
      }
/*
if(isManual())
    qDebug()<<getName()<<"start manual mode";
*/

return true;
}

void WLDrive::reset()
{
QMutexLocker locker(&MutexDrive);
qDebug()<<getName()<<"WLDrive::reset()";

if(Flag.get(fl_auto)) emit autoFinished();

resetAuto();

Flag.reset(fl_activ
          |fl_auto
          |fl_wait
          |fl_interp
          |fl_manual
          |fl_velmode
          |fl_pause
          |fl_resetastop);

setVerifyPosition();

m_nextPosition=m_nowPosition;

if(getAxis())
  {
  getAxis()->reset();
  getAxis()->restoreDelaySCurve();//set default
  getAxis()->setKF(1.0);

  if(isMotion()) getAxis()->sendGetDataAxis();
  }

emit finished();
}


int WLDrive::Mov(double dist)
{
return Mot(m_nowPosition.get(dim)+dist);
}

int WLDrive::Mot(double pos)
{    
if(pos==position()) return 0;

if(isMotion())
 { 
 qDebug()<<"unfinished previous move";
 emit sendMessage(getFullName(),tr("unfinished previous move"),-1);
 return -1;
 }

m_nextPosition=m_nowPosition;

if(isTruPosition()
 &&((pos<minPosition()&&pos<position())||(maxPosition()<pos&&pos>position()))
 &&!isInfinity())
 {
 qDebug()<<getName()<<"out limit now"<<position()<<">>"<<pos<<"("<<minPosition()<<maxPosition()<<")";
 emit sendMessage(getFullName(),tr("task out limit axis"),-1);
 return -1;
 }

m_nextPosition.set(pos,dim);

if(isDone())
 {
 qDebug()<<getName()<<"set mov to(update offset)"<<pos;
 m_nowPosition=m_nextPosition;
 Flag.set(fl_activ,false);
 return 0;
 }
else
 {
 if(calcForStartMotion()<0)  return -1;

 qDebug()<<getName()<<"set mov to"<<pos;
 Flag.set(fl_rot,m_nextPosition.step>m_nowPosition.step);
 Flag.set(fl_activ,true);
 return 1;
 }
}

int WLDrive::Vel(bool dir)
{
if(isMotion())
 {
 emit sendMessage(getFullName(),tr("unfinished previous move"),-1);
 return -1;
 }
/*
if(!isInfinity())
 {
 emit sendMessage(getFullName(),tr("err vellocity mode axis"),-1);
 return -1;
 }
*/

if(calcForStartMotion()<0)  return -1;

Flag.set(fl_rot,dir);
Flag.set(fl_activ
        |fl_velmode);

return 1;
}

int WLDrive::setCorMov(double cor)
{
QMutexLocker locker(&MutexDrive);

if(isMotion()
 ||cor==0.0) return -1;

if(Flag.get(fl_activ))
  return Mot(nextPosition()+cor);
  else
  return Mov(cor);
}

int WLDrive::addMov(double l)
{
QMutexLocker locker(&MutexDrive);

if(isMotion()||l==0.0) return -1;

if(Flag.get(fl_activ))
  return Mot(nextPosition()+l);
 else
    return Mov(l);
}

bool WLDrive::setMinMaxPosition(double minPos, double maxPos)
{
if(minPos<maxPos)
{
m_minPosition=minPos;
m_maxPosition=maxPos;

if(getAxis())
 {
 WLDrivePosition minDPos,maxDPos;

 minDPos.set(m_minPosition,dim);
 maxDPos.set(m_maxPosition,dim);

 getAxis()->setMinMaxPos(minDPos.step,maxDPos.step);

 emit changedMaxPosition(maxPos);
 emit changedMinPosition(minPos);
 }

return true;
}

return false;
}

int WLDrive::corPosition(double pos)
{
return setPosition((double)positionDrive().step*dimension()-(pos-getOrgPosition()));
}

bool WLDrive::isLatch2()
{
foreach(WLAxis *Axis,getAxisList()) {
   if(!Axis->isLatch2()) return false;
    }

return true;
}

bool WLDrive::isLatch3()
{
foreach(WLAxis *Axis,getAxisList()){
   if(!Axis->isLatch3()) return false;
   }

return true;
}


int WLDrive::setHomePosition(double pos)
{
m_homePosition=pos;

if(getAxis())
 {
 WLDrivePosition homeDPos;

 homeDPos.set(m_homePosition,dim);

 getAxis()->setHomePos(homeDPos.step);
 }

return 1;
}

bool WLDrive::isMotionSubAxis()
{
foreach(WLAxis *Axis,getSubAxisList())
 {
 if(Axis->isMotion()) return true;
 }

return false;
}



void WLDrive::setFinished()
{
if(isMotion()) return;

qDebug()<<getName()<<"WLDrive::setFinished"<<isMotion();

MutexDrive.lock();

waitAfterMotion();

if(Flag.get(fl_manual))
 {
 Flag.reset(fl_manual);
 setDone();
 }

if(isDone())
  {
  Flag.set(fl_activ
	      |fl_interp,0);
  qDebug()<<getName()<<"->finshed"<<m_nowPosition.step;
  emit finished();
  }
else
  {
  qDebug()<<getName()<<"->paused"<<m_nowPosition.step<<m_nextPosition.step;
  emit paused();
  }

MutexDrive.unlock();

if(Flag.get(fl_resetastop))
  {
  qDebug()<<getName()<<"resetAStop";
  Flag.reset(fl_resetastop);
  reset();
  }
}


void WLDrive::startMovManual(double _rot,double dist)
{
qDebug()<<getName()<<"start Manual rot="<<_rot;//<<isMotion();

dist=qAbs(dist);

if(!isEnable()
  ||isAuto()){
   return;
   }

setPercentManual(qAbs(_rot)*100);

if(isMotion()) 
   {
   if(((_rot>0.0f)&& rot())
    ||((_rot<0.0f)&&!rot()))
       startAccel();
   else
       startStop();

   return;
   }

setKSpeed(m_percentManual/100.0);

pad()->Load("main");

if(_rot<0.0) pad()->Load("mainMinus");

pad()->Load("manual");

setCurPad();

Flag.set(fl_manual,1);

if(dist!=0.0)
{    
if(_rot>0.0) {
 if(((position()+dist)<maxPosition()) || isInfinity() || !isTruPosition())
       setMov(dist);
    else
       setMot(maxPosition());

 }
 else{ //rot -dist {
 if(((position()-dist)>minPosition()) || isInfinity() || !isTruPosition()){
     setMov(-dist);
     }
     else{
     setMot(minPosition());
     }
 }
}
else if(isInfinity()){
      setVel(_rot>0.0);
      }
      else if(isTruPosition())  {
            if(_rot>0.0) {
              if(position()<maxPosition()) setMot(maxPosition());
              }
              else{
              if(position()>minPosition()) setMot(minPosition());
              }
            }
           else{
           setMov((_rot>0.0 ? 2:-2)*(maxPosition()-minPosition()));
           }

startMotion();
}

void WLDrive::resetAlarm()
{
foreach(WLAxis *axis,getAxisList())
 {
 axis->getOutput(AXIS_outRALM)->setOutPulse(1,100);
 }
}

void WLDrive::updateInALM()
{
if(getInput(AXIS_inALM))
 {
 sendMessage(getFullName(),tr("controller error (inALM)"),33);
 setTruPosition(false);
 }
}

/*
void WLDrive::inicial()
{

}
*/
bool WLDrive::setVmov(double V)
{
if(V>=0)
{
qDebug()<<getName()<<"set Vmov="<<V;
m_Vmov=V;

return true;
}
else
    return false;
}

bool WLDrive::setScurve(double Scur)
{
if(getAxis())
 {
 if(Scur<getAxis()->getDelaySCurve())
       Scur=getAxis()->getDelaySCurve();

 qDebug()<<getName()<<" Scur="<<Scur;

 if(isMotion()) qDebug()<<"error setScurve"<<Scur;

 getAxis()->setDelaySCurve(Scur,false);

 return true;
 }

return false;
}


bool WLDrive::setKGear(float k)
{
const double lastPosition=position();

if(dim.setKGear(k))
{
if(getAxis())
 {
 getAxis()->setKGear(k);

 for(int i=1;i<m_AxisList.size();i++)
             getAxis(i)->setKGear(k);

 setMinMaxPosition(minPosition(),maxPosition());
 }
setPosition(lastPosition);

return true;
}

return false;

}

void WLDrive::setInterp(bool set)
{
if(distance()!=0)
 {
 Flag.set(fl_interp,set);
 interPad=pad()->getData();
 }
}


void WLDrive::addInResolutionMov(WLIOPut *_inEnableMov,bool state)
{
resolutMovIOData+=WLIOData(_inEnableMov,state);

connect(_inEnableMov,SIGNAL(changed(int)),SLOT(updateInResolutionMov()),Qt::QueuedConnection);
}

void WLDrive::removeInResolutionMov(WLIOPut *_inEnableMov)
{
for(int i=0;i<resolutMovIOData.size();i++)
{
WLIOData data=resolutMovIOData.at(i);

if(data.IOput==_inEnableMov)
  {
  resolutMovIOData.removeAt(i);
  disconnect(_inEnableMov,SIGNAL(changed()),this,SLOT(updateInResolutionMov()));
  break;
  }
}
}

WLDrive::typeLogiFind WLDrive::addHomeLogicFindPos(WLDrive::typeLogiFind logic)
{
switch(logic)
{
case onlyMEL: return onlyMELHome;
case onlyPEL: return onlyPELHome;
case onlyMORG: return onlyMORGHome;
case onlyPORG: return onlyPORGHome;

default: return logic;
}

}

WLDrive::typeLogiFind WLDrive::removeHomeLogicFindPos(WLDrive::typeLogiFind logic)
{
switch(logic)
{
case onlyMELHome: return onlyMEL;
case onlyPELHome: return onlyPEL;
case onlyMORGHome:return onlyMORG;
case onlyPORGHome:return onlyPORG;

default: return logic;
}
}

bool WLDrive::isHomeLogicFindPos(WLDrive::typeLogiFind logic)
{
return   logic==onlyMELHome
       ||logic==onlyPELHome
       ||logic==onlyMORGHome
       ||logic==onlyPORGHome;
}

double WLDrive::calcRotaryInfEndPosition(double startPos, double endPos)
{
double sP,sPC,eP,ePC;

sP=modf(startPos/360.0,&sPC);
eP=modf(endPos/360.0,&ePC);

if(qAbs(eP-sP)>0.5)
 {
 if(eP>sP)
     eP-=1.0;
 else
     eP+=1.0;
 }

//qDebug()<<"calcRotaryInfEndPosition"<<startPos<<endPos<<"calc="<<(sPC+eP)*360.0;

return (sPC+eP)*360.0;
}

void WLDrive::updateInResolutionMov()
{
for(int i=0;i<resolutMovIOData.size();i++)
 if(!resolutMovIOData[i].isTry())
 {
 if(isMotion())  
   {
   startStop();
   sendMessage(getFullName(),tr("wrong resolution:")
              +(resolutMovIOData[i].IOput->isInput() ? "input:":"output:")
              +QString::number(resolutMovIOData[i].IOput->getIndex())
              +" ("+resolutMovIOData[i].IOput->getComment()+")",-15);
   }
 return;
 }
};

void WLDrive::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("name",         getName());
stream.writeAttribute("indexMAxis",   getIndexModuleAxisStr());

stream.writeAttribute("type", QString::number(getType()));

stream.writeAttribute("Dim",getDriveDim().toString());

if(!isInfinity())
 {
 stream.writeAttribute("MinPosition",  QString::number(minPosition(),'F',3));
 stream.writeAttribute("MaxPosition",  QString::number(maxPosition(),'F',3));
 }

stream.writeAttribute("Position",     QString::number(position(),'F',3));
stream.writeAttribute("orgPosition",  QString::number(getOrgPosition(),'F',3));
stream.writeAttribute("homePosition", QString::number(getHomePosition(),'F',3));
stream.writeAttribute("orgSize",      QString::number(getORGSize(),'F',3));
stream.writeAttribute("logicFindPos", QString::number(getLogicFindPos()));

QString str;

for (int i=0;i<MAXSLAVEAXIS;i++)
 {
 str+=QString::number(m_ofstSlaveAxis[i])+",";
 }

str.chop(1);

stream.writeAttribute("ofstSlaveAxis",str);
stream.writeAttribute("feedVFind1",     QString::number(getVFind1()));
stream.writeAttribute("feedVFind2",     QString::number(getVFind2()));
stream.writeAttribute("backDistFind",   QString::number(getBackDistFind()));
stream.writeAttribute("freeDistFind",   QString::number(getFreeDistFind()));
stream.writeAttribute("Pad",     m_Pad.toString());
/*
foreach(dataPad pad,m_Pad.getDataList())
 {
 stream.writeStartElement("Pad");
 stream.writeAttribute("data",pad.toString());
 stream.writeEndElement();
 }
*/
}

void WLDrive::readXMLData(QXmlStreamReader &stream)
{
if(!stream.attributes().value("name").isEmpty())
  setName(stream.attributes().value("name").toString());

setIndexModuleAxisStr(stream.attributes().value("indexMAxis").toString());

if(!stream.attributes().value("DimensionN").isEmpty()
 &&!stream.attributes().value("DimensionD").isEmpty())
setDimension(WLDriveDim::typeDim::oneStepSize,stream.attributes().value("DimensionN").toString().toDouble()/(1<<xPD)
                                             ,stream.attributes().value("DimensionD").toString().toDouble());

if(!stream.attributes().value("Dim").isEmpty())
   {
   WLDriveDim dd;

   dd.fromString(stream.attributes().value("Dim").toString());

   setDriveDim(dd);
   }

if(!stream.attributes().value("ofstSlaveAxis").isEmpty())
  {
  QStringList listOfst=stream.attributes().value("ofstSlaveAxis").toString().split(",");
  for (int i=0;i<MAXSLAVEAXIS&&i<listOfst.size();i++) {
      setOffsetAxis(i,listOfst[i].toDouble());
      }
  }

if(!stream.attributes().value("type").isEmpty())
   setType(static_cast<typeDrive>(stream.attributes().value("type").toInt()));
else
   setType();

if(!stream.attributes().value("MinPosition").isEmpty()
 &&!stream.attributes().value("MaxPosition").isEmpty())
    setMinMaxPosition(stream.attributes().value("MinPosition").toString().toDouble()
                     ,stream.attributes().value("MaxPosition").toString().toDouble());
else
    setInfinity();

qDebug()<<"position"<<stream.attributes().value("Position").toString();

setPosition(stream.attributes().value("Position").toDouble());

if(!stream.attributes().value("orgPosition").isEmpty()) 
       setOrgPosition(stream.attributes().value("orgPosition").toDouble());

if(!stream.attributes().value("homePosition").isEmpty()) 
       setHomePosition(stream.attributes().value("homePosition").toDouble());

if(!stream.attributes().value("orgSize").isEmpty()) 
       setORGSize(stream.attributes().value("orgSize").toString().toDouble());

if(!stream.attributes().value("logicFindPos").isEmpty()) 
       setLogicFindPos(static_cast<WLDrive::typeLogiFind>(stream.attributes().value("logicFindPos").toInt()));

if(!stream.attributes().value("feedVFind").isEmpty())  //for old style
       setVFind1(stream.attributes().value("feedVFind").toDouble());

if(!stream.attributes().value("feedVFind1").isEmpty())
       setVFind1(stream.attributes().value("feedVFind1").toDouble());

if(!stream.attributes().value("feedVFind2").isEmpty())
       setVFind2(stream.attributes().value("feedVFind2").toDouble());

if(!stream.attributes().value("backDistFind").isEmpty())
       setBackDistFind(stream.attributes().value("backDistFind").toDouble());

if(!stream.attributes().value("freeDistFind").isEmpty())
       setFreeDistFind(stream.attributes().value("freeDistFind").toDouble());

if(!stream.attributes().value("Pad").isEmpty())
       pad()->fromString(stream.attributes().value("Pad").toString());

Init();
}

void WLDrive::resetAuto()
{
qDebug()<<getName()<<"WLDrive::resetAuto()";

Flag.set(fl_auto,0);

emit changedAuto(false);

autoTypeDrive=autoNo;

if(getAxis())
    getAxis()->setDisableManual(false);
}



bool WLDrive::setAuto()
{
qDebug()<<getName()<<"WLDrive::setAuto()";

if(Flag.get(fl_auto)){
 return false;
 }
 else{
 Flag.set(fl_auto);

 setResetAfterStop(false);

 emit changedAuto(true);

 if(getAxis())
     getAxis()->setDisableManual(true);

 return true;
 }
}

int WLDrive::setMovToHome()
{
qDebug()<<"WLDrive::setMovToHome"<<getName()<<"setMovToHome";
return setMot(getHomePosition());
}

int WLDrive::setMovToORG()
{
qDebug()<<"WLDrive::setMovToORG()"<<getName()<<"setMovToORG";
return setMot(getOrgPosition());
}

bool WLDrive::getInput(typeInputAxis type,bool all)
{
foreach(WLAxis *axis,getAxisList()) {
    if(all) {
    if(!axis->getInput(type)->getNow()){    //if all true
       return false;
       }
    }
    else if(axis->getInput(type)->getNow()){    //if all false
         return true;
         }
   }
return  all;
}

bool WLDrive::startInterp()
{
QList <WLDrive*> Drives;

if(!isInterp()) return false;

qDebug()<<getName()<<"startInterp";

for(int i=0;i<m_driveList.size();i++)
{
     if(m_driveList[i]->isInterp()
 &&!m_driveList[i]->isMotion()
 &&(m_driveList[i]->distance()!=0.0))
    {
     if(m_driveList[i]->isWait())
       {
       qDebug()<<"waitDrive for interp"<<m_driveList[i]->getName();
       return false; //ждем когда все могут двигаться
       }
     Drives+=m_driveList[i];
     }
}

qDebug()<<"size interp"<<Drives.size();

if(Drives.isEmpty()) return true;

int const n=Drives.size();

double *kDist=new double[n];

dataPad  mainPad,Pad;

//находим самое длинное перемещение
int iDistMax=0;
float maxScur=0;

for(int i=1;i<n;i++) {
  if(qAbs((float)Drives[i]->distance())
    >qAbs((float)Drives[iDistMax]->distance()))
	iDistMax=i;

  if(maxScur<Drives[i]->getAxis()->getDelaySCurve())
       maxScur=Drives[i]->getAxis()->getDelaySCurve();
  }

mainPad=Drives[iDistMax]->pad()->getData(); //у кого самый большой путь
//находим коэффицинеты
for(int i=0;i<n;i++)
  kDist[i]=qAbs((float)Drives[i]->distance())
          /qAbs((float)Drives[iDistMax]->distance());

for(int i=0;i<n;i++)  
    qDebug()<<Drives[i]->distance()<<"K="<<kDist[i]<<"Pad"<<Drives[i]->pad()->getData().Vst<<Drives[i]->pad()->getData().Vma;

qDebug()<<"mPad0"<<mainPad.Vst<<mainPad.Vma<<mainPad.Aac<<mainPad.Ade;
//находим коэффиуиенты kTac kTde kVst kVfi для всех
for(int i=0;i<n;i++)
 {
 if(qAbs(Drives[i]->interPad.Aac/kDist[i])<qAbs(mainPad.Aac)) //если ускорение превышает возможное
    mainPad.Aac=Drives[i]->interPad.Aac/kDist[i];
    
 if(qAbs(Drives[i]->interPad.Ade/kDist[i])<qAbs(mainPad.Ade)) //если торможение превышает возможное
    mainPad.Ade=Drives[i]->interPad.Ade/kDist[i];

 if(Drives[i]->interPad.Vst<(kDist[i]*mainPad.Vst)) //если скорость превышает возможное
    mainPad.Vst*=Drives[i]->interPad.Vst/(kDist[i]*mainPad.Vst);
 
 if(Drives[i]->getVmov()!=0.0
  &&Drives[i]->getVmov()<Drives[i]->interPad.Vma)
     Drives[i]->interPad.Vma=Drives[i]->getVmov();

 if(Drives[i]->interPad.Vma<(kDist[i]*mainPad.Vma)) //если скорость превышает возможное
    mainPad.Vma*=Drives[i]->interPad.Vma/(kDist[i]*mainPad.Vma);
 }
qDebug()<<"mPad1"<<mainPad.Vst<<mainPad.Vma<<mainPad.Aac<<mainPad.Ade;;
//заполняем
Pad.Aac=mainPad.Aac;
Pad.Ade=mainPad.Ade;

for(int i=0;i<n;i++)
 {
 Drives[i]->interPad.Aac=kDist[i]*mainPad.Aac;
 Drives[i]->interPad.Ade=kDist[i]*mainPad.Ade;
 Drives[i]->interPad.Vst=kDist[i]*mainPad.Vst;
 Drives[i]->interPad.Vma=kDist[i]*mainPad.Vma;

 Drives[i]->setPad(Drives[i]->interPad);
 Drives[i]->getAxis()->setDelaySCurve(maxScur,false);
 Drives[i]->startMotion(0);
 }

return true;
}

void WLDrive::toStartAccel()
{
    Flag.reset(fl_pause);

    //if(getAxis())
    //   getAxis()->acc();

foreach(WLAxis *axis,getAxisList()) {
 axis->acc();
 }
}

void WLDrive::setKSpeed(float k)
{
if(k>0&&k<=1)
 { 
 if(isInterp())
      {
      for(int i=0;i<m_driveList.size();i++)
          if(m_driveList[i]->isInterp())
			  {
              m_driveList[i]->toSetKSpeed(k);
		      }
	  }
 else
   toSetKSpeed(k);
 }
}

void WLDrive::setPercentManual(float per)
{
if((0.0f<per)&&(per<=100.0f))
 {
 m_percentManual=per;

 qDebug()<<getName()<<"setManualPercent"<<per<<isManual();

 if(isMotion()
  &&!isAuto())
     setKSpeed(m_percentManual/100.0f);
 }

};


void WLDrive::toSetKSpeed(float k)
{
if(getAxis())
   getAxis()->setKF(k);
}

void WLDrive::toStartPause()
{
if(!isManual())
 {
 Flag.set(fl_pause);

// if(getAxis())
//    getAxis()->pause();

 foreach(WLAxis *axis,getAxisList()) {
  axis->pause();
  }
 }
}

void WLDrive::toStartStop()   
{
qDebug()<<getName()<<"toStartStop()";

Flag.reset(fl_pause);

foreach(WLAxis *axis,getAxisList()) {
 axis->sdStop();
 }
}

void WLDrive::startAccel()
{
if(isInterp())
   {
   for(int i=0;i<m_driveList.size();i++)
       if(m_driveList[i]->isInterp())  m_driveList[i]->toStartAccel();
   }
else
 toStartAccel();

QTimer::singleShot(10,this,SLOT(updateAuto()));
}

void WLDrive::startPause()
{
if(isInterp())
   {
   for(int i=0;i<m_driveList.size();i++)
       if(m_driveList[i]->isInterp())  m_driveList[i]->toStartPause();
   }
else
 toStartPause();
}

void WLDrive::startStop()
{
if(isInterp())
   {
   for(int i=0;i<m_driveList.size();i++)
       if(m_driveList[i]->isInterp())  m_driveList[i]->toStartStop();
   }
else
 toStartStop();
}

void WLDrive::startEMGStop()    
{
resetAuto();

if(isInterp())
   {
   for(int i=0;i<m_driveList.size();i++)
       if(m_driveList[i]->isInterp())
		   {
           m_driveList[i]->resetAuto();
           m_driveList[i]->toStartStop();///!!!
	       }
   }
else
 toStartEMGStop();
}


void WLDrive::toStartEMGStop()    
{
resetAuto(); 
if(getAxis())
   getAxis()->emgStop();
}

void WLDrive::updateInputs()
{
}

void WLDrive::resetDrives()
{
for(int i=0;i<m_driveList.size();i++)
    m_driveList[i]->reset();
}

void WLDrive::accelDrives()
{
for(int i=0;i<m_driveList.size();i++)
        m_driveList[i]->startAccel();
}

void WLDrive::pauseDrives()
{
for(int i=0;i<m_driveList.size();i++)
        m_driveList[i]->startPause();
}

bool WLDrive::isActivDrives()
{
bool ret=false;

for(int i=0;i<m_driveList.size();i++)
    if(m_driveList[i]->isActiv()) ret=true;

return ret;
}

bool WLDrive::isMotionDrives()
{
bool ret=false;

for(int i=0;i<m_driveList.size();i++)
    if(m_driveList[i]->isMotion()) ret=true;

return ret;
}


WLDrive *WLDrive::getDrive(QString name)
{
WLDrive *ret=nullptr;

name=name.toLower();

for(int i=0;i<m_driveList.size();i++)
 if(m_driveList[i]->getName().toLower()==name)
  {
  ret=m_driveList[i];
  break;
  }

return ret;
}

QList<WLDrive *> WLDrive::getDriveList()
{
return m_driveList;
}

void WLDrive::startStopDrives()
{
qDebug()<<"WLDrive::startStopDrives()";

foreach(WLDrive *drive,m_driveList)
          {
          if(drive->isAuto())
                drive->resetAuto();

          if(drive->isMotion())
              drive->setResetAfterStop();

          drive->startStop();
          }
}

void WLDrive::setMainPadDrives()
{
foreach(WLDrive *drive,m_driveList)
           drive->setMainPad();
}

bool WLDrive::Init()
{
setTruPosition(false);

if(getAxis())
 {
 m_posLast=getAxis()->getNowPos();
 return true;
 }

return false;
}

void WLDrive::setModuleAxis(WLModuleAxis *_ModuleAxis)
{
if(_ModuleAxis) {
    m_ModuleAxis=_ModuleAxis;
    m_AxisList.clear();
}
}


void WLDrive::setIndexModuleAxisStr(QString str)
{
QList <quint8> indexs;
QStringList List=str.split(",",QString::SkipEmptyParts);

qDebug()<<getName()<<"setIndexModuleAxisStr"<<str;

if(!getModuleAxis()) qDebug()<<getName()<<"no ModuleAxis";

setEnable(false);

foreach(WLAxis *axis,m_AxisList)
 {
 axis->reset(); 

 disconnect(axis,&WLAxis::finished,this,&WLDrive::updateAuto);
 disconnect(axis,&WLAxis::changedLatch2,this,&WLDrive::updateAuto);
 disconnect(axis,&WLAxis::changedLatch3,this,&WLDrive::updateAuto);

 if(axis==getAxis())
  {
  disconnect(axis,&WLAxis::changedFreq,this,&WLDrive::updateFreq);
  disconnect(axis,&WLAxis::changedError,this,&WLDrive::setError);
  disconnect(axis,&WLAxis::changedPosition,this,&WLDrive::updatePos);
  disconnect(axis,&WLAxis::finished,this,&WLDrive::setFinished);
  }
 }

m_AxisList.clear();

foreach(QString num,List)
 {
 if(0<=num.toInt()
     &&num.toInt()<m_ModuleAxis->getSizeAxis()
     &&m_AxisList.size()<MAXSLAVEAXIS)
            {
            WLAxis *axis=m_ModuleAxis->getAxis(num.toInt());
            qDebug()<<"Axis add"<<num.toInt()<<axis->isEnable();

            if(!axis) {sendMessage(getName(),tr(" index axis is not correct"),-1);continue;}

            connect(axis,&WLAxis::finished,this,&WLDrive::updateAuto,Qt::QueuedConnection);
            connect(axis,&WLAxis::changedLatch2,this,&WLDrive::updateAuto,Qt::QueuedConnection);
            connect(axis,&WLAxis::changedLatch3,this,&WLDrive::updateAuto,Qt::QueuedConnection);

            if(m_AxisList.isEmpty())
                {
                connect(axis,&WLAxis::changedFreq,this,&WLDrive::updateFreq);
                connect(axis,&WLAxis::changedError,this,&WLDrive::setError);
                connect(axis,&WLAxis::changedPosition,this,&WLDrive::updatePos,Qt::DirectConnection);
                connect(axis,&WLAxis::finished,this,&WLDrive::setFinished,Qt::QueuedConnection);
                }
            else
                {
                axis->setModeSub(getAxis()->getIndex());
                axis->setDisableLimit(true);
                axis->setPos(0);
                axis->setKF(1.0);
                }

            m_AxisList.append(axis);
            }
 }
}

QString WLDrive::getIndexModuleAxisStr()
{
QString str;

foreach(WLAxis *axis,m_AxisList)
 {
 str+=QString::number(axis->getIndex())+",";
 }

str.chop(1);

return str;
}

void WLDrive::setError( quint8 error)
{ 
if(error!=0) {
    reset();
    if(error==errorAxis_emg)
        setTruPosition(false);
    }
}

void WLDrive::setEnable(bool enable)
{
m_enable=enable;

foreach(WLAxis *Axis,getAxisList()) {
 Axis->setEnable(enable);
 }
}

void WLDrive::startTask()
{
 qDebug()<<getName()<<"startTask motion"<<isMotion()
          <<"activ"<<isActiv()
          <<"wait"<<isWait()
          <<"auto"<<isAuto();

if(isWait()) return;
    else
        if(startAuto()) return;
        else
            if(startInterp()) return;
            else
            {
            setCurPad();
            startMotion();
            }
}


void WLDrive::updatePos(qint32 Pos)
{
QMutexLocker locker0(&MutexDrive);

    if(Flag.get(fl_setpos))
    {
    if(m_nowPosition.getStepI32L()==Pos) {
        qDebug()<<getName()<<"WLDrive:setPos complete"<<Pos<<position();
        m_posLast=Pos;
        Flag.reset(fl_setpos); //позиция установлена
        }
    return;
    }else if(m_posLast!=Pos){
     qint64 newPos=m_nowPosition.step;

     newPos&=0xFFFFFFFF00000000;
     newPos|=(quint32)Pos;

     bool dir=isInfinity()? (Pos-m_posLast)>0: Pos>m_posLast;

     if(dir) {
       if(m_posLast<0&&Pos>=0)  {
       newPos+=(qint64)1<<32;
       //qDebug()<<"detect turn+"<<Pos;
       }
     }else{
       if(m_posLast>=0&&Pos<0){
         newPos-=(qint64)1<<32;
         //qDebug()<<"detect turn-"<<Pos;
         }
     }

    if(m_nowPosition.step!=newPos){
       Flag.set(fl_rot,newPos>m_nowPosition.step);
       m_posLast=Pos;
       //qDebug()<<">>"<<getName()<<Pos;
       m_nowPosition.step=newPos;
       waitAfterUpdatePosition();
	   }  
  }
}

void WLDrive::updateFreq(float F)
{
m_Vnow=dimension()*static_cast<double>(F);
emit changedVnow(m_Vnow);
}
 

void WLDrive::setLatch2(qint32 Pos)
{
Q_UNUSED(Pos)
qDebug()<<getName()<<"WLDrive::setLatch2()"<<Pos;
updateAuto();
}

void WLDrive::setLatch3(qint32 Pos)
{
Q_UNUSED(Pos);
qDebug()<<getName()<<"WLDrive::setLatch3()"<<Pos;
updateAuto();
}

void WLDrive::addSyhData(double pos)
{
WLDrivePosition drivePos;

drivePos.set(pos,dim);

if(getAxis())
   getAxis()->addSyhData(drivePos.step);
}


int WLDrive::setMovFind()  
{
QMutexLocker locker(&MutexDrive);
qDebug()<<getName()<<"WLDrive::setMovFind()";
if(setAuto())
   {
   setVerifyPosition(false);
   autoOperation=0; 
   autoTypeDrive=autoFind;
   return 1;
   }
  else
    sendMessage(getFullName(),tr("movement setup error"),-10);

return 0;
}; 

int  WLDrive::setMovTeach()  
{
QMutexLocker locker(&MutexDrive);
qDebug()<<getName()<<"setMovTeach()";
if(setAuto())
   {
   setVerifyPosition(false);
   autoOperation=0; 
   autoTypeDrive=autoTeach;
   return 1;
   }
   else
   sendMessage(getFullName(),tr("movement setup error"),-10);

return 1;
}; 

int WLDrive::setMovVerify()
{
QMutexLocker locker(&MutexDrive);
qDebug()<<getName()<<"WLDrive::setMovVerify()";
if(setAuto())
   {
   setVerifyPosition(false);
   autoOperation=0;
   autoTypeDrive=autoVerify;
   return 1;
   }
   else
   sendMessage(getFullName(),tr("movement setup error"),-10);

return 1;
};

void WLDrive::updateAuto()
{
qDebug()<<getName()<<"WLDrive::updateAuto<<"<<isAutoDrive()<<autoOperation<<"pause"<<isPause();
if(isAutoDrive()
 &&!isPause()
 &&getAxis())
 {
 qDebug()<<"autoTypeDrive"<<autoTypeDrive<<"autoTypeDrive"<<logicFindPos<<"isMotion()"<<isMotion()<<"isMotionSubAxis()"<<isMotionSubAxis();

 switch(logicFindPos)
 {
 case noFind:  if(autoTypeDrive==autoTeach)
                            setPosition(getOrgPosition());

               setTruPosition(true);
               reset();
	           break;

 case onlyORGHome:
 case onlyORG:   //if(!isMotion())
	             switch(autoOperation)
                   {
                    case 0: if(getInput(AXIS_inORG,true))
    					    {
							setTruPosition(false);

                            setMainPad();

                            m_orgPosCountList.clear();

                            foreach(WLAxis *Axis,getAxisList())
                             {
                             Axis->setInLatch(Axis->getInput(AXIS_inORG)->getIndex());
                             Axis->resetLatch();

                             if(Axis!=getAxis())
                                Axis->setPos(getAxis()->getNowPos()); //set position subAxis

                             Axis->setKF(1.0);

                             Axis->movPos(MASK_disubaxis,-1.5*getORGSize()/dim.value,m_VFind1/dim.value);//mov out org zone

                             m_orgPosCountList<<0.0;
                             }

                            autoOperation=1;
    						}
    					   else
    					    {                            
                            emit sendMessage(getFullName(),tr("wrong starting position")+"(inORG=1)",-8);
                            reset();
    					    }

    						break;
    
                   case 1: if(!isMotion()&&!isMotionSubAxis())
                           {
                           if(isLatch3()) //if all inOrg detect front
                            {
                            foreach(WLAxis *Axis,getAxisList()) {
                             Axis->resetLatch();
                             Axis->movPos(MASK_disubaxis,3*getORGSize()/dim.value,m_VFind1/dim.value);//mov in-out org zone back
                             }
                            autoOperation=2;
    					    }
    					   else
    						{                            
                            emit sendMessage(getFullName(),tr("no sensor signal")+"(inORG)",-212);
                            reset();
                            }
                           }else if(isLatch3()){
                                 foreach(WLAxis *Axis,getAxisList()){
                                  Axis->sdStop();
                                  }
                                 }
      	            	   break; 

                   case 2:  if(!isMotion()&&!isMotionSubAxis())
                            {
                            if(isLatch2()&&isLatch3())
    					     {                             
                             for(quint8 i=0;i<getAxisList().size();i++){
                              m_orgPosCountList[i]+=getLatch2Pos(i);
                              m_orgPosCountList[i]+=getLatch3Pos(i);
                              }

                             foreach(WLAxis *Axis,getAxisList()) {
                              Axis->resetLatch();
                              Axis->movPos(MASK_disubaxis,-3*getORGSize()/dim.value,m_VFind1/dim.value);//mov in-out org zone back
                              }

                             autoOperation=3;
    					     }
    					   else
                             {
                             emit sendMessage(getFullName(),tr("no sensor signal")+"(inORG)",-213);
                             reset();
    						 }
                            }else if(isLatch2()&&isLatch3()){
                                   foreach(WLAxis *Axis,getAxisList()){
                                   Axis->sdStop();
                                   }
                           }
      	            	   break; 

                 case 3: if(!isMotion()&&!isMotionSubAxis())
						   {
					       if(isLatch2()&&isLatch3())
    					      {                               
                              autoOperation=20;

                              for(quint8 i=0;i<getAxisList().size();i++){
                                m_orgPosCountList[i]+=getLatch2Pos(i);
                                m_orgPosCountList[i]+=getLatch3Pos(i);

                                m_orgPosCountList[i]/=4.0;
                                }

                              switch(autoTypeDrive)
                              {
                              case autoTeach: setOrgPosition(m_orgPosCountList.first());

                                              for(quint8 i=1;i<getAxisList().size();i++) {
                                                setOffsetAxis(i,m_orgPosCountList[i]-m_orgPosCountList.first());
                                                }

                                              setTruPosition();
                                              reset();

                                              break;

                              case autoFind: corPosition(m_orgPosCountList.first());

                                             if(!getSubAxisList().isEmpty())
                                              {
                                               WLDrivePosition DPos;

                                               for(quint8 i=0;i<getAxisList().size();i++)
                                                   {
                                                   DPos.set(getOffsetAxis(i),dim);
                                                   DPos.step-=(getAxis(i)->getNowPos()-m_orgPosCountList[i]/dimension());

                                                   if(DPos.getStepI32L()!=0){
                                                     getAxis(i)->movPos(MASK_disubaxis,DPos.getStepI32L(),m_VFind1/dim.value);
                                                     }
                                                   }

                                             QTimer::singleShot(250,this,SLOT(updateAuto()));
                                             }
                                             else
                                                {
                                                qDebug()<<"to end Find";
                                                goto endfindORG;
                                                }
                                            break;

                              case autoVerify:    QString str;

                                                  WLDrivePosition orgPos;
                                                  orgPos.set(getOrgPosition(),dim);

                                                  for(quint8 i=0;i<getAxisList().size();i++){
                                                      str+=QString(" axis %1: %2").arg(getAxis(i)->getIndex())
                                                                                     .arg(m_orgPosCountList[i]-getOrgPosition()
                                                                                     +getOffsetAxis(i),0,'f',5);

                                                      qDebug()<<"Verify"<<getLatch2Pos(i)<<getOrgPosition()<<getOffsetAxis(i);
                                                      }

                                                  emit sendMessage(getFullName(),str,1);

                                                  goto endfindORG;

                                                  break;


                              }
						      }
    					   else
                              {
                              emit sendMessage(getFullName(),tr("no sensor signal")+"(inORG)",-214);
                              reset();
      	            	      }
                           }else if(isLatch2()&&isLatch3()){
                                  foreach(WLAxis *Axis,getAxisList()){
                                  Axis->sdStop();
                                  }
                                 }
                          break;
                case 20:  endfindORG:

                         qDebug()<<" endfindORG:";
                         if(!isMotion()&&!isMotionSubAxis()){
                          reset();
                          setTruPosition(true);

                          if(logicFindPos==onlyORGHome) {
                            if(setMovToHome())  startMotion(0);
                            }

                          if(logicFindPos==onlyORG){
                            if(setMovToORG())  startMotion(0);
                            }
                          }
                          break;
                }

                break;
 case onlyPORG:
 case onlyMORG:
 case onlyPORGHome:
 case onlyMORGHome:
 case onlyPELHome:
 case onlyPEL:
 case onlyMELHome:
 case onlyMEL:
                typeInputAxis typePM;

                if(logicFindPos==onlyPEL
                 ||logicFindPos==onlyPELHome) {
                   typePM=AXIS_inPEL;
                   }
                else
                if(logicFindPos==onlyMEL
                 ||logicFindPos==onlyMELHome) {
                   typePM=AXIS_inMEL;
                   }
                   else {
                   typePM=AXIS_inORG;
                   }

                //qDebug()<<"auto PEL/MEL"<<isMotion()<<isMotionSubAxis();
                if(!isMotion()&&!isMotionSubAxis())
                switch(autoOperation)
                   {

                  case 0 :
                           foreach(WLAxis *Axis,getAxisList()){
                             if(Axis->getActIn(typePM)==WLIOPut::INPUT_actNo){
                                 emit sendMessage(getFullName(),tr("no set action")
                                                               +" Axis:"
                                                               +QString::number(Axis->getIndex())
                                                              ,-9);
                                 reset();
                                 break;
                             }
                            }

                           if(((typePM==AXIS_inPEL)&&(getInput(AXIS_inPEL)==0))
                             ||((typePM==AXIS_inMEL)&&(getInput(AXIS_inMEL)==0))
                             ||((typePM==AXIS_inORG)&&(getInput(AXIS_inORG)==0))) {
                             }
                            else if (getFreeDistFind()!=0.0)
                                {
                                setMainPad();

                                if(logicFindPos==onlyPEL
                                 ||logicFindPos==onlyPELHome
                                 ||logicFindPos==onlyPORG
                                 ||logicFindPos==onlyPORGHome) {
                                  setMov(-getFreeDistFind());
                                  }
                                  else if(logicFindPos==onlyMEL
                                        ||logicFindPos==onlyMELHome
                                        ||logicFindPos==onlyMORG
                                        ||logicFindPos==onlyMORGHome) {
                                         setMov(getFreeDistFind());
                                         }

                                startMotion(m_VFind1);

                                autoOperation=1;
                                break;
                                }

                  case 1:
                          if(((typePM==AXIS_inPEL)&&(getInput(AXIS_inPEL)==0))
                            ||((typePM==AXIS_inMEL)&&(getInput(AXIS_inMEL)==0))
                            ||((typePM==AXIS_inORG)&&(getInput(AXIS_inORG)==0)))
    					    {
							setTruPosition(false);

                            setMainPad(); //первое касание

                            m_orgPosCountList.clear();

                            foreach(WLAxis *Axis,getAxisList())
                             {
                             Axis->setInLatch(Axis->getInput(typePM)->getIndex());

                             if(typePM==AXIS_inORG)
                                 Axis->setActIn(AXIS_inLATCH,Axis->getActIn(AXIS_inORG));

                             Axis->resetLatch();

                             if(Axis!=getAxis())
                                Axis->setPos(getAxis()->getNowPos()); //set position subAxis

                             Axis->setKF(1.0);

                             if(logicFindPos==onlyPELHome
                              ||logicFindPos==onlyPEL
                              ||logicFindPos==onlyPORG
                              ||logicFindPos==onlyPORGHome)
                                 Axis->movVel(MASK_disubaxis|MASK_dir,m_VFind1/dim.value);
                             else
                                 Axis->movVel(MASK_disubaxis|0,m_VFind1/dim.value);

                             m_orgPosCountList<<0.0;
                             }

                            if(getBackDistFind()>0.0)
                               autoOperation=2;
                            else
                               autoOperation=10;                            

    						}
                           else
                            {
                            emit sendMessage(getFullName(),tr("wrong starting position")
                                                         +(typePM==AXIS_inPEL ? "(inPEL=0)" :"")
                                                         +(typePM==AXIS_inMEL ? "(inMEL=0)" :"")
                                                         +(typePM==AXIS_inORG ? "(inORG=0)" :"")
                                                         ,-8);
                            reset();
                            }
    						break;


                   case 2:  if(isLatch2())  //отезжаем обратно если все сработали
                             {
                             for(quint8 i=0;i<getAxisList().size();i++){
                              m_orgPosCountList[i]=getLatch2Pos(i);
                              }

                             if(getBackDistFind()<0)
                             foreach(WLAxis *Axis,getAxisList())
                              {
                              Axis->setInLatch(Axis->getInput(typePM)->getIndex());
                              Axis->setActIn(AXIS_inLATCH,WLIOPut::INPUT_actSdStop);
                              Axis->resetLatch();

                              //if(Axis!=getAxis())
                              //   Axis->setPos(getAxis()->getNowPos()); //set position subAxis

                              //Axis->setKF(1.0);
                              }

                             foreach(WLAxis *Axis,getAxisList())
                              {                                 
                              if(logicFindPos==onlyPELHome
                               ||logicFindPos==onlyPEL
                               ||logicFindPos==onlyPORG
                               ||logicFindPos==onlyPORGHome)
                                 Axis->movPos(MASK_disubaxis
                                             ,-qAbs(getBackDistFind()/dim.value)
                                             ,m_VFind1/dim.value);
                              else
                                 Axis->movPos(MASK_disubaxis
                                             ,qAbs(getBackDistFind()/dim.value)
                                             ,m_VFind1/dim.value);

                              }
                             autoOperation=3;
                             }
                             else
                             {
                             emit sendMessage(getFullName(),tr("no sensor signal")+"(2)"
                                                          +(typePM==AXIS_inPEL ? "(inPEL=0)" :"")
                                                          +(typePM==AXIS_inMEL ? "(inMEL=0)" :"")
                                                          +(typePM==AXIS_inORG ? "(inORG=0)" :"")
                                                          ,-214);
                             qDebug()<<getName()<<"no sensor signal (2)";
                             reset();
                             }

                            break;

                   case 3:if(((typePM==AXIS_inPEL)&&(getInput(AXIS_inPEL)==0))
                             ||((typePM==AXIS_inMEL)&&(getInput(AXIS_inMEL)==0))
                             ||((typePM==AXIS_inORG)&&(getInput(AXIS_inORG)==0)))//второе касание
                             {
                             setMainPad();

                             foreach(WLAxis *Axis,getAxisList())
                              {
                              Axis->setInLatch(Axis->getInput(typePM)->getIndex());

                              if(typePM==AXIS_inORG)
                                  Axis->setActIn(AXIS_inLATCH,Axis->getActIn(AXIS_inORG));

                              Axis->resetLatch();
                              }

                             foreach(WLAxis *Axis,getAxisList())
                              {
                              if(logicFindPos==onlyPELHome
                               ||logicFindPos==onlyPEL
                               ||logicFindPos==onlyPORGHome
                               ||logicFindPos==onlyPORG)
                                  Axis->movVel(MASK_disubaxis|MASK_dir,m_VFind2/dim.value);
                              else
                                  Axis->movVel(MASK_disubaxis|0,m_VFind2/dim.value);

                              }
                             autoOperation=10;
                             }
                             else
                             {
                             emit sendMessage(getFullName(),tr("no sensor signal")+"(3)"
                                                          +(typePM==AXIS_inPEL ? "(inPEL=0)" :"")
                                                          +(typePM==AXIS_inMEL ? "(inMEL=0)" :"")
                                                          +(typePM==AXIS_inORG ? "(inORG=0)" :"")
                                                          ,-214);
                             reset();
                             }


                              break;

                   case 10: if(isLatch2())
                              {
                              autoOperation=20;

                              foreach(WLAxis *Axis,getAxisList()){
                               qDebug()<<getName()<<"latchPos"<<Axis->getLatch2()<<"type"<<autoTypeDrive;
                               }

                              switch (autoTypeDrive)
                              {
                              case autoTeach: setOrgPosition(getLatch2Pos());

                                              for(quint8 i=1;i<getAxisList().size();i++) {
                                                setOffsetAxis(i,getLatch2Pos(i)-getLatch2Pos());
                                                }

                                              setTruPosition();
                                              reset();
                                              break;

                              case autoFind:  corPosition(getLatch2Pos());

                                              if(!getSubAxisList().isEmpty())
                                               {
                                                qint64 Offset=0;

                                                WLDrivePosition DPos;

                                                for(quint8 i=1;i<getAxisList().size();i++)
                                                    {
                                                    DPos.set(getOffsetAxis(i),dim);  //set offset position
                                                    DPos.step-=(getAxis(i)->getNowPos()-getLatch2PosL(i)); //move back to SD distance

                                                    if(((logicFindPos==onlyPEL
                                                       ||logicFindPos==onlyPELHome
                                                       ||logicFindPos==onlyPORG
                                                       ||logicFindPos==onlyPORGHome)&&(Offset<DPos.step))
                                                       ||
                                                        ((logicFindPos==onlyMEL
                                                        ||logicFindPos==onlyMELHome
                                                        ||logicFindPos==onlyMEL
                                                        ||logicFindPos==onlyMORGHome)&&(Offset>DPos.step))) Offset=DPos.step; // find max/min offset

                                                    }
                                               //qDebug()<<"Offset"<<Offset;

                                                for(quint8 i=0;i<getAxisList().size();i++)
                                                    {
                                                    DPos.set(getOffsetAxis(i),dim);
                                                    DPos.step-=(getAxis(i)->getNowPos()-getLatch2PosL(i));
                                                    DPos.step-=Offset;

                                                   if(DPos.getStepI32L()!=0){
                                                   //qDebug()<<"Mov Axis"<<DPos.getStepI32L();
                                                     getAxis(i)->movPos(MASK_disubaxis,DPos.getStepI32L(),m_VFind1/dim.value);
                                                     }
                                                   }

                                              QTimer::singleShot(250,this,SLOT(updateAuto()));
                                              }
                                              else
                                                 {
                                                 goto endfindPM;
                                                 }
                                             break;

                               case autoVerify:
                                               QString str;

                                               WLDrivePosition orgPos;
                                               orgPos.set(getOrgPosition(),dim);

                                               for(quint8 i=0;i<getAxisList().size();i++){
                                                   str+=QString(" axis %1: %2").arg(getAxis(i)->getIndex())
                                                                                  .arg(getLatch2Pos(i)-getOrgPosition()
                                                                                  +getOffsetAxis(i),0,'f',5);

                                                   qDebug()<<"Verify"<<getLatch2Pos(i)<<getOrgPosition()<<getOffsetAxis(i);
                                                   }

                                               emit sendMessage(getFullName(),str,1);

                                               if(!getSubAxisList().isEmpty())
                                               {
                                                qint32 pos=getAxis()->getNowPos(); //move back to SD distance

                                                for(quint8 i=1;i<getAxisList().size();i++)
                                                    {
                                                    qint32 p=(getAxis(i)->getNowPos()); //move back to SD distance

                                                    if(((logicFindPos==onlyPEL
                                                       ||logicFindPos==onlyPELHome
                                                       ||logicFindPos==onlyPORG
                                                       ||logicFindPos==onlyPORGHome)&&(pos>p))
                                                       ||
                                                       ((logicFindPos==onlyMEL
                                                       ||logicFindPos==onlyMELHome
                                                       ||logicFindPos==onlyMORG
                                                       ||logicFindPos==onlyMORGHome)&&(pos<p))) pos=p; // find max/min pos
                                                    }


                                                for(quint8 i=0;i<getAxisList().size();i++)
                                                    {
                                                    if(pos!=getAxis(i)->getNowPos()) {
                                                     getAxis(i)->movPos(MASK_disubaxis|MASK_abs,pos,m_VFind1/dim.value);
                                                     }
                                                    }

                                              QTimer::singleShot(250,this,SLOT(updateAuto()));
                                              }
                                               else
                                                  {
                                                  goto endfindPM;
                                                  }
                                              break;
                               }

    					      }
    					     else
                              {
                              emit sendMessage(getFullName(),tr("no sensor signal")+"(10)"
                                                           +(typePM==AXIS_inPEL ? "(inPEL=0)" :"")
                                                           +(typePM==AXIS_inMEL ? "(inMEL=0)" :"")
                                                           +(typePM==AXIS_inORG ? "(inORG=0)" :"")
                                                           ,-214);
                              reset();
    						  }
      	              	     break;

                     case 20:  endfindPM:

                              reset();
                              setTruPosition(true);

                              if(logicFindPos==onlyPELHome
                               ||logicFindPos==onlyMELHome
                               ||logicFindPos==onlyPORGHome
                               ||logicFindPos==onlyMORGHome) {
                                if(setMovToHome())
                                    startMotion(0);
                                }


                              if(logicFindPos==onlyPEL
                               ||logicFindPos==onlyMEL
                               ||logicFindPos==onlyPORG
                               ||logicFindPos==onlyMORG)
                                {
                                if(setMovToORG())
                                    startMotion(0);
                                }

                              break;
                    }
		            break;


 //default: reset();
 }

 }

qDebug()<<getName()<<"WLDrive::updateAuto>>";
}
