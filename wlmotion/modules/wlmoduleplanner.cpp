#include "wlmoduleplanner.h"
#include "wldevice.h"

bool compareSpindleData(const WLSpindleData &v1, const WLSpindleData &v2)
{
return v1.inValue < v2.inValue;
}

WLModulePlanner::WLModulePlanner(WLDevice *_Device)
    : WLModule(_Device)
{
setTypeModule(typeMPlanner);

m_sizeBuf=0;
m_free=0;
m_status=PLANNER_stop;
Flags.set(PLF_empty);
m_curIdElementBuf=0;
m_lastIdElementBuf=0;
m_indexRingElementBuf=0;

inProbe=&WLIOPut::In0;
inPause=&WLIOPut::In0;
inStop=&WLIOPut::In0;

m_validProbe2=false;
m_validProbe3=false;

updateTimer= new QTimer;
connect(updateTimer,&QTimer::timeout,this,&WLModulePlanner::callTrackPlanner);
updateTimer->start(500);
}

WLModulePlanner::~WLModulePlanner()
{

}

bool WLModulePlanner::setInput(typeInputPlanner type, quint8 num)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setInput<<(quint8)type<<num;

emit sendCommand(data);
return true;
}

bool WLModulePlanner::setIgnoreInput(typeInputPlanner type, quint8 ignore)
{
switch(type)
{
case PLANNER_inPause: m_ignoreInPause=ignore; break;
case PLANNER_inStop:  m_ignoreInStop=ignore; break;
default: return false;
}

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setIgnoreInput<<(quint8)type<<ignore;

emit sendCommand(data);
return true;
}


WLIOPut *WLModulePlanner::getInput(typeInputPlanner type)
{
WLIOPut *ret=nullptr;

switch(type)
{
case PLANNER_inProbe: ret=inProbe;break;
case PLANNER_inPause: ret=inPause;break;
case PLANNER_inStop:  ret=inStop; break;

default: ret=&WLIOPut::In0;
}

return ret;
}

void WLModulePlanner::setSpindleDataList(QList<WLSpindleData> dataList)
{
clearDataSpindle();

foreach(WLSpindleData sdata,dataList){
 addDataSpindle(sdata);
 }
}

void WLModulePlanner::clear()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_clear;

m_indexRingElementBuf=0;

emit sendCommand(data);
}

void WLModulePlanner::setModeRun(modeRunPlanner modeRun)
{
m_modeRun=modeRun;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setModeRun<<(quint8)m_modeRun;

emit sendCommand(data);
}

void WLModulePlanner::setInProbe(int index)
{
inProbe->removeComment("inProbe");

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

if(index>=ModuleIOPut->getSizeInputs()) index=0;

inProbe=ModuleIOPut->getInput(index);
inProbe->addComment("inProbe");

setInput(PLANNER_inProbe,index);
}

void WLModulePlanner::setInPause(int index)
{
inPause->removeComment("inPause");

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

if(index>=ModuleIOPut->getSizeInputs()) index=0;

inPause=ModuleIOPut->getInput(index);
inPause->addComment("inPause");

connect(inPause,&WLIOPut::changed,this,[=](int _index){if(inPause->getIndex()==_index
                                                        &&inPause->getCond()>1) emit changedPause(inPause->getNow());});

setInput(PLANNER_inPause,index);
}

void WLModulePlanner::setInStop(int index)
{
inStop->removeComment("inStop");

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

if(index>=ModuleIOPut->getSizeInputs()) index=0;

inStop=ModuleIOPut->getInput(index);
inStop->addComment("inStop");

connect(inStop,&WLIOPut::changed,this,[=](int _index){if(inStop->getIndex()==_index
                                                       &&inStop->getCond()>1) emit changedStop(inStop->getNow());});

setInput(PLANNER_inStop,index);
}

float WLModulePlanner::getDecSpindle() const
{
    return m_decSpindle;
}

float WLModulePlanner::getAccSpindle() const
{
    return m_accSpindle;
}

quint8 WLModulePlanner::getISOut() const
{
    return m_iSout;
}

typeElement WLModulePlanner::getTypeSOut() const
{
    return m_typeSOut;
}

void WLModulePlanner::setSizeBuf(int value)
{
    m_sizeBuf = value;
}

bool WLModulePlanner::setActInProbe(typeActionInput typeAct)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setActInProbe<<(quint8)typeAct;

qDebug()<<"WLModulePlanner::setActInProbe"<<(quint8)typeAct<<typeAct;

emit sendCommand(data);
return true;
}

bool WLModulePlanner::setActSafeProbe(typeActionInput typeAct)
{
m_actSafeProbe=typeAct;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setActSafeProbe<<(quint8)typeAct;

qDebug()<<"WLModulePlanner::setActSafeProbe"<<(quint8)typeAct<<m_actSafeProbe;

emit sendCommand(data);
return true;
}

void WLModulePlanner::setData(QDataStream &data)
{
quint8 type;

data>>type;

switch((typeDataPlanner)type)
 {
 case dataPlanner_F: //data>>nowPosition;
                     //emit changedPosition(nowPosition);
                       break;

 case dataPlanner_curSpindleInValue: {
                       data>>m_curSpindleData.inValue;
                       emit changedSOut(m_curSpindleData.inValue);
                       }
                       break;

case dataPlanner_curSpindleOutValue: {
                       data>>m_curSpindleData.outValue;
                       }
                       break;
default: break;
}

}

void WLModulePlanner::getData(typeDataPlanner type)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_getData<<(quint8)type;

emit sendCommand(data);
}

void WLModulePlanner::setFastChangeSOut(bool enable)
{
m_fastChangeSOut=enable;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_setFastChange<<(uint8_t)m_fastChangeSOut;

emit sendCommand(data);
}

void WLModulePlanner::callTrackPlanner()
{
getData(typeDataPlanner::dataPlanner_curSpindleInValue);
getData(typeDataPlanner::dataPlanner_curSpindleOutValue);
}

void WLModulePlanner::sendGetDataBuf()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_getDataPlanner;

qDebug()<<"WLModulePlanner::sendGetDataBuf()";

emit sendCommand(data);
}

void WLModulePlanner::update()
{    
sendGetDataBuf();
}

void WLModulePlanner::backup()
{
setKF(getKF());
setKSOut(getKSOut());
setSmoothAng(getSmoothAng());

setInProbe(getInput(PLANNER_inProbe)->getIndex());
setInPause(getInput(PLANNER_inPause)->getIndex());

setElementSpindle(getTypeSOut(),getISOut());

setAccSpindle(m_accSpindle);
setDecSpindle(m_decSpindle);

setSpindleDataList(getSpindleDataList());

setIAxisSlave(m_indexsAxis.data(),m_indexsAxis.size());

setActSafeProbe(getActSafeProbe());
}


bool WLModulePlanner::setIAxisSlave(quint8 *indexsAxis,quint8 size)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setISlaveAxis<<size;

m_indexsAxis.clear();

for(int i=0;i<size;i++)
  {
  m_indexsAxis<<indexsAxis[i];
  Stream<<indexsAxis[i];
  }

emit sendCommand(data);

return true;
}

bool WLModulePlanner::setHPause(quint8 enable,qint32 hPause)
{
Flags.set(PLF_usehpause,enable);

m_hPause=hPause;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setHPause<<enable<<m_hPause;

emit sendCommand(data);

return true;
}

bool WLModulePlanner::setElementSpindle(typeElement telement,quint8 i)
{
if(telement==typeElement::typeEOutPWM
 ||telement==typeElement::typeEAOutput)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_setOutElement<<(quint8)telement<<i;

emit sendCommand(data);

m_typeSOut=telement;
m_iSout=i;

return true;
}

return false;
}


bool WLModulePlanner::setAccSpindle(float acc)
{
if(acc<0) return false;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_setAcc<<acc;

emit sendCommand(data);

m_accSpindle=acc;

return true;
}

bool WLModulePlanner::setDecSpindle(float dec)
{
if(dec>0) return false;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_setDec<<dec;

emit sendCommand(data);

m_decSpindle=dec;

return true;
}

bool WLModulePlanner::resetElementSpindle()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_resetOutElement;

emit sendCommand(data);

m_typeSOut=typeEEmpty;
m_iSout=0;

return true;
}

bool WLModulePlanner::clearDataSpindle()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_clearData;

spindleDataList.clear();

emit sendCommand(data);

return true;
}

bool WLModulePlanner::addDataSpindle(WLSpindleData sdata)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_toSpindle<<(quint8)comSpindle_addData
       <<sdata.inValue<<sdata.outValue;

spindleDataList+=sdata;

qSort(spindleDataList.begin(), spindleDataList.end(), compareSpindleData);

emit sendCommand(data);

return true;
}


bool WLModulePlanner::addULine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 midPos[],float S,float Fmov,quint32 _id)
{
if(Mutex.tryLock())
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_addULine<<mask<<size;//<<endPos[0]<<endPos[1]<<endPos[2]<<S<<Fmov   ;

for(int i=0;i<size;i++)
   Stream<<indexs[i]<<endPos[i]<<midPos[i];

Stream<<S
	  <<Fmov
	  <<_id;

qDebug()<<"addBufULine en:"<<endPos[0]<<endPos[1]<<endPos[2]<<" mid:"<<midPos[0]<<midPos[1]<<midPos[2]<<"id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov;

m_free--;
//emit ChangedFreeBuf(getFreeBuf());

m_indexRingElementBuf++;
m_lastIdElementBuf=_id;

Flags.reset(PLF_empty);

emit sendCommand(data);
Mutex.unlock();
return true;
}
else {
return false;
}
}


bool WLModulePlanner:: addLine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],float S,float Fmov,quint32 _id)
{
if(Mutex.tryLock())
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner
      <<(quint8)comPlanner_addLine
      <<mask
      <<size;

for(int i=0;i<size;i++)
   Stream<<indexs[i]<<endPos[i];

Stream<<S
	  <<Fmov
      <<_id;

//ebug()<<"index"<<m_lastIndexElementBuf<<m_free;
qDebug()<<"addBufLine3D en:"<<endPos[0]<<endPos[1]<<endPos[2]<<endPos[3]<<"id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov<<"mask:"<<mask;
m_free--;
//emit ChangedFreeBuf(getFreeBuf());

m_indexRingElementBuf++;
m_lastIdElementBuf=_id;

Flags.reset(PLF_empty);

emit sendCommand(data);

Mutex.unlock();
return true;
}
else {
return false;
}
}

bool WLModulePlanner::addCirc(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 cenPosIJ[],float S,float Fmov,quint32 _id)
{
if(Mutex.tryLock())
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);


Stream<<(quint8)typeMPlanner
      <<(quint8)comPlanner_addCirc
	  <<mask
      <<size;

for(quint8 i=0;i<size;i++)
   Stream<<indexs[i]<<endPos[i];

Stream<<(qint32)cenPosIJ[0]<<cenPosIJ[1];

Stream<<S
	  <<Fmov
      <<_id;


qDebug()<<"addBufCirc3D en:"<<endPos[0]<<endPos[1]<<endPos[2]<<" cp:"<<cenPosIJ[0]<<cenPosIJ[1]<<" id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov;

m_free--;
//emit ChangedFreeBuf(getFreeBuf());

m_indexRingElementBuf++;
m_lastIdElementBuf=_id;

Flags.reset(PLF_empty);

emit sendCommand(data);

Mutex.unlock();
return true;
}
else {
return false;
}
}

bool WLModulePlanner::addDelay(quint32 delayms,float S, quint32 _id)
{
if(Mutex.tryLock())
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner
      <<(quint8)comPlanner_addDelay
      <<delayms
      <<S
      <<_id;

//ebug()<<"index"<<m_lastIndexElementBuf<<m_free;
qDebug()<<"addDelay:"<<delayms<<" S"<<S<<" id:"<<_id<<"indexR"<<m_indexRingElementBuf;
m_free--;
//emit ChangedFreeBuf(getFreeBuf());

m_indexRingElementBuf++;
m_lastIdElementBuf=_id;

Flags.reset(PLF_empty);

emit sendCommand(data);

Mutex.unlock();
return true;
}
else {
return false;
}

}


bool WLModulePlanner::startMov()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_start;

emit sendCommand(data);

if(getStatus()==PLANNER_stop)
       m_indexRingElementBuf=0;

return true;
}

bool WLModulePlanner::stopMov()
{
Flags.set(PLF_empty);

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_stop;

m_indexRingElementBuf=0;

emit sendCommand(data);
return true;
}

bool WLModulePlanner::pauseMov()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_pause;

emit sendCommand(data);

return true;
}

bool WLModulePlanner::setKF(float _KF)
{
m_KF=_KF;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setKF<<_KF;

emit sendCommand(data);
return true;    
}

bool WLModulePlanner::setKFpause(float _F)
{
if((0.0f<_F)&&(_F<=1.0f))
{
m_KFpause=_F;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setKFpause<<m_KFpause;

emit sendCommand(data);
return true;
}
}

bool WLModulePlanner::setSmoothAng(float ang_gr)
{
if(0<=ang_gr&&ang_gr<=30)  
{
m_smoothAng=ang_gr;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setSmoothAng<<ang_gr;

emit sendCommand(data);
return true;
}
else
return false;
}

bool WLModulePlanner::setSOut(float s)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

qDebug()<<"setSOutBuf"<<s;

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setSOut<<s;

emit sendCommand(data);
return true;
}

bool WLModulePlanner::setKSOut(float k)
{
m_KSOut=k;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

qDebug()<<"setKSOutBuf"<<k;

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_setKSOut<<k;

emit sendCommand(data);
return true;
}

bool WLModulePlanner::setEnableSOut(quint8 enable)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMPlanner<<(quint8)comPlanner_enableSOut<<enable;

emit sendCommand(data);
return true;
}


void WLModulePlanner::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("SmoothAngGr",QString::number(getSmoothAng()));
stream.writeAttribute("KFpause",QString::number(getKFpause()));
stream.writeAttribute("SOut",QString::number(getTypeSOut())
                        +","+QString::number(getISOut()));

stream.writeAttribute("accSOut",QString::number(getAccSpindle()));
stream.writeAttribute("decSOut",QString::number(getDecSpindle()));

stream.writeAttribute("fastChangeSOut",QString::number(isFastChangeSOut()));

stream.writeAttribute("inProbe",QString::number(getInput(PLANNER_inProbe)->getIndex()));
stream.writeAttribute("inPause",QString::number(getInput(PLANNER_inPause)->getIndex()));
stream.writeAttribute("inStop",QString::number(getInput(PLANNER_inStop)->getIndex()));

quint8 index=0;

foreach(WLSpindleData sdata,getSpindleDataList())
 {
 stream.writeStartElement("spindleData");

 stream.writeAttribute("index",QString::number(index++));
 stream.writeAttribute("inValue",QString::number(sdata.inValue,'f',5));
 stream.writeAttribute("outValue",QString::number(sdata.outValue,'f',5));

 stream.writeEndElement();
 }
}

void WLModulePlanner::readXMLData(QXmlStreamReader &stream)
{
clearDataSpindle();

while(!stream.atEnd())
{
if(!stream.attributes().value("SmoothAngGr").isEmpty()) 
	 setSmoothAng(stream.attributes().value("SmoothAngGr").toString().toFloat());

if(!stream.attributes().value("KFpause").isEmpty())
     setKFpause(stream.attributes().value("KFpause").toString().toFloat());

if(!stream.attributes().value("inProbe").isEmpty())
    setInProbe( stream.attributes().value("inProbe").toString().toInt());

if(!stream.attributes().value("inPause").isEmpty())
    setInPause( stream.attributes().value("inPause").toString().toInt());

if(!stream.attributes().value("inStop").isEmpty())
    setInStop( stream.attributes().value("inStop").toString().toInt());

if(!stream.attributes().value("SOut").isEmpty())
   {
   QStringList list=stream.attributes().value("SOut").toString().split(",");

   if(list.size()==2)
     {
     setElementSpindle(static_cast<typeElement>(list.at(0).toUShort())
                     ,(quint8)list.at(1).toUShort());
     }
   }

if(!stream.attributes().value("accSOut").isEmpty())
     setAccSpindle(stream.attributes().value("accSOut").toFloat());

if(!stream.attributes().value("decSOut").isEmpty())
     setDecSpindle(stream.attributes().value("decSOut").toFloat());

if(!stream.attributes().value("fastChangeSOut").isEmpty())
     setFastChangeSOut(stream.attributes().value("fastChangeSOut").toInt());

stream.readNextStartElement();

if(stream.name()==metaObject()->className()) break;
if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

if(stream.name()=="spindleData")
       {
       WLSpindleData sdata;

       sdata.inValue=stream.attributes().value("inValue").toString().toFloat();
       sdata.outValue=stream.attributes().value("outValue").toString().toFloat();

       addDataSpindle(sdata);
       }
}

}

void  WLModulePlanner::readCommand(QByteArray Data)
{
quint8 index,ui1,ui2,ui3,ui4;
quint32 ui32;
float f1,f2;
qint32 l;

QDataStream Stream(&Data,QIODevice::ReadOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Mutex.lock();

Stream>>ui1;

switch(ui1)
  {
    case comPlanner_setData:  Stream>>index;//index8

                              setData(Stream);
                              break;

   case sendPlanner_data:
		                 Stream>>ui1;//size
                         Stream>>ui2;//lastIndex uint8_t
                         Stream>>ui3;//status
 					     Stream>>ui4;//flags
						 Stream>>ui32;									
                         Stream>>f1;//k element complete
                         Stream>>f2;//Star

                         //emit changedSOut(f2);

                         qDebug()<<"WLModulePlanner sendPlanner_data (cur/new) empty="<<((Flags.m_Data&PLF_empty)!=0)<<"/"<<((ui4&PLF_empty)!=0)
                                                  <<" lastIndex(256)="<<m_indexRingElementBuf<<"/"<<ui2
                                                  <<"status="<<m_status<<"/"<<ui3<<"curId"<<ui32;


                         if((m_indexRingElementBuf==ui2)
                         ||((m_status!=PLANNER_stop)&&(ui3==PLANNER_stop)))
                             {
                             m_curIdElementBuf=ui32;

                             if(m_status!=(statusPlanner)(ui3)
                              //||MPLANNER_pause==(statusPlanner)(ui3)
                                  )
                                  {
                                  if((m_status!=PLANNER_stop)&&(ui3==PLANNER_stop))
                                      {
                                      m_indexRingElementBuf=0;
                                      }

                                  m_status=(statusPlanner)(ui3);

                                  qDebug()<<"WLModulePlanner changed Status Planner"<<m_status<<m_indexRingElementBuf;

                                  emit changedStatus(m_status);                                  }



                             if(Flags.m_Data!=ui4)
                                 {
                                 uint8_t lastFlag=Flags.m_Data;

                                 Flags.m_Data=ui4;

                                 if((Flags.m_Data&PLF_empty)!=(lastFlag&PLF_empty))
                                     {
                                     qDebug()<<"WLModulePlanner changedFlag PLF_EMPTY"<<(Flags.m_Data&PLF_empty);
                                     emit changedEmpty(Flags.m_Data&PLF_empty);
                                     }
                                 }

                             emit changedCurIElement(m_curIdElementBuf);
                             emit changedFree(m_free=ui1);
                             }
                           break;

    case sendPlanner_rInProbe:
                              mutexProbe.lock();

                               m_posProbe2.clear();

                               for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
                                   Stream>>l;
                                   m_posProbe2+=l;
                                   }
                               m_validProbe2=true;

                              mutexProbe.unlock();

                              qDebug()<<"WLModulePlanner detect RProbePlanner";

                              emit changedProbe(true);
                              break;

    case sendPlanner_fInProbe:
                               mutexProbe.lock();

                                m_posProbe3.clear();
                                for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
                                    Stream>>l;
                                    m_posProbe3+=l;
                                    }
                                m_validProbe3=true;

                               mutexProbe.unlock();

                               qDebug()<<"WLModulePlanner detect FProbePlanner";

                               emit changedProbe(false);
                               break;

	case  sendModule_prop: Stream>>ui1; 
                           m_sizeBuf=ui1;

                           setReady(true);

                           update();
                           break;
	case sendModule_error:   
				                  Stream>>ui1;  //Error
								  Stream>>index;                                  

                                  if(ui1>startIndexErrorModule)
								   {
                                   qDebug()<<"WLModulePlanner error module"<<ui1;
                                   emit sendMessage("WLModulePlanner module"+getErrorStr(errorModule,ui1),"",-(int)(ui1));
								   }
                                   else{
                                   switch(ui1)
                                    {
                                    case errorPlanner_waxis:    qDebug()<<"WLModulePlanner error axis"<<index;
                                                                emit sendMessage("WLMotionPlanner+Axis:"+getErrorStr(errorAxis,index),"",-(int)(ui1));break;

                                    case errorPlanner_welement: qDebug()<<"WLModulePlanner error element"<<index;
                                                                emit sendMessage("WLMotionPlanner+Elementis:"+getErrorStr(errorElementPlanner,index),"",-(int)(ui1));break;

                                    default:                    qDebug()<<"WLModulePlanner error"<<index;
                                                                emit sendMessage("WLMotionPlanner"+getErrorStr(errorPlanner,ui1),"",-(int)(ui1));break;
                                    }
                                   }

                                  emit reset();
                   			      break;
								  
    case sendPlanner_signal:    Stream>>ui1;
				                  /*
					              switch(ui1)
								  {
								  case _sigChangedSOut_f32: Stream>>f1; emit  ChangedSOut(f1); break;								  								  
								  }
								  */
					
					              break;
			   }
				  
Mutex.unlock();

}
