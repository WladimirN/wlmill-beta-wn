#include "wluart.h"
#include "wlmoduleuart.h"

WLUART::WLUART(WLModuleUART *_ModuleUART)
    : WLElement(_ModuleUART)
{
setTypeElement(typeEDCan);
}

WLUART::~WLUART()
{

}

bool WLUART::setBaudrate(quint32 _baudrate)
{
if(_baudrate>0) {
    m_baudrate=_baudrate;
    }else {
    return false;
    }

QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comUART_setData<<(quint8)dataUART_baudrate<<(quint8)getIndex()<<m_baudrate;

emit sendCommand(data);
return true;
}

bool WLUART::setEnable(bool enable)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comUART_setEnable<<(quint8)getIndex()<<(quint8)enable;

emit sendCommand(data);
return true;
}


bool WLUART::sendGetData()
{
sendGetData(dataUART_baudrate);

return true;
}

void WLUART::setReciveData(QByteArray buf)
{
if((m_recive.size()+buf.size())<MaxReciveBufSize)
    m_recive+=buf;
}

bool WLUART::sendGetData(typeDataUART type)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comUART_getData<<getIndex()<<(quint8)type;

emit sendCommand(data);

return true;
}

void WLUART::setData(QDataStream &Stream)
{
quint8 ui1;

Stream>>ui1;

switch(ui1)
{
case dataUART_baudrate:   Stream>>m_baudrate;   emit changed(getIndex()); break;
default: break;
}

}

bool WLUART::setDelayFrame(quint16 delay)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

m_delayFrame=delay;

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comUART_setDelayFrame<<getIndex()<<m_delayFrame;

emit sendCommand(data);

return true;
}

QByteArray WLUART::takeReciveData()
{
QByteArray ret=m_recive;
clearReciveData();

return ret;
}

void WLUART::clearReciveData()
{
m_recive.clear();
}

bool WLUART::isEmptyReciveData()
{
return m_recive.isEmpty();
}

bool WLUART::transmitData(QByteArray trdata)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comUART_transmit<<getIndex();

for(int i=0;i<trdata.size();i++)
       Stream<<(quint8)trdata[i];

emit sendCommand(data);

return true;
}

QString WLUART::getReciveStr(int len)
{
if(isEmptyReciveData()){
 return QString();
 }

QByteArray ba;

for(int i=0;i<m_recive.size();i++)
    qDebug()<<m_recive.at(i);

if(m_recive.size()<=len || len<=0) {
 ba=m_recive;
 m_recive.clear();
 }
 else {
 ba=m_recive.mid(0,len);
 m_recive=m_recive.mid(len);
 }

return QString::fromUtf8(ba);
}

quint8 WLUART::getReciveByte()
{
if(m_recive.isEmpty())
    return 0;

quint8 ret=(quint8)m_recive[0];

m_recive=m_recive.mid(1);

return ret;
}

double WLUART::getReciveNum(int type, int n)
{
if(isEmptyReciveData()){
 return 0;
 }

QByteArray ba;

if(m_recive.size()<=n) {
 ba=m_recive;
 m_recive.clear();
 }
 else {
 ba=m_recive.mid(0,n);
 m_recive=m_recive.mid(n);
 }

switch(type)
{
case 1: return ba.toUShort();
case 2: return ba.toShort();

case 3: return ba.toInt();
case 4: return ba.toUInt();

case 5: return ba.toLong();
case 6: return ba.toULong();

case 7: return ba.toFloat();
case 8: return ba.toDouble();
}

return 0;
}

void WLUART::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("baudrate", QString::number(getBaudrate()));
stream.writeAttribute("delayFrame", QString::number(getDelayFrame()));
}

void WLUART::readXMLData(QXmlStreamReader &stream)
{
bool ok;

if(!stream.attributes().value("baudrate").isEmpty())
    setBaudrate(stream.attributes().value("baudrate").toString().toInt(&ok));
if(!stream.attributes().value("delayFrame").isEmpty())
    setDelayFrame(stream.attributes().value("delayFrame").toString().toInt(&ok));

}

