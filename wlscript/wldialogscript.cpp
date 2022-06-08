#include "wldialogscript.h"

WLDialogScript::WLDialogScript(QWidget *parent)
    : QDialog(parent)
{

}

WLDialogScript::~WLDialogScript()
{

}

int WLDialogScript::question(QString txt)
{    
QMessageBox ques(QMessageBox::Question,tr("Question:"),txt,QMessageBox::Yes|QMessageBox::No);

ques.setWindowFlags(ques.windowFlags()|Qt::FramelessWindowHint);

QFont font=ques.font();
font.setPointSizeF(font.pointSize()*scaleFont);

ques.setFont(font);

ques.show();

if(ques.exec())
    retOk=1;
else
    retOk=0;

return retOk;
}

double WLDialogScript::enterNum(QString txt,double def,int decimals)
{
WLEnterNum EnterNum(this);
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
WLEnterString EnterString(this);

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
str = QFileDialog::getSaveFileName(this, txt,lastFile,"(*.txt)");

retOk=false;

if(!str.isEmpty())
  {
  retOk=true;
  }


return str;
}



int WLDialogScript::message(QString txt)
{
QMessageBox msg(QMessageBox::Information, tr("Message: "),txt,QMessageBox::Ok,this);

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


