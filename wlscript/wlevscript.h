#ifndef WLEVSCRIPT_H
#define WLEVSCRIPT_H

#include <QThread>
#include <QScriptEngine> 
#include <QTimer> 
#include <QDebug> 
#include <QSettings>
#include <QMutex>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
/*
class WLValueScript: public QObject
{
Q_OBJECT

public:
    WLValueScript(QString file):values(file, QSettings::IniFormat) {}
   ~WLValueScript() {}

Q_INVOKABLE void     set(QString name,QVariant value)     {values.setValue(name,value);}
Q_INVOKABLE QVariant get(QString name,QVariant defvalue) {return values.value(name,defvalue);}

private:

QSettings values;
};
*/

struct SLoadCode
{
QString nameFile;
QString code;
};


struct WLTaskScript
{
enum typeTask{Func,Script};

QString taskStr;
typeTask type=Func;

bool detectError=true;
};

struct WLObjectScript
{
QObject *obj=nullptr;
QString name;
};

struct WLValueScript
{
QString name;
QScriptValue value;
};

#define DEF_MAXTASKLIST 50


class WLEVScript : public QThread
{
	Q_OBJECT

public:
    WLEVScript(QObject *parent=nullptr);
	~WLEVScript();

void run();

QScriptEngine *engine=nullptr;

QElapsedTimer timeProcess;

void setIncludePath(QString path) {includePath=path;}
void setEnableTimerTask(bool en=true){enableTimerTask=en;}

private:

QString includePath=QCoreApplication::applicationDirPath();

bool m_busy=false;
bool enableTimerTask=false;

QString baseCode;
QList<WLTaskScript> taskScriptList;
QScriptValueList vList;

QList <SLoadCode> listCode;

QString allCode;
QString m_beforeInitScript;
QString m_afterInitScript;

QMutex Mutex;
QMutex MutexTask;
QMutex MutexCode;

bool m_enable=false;
bool ready=false;

QList<QTimer*> timeoutList;
QList<QTimer*> intervalList;

private:
void updateComment(QString txt);
//private slots:
	//void evalCode() {qDebug()<<"retScript="<<engine->evaluate(code).toString();};
public:
    bool setBaseCode(QString _code) ;
 QString getCode() {return baseCode;}

Q_INVOKABLE bool runFunction(QString _func,bool _detectError=true);
Q_INVOKABLE bool runScript(QString _script,bool _detectError=true);

    bool setProperty(QString name,QScriptValue value);
    bool addObject(QObject *obj,QString name);
    bool setObject(QObject *obj,QString name);
    bool isEnable() {return m_enable;}
    bool isReady() {return ready;}

    void setBeforeInitScript(QString script) {m_beforeInitScript=script;}
    void setAfterInitScript(QString script)  {m_afterInitScript=script;}
private:
    QList <WLObjectScript> m_objList;
    QList <WLValueScript>  m_valList;

public slots:

    void reset();
    void includeFile(QString nameFile);
    void setEnable(bool enable=true);

public:

Q_INVOKABLE int setTimeout(QString func,long ms);
Q_INVOKABLE int setInterval(QString func,long ms);

Q_INVOKABLE void clearTimeout(int id=0);
Q_INVOKABLE void clearInterval(int id=0);

Q_INVOKABLE bool isInterval(int id);
Q_INVOKABLE bool isTimeout(int id);

Q_INVOKABLE void console(QString txt) {emit sendConsole(txt);}
Q_INVOKABLE void process() {if(timeProcess.elapsed()>5)
                             {
                             QCoreApplication::processEvents();
                             timeProcess.start();
                             }
                            }

Q_INVOKABLE bool isBusy() {return m_busy||engine->isEvaluating();}

private :
    void includeOneFile(QString nameFile);
    void evalTask(WLTaskScript taskScript);
    void evalCode(SLoadCode loadCode);

private slots:   

    void evalCodes();
    void evalTasks();
    void initCode(QString);

    void eval() {qDebug()<<"retScript";}

	void setMessage(QString txt,int code) {emit sendMessage(tr("message"),txt,code);}

signals:
    void complete(QString);
    void changedBusy(bool);
    void changedComment(QString,QString);
	void sendMessage(QString,QString,int);
    void error(QString);

    void sendConsole(QString);
};

#endif // WLEVSCRIPT_H
