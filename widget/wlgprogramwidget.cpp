#include "wlgprogramwidget.h"

WLGProgramWidget::WLGProgramWidget(WLGProgram *_Program,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    m_ListModel = new WLGProgramListModel(_Program,this);

	m_GProgram=_Program;

	m_changedProgram=false;

	connect(ui.textProgram,SIGNAL(cursorPositionChanged ()),SLOT(onChangedPositionTextProgram()));
    connect(ui.textProgram,SIGNAL(textChanged()),SLOT(onChangedTextProgram()));


   ui.listProgram->setModel(m_ListModel);
   ui.listProgram->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

   ui.listProgram->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   connect(ui.listProgram->selectionModel(),&QItemSelectionModel::currentChanged,
                                        this,&WLGProgramWidget::onChangedPositionListProgram);


    ui.progressBar->setVisible(false);

    ui.sbPosition->setKeyboardTracking(false);

    connect(ui.sbPosition,QOverload<int>::of(&QSpinBox::valueChanged),ui.pbToLine,&QPushButton::click);

    connect(ui.pbToLine,&QPushButton::clicked,this,[=](){
    setEditElement(ui.sbPosition->value());
    emit changedEditElement(ui.sbPosition->value());
    
     
    });

    connect(m_GProgram,SIGNAL(changedProgram()),SLOT(loadTextProgram()));
    connect(m_GProgram,SIGNAL(changedShowProgress(int)),ui.progressBar,SLOT(setValue(int)));

	connect(ui.pbAccept,SIGNAL(clicked()),SLOT(onAccept()));
	connect(ui.pbBackup,SIGNAL(clicked()),SLOT(onBackup()));
	connect(ui.pbReload,SIGNAL(clicked()),SLOT(onReload()));
    connect(ui.pbOpenFile,SIGNAL(clicked()),SLOT(onOpenFile()));
    connect(ui.pbTrack,SIGNAL(toggled(bool)),SLOT(setTrack(bool)));
    connect(ui.pbClear,&QPushButton::clicked,m_GProgram,&WLGProgram::clear);

	connect(this,SIGNAL(changed(bool)),ui.pbAccept,SLOT(setEnabled(bool)));
	connect(this,SIGNAL(changed(bool)),ui.pbBackup,SLOT(setEnabled(bool)));	
		
	ui.textProgram->setEnabled(false);
    m_GCodeSH = new WLGCodeSH(ui.textProgram->document());

    ui.listProgram->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.listProgram, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenuListProgram(QPoint)));

    QTimer *timer=new QTimer;

    connect(timer,SIGNAL(timeout()),this,SLOT(updateTrack()));
    timer->start(100);    
}

WLGProgramWidget::~WLGProgramWidget()
{

}

void WLGProgramWidget::onUpdate()
{
m_GProgram->updateShowTraj();
}

void WLGProgramWidget::onAccept()
{
if((QMessageBox::question(this, tr("Confirmation:"),
       tr("save program?"),
       QMessageBox::Yes|QMessageBox::No))== QMessageBox::Yes)
   {
   saveTextProgram();
   emit changed(m_changedProgram=false);
   }
}

void  WLGProgramWidget::showListProgram(int iCenter,bool center)
{
}

void  WLGProgramWidget::loadTextProgram()
{
qDebug()<<"loadTextProgram"<<m_GProgram->getElementCount();

QString buf;
bool backupPos=false;

ui.labelName->setText(tr("name: ")+m_GProgram->getName());
ui.labelName->setToolTip(m_GProgram->getNameFile());

if(m_lastNameProgram==m_GProgram->getNameFile()){
 backupPos=true;
 }

m_lastNameProgram=m_GProgram->getNameFile();

if(m_GProgram->getElementCount()<20000){
 int pos=ui.textProgram->textCursor().position();
 QString txt=m_GProgram->getTextProgram();

 ui.stackedWidget->setCurrentIndex(1);
 ui.textProgram->setEnabled(true);
 ui.textProgram->setPlainText(txt);

 if(backupPos) {
     QTextCursor cursor = ui.textProgram->textCursor();
     cursor.setPosition(pos);

     ui.textProgram->setTextCursor(cursor);
     }

 }
 else{
 ui.stackedWidget->setCurrentIndex(0);
 }

ui.sbPosition->setRange(0,m_GProgram->getElementCount());

m_changedProgram=false;
emit changed(m_changedProgram);
}

void WLGProgramWidget::showContextMenuListProgram(const QPoint &pos)
{
// Handle global position
QPoint globalPos = ui.listProgram->mapToGlobal(pos);

// Create menu and insert some actions
QMenu myMenu;
myMenu.addAction(tr("copy"), this, SLOT(copyTextListProgram()));

// Show context menu at handling position
myMenu.exec(globalPos);
}

void WLGProgramWidget::copyTextListProgram()
{
QClipboard *clipboard=QGuiApplication::clipboard();

clipboard->setText(m_ListModel->data(ui.listProgram->currentIndex(),Qt::DisplayRole).toString());
}


void WLGProgramWidget::updateTrack()
{
static long lastTrack=0;

if(m_trackElementF
 &&ui.textProgram->isReadOnly()
 &&lastTrack!=m_GProgram->getLastMovElement())
  {
  lastTrack=m_GProgram->getLastMovElement();

  setEditElement(lastTrack);

  if(isListProgram())
     {
     //iEditElement=lastTrack;
    // showListProgram(iEditElement,true);
      ui.listProgram->scrollTo(m_ListModel->index(iEditElement, 0),  QAbstractItemView::PositionAtCenter);
     }
  else
     {
     //setEditElement(lastTrack);
     ui.textProgram->centerCursor();
     }

  ui.sbPosition->setValue(lastTrack);
  }
}

void WLGProgramWidget::saveTextProgram()
{
m_GProgram->setTextProgram(ui.textProgram->toPlainText());
}

void WLGProgramWidget::onBackup()
{
//qDebug()<<"onBackup";
loadTextProgram();
emit changed(m_changedProgram=false);
}

void WLGProgramWidget::onReload()
{
m_GProgram->reloadFile(true);
}


void WLGProgramWidget::onChangedTextProgram()
{
qDebug()<<"onChangedTextProgram";
if(ui.textProgram->isEnabled()&&(m_changedProgram==false))
	{
	emit changed(m_changedProgram=true);
    }
}

void WLGProgramWidget::setEditDisabled(bool dis)
{
ui.pbAccept->setDisabled(dis ? true : !m_changedProgram);
ui.pbBackup->setDisabled(dis ? true : !m_changedProgram);

ui.pbReload->setDisabled(dis);
ui.pbClear->setDisabled(dis);

ui.textProgram->setReadOnly(dis);
}
	

void WLGProgramWidget::setEditElement(int iElement)
{
if(0<=iElement&&iElement<=m_GProgram->indexData.size())
{
iEditElement=iElement;

 if(!isListProgram()){

  if(m_changedProgram) return;

  int pos=0;

  QTextCursor cursor=ui.textProgram->textCursor();
  if(iElement==0){
    pos=0;
    }else{
    pos=m_GProgram->indexData[iElement-1].offsetInFile;
    }

  cursor.setPosition(pos);
  ui.textProgram->setTextCursor(cursor);
  ui.textProgram->centerCursor();
  }
 else {
  ui.listProgram->setCurrentIndex(m_ListModel->index(iElement, 0));
  ui.listProgram->scrollTo(m_ListModel->index(iEditElement, 0),  QAbstractItemView::PositionAtCenter);
  }
}
}

void WLGProgramWidget::onChangedPositionTextProgram()
{
QTextCursor cursor=ui.textProgram->textCursor();
int posF=cursor.position();

if(!m_changedProgram)
for(int i=0;i<m_GProgram->getElementCount();i++)
     {
     if(posF<=(m_GProgram->indexData[i].offsetInFile-1))
	    {        
        iEditElement=i;
        emit changedEditElement(iEditElement);
		break;
	    }
     }
}

void WLGProgramWidget::onChangedPositionListProgram(const QModelIndex & current, const QModelIndex & previous)
{
iEditElement=current.row();

emit changedEditElement(iEditElement);
}
