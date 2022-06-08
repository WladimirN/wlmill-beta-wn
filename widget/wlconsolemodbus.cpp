#include "wlconsolemodbus.h"
#include "ui_wlconsolemodbus.h"

WLConsoleModbus::WLConsoleModbus(QWidget *parent) :
    QDialog (parent),
    ui(new Ui::WLConsoleModbus)
{
    ui->setupUi(this);

    connect(ui->pbSend,&QPushButton::clicked,this,&WLConsoleModbus::sendData);

    QRegExp rx("[A-Fa-f0-9 ]*");

    QValidator *validator = new QRegExpValidator (rx,this);

    ui->lineEdit->setValidator(validator);
}

WLConsoleModbus::~WLConsoleModbus()
{
    delete ui;
}

void WLConsoleModbus::setModuleModbus(WLModuleDModbus *_Modbus)
{
if(m_Modbus)
 {


 }

m_Modbus=_Modbus;

connect(m_Modbus,&WLModuleDModbus::reciveCallData,this,&WLConsoleModbus::reciveData);
connect(m_Modbus,&WLModuleDModbus::timeoutCallData,this,&WLConsoleModbus::timeoutData);
}

void WLConsoleModbus::sendData()
{
QByteArray data;
QString str_src=ui->lineEdit->text().trimmed();

QString str;

str_src.remove(" ");

while(str_src.size()>0)
{
str+=str_src.at(0);
str_src.remove(0,1);

    if(str_src.size()>0)
    {
    str+=str_src.at(0);
    str_src.remove(0,1);
    }

if(str_src.size()>0)  str+=" ";
}

ui->lineEdit->setText(str);

QStringList list=ui->lineEdit->text().split(" ");

foreach(QString byte,list)
{
data+=byte.toInt(nullptr,16);
}

ui->textEdit->append(QTime::currentTime().toString("hh:mm:ss:zzz")+" >>>:"+ui->lineEdit->text());

if(m_Modbus)
   m_Modbus->sendCallData(data);
}

void WLConsoleModbus::reciveData(QByteArray data)
{
QString str;


for(quint16 i=0;i<data.size();i++){
   QString sym=QString::number((quint8)data[i],16).toUpper();

   if(sym.size()==1) sym.prepend("0");

   str+=sym+" ";
   }

ui->textEdit->append(QTime::currentTime().toString("hh:mm:ss:zzz")+" <<<:"+str+"\r"+"\n");
}

void WLConsoleModbus::timeoutData(QByteArray data)
{
QString str;

for(quint16 i=0;i<data.size();i++){
    QString sym=QString::number((quint8)data[i],16).toUpper();

    if(sym.size()==1) sym.prepend("0");

    str+=sym+" ";
   }

ui->textEdit->append(QTime::currentTime().toString("hh:mm:ss:zzz")+" ERR:"+str+"\r"+"\n");
}
