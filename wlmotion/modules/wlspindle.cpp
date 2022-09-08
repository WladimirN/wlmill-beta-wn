#include "wlspindle.h"
#include "wlmodulespindle.h"
#include "wldevice.h"

bool compareSpindleData(const WLSpindleData &v1, const WLSpindleData &v2)
{
return v1.inValue < v2.inValue;
}

WLSpindle::WLSpindle(WLModuleSpindle *_ModuleSpindle)
    :WLElement(_ModuleSpindle)
{
setTypeElement(typeESpindle);

outENBSpindle=&WLIOPut::Out;
}


WLSpindle::~WLSpindle()
{

}


bool WLSpindle::clearDataSpindle()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_clearData;

spindleDataList.clear();

emit sendCommand(data);

return true;
}

void WLSpindle::setFastSOut(bool enable)
{
m_fastSOut=enable;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setFastChange<<getIndex()<<(uint8_t)m_fastSOut;

emit sendCommand(data);
}

bool WLSpindle::addDataSpindle(WLSpindleData sdata)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_addData<<getIndex()
       <<sdata.inValue<<sdata.outValue;

spindleDataList+=sdata;

qSort(spindleDataList.begin(), spindleDataList.end(), compareSpindleData);

emit sendCommand(data);

return true;
}

void WLSpindle::setDataList(QList<WLSpindleData> dataList)
{
clearDataSpindle();

foreach(WLSpindleData sdata,dataList){
 addDataSpindle(sdata);
}
}

void WLSpindle::setOutENB(int index)
{
outENBSpindle->removeComment("outENBSpindle"+QString::number(getIndex()));

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

if(index>=ModuleIOPut->getSizeOutputs()) index=0;

outENBSpindle=ModuleIOPut->getOutput(index);
outENBSpindle->addComment("outENBSpindle"+QString::number(getIndex()));

setOutputSpindle(SPINDLE_outENB,index);
}

bool WLSpindle::setElementSOut(typeElement telement,quint8 i)
{
if(telement==typeElement::typeEOutPWM
 ||telement==typeElement::typeEAOutput)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setOutElement<<getIndex()<<(quint8)telement<<i;

emit sendCommand(data);

m_typeSOut=telement;
m_iOut=i;

return true;
}

return false;
}

bool WLSpindle::resetElementSpindle()
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_resetOutElement<<getIndex();

emit sendCommand(data);

m_typeSOut=typeEEmpty;
m_iOut=0;

return true;
}

void  WLSpindle::setData(QDataStream &data)
{
quint8 type;

data>>type;

switch((typeDataSpindle)type)
 {
 case dataSpindle_Scur:{
                       data>>m_curSpindleData.inValue;
                       emit changedInValue(m_curSpindleData.inValue);
                       }
                       break;

case dataSpindle_OutCur:
                      {
                      data>>m_curSpindleData.outValue;
                      emit changedOutValue(m_curSpindleData.outValue);
                      }
                      break;
default: break;
}

}

bool WLSpindle::sendGetData(typeDataSpindle type)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_getData<<getIndex()<<(quint8)type;

emit sendCommand(data);

return true;
}


bool WLSpindle::sendGetData()
{
sendGetData(dataSpindle_Scur);
sendGetData(dataSpindle_OutCur);

return true;
}

void WLSpindle::update()
{
sendGetData();
}

void WLSpindle::backup()
{
setAcc(m_acc);
setDec(m_dec);

setDataList(getDataList());
}


WLIOPut *WLSpindle::getInputSpindle(typeInputSpindle type)
{
return nullptr;
}

WLIOPut *WLSpindle::getOutput(typeOutputSpindle type)
{
switch(type)
{
case SPINDLE_outENB:  return outENBSpindle;
}

return nullptr;
}

bool WLSpindle::setInputSpindle(typeInputSpindle type, quint8 num)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setInput<<getIndex()<<(quint8)type<<num;

emit sendCommand(data);
return true;
}

bool WLSpindle::setOutputSpindle(typeOutputSpindle type,quint8 num)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setOutput<<getIndex()<<(quint8)type<<num;

emit sendCommand(data);
return true;
}


bool WLSpindle::setAcc(float acc)
{
if(acc<0) return false;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setAcc<<getIndex()<<acc;

emit sendCommand(data);

m_acc=acc;

return true;
}

bool WLSpindle::setDec(float dec)
{
if(dec>0) return false;

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comSpindle_setDec<<getIndex()<<dec;

emit sendCommand(data);

m_dec=dec;

return true;
}


void WLSpindle::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("SOut",QString::number(getTypeSOut())
                        +","+QString::number(getISOut()));

stream.writeAttribute("accSOut",QString::number(getAcc()));
stream.writeAttribute("decSOut",QString::number(getDec()));

stream.writeAttribute("fastChangeSOut",QString::number(isFastChangeSOut()));

stream.writeAttribute("outENBSpindle",QString::number(getOutput(SPINDLE_outENB)->getIndex()));

quint8 i=0;

foreach(WLSpindleData sdata,getDataList())
 {
 stream.writeStartElement("data");

 stream.writeAttribute("i",QString::number(i++));
 stream.writeAttribute("inValue",QString::number(sdata.inValue,'f',5));
 stream.writeAttribute("outValue",QString::number(sdata.outValue,'f',5));

 stream.writeEndElement();
 }
}

void WLSpindle::readXMLData(QXmlStreamReader &stream)
{
clearDataSpindle();

while(!stream.atEnd())
{
if(!stream.attributes().value("accSOut").isEmpty())
     setAcc(stream.attributes().value("accSOut").toFloat());

if(!stream.attributes().value("decSOut").isEmpty())
     setDec(stream.attributes().value("decSOut").toFloat());

if(!stream.attributes().value("fastChangeSOut").isEmpty())
     setFastSOut(stream.attributes().value("fastChangeSOut").toInt());

if(!stream.attributes().value("outENB").isEmpty())
    setOutENB(stream.attributes().value("outENB").toString().toInt());


if(!stream.attributes().value("SOut").isEmpty())
   {
   QStringList list=stream.attributes().value("SOut").toString().split(",");

   if(list.size()==2)
     {
     setElementSOut(static_cast<typeElement>(list.at(0).toUShort())
                                    ,(quint8)list.at(1).toUShort());
     }
   }

stream.readNextStartElement();

if(stream.name()==metaObject()->className()) break;
if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

if(stream.name()=="data")
       {
       WLSpindleData sdata;

       sdata.inValue=stream.attributes().value("inValue").toString().toFloat();
       sdata.outValue=stream.attributes().value("outValue").toString().toFloat();

       addDataSpindle(sdata);
       }

}
}
