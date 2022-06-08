#include "wlmoduleaxis.h"
#include "wldevice.h"

WLModuleAxis::WLModuleAxis(WLDevice *_Device)
    : WLModule(_Device)
{
setTypeModule(typeMAxis);

Fmax=0;

inEMGStop=&WLIOPut::In0;
inSDStop=&WLIOPut::In0;

updateTimer= new QTimer;
connect(updateTimer,SIGNAL(timeout()),SLOT(callTrackAxis()));
updateTimer->start(100);

Init(1);
}

WLModuleAxis::~WLModuleAxis()
{
while(Axis.isEmpty())
    delete Axis.takeLast();
}


bool WLModuleAxis::Init(int sizeAxis)
{
if(sizeAxis<1
 ||Axis.size()== sizeAxis
 ||isReady()) return false;

WLAxis *axis;
long l=0;

if(sizeAxis>Axis.size())
 for(quint8 i=Axis.size();i<sizeAxis;i++ )
  { 
  axis = new WLAxis(this);
  axis->setIndex(i);
  axis->setParent(this);

  connect(axis,SIGNAL(sendCommand(QByteArray)),SLOT(setCommand(QByteArray)));
  Axis+=axis;
  }
else
 while(sizeAxis!=Axis.size())
  {
  axis=Axis.takeLast();
  disconnect(axis,SIGNAL(sendCommand(QByteArray)),this,SLOT(setCommand(QByteArray)));
  delete axis;
  }
update();
return true;
}

WLAxis *WLModuleAxis::getAxis(int index)
{
Q_ASSERT(index<getSizeAxis());

return index<getSizeAxis() ? Axis[index]:nullptr;
}


void WLModuleAxis::callDataAxis()
{
for(int i=0;i<getSizeAxis();i++) Axis[i]->sendGetDataAxis();
}

void WLModuleAxis::callTrackAxis()
{
for(int i=0;i<getSizeAxis();i++)
  {
  Axis[i]->getData(typeDataAxis::dataAxis_pos);
  Axis[i]->getData(typeDataAxis::dataAxis_F);
  }
}

void WLModuleAxis::update()
{
foreach(WLAxis *axis,Axis)
    axis->update();
}

void WLModuleAxis::backup()
{
foreach(WLAxis *axis,Axis)
     axis->backup();

setInSDStop(getInput(MAXIS_inSDStop)->getIndex());
setInEMGStop(getInput(MAXIS_inEMGStop)->getIndex());
}

void WLModuleAxis::readCommand(QByteArray Data)
{
quint8 index,ui1,ui2;
qint32 l1;
QString str;

float f1;

QDataStream Stream(&Data,QIODevice::ReadOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream>>ui1;

switch(ui1)
{
 case comAxis_setData:  Stream>>index;//index8

                       if(index<getSizeAxis()) Axis[index]->setData(Stream);
                       break;

 case sendAxis_data:   Stream>>index;//index8
 	                   Stream>>ui1;  //Status4|Mode4
 				       Stream>>ui2;  //Flag8
 			           Stream>>l1;   //Pos32
 				       Stream>>f1;   //Freq32
                        
					   if(index<getSizeAxis())
					    { 				   		
                        Axis[index]->setDataAxis(ui1,ui2,l1,qAbs(f1));
					    }
					    else
                       qDebug()<<"Error indexAxis";
 			           break;


case  sendModule_prop: Stream>>ui1;
					   Stream>>l1;

                       Fmax=l1;			

					   Init(ui1);

                       setReady(true);
                       break;

 case sendModule_error:
	                Stream>>ui1; 

                    if(ui1>200){
                      if(ui1==0xFF) {
                      char buf[64];
                      buf[Stream.readRawData(buf,sizeof(buf))]='\0';

                      emit sendMessage("ModuleAxis ",buf,(int)(-ui1));
                      }else{
                      Stream>>index;
                      emit sendMessage("ModuleAxis "+getErrorStr(errorModule,ui1),QString::number(index),(int)(-ui1));
                      }
                    }
                    else
                    {
                    Stream>>index;
				    if(index<getSizeAxis())
					    {
						emit sendMessage("Axis "+getErrorStr(errorAxis,ui1),QString::number(index),(int)(-ui1));	

                   	    Axis[index]->setError(ui1);       									  
					    }
                    }
                   break;
}			
}
bool WLModuleAxis::setInputMAxis(typeIOPutAXIS type,quint8 num)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)typeMAxis<<(quint8)comAxis_setInputM<<(quint8)type<<num;

emit sendCommand(data);
return true;
}


void WLModuleAxis::setInEMGStop(int index)
{
disconnect(inEMGStop,SIGNAL(changed(int)),this,SIGNAL(changedInEMGStop()));

inEMGStop->removeComment("inEMGStop");

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

if(index>=ModuleIOPut->getSizeInputs()) index=0;

inEMGStop=ModuleIOPut->getInput(index);;
inEMGStop->addComment("inEMGStop");

connect(inEMGStop,SIGNAL(changed(int)),this,SIGNAL(changedInEMGStop()));

setInputMAxis(IO_inEMGStop,index);
}

void  WLModuleAxis::setInSDStop(int index)
{
disconnect(inSDStop,SIGNAL(changed(int)),this,SIGNAL(changedInSDStop()));

inSDStop->removeComment("inSDStop");

WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

if(index>=ModuleIOPut->getSizeInputs()) index=0;

inSDStop=ModuleIOPut->getInput(index);;
inSDStop->addComment("inSDStop");


connect(inSDStop,SIGNAL(changed(int)),this,SIGNAL(changedInSDStop()));

setInputMAxis(IO_inSDStop,index);
}


WLIOPut *WLModuleAxis::getInput(typeInputMAXIS type)
{
WLIOPut *ret=NULL;
switch(type)
{
case MAXIS_inEMGStop: ret=inEMGStop; break;
case MAXIS_inSDStop:  ret=inSDStop;  break;
}
return ret;
}

void WLModuleAxis::readXMLData(QXmlStreamReader &stream)
{
quint8 index;
int size=4;

if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();    

Init(size);

if(!stream.attributes().value("inEMGStop").isEmpty()) setInEMGStop(stream.attributes().value("inEMGStop").toString().toInt());    
if(!stream.attributes().value("inSDStop").isEmpty())  setInSDStop(stream.attributes().value("inSDStop").toString().toInt());

while(!stream.atEnd())
{
stream.readNextStartElement();

if(stream.name()==metaObject()->className()) break;
if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

if(stream.name()=="Axis")
	   {       
	   index=stream.attributes().value("index").toString().toInt();
       if(index<getSizeAxis())
           Axis[index]->readXMLData(stream);
       }
}

}

void WLModuleAxis::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("size",QString::number(getSizeAxis()));

stream.writeAttribute("inEMGStop",QString::number(inEMGStop->getIndex()));
stream.writeAttribute("inSDStop",QString::number(inSDStop->getIndex()));
stream.writeAttribute("Fmax",QString::number(Fmax));

for(int i=0;i<getSizeAxis();i++)
 {
 stream.writeStartElement("Axis");
 stream.writeAttribute("index",QString::number(i));
   Axis[i]->writeXMLData(stream);
 stream.writeEndElement();
 }

}
