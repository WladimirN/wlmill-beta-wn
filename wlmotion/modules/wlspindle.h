#ifndef WLSpindle_H
#define WLSpindle_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

#include "wlflags.h"
#include "wlmoduleioput.h"

//Spindle
#define comSpindle_setEnable   1 //

#define comSpindle_addData     2 //
#define comSpindle_clearData   3 //

#define comSpindle_setOutElement  4 //
#define comSpindle_resetOutElement  5 //

#define comSpindle_setAcc  6 //
#define comSpindle_setDec  7 //

#define comSpindle_setOutput 8 //
#define comSpindle_setFastChange 9 //

#define comSpindle_setInput 10 //

#define comSpindle_setData 128 //
#define comSpindle_getData 129 //

#define sendMSpindle_data 1 //

#define SF_enable  1<<0
#define SF_useadec 1<<1
#define SF_fastch  1<<2

enum typeInputSpindle{SPINDLE_inORG};
enum typeOutputSpindle{SPINDLE_outENB};

enum typeDataSpindle{
     dataSpindle_Scur
    ,dataSpindle_Star
    ,dataSpindle_OutCur
    ,dataSpindle_flag
  };


typedef struct
{
float  inValue=0;
float outValue=0;

}WLSpindleData;


class WLModuleSpindle;

class WLSpindle : public WLElement
{
	Q_OBJECT

public:
	
explicit WLSpindle(WLModuleSpindle *_ModuleSpindle);
        ~WLSpindle();

private:

WLFlags Flags;

float m_acc=0;
float m_dec=0;

typeElement m_typeSOut;
     quint8 m_iOut;
       bool m_fastSOut=false;

WLSpindleData m_curSpindleData;
QList<WLSpindleData> spindleDataList;

WLIOPut *outENBSpindle;

private:

   bool setInputSpindle(typeInputSpindle type,quint8 num);
   bool setOutputSpindle(typeOutputSpindle type,quint8 num);

public:

   QList<WLSpindleData> getDataList() {return spindleDataList;}

   void setDataList(QList<WLSpindleData> dataList);

   WLSpindleData getCurData(){return m_curSpindleData;}

   bool addDataSpindle(WLSpindleData data);

   void setOutENB(int index);
signals:
 
 void changedError(quint8);
 void changedFreq(float);
 void changedValue(float);
 void changed(int);

public:

 WLIOPut*  getInputSpindle(typeInputSpindle type);
 WLIOPut*  getOutput(typeOutputSpindle type);

 bool setElementSOut(typeElement telement,quint8 i);
 bool resetElementSpindle();
 bool clearDataSpindle();

 quint8 getISOut() {return m_iOut;}
 typeElement getTypeSOut() {return m_typeSOut;}

 bool isFastChangeSOut() {return m_fastSOut;}
 void setFastSOut(bool enable=true) ;

 float getAcc() const {return m_acc;}
 float getDec() const {return m_dec;}

 bool setAcc(float acc);
 bool setDec(float dec);

 void setData(QDataStream&);

 bool sendGetData(typeDataSpindle type);
 bool sendGetData();

public slots:
virtual void update();
virtual void backup();

signals:
   void changedInValue(float);
   void changedOutValue(float);

public:

virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);    
};



#endif // WLSpindle_H
