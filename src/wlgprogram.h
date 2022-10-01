#ifndef WLGProgram_H
#define WLGProgram_H

#include <QObject>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QMutex>

#include "wlgcode.h"
#include "wlgmodel.h"
#include "wlelementtraj.h"

#define DEF_maxShowPoints 1000000

struct WLElementGProgram
{
qint64 offsetInFile;
qint32 offsetPoint;
};

struct WLShowPointProgram
{
 WL3DPointf pos;
 WL3DPointf color;
      float select;
};

class WLGProgram : public QObject
{
	Q_OBJECT

private: 
    bool activ;

    quint32 m_maxShowPoints;

    WLGCode    *m_showGCode;
    WLGCodeData m_showGCodeData;

    QString m_fileName;

    long m_totalKadr;
    long m_buildElement;

	long time;

    bool m_build;
    bool m_buildShow;
	bool updateF;
	QFile File;

	QThread *threadProg;

    quint32 iActivElement;
    quint32 iLastMovElement;

    WL3DPointf maxShowPoint;
    WL3DPointf minShowPoint;

    WL3DPointf m_colorF;
    WL3DPointf m_colorG;
    WL3DPointf m_colorBl;

    WLGModel   m_GModel;

    QList <int> toolList;
    QList <int> scList;

public:
    QList <WLElementGProgram> indexData;
    QList <WLShowPointProgram>  showPoints;

	QMutex Mutex;	
    QMutex MutexShowPoint;
	QMutex MutexBuild;
    QMutex MutexShowBuild;
	
    QString getTextElement(qint32 index);
	QString getTextProgram();

    WL3DPointf getMaxShowPoint() {return maxShowPoint;}
    WL3DPointf getMinShowPoint() {return minShowPoint;}

    bool isBuildShow() {return m_buildShow;}

    void setMaxShowPoints(quint32 val) {if(val>0) m_maxShowPoints=val;}
    quint32 maxShowPoints() {return m_maxShowPoints;}
public:
    WLGProgram(QObject *parent);
    ~WLGProgram();

bool isOpenFile() {return File.isOpen();}
void setGModelData(WLGModelData _GModelData) {m_GModel.setData(_GModelData); updateShowTraj();}

Q_INVOKABLE quint32 getActivElement()   {return iActivElement;}
Q_INVOKABLE quint32 getLastMovElement() {return iLastMovElement;}

QList <int> getSCList() {return scList;}
QList <int> getToolList() {return toolList;}

void setLastMovElement(quint32 val) {iLastMovElement=val;}
void setTextProgram(QString txt);

const QString getNameFile()    {return m_fileName;}
      QString getName()        {return QFileInfo(m_fileName).baseName();}

bool loadFile(QString file,bool m_build=false);
void reloadFile(bool build=false) {loadFile(m_fileName,build);}
void saveFile(QString file);


   bool isActiv() {return activ;}

   long getTotalKadr(){return m_totalKadr;}
   void setTotalKadr(long t) {qDebug()<<"sendTotalKadr "<<t; emit changedTotalKadr(m_totalKadr=t);} 

    int getElementCount() {return indexData.size();}

    void setShowGCode(WLGCode *GCode) {m_showGCode=GCode;}

static bool translate(QString dataStr,QList <WLElementTraj> &curListTraj,WLGCode *GCode,qint32 _index=0,bool GCodeOnly=false);

private:

static  bool convertLine(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);
static  bool convert_straight_comp1(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);
static  bool convert_straight_comp2(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);

static  bool convertArc(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);
static  bool convert_arc_comp1(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);
static  bool convert_arc_comp2(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode);


private:
    long getNKadr(QString data);
    void setActiv(bool a) {activ=a; emit changedActiv(activ);}
	
	
static bool calcDrill(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGPoint &lastGPoint,WLGCode *GCode);

public slots:

    void saveFile() {saveFile(m_fileName);}


    void updateShowTraj() {
                          qDebug()<<"updateShowTraj()";
                          m_buildShow=false;
                          QTimer::singleShot(200,this,SLOT(updateShowTraj_p()));
                          }

    void clear();

private slots:
   void calcTime();
   void buildShowTraj(WLGCodeData GCodeData);

   void updateShowTraj_p() {if(m_showGCode)
                                 {
                                 m_buildShow=true;

                                 m_showGCodeData=m_showGCode->getData();

                                 buildShowTraj(m_showGCodeData);
                                 }
                           }

public slots:

 void stopBuildShow() {m_buildShow=false;}
 void setActivElement(int i) {if(iActivElement!=i) emit changedActivElement(iActivElement=i);}
 
signals:

  void startedBuildShow();
  void changedShowTraj();
  void changedShowProgress(int);

  void changedTrajSize(int);
  void changedActiv(bool);
  void changedActivElement(int);

  void changedProgram(QString);
  void changedProgram();
  
  
  void changedBuildTrajPercent(int);

  void changedTime(long);

  void changedTotalKadr(long total);
  void sendMessage(QString,QString,int);

  void changedCurrentIndex(long index);
};


#endif // WLGProgram_H
