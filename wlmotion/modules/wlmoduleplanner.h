#ifndef WLModulePLANNER_H
#define WLModulePLANNER_H

#include <QObject>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStringList>
#include <QMutex>
#include <QTimer>

#include "wlmodule.h"
#include "wlaxis.h"
#include "wlmoduleioput.h"
#include "wlmodulespindle.h"

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

//Planner
#define comPlanner_addCirc       2 //
#define comPlanner_addLine       3 //
#define comPlanner_start         4 //
#define comPlanner_stop          5 //
#define comPlanner_pause         6 //
#define comPlanner_reset         7 //
#define comPlanner_setKF         8 //
#define comPlanner_setSmoothAng  9 //установка предела сглаживания 
#define comPlanner_setSOut         10 
#define comPlanner_setKFpause       11
#define comPlanner_setMaxNView     12
#define comPlanner_clear           13
#define comPlanner_toSpindle        14//to spindle command
#define comPlanner_setKSOut        15 
#define comPlanner_addULine        16
#define comPlanner_setISlaveAxis   17
#define comPlanner_enableSOut      18

#define comPlanner_setActInProbe  19 //set action in probe
#define comPlanner_setInput       20 //setInput

#define comPlanner_setHPause      21 //set offset z detectpause;
#define comPlanner_setModeRun     22 //set mode run
#define comPlanner_setActSafeProbe 23 //set action safe probe

#define comPlanner_setIgnoreInput 25 //set ignore input
#define comPlanner_addDelay   26 //add Delay_ms element
#define comPlanner_setISpindle 27 //set Spindle

#define comPlanner_getDataPlanner    101

#define comPlanner_setData 128
#define comPlanner_getData 129

#define comPlanner_getProp    102

#define sendPlanner_rInProbe  19 //send position inProbe (rise / front)
#define sendPlanner_fInProbe  20 //send position inProbe (fall \ front)

#define sendPlanner_signal 200
/* 
enum typeSignalBuf{_sigChgEmptyBuf_ui8   
                  ,_sigChgSOut_f32
                  ,_sigChgFreeSizeBuf_ui8}; 
*/ 
#define sendPlanner_data   201
//#define sendPlanner_prop   202 //запрос данных модуля

//#define sendPlanner_error  255

#define MASK_abs       1<<1 //absolute coordinate
#define MASK_circxyz   1<<2 //3 axis circ
#define MASK_ccw       1<<3 //counter clock wise
#define MASK_fast      1<<4 //fast
#define MASK_ensmooth  1<<5 //enable smooth

#define errorPlanner_emg       1
#define errorPlanner_buffull   2
#define errorPlanner_setdata   3
#define errorPlanner_waxis     4
#define errorPlanner_welement  5 


#define errorULine_pos   1
#define errorULine_mov   2
#define errorCirc_pos    3
#define errorCirc_radius 4
#define errorLine_df     5
#define errorLine_count  6

#define PLF_enable     (1<<0)
#define PLF_safeprobe  (1<<1)
#define PLF_empty      (1<<2)
#define PLF_moving     (1<<3)
#define PLF_chgdata    (1<<4)
#define PLF_usehpause  (1<<5)

#define PLF_enableSOut (1<<7)

const QString errorPlanner("0,no error\
,1,emg stop\
,2,buf is full\
,3,wrong set data");

const QString errorElementPlanner("0,no error\
,1,wrong ULine position\
,2,wrong ULine mov distance\
,3,wrong Circ position\
,4,wrong Circ radius\
,5,error Line calc\
,6,wrong Line count");

enum typeDataPlanner{
                     dataPlanner_curSpindleInValue
                    ,dataPlanner_curSpindleOutValue
                    ,dataPlanner_F
                    };


enum statusPlanner{PLANNER_stop
                  ,PLANNER_run
                  ,PLANNER_pause
                  ,PLANNER_paused
                  ,PLANNER_stopped
                  ,PLANNER_waitStart};

enum modeRunPlanner{PLANNER_normal
                   ,PLANNER_oneElement
                   ,PLANNER_oneBlock};

enum typeInputPlanner{PLANNER_inProbe
                     ,PLANNER_inPause
                     ,PLANNER_inStop};





class WLModulePlanner : public WLModule
{
	Q_OBJECT

public:
    enum typeInputSpindle{SPINDLE_inEMGStop};
    enum typeOutputSpindle{SPINDLE_outENB};

public:
    WLModulePlanner(WLDevice *_Device);
	~WLModulePlanner();

private:

QMutex Mutex;
QMutex mutexProbe;

statusPlanner  m_status=PLANNER_stop;
modeRunPlanner m_modeRun=PLANNER_normal;

WLFlags Flags;
int m_sizeBuf;
int m_free;

float m_KFpause=0.25f;

quint8  m_indexRingElementBuf=0;
quint32 m_curIdElementBuf=0;
quint32 m_lastIdElementBuf=0;

float m_accSpindle=0;
float m_decSpindle=0;

float m_KF=1;
float m_KSOut=1;

float m_smoothAng=15;

qint32  m_hPause;

WLIOPut *inProbe;
WLIOPut *inPause;
WLIOPut *inStop;

typeActionInput m_actSafeProbe=INPUT_actEmgStop;

bool m_ignoreInPause=false;
bool m_ignoreInStop=false;

QList<quint8> m_axisList;

QList<qint32> m_posProbe2;
QList<qint32> m_posProbe3;

bool m_validProbe2=false;
bool m_validProbe3=false;

QVector <quint8> m_indexsAxis;

QTimer *updateTimer;

quint8 iSpindle=255;

private:
     bool setInput(typeInputPlanner getTypeModule,quint8 num);
     bool setIgnoreInput(typeInputPlanner getTypeModule,quint8 ignore);

public:
     WLIOPut*  getInput(typeInputPlanner type);

     void clear();

     void setModeRun(modeRunPlanner modeRun);
     modeRunPlanner getModeRun(){return m_modeRun;}

     WLSpindle* getSpindle();

     bool setISpindle(quint8 index);

     void setInProbe(int index);
     void setInPause(int index);
     void setInStop(int index);

     bool isIgnorePause() {return  m_ignoreInPause;}
     bool isIgnoreStop()  {return  m_ignoreInStop;}

     int getSizeBuf() {return m_sizeBuf;}
     int getCountBuf() {return m_sizeBuf-m_free;}

     float getKFpause() {return m_KFpause;}
     float getSmoothAng() {return m_smoothAng;}

  quint32 getCurIdElement() {return m_curIdElementBuf;}
  quint32 getLastIdElement() {return m_lastIdElementBuf;}

    bool setIAxisSlave(quint8 *indexsAxis,quint8 size);
    QVector <quint8> getIAxisSlave() {return m_indexsAxis;}
    quint8 getIAxis(uint8_t i) {return m_indexsAxis.indexOf(i);}

    bool setHPause(quint8 enable,qint32 hPause);

    bool clearDataSpindle();    

    bool addULine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 midPos[],float S,float Fmov,quint32 _id);
    bool addLine(quint8 mask,quint8 size ,quint8 indexs[],qint32 endPos[],float S,float Fmov,quint32 _id);
    bool addCirc(quint8 mask,quint8 size ,quint8 indexs[],qint32 endPos[],qint32 cenPosIJ[],float S,float Fmov,quint32 _id);
    bool addDelay(quint32 delayms,float S,quint32 _id);

	bool startMov();
	bool stopMov();
	bool pauseMov();

	bool setKF(float _KF);
   float getKF()  {return m_KF;}
    bool setKFpause(float _KF);

	bool setSmoothAng(float ang_gr);

	bool setSOut(float s);
	bool setKSOut(float k);
   float getKSOut(){return m_KSOut;}

	bool setEnableSOut(quint8 enable);
		
     int getFree()   {return m_free;}
    void setSizeBuf(int value);

    bool setActInProbe(typeActionInput typeAct);
    bool setActSafeProbe(typeActionInput typeAct);

    bool setIgnoreInPause(bool ignore) {return setIgnoreInput(PLANNER_inPause,ignore);}
    bool setIgnoreInStop(bool ignore)  {return setIgnoreInput(PLANNER_inStop,ignore);}

    bool isIgnoreInPause() {return m_ignoreInPause;}
    bool isIgnoreInStop()  {return m_ignoreInStop;}

    typeActionInput  getActSafeProbe(){return m_actSafeProbe;}

statusPlanner getStatus()  const {return m_status;}

   bool isEmpty()  {return Flags.get(PLF_empty);}
   bool isMoving() {return Flags.get(PLF_moving);}
   bool isBusy()  {return !isEmpty()||isMoving();}

   qint32 getProbe2(int index) {if(0<=index&&index<m_posProbe2.size()) return m_posProbe2.at(index); else return 0;}
   qint32 getProbe3(int index) {if(0<=index&&index<m_posProbe3.size()) return m_posProbe3.at(index); else return 0;}

   void resetProbe() {m_validProbe2=m_validProbe3=false;}

   bool isProbe2() {return m_validProbe2;}
   bool isProbe3() {return m_validProbe3;}

   void setData(QDataStream &data);
   void getData(typeDataPlanner getTypeModule);


private slots:
   void callTrackPlanner();

public slots:
	void sendGetDataBuf();

public slots:
virtual void update();
virtual void backup();

signals:
    void changedFree(int);
    void changedStatus(int);
    void changedCurIElement(int);
    void changedSOut(float);
    void changedProbe(bool);
    void changedPause(bool);
    void changedStop(bool);
    void changedEmpty(bool);
    void reset();

public:
virtual void getProp() {}
virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
virtual void readCommand(QByteArray data); 

                           };
#endif // WLModulePLANNER_H

