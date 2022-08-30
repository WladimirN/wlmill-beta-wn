#ifndef WLGMACHINE_H
#define WLGMACHINE_H

#include <QDebug>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTextCodec>

#include "wlelementtraj.h"
#include "wlgdrive.h"
#include "wlmotion.h"
#include "wlevscript.h"
#include "wl3dpoint.h"
#include "wl6dpoint.h"
#include "wlenternum.h"
#include "wlgprogram.h"
#include "wlgcode.h"
#include "wlgmodel.h"
#include "wlmachine.h"
#include "wljoysticks.h"
#include "wlheightmap.h"

#define out_spindleCW  0
//#define out_spindleCCW 1
#define out_coolS      1
#define out_coolM      2

#define sizeDrive 4

enum typeProbe{probeXY,probeZ};

struct SGProbe
{
WLGPoint awaitPoint;

WLGPoint probePoint;

  double Zback=100;
  double  dist=15;
  double distA=10;
  double F1=100;
  double F2=0;
  double angle=0;

    bool ex=false;
    bool error=false;
    bool doubleProbe=false;

static double FProbe1;
static double FProbe2;
static double backDist;
static double headDiam;
static bool   enDoubleProbe;
static typeActionInput typeStop;

enum typeProbe type=probeXY;

QString getStartXYStr() {return QString("X%1Y%2").arg(awaitPoint.x-dist*cos(M_PI/180.0*angle),0,'f',5)
                                                 .arg(awaitPoint.y-dist*sin(M_PI/180.0*angle),0,'f',5);}

double sumDist() {return dist+distA;}
};

struct SCorrectSOut
{
float Sadj;
float Scor;

void reset()
{Sadj=Scor=0;}
};



class WLGMachine : public WLMachine
{
	Q_OBJECT

Q_PROPERTY(bool busy  READ isBusy NOTIFY changedBusy())

private:
    enum StateMMachine{noInit,Init,Ready};
    enum TypeAutoMMachine{AUTO_no
                         ,AUTO_DProbe
                         ,AUTO_HProbe
                         ,AUTO_HTProbe
                         ,AUTO_GProbe};


    WLGPoint m_nowBL;
    WLGModel m_GModel;
    WLGPoint lastMillGPoint;
    WLHeightMap m_HeightMap;

    bool  m_busy=false;
    bool  m_runList=false;

    bool  m_waitMScript=false;
    bool  m_pause=false;
    bool  m_waitPause=false;
    bool  m_blcomp=false;

    bool  m_blnextmov=false;
    bool  m_continuemov=false;

    bool  m_useGModel=false;
    bool  m_runGProgram =false;
    bool  m_useMPG=false;
    bool  m_useCorrectSOut=false;
    bool  m_readyRunList=false;

    bool  m_useHMap=false;
    bool  m_useHPause=false;
    bool  m_continue=false;
    bool  m_sStop=false;

    bool m_safeProbe=true;
    bool m_autoStartGCode=false;
    bool m_autoSetSafeGProbe=true;

    double m_distG1StartAt=10;
    double m_feedG1StartAt=0;

    typeActionInput m_actSafeProbe=INPUT_actEmgStop;

    QStringList preRunProgramList;

    enum statesSpindle{Stop,CW,CCW};

    statesSpindle stateSpindle=Stop;
    statesSpindle pauseStateSpindle=Stop;

    bool coolWater=false;
    bool coolAir=false;
public:
    WLGMachine(WLGProgram *_Program,WLEVScript *_MScript,WLEVScript *_LScript,QObject *parent=nullptr);
    ~WLGMachine();

 QMutex MutexFinished;
 QMutex MutexStart;

QList<WLElementTraj>      MillTraj;
QList<WLElementTraj>      baseTraj;
QList<WLElementTraj>  showMillTraj;   

 QMutex MutexTraj;//listMutex;
 QMutex MutexMillAuto;
 QMutex MutexMillTraj;
 QMutex MutexShowTraj;
 QMutex MutexSaveConfig;
 QMutex MutexScript;

Q_INVOKABLE bool isSpindleStop() {return stateSpindle==Stop;}
Q_INVOKABLE bool isSpindleCW()   {return stateSpindle==CW;}
Q_INVOKABLE bool isSpindleCCW()  {return stateSpindle==CCW;}

Q_INVOKABLE bool isIngnoreInPause() {return motDevice->getModulePlanner()->isIgnoreInPause();}
Q_INVOKABLE bool isIngnoreInStop()  {return motDevice->getModulePlanner()->isIgnoreInStop();}

Q_INVOKABLE void setIgnoreInPause(bool ingnore=true) {motDevice->getModulePlanner()->setIgnoreInPause(ingnore);}
Q_INVOKABLE void setIgnoreInStop(bool ingnore=true)  {motDevice->getModulePlanner()->setIgnoreInStop(ingnore);}

Q_INVOKABLE bool runGCode(QString gtxt); 
Q_INVOKABLE bool runGProgram(int istart=0);

Q_INVOKABLE bool isRunGProgram() {return m_runGProgram;}

Q_INVOKABLE bool loadGProgram(QString file,bool build=true);
Q_INVOKABLE QString getNameGProgram(bool full=true) {return full ? m_Program->getNameFile():m_Program->getName();}

Q_INVOKABLE bool isCompleteGProgram() {return m_iProgram==m_Program->getElementCount();}

Q_INVOKABLE quint32 getElementCountGProgram()  {return m_Program->getElementCount();}
Q_INVOKABLE quint32 getActivElementGProgram()  {return m_Program->getActivElement();}
Q_INVOKABLE quint32 getLastMovElementGProgram(){return m_Program->getLastMovElement();}

bool isRunList()      {return m_runList;}
bool isReadyRunList() {return m_readyRunList;}

bool isPossiblyManual();

bool isPossiblyEditModeRun();

 int getMillTrajSize() {return MillTraj.size();}

 quint32 getIProgram() {return m_iProgram;}

QList<WLElementTraj> getTraj() {QMutexLocker locker(&MutexMillTraj); return MillTraj;}
long getTrajIProgramSize();

WLGModel *getGModel() {return &m_GModel;}
    void updateGModel();
private:

void addHeightMap(QList<WLElementTraj> &Traj);
void addBacklash(QList<WLElementTraj> &Traj);
void addCalcGModel(QList<WLElementTraj> &addTraj);
void addRotaryPosition(WLGPoint &lastPoint,QList<WLElementTraj> &addTraj);
void addSmooth(QList<WLElementTraj> &addTraj);
void enableBacklash(bool enable);

QList<WLElementTraj>  addCirclePoints(WLElementTraj  ETraj);

private:
QElapsedTimer m_programTime;
        long m_elementsTime;
//HANDLE mx;

float m_percentF=100;
float m_percentS=100;

private: 	

WLGCodeData m_GCodePause;
WLGCode m_GCode;

QList <SCorrectSOut> m_correctSList;

  double m_FG1=100;
  double m_Fbacklash=100;

    float m_minS=1000;
    float m_maxS=24000;

    float m_minSOut=0;
    float m_maxSOut=100;

    float tarSOut=0;
    float curSOut=0;

    float m_smoothAng=10;
    long m_iElementProgram=0;

	WL3DPoint bufNowBL;

    double m_mainDim=0.001;
    //int m_curTool;
    double m_offsetHTool=0;

    WLGProgram *m_Program=nullptr;

    QString m_CodeMScript;
    QString m_CodeLScript;

//    QTimer *timerLoopPLC;

    double m_baseOffsetTool=0;


    QList <double> m_percentManualList;

    QList<SGProbe> GProbeList;

public:
    void setPercentManualStr(QString str);
    QString getPercentManualStr();

public slots:
    void  plusPercentManual();
    void minusPercentManual();

public:
    QList <SCorrectSOut> correctSList() {return m_correctSList;}

    QList<WLGDrive *> getGDrives();

    QString correctSOut();
	void setStringCorrectSOut(QString str);

    WLGCode   *getGCode()  {return &m_GCode;}
    WLHeightMap *getHeightMap() {return &m_HeightMap;}
private:
    WLJoysticks m_Joysticks;

private:
    WLEVScript *m_MScript=nullptr;
    WLEVScript *m_LScript=nullptr;



private:
  void initMScript();
  void initLScript();
  void initJoystick();
  void initHeightMap();

  void initValuesScript();

   WLMotion *motDevice;
public:
   WLMotion* getMotionDevice() {return motDevice;}

 public:

  WLMPG *getMPG();
public:

  bool isBusy() {return m_busy;}

  double getTimeElement() const {double ret=m_programTime.elapsed(); return m_iProgram==m_elementsTime ? 0 : ret/(m_iProgram-m_elementsTime);}

    void addCurrentSCor();
    void clearSCorList();

    bool isUseHMap()                {return m_useHMap;}
    void setEnableHMap(bool enable) {m_useHMap=enable;}

    bool isUseGModel()               {return m_useGModel;}
    void setEnableGModel(bool enable){m_useGModel=enable;}

    bool isUseHPause()               {return m_useHPause;}
    void setEnableHPause(bool enable){m_useHPause=enable;}

    bool isUseMPG()               {return m_useMPG;}
    void setUseMPG(bool enable)   {m_useMPG=enable;}

double getBaseOffsetTool() {return m_baseOffsetTool;}
void setBaseOffsetTool(double offset) {m_baseOffsetTool=offset;}

float getVMax() {WLModuleAxis *ModuleAxis=motDevice->getModuleAxis();
                 if(ModuleAxis)
					 return ModuleAxis->getFmax()*m_mainDim;
				 else
					 return 50000;
                }

 void setFeedVBacklash(float _VBacklash) {if(_VBacklash>=0) m_Fbacklash=_VBacklash;}
float VBacklash() {return m_Fbacklash;}


float maxSOut() {return m_maxSOut;}
float minSOut() {return m_minSOut;}

bool setRangeSOut(float min,float max) {if(min<max&&min>=0&&max>0) {m_maxSOut=max;m_minSOut=min; return true;} else return false;}
bool setRangeS(float min,float max) {if(min<max&&min>=0&&max>0) {m_maxS=max;m_minS=min; return true;} else return false;}

float Smax() {return m_maxS;}
float Smin() {return m_minS;}

float getSTar() {return tarSOut;}
float calcSOut(float S);

bool isBLNextMov()      {return m_blnextmov;}
bool isContinueMov()    {return m_continuemov;}

WLGPoint getAxisPosition();
WLGPoint getCurrentPosition(bool real=0);
WLGPoint getCurrentPositionActivSC();

virtual void   setAuto();
virtual void resetAuto();



virtual void  updateAuto();

void updateGMachineAuto();
bool updateGProbe();
bool updateProgram();

//bool updateProbe();
//bool updateHProbe();
//bool updateHToolProbe();

void updateMainDimXYZ();

private:
   int  iOperation=0;
quint32 m_iProgram=0;

WLDrive *driveProbe=nullptr;
   bool iDriveDir=0;

 double m_hPause=0;
 TypeAutoMMachine m_typeAutoMMachine=AUTO_no;

public:
Q_INVOKABLE void setSafeProbe(bool enable=true);
Q_INVOKABLE void resetSafeProbe() {setSafeProbe(false);}
Q_INVOKABLE bool isSafeProbe() {return m_safeProbe;}

void setActSafeProbe(typeActionInput act) {m_actSafeProbe=act;}
typeActionInput getActSafeProbe() {return m_actSafeProbe;}

private:
  int updateMovPlanner();
  int updateMovProgram();  

virtual  bool verifyReadyAutoMotion();
virtual  bool verifyReadyMotion();

private slots:
  void init();
  void updateBusy();

public:
  void setHPause(double _hPause) {m_hPause=_hPause;}
  double HPause() {return m_hPause;}

  bool isEmptyMotion();

  bool isRunMScript()   {return   m_MScript->isBusy();}

  float getCurSpeed();
  float getCurSOut() {return curSOut;}

public:

Q_INVOKABLE bool getInProbe()      {return motDevice->getModulePlanner()->getInput(PLANNER_inProbe)->getNow();}
Q_INVOKABLE bool getInSDStop()     {return motDevice->getModuleAxis()->getInput(MAXIS_inSDStop)->getNow();}
Q_INVOKABLE bool getInEMGStop()    {return motDevice->getModuleAxis()->getInput(MAXIS_inEMGStop)->getNow();}
Q_INVOKABLE bool getInPause()      {return motDevice->getModulePlanner()->getInput(PLANNER_inPause)->getNow();}
Q_INVOKABLE bool getInStop()       {return motDevice->getModulePlanner()->getInput(PLANNER_inStop)->getNow();}

Q_INVOKABLE bool getInput(int index)           {return getIn(index);}//old command
Q_INVOKABLE bool getOutput(int index)          {return getOut(index);}//old command
Q_INVOKABLE void setOutput(int index,bool set) {setOut(index,set);}//old command

Q_INVOKABLE bool getIn(int index)  {return getMotionDevice()->getIn(index);}
Q_INVOKABLE bool getOut(int index) {return getMotionDevice()->getOut(index);}

Q_INVOKABLE void setOut(int index,bool set) {getMotionDevice()->setOut(index,set);}
Q_INVOKABLE void setOutPulse(int index,bool set,quint32 time) {getMotionDevice()->setOutPulse(index,set,time);}
Q_INVOKABLE void setOutTog(int index) {getMotionDevice()->setOutTog(index);}

Q_INVOKABLE void  setAOut(int index,float value) {getMotionDevice()->setAOut(index,value);}
Q_INVOKABLE float getAOut(int index) {return getMotionDevice()->getAOut(index);}
Q_INVOKABLE float getAIn(int index) {return getMotionDevice()->getAIn(index);}

Q_INVOKABLE void resetProbe() {if(getMotionDevice()->getModulePlanner()) getMotionDevice()->getModulePlanner()->resetProbe();}

Q_INVOKABLE void setActProbe(int act) {if(getMotionDevice()->getModulePlanner())  getMotionDevice()->getModulePlanner()->setActInProbe((typeActionInput)act);}
Q_INVOKABLE bool isProbe();

Q_INVOKABLE double getProbe(QString name);
Q_INVOKABLE double getProbeSC(QString name) {if(getMotionDevice()->getModulePlanner()){
                                                WLDrive *drive=getDrive(name);
                                                 if(drive){
                                                 return getProbeGPointSC().get(drive->getName());
                                                 }
                                                }
                                            return 0;
                                            }


Q_INVOKABLE  void setSOut(float S);
Q_INVOKABLE  void enableSOut(bool enable) {
                                          setSOut(m_GCode.getValue('S'));
                                          motDevice->getModulePlanner()->setEnableSOut(enable);
                                          } 


   WLGPoint getProbeGPoint();
   WLGPoint getProbeGPointSC();
//Q_INVOKABLE bool isTryProbe(bool dir);
//Q_INVOKABLE double getProbePosition(QString nameDrive,bool dir);

Q_INVOKABLE void rotAboutRotPointSC(int i,float a);


Q_INVOKABLE bool isActiv() {
                           WLModulePlanner *ModulePlanner=motDevice->getModulePlanner();

                           //qDebug()<<"isActiv"<<!MillTraj.isEmpty()
                           //        <<ModulePlanner->isBusy()
                           //        <<isAuto()
                           //        <<isPause();


                           return  !MillTraj.isEmpty()
                                  ||ModulePlanner->isBusy()
                                  ||isAuto()
                                  ||isPause();
                           }

Q_INVOKABLE    void setCurPositionSC(QString nameCoord,double pos);
Q_INVOKABLE double  getCurPositionSC(QString name);

Q_INVOKABLE    void setCurPositionSCT(QString nameCoord,double pos);
Q_INVOKABLE double  getCurPositionSCT(QString name);

Q_INVOKABLE    void setCurPosition(QString nameCoord,double pos);
Q_INVOKABLE double  getCurPosition(QString name);
Q_INVOKABLE QString getCurPositionStr();

Q_INVOKABLE void setTruPositionDrive(QString nameCoord,bool tru);

Q_INVOKABLE	bool isActivDrive(QString name);

Q_INVOKABLE double getOffsetHTool()            {return m_offsetHTool; }
Q_INVOKABLE   void setOffsetHTool(double ofst) {m_offsetHTool=ofst;}
public:

   void resetALM(QString name) {WLDrive::getDrive(name)->resetAlarm();}

   void addElementTraj(QList<WLElementTraj>  ListTraj);

   bool loadConfig();

 private:
    void continueMovList();
    void sendPause(QString msg,int code=-1) {setMessage(metaObject()->className(),msg,code);}

public slots:
    void runMCode(int iM);
    void runMScript(QString txt);

public:
    virtual void addDrive(WLGDrive *millDrive);
    virtual void removeDrive(WLGDrive *millDrive);

private slots:

   void setCompleteScript(QString);
   void setFinished();

   void updateInput();

   void setDataSOut(float val);

 public slots: 

	void saveConfig();
    void saveMScript(QString txt);
    void saveLScript(QString txt);

    void setMessage(QString name,QString data,int code);


    void setPause()   {setPause(true);} //плавная остановка
    void resetPause() {setPause(false);}//плавная остановка

	void reset();

    QString nameClass() {return metaObject()->className();}

	public slots: 	

    void startMov();
    void stopMov();//остановка

    void setEnable(bool on=true);

    public:
    bool isAutoStartGCode()                  {return m_autoStartGCode;}
    void setAutoStartGCode(bool enable=true) {m_autoStartGCode=enable;}

    bool  isAutoSetSafeProbe()                 {return m_autoSetSafeGProbe;}
    void setAutoSetSafeProbe(bool enable=true) {m_autoSetSafeGProbe=enable;}

    void setDriveManualWhell(QString name,quint8 X1=1,bool vmode=false);
    WLGDrive *getDrive(QString nameDrive,bool send=false);


    public:

            void goDriveTouch(QString nameDrive,int dir,float F);

Q_INVOKABLE void clearGProbe();

Q_INVOKABLE void setFGProbe(double F) {if(F>=0) SGProbe::FProbe1=F;}
Q_INVOKABLE void setF1GProbe(double F) {setFGProbe(F);}
Q_INVOKABLE void setF2GProbe(double F) {if(F>=0) SGProbe::FProbe2=F;}

Q_INVOKABLE void   setBackDistGProbe(double _backDist)   {if(_backDist>0) SGProbe::backDist=_backDist;}
Q_INVOKABLE double getBackDistGProbe() {return SGProbe::backDist;}

Q_INVOKABLE void   setHeadDiamGProbe(double _headDiam) {if(_headDiam>=0) SGProbe::headDiam=_headDiam;}
Q_INVOKABLE double getHeadDiamGProbe() {return SGProbe::headDiam;}

Q_INVOKABLE void setSDStopGProbe(bool sd) {SGProbe::typeStop = sd ? INPUT_actSdStop:INPUT_actEmgStop;}

Q_INVOKABLE void enableDoubleGProbe(bool en) {SGProbe::enDoubleProbe=en;}

Q_INVOKABLE void addGProbeXY(double x,double y,double z,double angle,double _dist,double _distA=0);
Q_INVOKABLE void addGProbeZ(double x,double y,double z,double dist,double distA=0);

Q_INVOKABLE void goGProbe() {runGProbe();}
Q_INVOKABLE void runGProbe();
Q_INVOKABLE bool isRunGProbe() {return isAuto()&&(m_typeAutoMMachine==AUTO_GProbe);}
Q_INVOKABLE bool isCompleteGProbe();

Q_INVOKABLE double getGProbe(int index,QString name);
Q_INVOKABLE double getGProbeSC(int index,QString name);
	
Q_INVOKABLE bool isPause() {return m_pause;}
Q_INVOKABLE  void setPause(bool en);//плавная остановка

Q_INVOKABLE float getPercentS()  {return m_percentS;}
Q_INVOKABLE float getPercentF() {return m_percentF;}

Q_INVOKABLE  double getDistG1StartAt() const;
Q_INVOKABLE    void setDistG1StartAt(double distG1StartAt);

Q_INVOKABLE  double getFeedG1StartAt() const;
Q_INVOKABLE    void setFeedG1StartAt(double feedG1StartAt);

public slots:

    void setBLNextMov(bool enable)  {m_blnextmov=enable;}
    void setContinueMov(bool enable){m_continuemov=enable;}

    void setPercentF(double per) {if(0<=per&&per<=300)   {emit changedPercentSpeed(m_percentF=per);}
                                      motDevice->getModulePlanner()->setKF(m_percentF/100);}

    void setPercentS(double per) {if(0.1<=per&&per<=300)   {emit changedPercentSOut(m_percentS=per);}
                                     motDevice->getModulePlanner()->setKSOut(m_percentS/100); }
	                                    

signals:

    void changedPossibleManual(bool);
    void changedPossibleEditModeRun(bool);

    void saveLog(QString,QString);

    void changedEMG(bool);
    void changedRDY(bool);

    void changedSValue(int);   

    void changedBusy(bool);

    void changedReadyRunList(bool);

    void changedCurrentIndex(long);
    void changedSpeedF(float);

    void changedPause(bool);
	
    void changedHomePosition();

    void changedPercentSpeed(double);
    void changedPercentSOut(double);

    void changedCurTool(int);

	void finished();
	
	void noActiv();

    void changedMotion(bool);

	void sendMessage(QString name,QString data,int code);

	void error();

    void changedTrajSize(int);

private slots:

    void updateStatusMPlanner(int status);
    void updateInProbe(bool);
    void updatePosible();
	
friend class WLPositionWidget;
};
#endif // WLGMACHINE_H
