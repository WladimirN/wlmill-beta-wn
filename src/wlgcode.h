#ifndef WLGCODE_H
#define WLGCODE_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "wl3dpoint.h"
#include "wl6dpoint.h"
#include "wlframe.h"
#include "wldata.h"
#include "wlgpoint.h"

#define GNO_err  1
#define GNO_use  0
#define GErr    -1

#define sizeSC 7
#define maxPG154 100

#define GCodeSize 1000

#define GCodeSize 1000

typedef struct GPar
{
double value;
bool   valid;

GPar()                {value=0;valid=false;}
void set(double _val) {value=_val; valid=true;}

}GPar;

#define GPointNames "X,Y,Z,A,B,C,U,V,W"


struct WLGCodeData
{
 WLData  dataTools;
 WLData  dataSC;

 WLGPoint G43Position;
 WLGPoint G28Position;

 WLGPoint G51Scale;

 WLGPoint  curGPoint;//program!!!
 WLGPoint lastGPoint;//program!!!

 WL3DPoint curPoint;//G41/42
 WL3DPoint endPoint;//G41/42

 int iOfstTool=0;
 int iSC=0;

  double drillPlane=0;
 quint32 drillPms=0;
 quint32 G4Pms=0;

 double G64P=0;
 double G64Q=0;

 bool GCode[GCodeSize];

 bool absIJK;
 bool stopMode;
 bool initDrillPlane;

 QString strRunProgram;
 QString strInit="G64 P0.05 Q0.05";

 double backOffsetLonfDrill=1;

 bool cutter_comp_firstmove=true;

 GPar gX;
 GPar gY;
 GPar gZ;

 GPar gI;
 GPar gJ;
 GPar gK;

 GPar gA;
 GPar gB;
 GPar gC;

 GPar gF;

 GPar gR;
 GPar gP;
 GPar gQ;

 GPar gS;
 GPar gT;
 GPar gD;

 GPar gH;

 WLGCodeData();
};

class WLGCode: public QObject
{
 Q_OBJECT

public:

    enum Code
     {
     fast_motion=00,

     line       =01,
     circle_cw  =02,
     circle_ccw =03,

     wait_motion=04,

     plane_xy   =17,
     plane_zx   =18,
     plane_yz   =19,

     drill      =81,
     drill_long =83,
     drill_off  =80,

     absolute   =90,
     incremental=91,

     plane_drill =98,
     plane_drillR =99
     };

public:
    WLGCode();
    WLGCode(WLGCodeData data);

    ~WLGCode();

    bool isStopMode() {return m_data.stopMode;}

    QList<int> loadStr(QString gstr);

    double getG64Q() {return m_data.G64Q;}
    double getG64P() {return m_data.G64P;}

    void setG64PQ(float smooth,float simply) {if(smooth>=0&&simply>=0) {m_data.G64P=smooth;m_data.G64Q=simply;}}

    bool isInitDrillPlane()        {return  m_data.initDrillPlane;}
    void setDrillPlane(double pos) {m_data.drillPlane=pos;}
    double getDrillPlane()         {return m_data.drillPlane;}
    quint32 getDrillPms()            {return m_data.drillPms;}
    quint32 getG4Pms()   {return m_data.G4Pms;}

    void init();

   double getValue(char);
     bool isValidValue(char);
     bool setValue(char name,double data);

     bool isValidGCode(QString Gx="G0");

   WLGPoint getCurPoint();

   WLGPoint getPointGCode(WLGPoint lastGPoint,bool scale=true);

   WLGPoint getPointG28(WLGPoint lastGPoint);
   WLGPoint getPointG53(WLGPoint lastGPoint);
   WLGPoint getPointIJK(WLGPoint lastGPoint);
   WLGPoint getPointSC(int iSC,WLGPoint GPoint,bool back=false);
   WLGPoint getPointActivSC(WLGPoint GPoint,bool back=false);

   WLGPoint movPointToActivSC(int iLastSC,WLGPoint &lastGPoint);
   WLGPoint movPointToActivToolOfst(int iLastTOfst,WLGPoint &lastGPoint);

   static WLGPoint convertPlane(WLGPoint Point,int plane,bool front);

   int getActivSC(WLGPoint *P=nullptr);

   WLGPoint getOffsetSC(int i,bool *ok=nullptr);
   WLGPoint getOffsetActivSC(bool *ok=nullptr) {return getOffsetSC(m_data.iSC,ok);}
   WLGPoint getRefPointSC(int i,int iref,bool *ok=nullptr);
   WLGPoint getRefPoint0SC(int i,bool *ok=nullptr){return getRefPointSC(i,0,ok);}
   WLGPoint getRefPoint1SC(int i,bool *ok=nullptr){return getRefPointSC(i,1,ok);}

   bool setOffsetActivSC(WLGPoint P)    {return setOffsetSC(m_data.iSC,P);}
   bool setOffsetSC(int i,WLGPoint P,bool send=true);

   bool setRefPointSC(int i,int iref, WLGPoint P);
   bool setRefPoint0SC(int i,WLGPoint P){return setRefPointSC(i,0,P);}
   bool setRefPoint1SC(int i,WLGPoint P){return setRefPointSC(i,1,P);}

   void rotAboutRotPointSC(int i,float a);

   bool calcCenterPointR(WLGPoint startPoint,WLGPoint endPoint);

    int setGCode(QString val);
    int setGCode(int val) {return setGCode(QString::number(val));}

    void resetGCode(int iG=-1);

    bool getMCode(int);

    void reset(void) {resetValid();resetGCode();}

    WLGPoint getG28Position()        {return m_data.G28Position;}
    void setG28Position(WLGPoint hp) {m_data.G28Position=hp;}

    WLGPoint getG43Position()        {return m_data.G43Position;}
    void setG43Position(WLGPoint hp) {m_data.G43Position=hp;}

    int getPlane();

    QString getActivGCodeString();
    QStringList getContextGCodeList();

    double getHToolOfst();
    double getDToolOfst();
    double getCompToolRadius() {return qAbs(getDToolOfst()/2.0);}

    WLGPoint getGToolOfst(int ikey=-1);

    void verifyG51();
    void verifyG43();

    QString getStrRunProgram() {return m_data.strRunProgram;}
    QString getStrInit()       {return m_data.strInit;}

    void setStrRunProgram(QString str) {m_data.strRunProgram=str;}
    void setStrInit(QString str) {loadStr(m_data.strInit=str);}

    void setOffsetBackLongDrill(double offset=0) {if(offset>=0) m_data.backOffsetLonfDrill=offset;}
    double getOffsetBackLongDrill() {return  m_data.backOffsetLonfDrill;}

    void setData(const WLGCodeData &data);

    WLGCodeData getData() const;

    WLGCodeData *data() {return &m_data;}

    int getCompSide();

    void setTool(int ikey,WLEData Tool);
    void setSC(int ikey,WLEData SC);

    WLEData const getSC(int ikey);
    WLEData const getTool(int ikey);

    QVariant getDataTool(int ikey,QString key,QVariant defvalue);
    QVariant getDataSC(int ikey,QString key,QVariant defvalue);

    WLData *getDataTool() {return &m_data.dataTools;}
    WLData *getDataSC()   {return &m_data.dataSC;}


private:
    void resetValid();

    QStringList GValidList;

public:

    static bool detectMCode(QString gstr);
public:

    Q_INVOKABLE void setOffsetTool(int index=0) {data()->iOfstTool=index;
                                                emit changedTool(index);}
    Q_INVOKABLE int getOfstTool();

    Q_INVOKABLE void setXDiam(bool en=true) {setGCode(en ? 7 : 8);}
    Q_INVOKABLE bool isXDiam()              {return isGCode(8);}

    Q_INVOKABLE void removeTool(int ikey);
    Q_INVOKABLE void setDataTool(int ikey,QString key,QVariant value,bool send=true);
    Q_INVOKABLE void setDataCurTool(QString key,QVariant value,bool send=true){setDataTool(getT(),key,value,send);}

    Q_INVOKABLE void removeSC(int ikey);
    Q_INVOKABLE void setDataSC(int ikey,QString key,QVariant value,bool send=true);
    Q_INVOKABLE void setDataCurSC(QString key,QVariant value,bool send=true){setDataTool(getSC(),key,value,send);}

    Q_INVOKABLE double  getDataToolNum(int ikey,QString key,double  defvalue) {return getDataTool(ikey,key,defvalue).toDouble();}
    Q_INVOKABLE QString getDataToolStr(int ikey,QString key,QString defvalue) {return getDataTool(ikey,key,defvalue).toString();}

    Q_INVOKABLE double  getDataCurToolNum(QString key,double  defvalue) {return getDataTool(getT(),key,defvalue).toDouble();}
    Q_INVOKABLE QString getDataCurToolStr(QString key,QString defvalue) {return getDataTool(getT(),key,defvalue).toString();}

    Q_INVOKABLE void setHTool(int i,float h);
    Q_INVOKABLE void setDTool(int i,float d);

    Q_INVOKABLE     int getT()   {return getValue('T');}
    Q_INVOKABLE     int getSC() {return m_data.iSC;}
    Q_INVOKABLE    void setSC(int isc);

    Q_INVOKABLE QString getSCGStr(int iSC=-1);//G54... G154 P...
    Q_INVOKABLE QString getTGStr(int iTool=-1);//T1

    Q_INVOKABLE double getValue(QString name);

    Q_INVOKABLE double  getGSC(){return m_data.iSC+53;}

    Q_INVOKABLE double getHTool(int index);
    Q_INVOKABLE double getDTool(int index);

    Q_INVOKABLE  bool isGCode(int i) {return m_data.GCode[i];}

    Q_INVOKABLE  void push() {m_dataStack=m_data;}

    Q_INVOKABLE  void  pop() {
                              m_dataStack.dataTools=m_data.dataTools;
                              m_data=m_dataStack;
                             }

    Q_INVOKABLE void readToolFile(QString _fileName);
    Q_INVOKABLE void writeToolFile(QString _fileName);

    Q_INVOKABLE void readSCFile(QString _fileName);
    Q_INVOKABLE void writeSCFile(QString _fileName);
private:
    WLGCodeData m_data;

    WLGCodeData m_dataStack;

signals:

void changedF();
void changedTool(int);
void changedSC(int);

public:

virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
};

#endif // WLGCODE_H
