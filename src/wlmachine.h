#ifndef WLMACHINE_H
#define WLMACHINE_H

#include <QObject>
#include <QMutex>
#include <QThread>

#include "wldrive.h"

class WLMachine : public QThread
{
Q_OBJECT

public:
    enum TypeAutoMachine{AUTO_no=0
                        ,AUTO_DrivesFind};

public:
    WLMachine(QObject *parent=nullptr);

   ~WLMachine();

public:
virtual void addDrive(WLDrive *_drive);
virtual void removeDrive(WLDrive *_drive);

   QList<WLDrive*> getDrives() {return m_Drives;}

public:
QString getStrFindDrivePos()            {return m_strFindDrivePos;}
   void setStrFindDrivePos(QString str) {m_strFindDrivePos=str;}

private:

 bool m_auto=false;
 bool m_ready=false;
 bool m_enable=false;

 double m_percentManual=25.0f;

 enum TypeAutoMachine m_typeAutoMachine=AUTO_no;
 QMutex MutexAuto;

private:
//for find position
QString m_strFindDrivePos;
QList<QString> m_listFindDrivePos;
QList<WLDrive*> m_Drives;

bool updateDrivesFindPos();

private:

virtual bool verifyReadyMotion()     {return true;}
virtual bool verifyReadyAutoMotion() {return true;}
virtual bool isPossiblyManual()      {return true;}

public:

  void SDStop();

           bool isAuto() {return m_auto;}
virtual void   setAuto() {emit changedAuto(m_auto=true);}
virtual void resetAuto() {emit changedAuto(m_auto=false);}

  bool isReady() {return m_ready;}

Q_INVOKABLE bool goDriveManual(QString name,double IncDec,float step=0);
Q_INVOKABLE bool goDriveFind(QString nameDrive);
Q_INVOKABLE bool goDriveTeach(QString nameDrive);
Q_INVOKABLE bool goDriveVerify(QString nameDrive);

Q_INVOKABLE bool   goDrivesFind();
Q_INVOKABLE void setDrivesFinded();

Q_INVOKABLE bool isEnable() {return  m_enable;}

  bool isActivDrives()  {return  WLDrive::isActivDrives();}

public:
  virtual void setEnable(bool enable);
  virtual void updateAuto();

protected:
  void setReady(bool ready);

public slots:
   void setPercentManual(double per);

public:

 Q_INVOKABLE double getPercentManual() {return m_percentManual;}

signals:
   void changedAuto(bool);
   void changedReady(bool);
   void changedPercentManual(double);
   void changedEnable(bool);

protected:
virtual void init() {}
        void run() {init();exec();}
};

#endif // WLMACHINE_H
