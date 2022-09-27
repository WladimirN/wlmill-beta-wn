#ifndef WLModuleOscp_H
#define WLModuleOscp_H

#include <QObject>
#include <QDebug>
#include <QStringList>

#include "wlmodule.h"

//Osciloscope
#define comMOscp_setRun    1 //
#define comMOscp_setSourceChannel  2 //
#define comMOscp_setPeriod   3 //

#define sendMOscp_dataCh     4 //

enum typeValueOScp{OSC_empty,OSC_u8,OSC_i8,OSC_u16,OSC_i16,OSC_u32,OSC_i32,OSC_f,OSC_d,OSC_dl};

typedef struct
{
uint8_t index;

enum WLModule::typeModule module=WLModule::typeDevice;
enum WLElement::typeElement element=WLElement::typeEEmpty;
uint8_t indexElement;
uint8_t typeData;

double value;

}WLOscp; //channel Oscp


class WLModuleOscp : public WLModule
{
	Q_OBJECT

public:
    WLModuleOscp(WLDevice *_Device);
   ~WLModuleOscp();

    bool Init(int _size);

private:
   QList <WLOscp*> Oscp;

   uint32_t period=1000;

private:
   bool InitOscp(int size);

public:     
   int getSizeOscp() {return Oscp.size();}
   WLOscp *getOscp(int index);

public:
   bool setSourceChannel(quint8 indexch,enum WLModule::typeModule,enum WLElement::typeElement = typeEEmpty, uint8_t indexElement = 0,uint8_t typeData = 0);
   bool setRun(bool run=true);

public slots:
//virtual void update();
//virtual void backup();

signals:
   void changedValues(double,QList<double>);

public:

 virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
virtual void readCommand(QByteArray data); 
};

#endif // WLModuleOscp_H

