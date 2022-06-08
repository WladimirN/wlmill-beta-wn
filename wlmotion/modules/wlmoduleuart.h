#ifndef WLModuleUART_H
#define WLModuleUART_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"
#include "wluart.h"


class WLModuleUART : public WLModule
{
	Q_OBJECT

public:
    WLModuleUART(WLDevice *_Device);
   ~WLModuleUART();

    bool Init(int sizeUART);
	
private:
   QList <WLUART*> UART;

public:
	
    int getSizeUART() {return UART.size();}

    WLUART* getUART(int m_index);

public slots:

virtual void update();
virtual void backup();

public:

virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
virtual void readCommand(QByteArray data); 
};

#endif // WLModuleUART_H

