#include "wlmoduleoscp.h"

WLModuleOscp::WLModuleOscp(WLDevice *_Device)
    : WLModule(_Device)
{
setTypeModule(typeMOscp);

Init(1);
}

WLModuleOscp::~WLModuleOscp()
{
while(Oscp.isEmpty())
      delete Oscp.takeLast();
}

bool WLModuleOscp::Init(int _size)
{
if(InitOscp(_size))
{
update();
return true;
}
else
 return false;
}

bool WLModuleOscp::InitOscp(int size)
{
if(size<1
 ||Oscp.size()== size
 ||isReady()) return false;

WLOscp *nOscp;

if(size>Oscp.size())
 for(int i=Oscp.size();i<size;i++ )
  {
  nOscp = new WLOscp;
  nOscp->index=i;
  Oscp+=nOscp;
  }
else
    while(Oscp.size()!= size)
	  {	  
      nOscp=Oscp.takeLast();

      delete nOscp;
      }

return true;
}

WLOscp *WLModuleOscp::getOscp(int index)
{
Q_ASSERT(((index<Oscp.size()))&&(index<255));

    return index<Oscp.size() ? Oscp.at(index): nullptr;
}

bool WLModuleOscp::setSourceChannel(quint8 indexch, WLModule::typeModule module, WLElement::typeElement element, uint8_t indexElement, uint8_t typeData)
{
WLOscp *Oscp=getOscp(indexch);

if(Oscp){
Oscp->module=module;
Oscp->element=element;
Oscp->indexElement=indexElement;
Oscp->typeData=typeData;


QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMOscp
      <<(quint8)comMOscp_setSourceChannel
      <<(quint8)indexch
      <<(quint8)Oscp->module
      <<(quint8)Oscp->element
      <<(quint8)Oscp->indexElement
      <<(quint8)Oscp->typeData;

emit sendCommand(data);
return true;
}

return false;
}

bool WLModuleOscp::setRun(bool run)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMOscp<<(quint8)comMOscp_setRun<<(quint8)run;

emit sendCommand(data);
return true;
}


void  WLModuleOscp::readCommand(QByteArray Data)
{
quint8 index,ui1;

QDataStream Stream(&Data,QIODevice::ReadOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream>>ui1;

switch(ui1)
{
case sendMOscp_dataCh : {
                        quint32 time_us;
                        Stream>>time_us;

                        QList <double> values;

                        for(int i=0;!Stream.atEnd();i++){
                         quint8 type;

                         Stream>>type;

                          switch(type) {
                          case OSC_u8: {
                                       quint8 u8;
                                       Stream>>u8;
                                       Oscp[i]->value=u8;
                                       }
                                       break;

                          case OSC_i8: {
                                       qint8 i8;
                                       Stream>>i8;
                                       Oscp[i]->value=i8;
                                       }
                                       break;

                          case OSC_u16: {
                                        quint16 ui16;
                                        Stream>>ui16;
                                        Oscp[i]->value=ui16;
                                        }
                                        break;

                          case OSC_i16: {
                                        qint16 i16;
                                        Stream>>i16;
                                        Oscp[i]->value=i16;
                                        }
                                        break;

                          case OSC_u32:{
                                       quint32 ui32;
                                       Stream>>ui32;
                                       Oscp[i]->value=ui32;
                                       }
                                       break;

                          case OSC_i32:{
                                       qint32 i32;
                                       Stream>>i32;
                                       Oscp[i]->value=i32;
                                       }
                                       break;
                          case OSC_f:  {
                                       float f32;
                                       Stream>>f32;
                                       Oscp[i]->value=f32;
                                       }
                                       break;
                          case OSC_d: {
                                      double d;
                                      Stream>>d;
                                      Oscp[i]->value=d;
                                      }
                                      break;
                          case OSC_dl: {
                                       qint64 dl;
                                       Stream>>dl;
                                       Oscp[i]->value=dl;
                                       }
                                       break;
                          case OSC_empty: Oscp[i]->value=0;break;
                          }

                         values+=Oscp[i]->value;
                         }

                        emit changedValues((double)time_us/1000000,values);
                        }
                        break;

/*
case comOscp_setData: Stream>>index;//index8
                         if(index<getSizeOscp()){
                          Oscp[index]->setData(Stream);
                          }
                         break;
*/
case  sendModule_prop: Stream>>ui1;

                       Init(ui1);

                       setReady(true);
                       break;

case  sendModule_error:
	                Stream>>ui1;//index8
                        Stream>>index;  //Error

                    emit sendMessage("WLModuleOscp "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
                    break;
}

}

void WLModuleOscp::readXMLData(QXmlStreamReader &stream)
{
quint8 index;

int size=1;

if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();

Init(size);
/*
while(!stream.atEnd())
{
stream.readNextStartElement();

if(stream.name()==metaObject()->className()) break;
if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

if(stream.name()=="WLOscp" )
       {
       index=stream.attributes().value("index").toString().toInt();
       if(index<getSizeOscp())
           Oscp[index]->readXMLData(stream);
      }
}
*/
}

void WLModuleOscp::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("size",QString::number(getSizeOscp()));
/*
for(int i=0;i<getSizeOscp();i++)
 {
 stream.writeStartElement("WLOscp");
 stream.writeAttribute("index",QString::number(i));
   Oscp[i]->writeXMLData(stream);
 stream.writeEndElement();
 }*/
}

