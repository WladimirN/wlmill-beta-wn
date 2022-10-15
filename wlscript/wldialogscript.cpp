#include "wldialogscript.h"

WLDialogScript::WLDialogScript(QWidget *parent)
    : QObject(parent)
{
parentWidget=parent;
}

WLDialogScript::~WLDialogScript()
{

}

int WLDialogScript::question(QString txt)
{    
QMessageBox ques(QMessageBox::Question,tr("Question:"),txt,QMessageBox::Yes|QMessageBox::No);

connect(this,&WLDialogScript::close,&ques,&QMessageBox::close);

ques.setWindowFlags(ques.windowFlags()|Qt::FramelessWindowHint);

QFont font=ques.font();
font.setPointSizeF(font.pointSize()*scaleFont);

ques.setFont(font);
ques.show();

if(ques.exec()==QMessageBox::Yes)
    retOk=1;
else
    retOk=0;

return retOk;
}

double WLDialogScript::enterNum(QString txt,double def,int decimals)
{
WLEnterNum EnterNum(parentWidget);

connect(this,&WLDialogScript::close,&EnterNum,&QMessageBox::close);

EnterNum.setMinMaxNow(-100000,100000,def);
EnterNum.setDecimals(decimals);
EnterNum.setLabel(txt);

EnterNum.setWindowFlags(EnterNum.windowFlags()|Qt::FramelessWindowHint);

QFont font=EnterNum.font();
font.setPointSizeF(font.pointSize()*scaleFont);

EnterNum.setFont(font);

EnterNum.show();

retOk=false;

if(EnterNum.exec())
  {
  retOk=true;
  return num=EnterNum.getNow();
  }

return num=def;
}

QString WLDialogScript::enterString(QString txt,QString def)
{
WLEnterString EnterString(parentWidget);

connect(this,&WLDialogScript::close,&EnterString,&QMessageBox::close);

EnterString.setLabel(txt);
EnterString.setString(def);

EnterString.setWindowFlags(EnterString.windowFlags()|Qt::FramelessWindowHint);

QFont font=EnterString.font();
font.setPointSizeF(font.pointSize()*scaleFont);

EnterString.setFont(font);

EnterString.show();

retOk=false;

if(EnterString.exec()){
  retOk=true;

  return str=EnterString.getString();
  }

return str=def;
}

QString WLDialogScript::enterSaveFile(QString txt,QString lastFile)
{
str = QFileDialog::getSaveFileName(parentWidget, txt,lastFile,"(*.txt)");

retOk=false;

if(!str.isEmpty()) {
  retOk=true;
  }

return str;
}

QString WLDialogScript::enterLoadFile(QString txt, QString lastFile)
{
str = QFileDialog::getOpenFileName(parentWidget, txt,lastFile,"(*.txt)");

retOk=false;

if(!str.isEmpty()) {
  retOk=true;
  }

return str;
}



int WLDialogScript::message(QString txt)
{
QMessageBox msg(QMessageBox::Information, tr("Message: "),txt,QMessageBox::Ok,parentWidget);

connect(this,&WLDialogScript::close,&msg,&QMessageBox::close);

msg.setWindowFlags(msg.windowFlags()|Qt::FramelessWindowHint);

QFont font=msg.font();
font.setPointSizeF(font.pointSize()*scaleFont);
msg.setFont(font);


retOk=false;

if(msg.exec())
    {
    retOk=true;
    }

return  retOk;
}


