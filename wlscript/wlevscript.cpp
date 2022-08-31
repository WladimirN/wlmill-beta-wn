#include "wlevscript.h"

#include <QTextCodec>

WLEVScript::WLEVScript(QObject *parent)
	: QThread(parent)
{

}

WLEVScript::~WLEVScript()
{

}

bool WLEVScript::setBaseCode(QString _code)
{
QMutexLocker locker(&Mutex);

if(isBusy())  
 {
 emit sendMessage("scriptEngine","busy",-1);
 return 0;
 }
else 
 {
 clearTimeout();
 clearInterval();

 baseCode=_code;

 updateComment(baseCode);

 SLoadCode LC;

 LC.code=baseCode;

 listCode+=LC;

 updateComment(baseCode);

 QTimer::singleShot(50 ,this,&WLEVScript::evalCodes);

 return 1;
 }
}

bool WLEVScript::runFunction(QString _func,bool _detectError)
{
qDebug()<<"WLEVScript::runFunction"<<_func<<" detError"<<_detectError;

QMutexLocker locker(&Mutex);
WLTaskScript funcSript;
bool ret = false;

funcSript.taskStr=_func;
funcSript.detectError=_detectError;
funcSript.type=WLTaskScript::Func;

if(isEnable())
 {  
 emit changedBusy(m_busy=true);

 taskScriptList+=funcSript;

 if(taskScriptList.size()<DEF_MAXTASKLIST)
  {
  QTimer::singleShot(0,this,&WLEVScript::evalTasks);
  ret=true;
  }
 else
  {
  setEnable(false);
  emit sendMessage(tr("script"),tr("large list of tasks, script set disable"),-1);
  }
 }

return ret;
}

bool WLEVScript::runScript(QString _script,bool _detectError)
{
QMutexLocker locker(&Mutex);
WLTaskScript taskSript;
bool ret = false;

taskSript.taskStr=_script;
taskSript.detectError=_detectError;
taskSript.type=WLTaskScript::Script;

if(m_enable)
 {
 emit changedBusy(m_busy=true);

 taskScriptList+=taskSript;

 if(taskScriptList.size()<DEF_MAXTASKLIST)
  {
  QTimer::singleShot(0,this,&WLEVScript::evalTasks);
  ret=true;
  }
 else
  {
  setEnable(false);
  emit sendMessage(tr("script"),tr("large list of tasks, script set disable"),-1);
  }
 }

return ret;
}

bool WLEVScript::setValue(QString name, QVariant value)
{
QMutexLocker locker(&Mutex);

if(engine)
  {
  engine->globalObject().setProperty(name,engine->toScriptValue(value));
  return true;
  }

return false;
}

QVariant WLEVScript::getValue(QString name,QVariant def)
{
QScriptValue svalue=engine->globalObject().property(name);

if(svalue.isValid()){
    return svalue.toVariant();
    }else {
    return def;
    }
}

bool WLEVScript::addObject(QObject *obj, QString name)
{
if(obj){     
 WLObjectScript objScript;

 objScript.obj = obj;
 objScript.name = name;

 m_objList.append(objScript);

 QScriptValue sValue = engine->newQObject(objScript.obj);
 engine->globalObject().setProperty(objScript.name,sValue);

 return true;
 }

return false;
}

bool WLEVScript::setObject(QObject *obj, QString name)
{
if(obj){
 QScriptValue sValue = engine->newQObject(obj);
 engine->globalObject().setProperty(name,sValue);

 return true;
 }

return false;
}

bool WLEVScript::setProperty(QString name,QScriptValue value)
{
engine->globalObject().setProperty(name,value);

return true;
}

void WLEVScript::reset()
{
qDebug()<<"WLEVScript::reset";
QMutexLocker locker(&Mutex);

taskScriptList.clear();

if(isBusy())
    {
     QScriptValue sv;

     engine->abortEvaluation(sv);

     emit changedBusy(m_busy=false);
     emit sendMessage(tr("script"),tr("break"),1);
     }
}

void WLEVScript::includeOneFile(QString nameFile)
{
qDebug()<<"WLEVScript::includeOneFile"<<nameFile;

QMutexLocker locker(&Mutex);

QFile file(nameFile);

if(file.open(QIODevice::ReadOnly|QIODevice::Text))
 {
 SLoadCode LC;
 LC.nameFile=nameFile;
 LC.code=QTextCodec::codecForName("Windows-1251")->toUnicode(file.readAll());

 listCode+=LC;

 QTimer::singleShot(50 ,this,&WLEVScript::evalCodes);

 file.close();
 }
 else
 {
 emit sendMessage(tr("script")+". no file:",nameFile,-1);
 }
}

void WLEVScript::includeFile(QString nameFile)
{
qDebug()<<"WLEVScript::includeFile()"<<includePath+nameFile;

QFileInfo info(includePath+nameFile);
QStringList fileList;
QDir dir(info.path());
QString mask(info.fileName());

qDebug()<<"WLEVScript::includeFile()"<<info.path()<<mask;

dir.setFilter(QDir::Files);

if(!mask.isEmpty()) {
  dir.setNameFilters(QStringList()<<mask);
  }

fileList=dir.entryList();

//qDebug()<<fileList.size();

if(fileList.isEmpty())
{
emit sendMessage(tr("script")+": no file:",nameFile,-1);
}
else
{
 foreach(QString nameFile,fileList)
 {
 QFileInfo info(nameFile);

 //qDebug()<<nameFile;
 if(info.completeSuffix()=="js")
   {
   includeOneFile(dir.path()+"//"+nameFile);
   }
 }
}
}

void WLEVScript::setEnable(bool enable)
{
if(m_enable!=enable){

  m_enable=enable;

 if(m_enable){

 Mutex.lock();

 engine->deleteLater();

 engine= new QScriptEngine;

 engine->setProcessEventsInterval(1000);// не работает в Rasberry

 foreach(WLObjectScript objScript,m_objList)
    {
    QScriptValue sValue = engine->newQObject(objScript.obj);
    engine->globalObject().setProperty(objScript.name,sValue);
    }

 Mutex.unlock();

 SLoadCode LC;

 LC.code=baseCode;

 listCode+=LC;

 evalCodes();
 //QTimer::singleShot(0 ,this,&WLEVScript::evalCodes);
 }
 else {
 }

}



}

int  WLEVScript::setTimeout(QString task, long ms)
{
if(!enableTimerTask) return 0;

QTimer *timer=new QTimer(this);

timeoutList+=timer;

connect(timer,&QTimer::timeout,this,[=](){runScript(task);});

timer->setSingleShot(true);

timer->start(ms < 10 ? 10:ms);

qDebug()<<"WLEVScript::setTimeout"<<task<<ms<<"total:"<<timeoutList.size();

return timer->timerId();
}

int WLEVScript::setInterval(QString task, long ms)
{
if(!enableTimerTask) return 0;

QTimer *timer=new QTimer(this);

intervalList+=timer;

connect(timer,&QTimer::timeout,this,[=](){runScript(task);});

timer->start(ms < 10 ? 10:ms);

qDebug()<<"WLEVScript::setInterval"<<task<<ms<<"total:"<<intervalList.size();

return timer->timerId();
}

void WLEVScript::clearTimeout(int id)
{
qDebug()<<"WLEVScript::clearTimeout"<<id;
if(id==0)
 {
 while(!timeoutList.isEmpty())
  {
  QTimer *timer=timeoutList.takeFirst();
  timer->stop();
  timer->deleteLater();
  }
}else{
 foreach(QTimer *timer,timeoutList)
  {
  if(timer->timerId()==id)
    {
    timeoutList.removeOne(timer);
    timer->stop();
    timer->deleteLater();
    }
  }
 }
}

void WLEVScript::clearInterval(int id)
{
qDebug()<<"WLEVScript::clearInterval"<<id;
if(id==0)
 {
 while(!intervalList.isEmpty())
  {
  QTimer *timer=intervalList.takeFirst();
  timer->stop();
  timer->deleteLater();
  }
}else{
 foreach(QTimer *timer,intervalList)
  {
  if(timer->timerId()==id)
    {
    intervalList.removeOne(timer);
    timer->stop();
    timer->deleteLater();
    }
  }
}
}

bool WLEVScript::isTimeout(int id)
{
if(id)   {
  foreach(QTimer *timer,timeoutList)
  {
  if(timer->timerId()==id)
      return true;
  }
}else {
 return !timeoutList.isEmpty();
}

return false;
}

bool WLEVScript::isInterval(int id)
{
if(id)   {
  foreach(QTimer *timer,intervalList) {
  if(timer->timerId()==id)
      return true;
  }

}else {
 return !intervalList.isEmpty();
}

return false;
}


void WLEVScript::run()
{
m_busy=false;

engine= new QScriptEngine(parent());
engine->setProcessEventsInterval(1000);// не работает в Rasberry

addObject(this,"SCRIPT");

qDebug()<<"WLEVScript run"<<"thread"<<(this);

ready=true;

exec();
}

void WLEVScript::updateComment(QString txt)
{
QStringList list;
QRegExp RegExp("function[\\s]+[\\S]+[(][^(]*[)][\\s]*[/]*");
int pos=0;

while ((pos = RegExp.indexIn(txt, pos)) != -1)
{
list+=txt.mid(pos,RegExp.matchedLength());

qDebug()<<list.last();

pos+=RegExp.matchedLength();
}
}

void WLEVScript::evalTasks()
{
if(engine->isEvaluating()) return;

QMutexLocker locker(&MutexTask);

while(!taskScriptList.isEmpty()) {

 Mutex.lock();
 WLTaskScript task=taskScriptList.takeFirst();
 Mutex.unlock();

 evalTask(task);
 }

Mutex.lock();
if(taskScriptList.isEmpty())
   emit changedBusy(m_busy=false);
Mutex.unlock();
}

void WLEVScript::evalTask(WLTaskScript taskScript)
{
qDebug()<<"WLEVScript::evalTask"<<taskScript.taskStr<<taskScript.type;

//Mutex.unlock();
QScriptValue svfunc;

timeProcess.start();

switch(taskScript.type)
{case WLTaskScript::Func:if(taskScript.detectError)
                          {
                          QString find=taskScript.taskStr;

                          find.remove(QRegExp("[(].*[)]"));

                          if(allCode.contains(QRegExp("function[\\s]+"+find+"[(][^(]*[)][\\s]*[/]*"))){

                             svfunc = engine->evaluate(taskScript.taskStr);

                            if(svfunc.isError())   {
                               qDebug()<<"WLEVScript::evalTask"<<taskScript.taskStr<<"error func:"<<svfunc.toString();
                               emit error(svfunc.toString());
                               emit sendMessage("scriptEngine",svfunc.toString(),-1);
                               }else {
                               qDebug()<<"WLEVScript::evalTask"<<taskScript.taskStr<<"complete func";
                               emit complete(taskScript.taskStr);
                               }

                            //qDebug()<<"retScript"<<svfunc.property("X").toString();
                            //qDebug()<<"retScript"<<svfunc.property("Y").toString();
                            //qDebug()<<"retScript"<<svfunc.property("Z").toString();

                            //qDebug()<<"retScript"<<svfunc.property(0).toString();
                            //qDebug()<<"retScript"<<svfunc.property(1).toString();
                            //qDebug()<<"retScript"<<engine->importedExtensions()[0];
                            }else{ //no include func
                            //qDebug()<<"WLEVScript::evalFunc no:"<<funcScript.func;
                            emit complete(taskScript.taskStr);
                            }
                          }else {
                          svfunc = engine->evaluate(taskScript.taskStr);
                          }
                           break;

case WLTaskScript::Script:   svfunc = engine->evaluate(taskScript.taskStr);

                             if(taskScript.detectError&&svfunc.isError()) {
                                clearTimeout();
                                clearInterval();

                                qDebug()<<"WLEVScript::evalTask"<<taskScript.taskStr<<"error script:"<<svfunc.toString();
                                emit error(svfunc.toString());
                                emit sendMessage("scriptEngine",svfunc.toString(),-1);
                                }
                                else{
                                qDebug()<<"WLEVScript::evalTask"<<taskScript.taskStr<<"complete script";
                                emit complete("");
                                }

                             break;
}
}

void WLEVScript::initCode(QString initFunc)
{
bool en=m_enable;

m_enable=true;
runScript(initFunc,false);
m_enable=en;
}

void WLEVScript::evalCodes()
{
QMutexLocker locker(&MutexCode);

m_busy=true;

while(!listCode.isEmpty()){

Mutex.lock();
SLoadCode LC=listCode.takeFirst();
Mutex.unlock();

evalCode(LC);

emit complete(LC.code);
}

m_busy=false;
};


void WLEVScript::evalCode(SLoadCode LC)
{
//QMutexLocker locker(&Mutex);
//
//m_busy=true;

if(!LC.code.isEmpty())
{
QScriptValue svcode=engine->evaluate(LC.code);

if(svcode.isError())
 {
 emit error(svcode.toString());
 emit sendMessage("scriptEngine:"+LC.nameFile,svcode.toString(),-1);
 }
 else {
 allCode+=LC.code;
 qDebug()<<" WLEVScript::evalCode Complete"<<LC.nameFile<<svcode.isError();

 if(LC.nameFile.isEmpty()){
    initCode(m_beforeInitScript+"\r\n"+"init()"+"\r\n"+m_afterInitScript);
    }
    else{
         setProperty(QFileInfo(LC.nameFile).baseName()+"FileINI"
                    ,QFileInfo(LC.nameFile).path()+"//"+QFileInfo(LC.nameFile).baseName()+".ini");

         setProperty(QFileInfo(LC.nameFile).baseName()+"Path"
                    ,QFileInfo(LC.nameFile).path()+"//");

         initCode(QFileInfo(LC.nameFile).baseName()+"Init()");
    }
 }
}

//emit complete(LC.code);
//
//m_busy=false;
};
