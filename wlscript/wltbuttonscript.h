#ifndef WLTBUTTONSCRIPT_H
#define WLTBUTTONSCRIPT_H

#include <QCoreApplication>
#include <QThread>
#include <QTimer> 
#include <QDebug> 
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QWidgetAction>
#include <QKeySequence>
#include <QPushButton>
#include <QBoxLayout>

#include "wlevscript.h"


class WLTButtonScript: public QToolButton
{
Q_OBJECT

public:
     WLTButtonScript(QString _name,QString _txt="",QString _script="",QWidget *parent=nullptr):QToolButton(parent) {
         if(_txt.isEmpty())    _txt=_name;
         if(_script.isEmpty()) _script=_txt+"()";

         m_name=_name;
         m_scriptPress=_script;
         m_menu= new QMenu();

         setText(_txt);
         setToolTip(_name);

         m_vaction=defaultAction();

         popup = new QWidget(this);
         popup->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

         popupLayout = new QVBoxLayout(popup);
         popupLayout->setMargin(2);

         waction = new QWidgetAction(this);
         waction->setDefaultWidget(popup);

         m_menu = new QMenu(this);
         m_menu->addAction(waction);

         connect(this,&WLTButtonScript::clicked,this,[=](){
             if(!isCheckable())
              {
              qDebug()<<"TButtonScript pressed - "<<m_scriptPress;
              emit sendRunScript(m_scriptPress);
              }
         });

         connect(this,&WLTButtonScript::pressed,this,[=](){
             if(isCheckable())
              {
              qDebug()<<"TButtonScript pressed - "<<m_scriptPress;
              emit sendRunScript(m_scriptPress);
              }
         });

         connect(this,&WLTButtonScript::released,this,[=](){
             if(isCheckable())
                {
                qDebug()<<"TButtonScript release"<<m_scriptRelease;
                emit sendRunScript(m_scriptRelease);
                }
         });

         }

    ~WLTButtonScript() {}

static  void setIconPath(QString path) {iconPath=path;}

private:
   QString  m_scriptPress;
   QString  m_scriptRelease;

   QWidgetAction *waction=nullptr;
   QWidget  *popup=nullptr;
   QMenu    *m_menu=nullptr;

   QAction *m_vaction=nullptr;
   QString  m_name;

   bool defVisible=false;
   bool m_lock=false;

   QList <WLTButtonScript*> tbuttonList;

   QBoxLayout *popupLayout = nullptr;

static  QString  iconPath;

public:

QString getName() {return m_name;}
void setVAction(QAction *action) {m_vaction=action;}
QAction *getVAction() {return m_vaction;}

Q_INVOKABLE  bool isShow()    {return m_vaction ? m_vaction->isVisible():isVisible();}
Q_INVOKABLE  bool isEnabled() {return m_vaction ? m_vaction->isEnabled():isEnabled();}
//Q_INVOKABLE  bool isChecked() {return m_action->isChecked();}
/*
Q_INVOKABLE  void setCheckable(bool en) {m_action->setCheckable(en);}
Q_INVOKABLE  void setChecked(bool ch)   {m_action->setChecked(ch);}
*/

Q_INVOKABLE  void setText(QString _text)      {QToolButton::setText(_text);}
Q_INVOKABLE  void setToolTip(QString _tooltip)   {QToolButton::setToolTip(_tooltip);}

Q_INVOKABLE  void setScript(QString _script="")         {m_scriptPress=_script;}
Q_INVOKABLE  void setScriptRelease(QString _script="")  {m_scriptRelease=_script;}
/*
Q_INVOKABLE  void setScriptSub(int index,QString _script="")         {if(0<=index&&index<actionsList.size()) actionsList.at(index)->setScript(_script);}
Q_INVOKABLE  void setScriptReleaseSub(int index,QString _script="")  {if(0<=index&&index<actionsList.size()) actionsList.at(index)->setScriptRelease(_script);}

Q_INVOKABLE  void setChekableSub(int index,bool en){if(0<=index&&index<actionsList.size()) actionsList.at(index)->setCheckable(en);}
Q_INVOKABLE  void setChekedSub(int index,bool ch)  {if(0<=index&&index<actionsList.size()) actionsList.at(index)->setChecked(ch);}
*/
Q_INVOKABLE  void setIconSub(int index,QString file="") {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setIcon(file);}
Q_INVOKABLE  void setIconFromSub(int index,QString file="") {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setIconFrom(file);}

Q_INVOKABLE  void setToolTipSub(int index,QString txt)  {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setToolTip(txt);}
Q_INVOKABLE  void setTextSub(int index,QString txt)     {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setText(txt);}
Q_INVOKABLE  void setShortcutSub(int index,QString txt) {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setShortcut(txt);}
Q_INVOKABLE  void setVisibleSub(int index,bool en)      {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setVisible(en);}
Q_INVOKABLE  void setEnabledSub(int index,bool en)      {if(0<=index&&index<tbuttonList.size()) tbuttonList.at(index)->setEnabled(en);}

Q_INVOKABLE   int addButtonMenu(QString txt,QString script,QString toolTip="")
                                                            {
                                                            WLTButtonScript *tbutton = new WLTButtonScript(txt,txt,script,this);

                                                            if(!toolTip.isEmpty())
                                                                tbutton->setToolTip(toolTip);

                                                            tbutton->setIconSize(iconSize());

                                                            connect(this,&WLTButtonScript::setIconSize,tbutton,&WLTButtonScript::setIconSize);
                                                            connect(tbutton,&WLTButtonScript::sendRunScript,this,&WLTButtonScript::sendRunScript);
                                                            connect(tbutton,&WLTButtonScript::clicked,m_menu,&QMenu::close);

                                                            if(tbuttonList.isEmpty())
                                                                 {
                                                                 setPopupMode(QToolButton::InstantPopup);
                                                                 setMenu(m_menu);
                                                                 }

                                                            popupLayout->addWidget(tbutton);

                                                            tbuttonList+=tbutton;

                                                            return  tbuttonList.size()-1;
                                                            }

public:

public slots:
   void setIcon(QString file)     {QToolButton::setIcon(QIcon(WLTButtonScript::iconPath+file));}
   void setIconFrom(QString file) {QToolButton::setIcon(QIcon(file));}

   void setLock(bool lock=true)  {m_lock=lock;}
   bool isLock()                 {return m_lock;}

   void setShortcut(QString txt) {QToolButton::setShortcut(QKeySequence(txt));}

   void setShow(int i)       {if(m_vaction) m_vaction->setVisible(i);}
 //void setEnabled(bool en)  {if(m_vaction) m_vaction->setEnabled(en);}


   void reset(){m_scriptPress.clear();
                m_scriptRelease.clear();
                clearMenu();
                setCheckable(false);
                }

   void clearMenu() {while(!tbuttonList.isEmpty()){
                      popupLayout->removeWidget(tbuttonList.first());
                      delete tbuttonList.takeFirst();
                      }

                     setMenu(nullptr);
                     setPopupMode(QToolButton::DelayedPopup);
                    }

signals:
   void sendRunScript(QString);

};


class WLTBarScript: public QToolBar
{
Q_OBJECT

public:

explicit WLTBarScript(WLEVScript *_script, QString title,QWidget *parent=nullptr):QToolBar(title,parent)
     {
     script=_script;

     }

 ~WLTBarScript() {}

WLTButtonScript *getButton(QString name)
{
foreach(WLTButtonScript *tbScript,tbScriptList)
    if(tbScript->getName()==name)
        return tbScript;

return nullptr;
}

private:

WLEVScript *script=nullptr;

QList <WLTButtonScript*> tbScriptList;

signals:
 void runScript(QString);

public:

 void addLockButton(QString _name,QString _txt="",QString _script="",QString toolTip=""){
                   if(!getButton(_name))
                    {
                    WLTButtonScript *tbScript = new WLTButtonScript(_name,_txt,_script,static_cast<QWidget*>(parent()));

                    tbScript->setLock(true);

                    if(!toolTip.isEmpty())
                        tbScript->setToolTip(toolTip);

                    QAction *act;

                    act=addWidget(tbScript);

                    tbScript->setVAction(act);

                    tbScriptList.append(tbScript);

                    script->addObject(tbScript,_name);

                    tbScript->setIconSize(iconSize());

                    connect(tbScript,&WLTButtonScript::sendRunScript,this,&WLTBarScript::runScript);
                    }
                   }

 Q_INVOKABLE void addButton(QString _name,QString _txt="",QString _script="",QString toolTip=""){
                   if(!getButton(_name))
                    {
                    WLTButtonScript *tbScript = new WLTButtonScript(_name,_txt,_script,static_cast<QWidget*>(parent()));

                    if(!toolTip.isEmpty())
                        tbScript->setToolTip(toolTip);

                    QAction *act;

                    act=addWidget(tbScript);

                    tbScript->setVAction(act);                    

                    tbScriptList.append(tbScript);

                    script->setObject(tbScript,_name);

                    tbScript->setIconSize(iconSize());

                    connect(tbScript,&WLTButtonScript::sendRunScript,this,&WLTBarScript::runScript);
                    }
                   }

 Q_INVOKABLE void removeButtons(){
                    WLTButtonScript *tbScript;

                    for(int i=0;i<tbScriptList.size();i++){

                     tbScript=tbScriptList[i];

                     if(!tbScript->isLock()){
                       tbScriptList.removeAt(i--);
                       removeAction(tbScript->getVAction());
                       delete tbScript;
                       }
                     }

                   }


};


#endif // WLTBUTTONSCRIPT_H
