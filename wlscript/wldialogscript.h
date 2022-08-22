#ifndef WLDIALOGSCRIPT_H
#define WLDIALOGSCRIPT_H

#include <QObject>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QMutex>
#include <QTimer>

#include "wlenternum.h"
#include "wlenterstring.h"

class WLDialogScript : public QObject
{
	Q_OBJECT

public:
    WLDialogScript(QWidget *parent);
    ~WLDialogScript();

private:    
bool    retOk=false;
double  scaleFont=1.5;
double  num=0;
QString str;
QWidget *parentWidget=nullptr;

public:

Q_INVOKABLE double getNum() {return num;}
Q_INVOKABLE QString getString() {return str;}

Q_INVOKABLE void setScaleFont(double scale) {if(0<scale&&scale<10)scaleFont=scale;}
Q_INVOKABLE int question(QString txt);
Q_INVOKABLE int message(QString txt);
Q_INVOKABLE double enterNum(QString txt,double def=0,int decimals=0);
Q_INVOKABLE QString enterString(QString txt,QString def);
Q_INVOKABLE QString enterSaveFile(QString txt,QString lastFile);

Q_INVOKABLE int isOk()     {return  retOk;}
Q_INVOKABLE int isCancel() {return !retOk;}
Q_INVOKABLE int isShow()   {return      0;}
	
public slots:
  void close() {}
};

#endif //  WLDIALOGSCRIPT_H
