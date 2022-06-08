#include "wlmessmanager.h"

WLMessManager::WLMessManager(QWidget *parent)
    : QToolButton(parent)
{

    setIconSize(QSize(32,32));
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    setIcon(QIcon(":/data/icons/notify.png"));

    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::InstantPopup);

    QWidget *popup = new QWidget(this);
    comboBox = new QListWidget(popup);
    //comboBox->setMaxVisibleItems(10);
    comboBox->setMaximumHeight(600);
    comboBox->setMinimumWidth(700);

    pushButtonCErr = new QPushButton(popup);
    pushButtonCErr->setText(tr("clear"));

    pushButtonCErr->setEnabled(false);
    comboBox->setEnabled(false);


    QBoxLayout *popupLayout = new QHBoxLayout(popup);
    popupLayout->setDirection(QBoxLayout::TopToBottom);
    popupLayout->setMargin(2);
    popupLayout->addWidget(comboBox);
    popupLayout->addWidget(pushButtonCErr);

    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(popup);

    menu = new QMenu(this);
    menu->addAction(action);
    setMenu(menu);

    connect(pushButtonCErr,&QPushButton::clicked,this,&WLMessManager::clear);
    connect(pushButtonCErr,&QPushButton::clicked,menu,&QMenu::close);
    //connect(popup,&QWidget::show,[=](){qDebug()<<"show";});
    //connect(action,&QWidgetAction::triggered,[=](){qDebug()<<"triggered";});

    //connect(this,&QAction::triggered,[=](){qDebug()<<"triggered";});
    //connect(this,&QAction::toggled,[=](){qDebug()<<"toggled";});
    connect(menu,&QMenu::aboutToShow,[=](){setIcon(QIcon(":/data/icons/notify.png"));
                                           notify_red=false;
                                          });
    //connect(this,&QToolButton::pressed,[=](){qDebug()<<"pressed";});
    setIcon(QIcon(":/data/icons/notify.png"));
}

WLMessManager::~WLMessManager()
{

}

void WLMessManager::updateFlash()
{
  if(count_flash==0)
     {	 
     if(List.isEmpty())
      pixmap.fill(QColor(0,155,0));
	 else
      pixmap.fill(QColor(155,0,0));	 
     }
   else
     {
     pixmap.fill(QColor(cur?255:155,0,0));
	 cur=!cur;
	 count_flash--;
	 }

//ui.label->setPixmap(pixmap);
}


void WLMessManager::clear()
{
QMutexLocker locker(&mutex);
count_flash=0;
List.clear();
updateList();
}

void WLMessManager::updateList()
{
QString time;
QString contSTR;

comboBox->clear();

pushButtonCErr->setDisabled(List.isEmpty());
comboBox->setDisabled(List.isEmpty());

for(int i=0;i<List.size();i++)
  {
  time=List[i].time.toString("hh:mm:ss:zzz");
  contSTR="...x"+QString::number(List[i].count);

  if(List[i].code<0)
    comboBox->addItem(time+" >ERR "+List[i].name+":"+List[i].mess+"("+QString::number(List[i].code)+")"+contSTR);
  else															
    comboBox->addItem(time+" >MSG "+List[i].name+":"+List[i].mess+"("+QString::number(List[i].code)+")"+contSTR);
  }

setText(QString::number(List.size()));
}

void WLMessManager::setMessage(QString name,QString mess,int code)
{
errData nData;

bool add=false;

mutex.lock();

nData.time=QTime::currentTime();
nData.name=name;
nData.mess=mess;
nData.code=code;
nData.count=1;

for(int i=0;i<List.size();i++)//find
 if(nData.name==List[i].name
  &&nData.mess==List[i].mess
  &&nData.code==List[i].code)
   {
   if(List[i].count<=100)
     {
     List[i].count++;
     }

   List[i].time=nData.time; 
   nData.count=List[i].count;
   add=true;
   
   List.move(i,0);
   break;
   }

if(!add)
{
List.prepend(nData);
emit saveLog("WLMessageManager",name+";"+mess+";"+QString::number(code));
}

if(code<=0) count_flash=10;

updateList();

mutex.unlock();

if(code<=0)
{    
if((mutexShow.tryLock())&&(nData.count<=100))
 {

 if(code==0) QMessageBox::information(this, tr("Message: "),name+":"+mess,QMessageBox::Ok);
     else
      if(code<0) QMessageBox::critical(this, tr("Error: "),name+":"+mess,QMessageBox::Ok);
 
 mutexShow.unlock();
 }
 else
 notify_red=true;
}

if(notify_red)
 setIcon(QIcon(":/data/icons/notify_red.png"));
else
 setIcon(QIcon(":/data/icons/notify_green.png"));

}

