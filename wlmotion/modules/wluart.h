#ifndef WLUART_H
#define WLUART_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"

#define comUART_transmit     1  //передать
#define comUART_setEnable    3
#define comUART_setDelayFrame 4


#define sendUART_recive    20  //принял

#define defTimerTx 100
#define defTimerRx 150

#define comUART_setData 128
#define comUART_getData 129

enum typeDataUART{
  dataUART_baudrate
  };



const QString errorUART("0,no error\
,1,size UART\
,2,parity error\
,3,noise error\
,4,frame error\
,5,overrun error\
,6,DMA transfer error");

#define MaxReciveBufSize 1000

class WLModuleUART;

class WLUART : public WLElement
{
	Q_OBJECT

public:
	
 WLUART(WLModuleUART *_ModuleUART);
~WLUART();

private:

 quint8   error=0;
 quint8   flag=0;
 quint16  m_delayFrame=12;

 uint32_t m_baudrate=9600;

QByteArray m_recive;

public:
   void setError(quint8 _error) {emit changedError(error=_error);}

signals:

 void changedError(quint8);

public:

     bool setBaudrate(quint32 _baudrate);
  quint32 getBaudrate() {return m_baudrate;}
     bool setEnable(bool enable);

     bool sendGetData(typeDataUART type);
     bool sendGetData();

     void setReciveData(QByteArray buf);
     void setData(QDataStream&);

     bool setDelayFrame(quint16 delay_ms);
  quint16 getDelayFrame() {return m_delayFrame;}

     QByteArray takeReciveData();

     void clearReciveData();
     bool isEmptyReciveData();

     bool transmitData(QByteArray data);

     QString getReciveStr(int len=0);
     quint8  getReciveByte();

     double getReciveNum(int type, int n);

signals:

  void changed(int);

public:

virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
};



#endif // WLDCAN_H
