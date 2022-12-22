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

bool chError=true;
};

struct WLObjectScript
{
QObject *obj=nullptr;
QString name;
QString clearScript;
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

QStringList allFunc;
QString allCode;
QStringList m_beforeInitScriptList;
QStringList m_afterInitScriptList;

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
    bool setBaseCode(QString _code,bool eval=false) ;
 QString getCode() {return baseCode;}

Q_INVOKABLE bool runFunction(QString _func,bool _chError=true);
Q_INVOKABLE bool runScript(QString _script,bool _chError=true);
Q_INVOKABLE QVariant getValue(QString name,QVariant def = QVariant(),double timeout=0);
Q_INVOKABLE bool setValue(QString name,QVariant,double timeout=0);
Q_INVOKABLE bool isFuncDefined(QString name);

    bool setProperty(QString name,QScriptValue value);
    bool addObject(QObject *obj,QString name,QString clearScript="");
    bool setObject(QObject *obj,QString name);
    bool isEnable() {return m_enable;}
    bool isReady() {return ready;}

    void addBeforeInitScript(QString script) {m_beforeInitScriptList+=script;}
    void addAfterInitScript(QString script)  {m_afterInitScriptList+=script;}

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
