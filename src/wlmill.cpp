#include "wlmill.h"
#include <QTimer>
#include <QToolBox>
#include <QColor>
#include <QLineEdit>
#include <QTime>
#include <QString>

#include <QMessageBox>
#include <QTextCursor>
#include <QDockWidget>
#include <QHBoxLayout> 
#include <QVBoxLayout> 
#include <QDebug> 
#include <QDir>
#include <QXmlStreamReader> 

#include "wleditmpgwidget.h"
#include "wleditmillwidget.h"
#include "wleditgcode.h"
#include "wlpamwidget.h"
#include "wldrivewidget.h"
#include "wlenternum.h"

#include "wleditpoint.h"
#include "wlgdrivewidget.h"
#include "wldevicewidget.h"
#include "wltimerscript.h"
#include "wlgmodelwidget.h"
#include "wlconsolemodbus.h"
#include "wlconsolescript.h"
#include "wlspindlewidget.h"
#include "wlgsctablemodel.h"
#include "wlgtoolstablemodel.h"

#include <QJoysticks.h>

WLMill::WLMill(QWidget *parent)
: QMainWindow(parent)
{
    setEnabled(false);

    splash =  new QSplashScreen(QPixmap(":/data/icons/logoWLMill.png"));
    splash->showMessage("WLDEV 2007-2022");
    splash->setWindowFlags(splash->windowFlags()|Qt::WindowStaysOnTopHint);

    #ifndef QT_DEBUG
    splash->show();
    #endif

    QDir dir;

    dir.mkdir(_iconsGMPath);

    WLTButtonScript::setIconPath(_iconsGMPath);

    QTimer::singleShot(3000,splash,SLOT(close()));

    m_bclose=false;
    m_lifeM=0;

    QTimer *timerLifeM=new QTimer;
    connect(timerLifeM,SIGNAL(timeout()),SLOT(addLifeM()));
    timerLifeM->start(60*1000);

    MessManager = new WLMessManager(this);

    connect(this,SIGNAL(sendMessage(QString)),MessManager,SLOT(setMessage(QString)),Qt::QueuedConnection);

//    MessManager =new WLMessManager(this);
//    connect(this,SIGNAL(sendMessage(QString)),MessManager,SLOT(setMessage(QString)),Qt::QueuedConnection);

    Program = new WLGProgram(nullptr);
    connect(Program,SIGNAL(sendMessage(QString,QString,int)),MessManager,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);

//  connect(Program,SIGNAL(sendMessage(QString,QString,int)),MessManager,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);

    MScript=new WLEVScript();
    MScript->start();

    LScript=new WLEVScript();
    LScript->moveToThread(LScript);
    LScript->start();

    while(!MScript->isReady()
        ||!LScript->isReady()) QThread::msleep(50);

    LScript->addObject(new WLFileScript(LScript),"FILE");
    LScript->addObject(new WLTimerScript(LScript),"TIMER");

    MScript->addObject(new WLFileScript(MScript),"FILE");
    MScript->addObject(new WLTimerScript(MScript),"TIMER");
    MScript->addObject(this,"WLMILL");

    DialogM= new WLDialogScript(this);

    MScript->addObject(DialogM,"DIALOG");
    connect(MScript,&WLEVScript::reset,DialogM,&WLDialogScript::close);

    MillMachine = new WLGMachine(Program,MScript,LScript);

    connect(MillMachine,SIGNAL(sendMessage(QString,QString,int)),MessManager,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);

    Program->setShowGCode(MillMachine->getGCode());

//  connect(MillMachine,SIGNAL(sendMessage(QString,QString,int)),MessManager,SLOT(setMessage(QString,QString,int)),Qt::QueuedConnection);
    connect(MillMachine->getGCode(),SIGNAL(changedSC(int)),Program,SLOT(updateShowTraj()),Qt::DirectConnection);

    connect(MillMachine,&WLGMachine::changedReady,this,&WLMill::readyMachine);


    tabWidget = new QTabWidget(this);

    tabWidget->setFocusPolicy(Qt::NoFocus);

    VisualWidget=new WLVisualWidget(Program,MillMachine);

    tabWidget->addTab(VisualWidget,tr("Visual"));

    #ifdef DEF_CAMERA
    camera = new WLCamera(this);
    tabWidget->addTab(camera,tr("Camera"));
    #endif

    setCentralWidget(tabWidget);

    Log = new WLLog(this);

    connect(MessManager,SIGNAL(saveLog(QString,QString)),Log,SLOT(writeLog(QString,QString)));

    createDockProgram();	

    createTBar1();
    createTBar2();

    QTimer *autoSaveTimer = new QTimer;
    connect(autoSaveTimer,SIGNAL(timeout()),SLOT(saveConfigINI()));
    autoSaveTimer->start(5*60*1000);

    setWindowTitle("WLMill");

    setTabPosition(Qt::LeftDockWidgetArea,QTabWidget::East);
    setTabPosition(Qt::RightDockWidgetArea,QTabWidget::West);
    setTabPosition(Qt::TopDockWidgetArea,QTabWidget::South);
    setTabPosition(Qt::BottomDockWidgetArea,QTabWidget::North);

    MillMachine->start();

}

void WLMill::createTBar2()
{
WLTBarScript *tBar = new WLTBarScript(MScript,tr("toolbar 2"),this);

MScript->addObject(tBar,"TOOLBAR2");
MScript->addBeforeInitScript("TOOLBAR2.removeButtons();");

connect(tBar,&WLTBarScript::runScript,this,[=](QString txt){MScript->runScript(txt);});

tBar->setIconSize(QSize(48,48));

tBar->setObjectName("tb2");
addToolBar(tBar);
}

void WLMill::createTBar1()
{
WLTBarScript *tBar = new WLTBarScript(MScript,tr("toolbar 1"),this);

MScript->addObject(tBar,"TOOLBAR1");
MScript->addBeforeInitScript("TOOLBAR1.removeButtons();");

connect(tBar,&WLTBarScript::runScript,this,[=](QString txt){MScript->runScript(txt);});

tBar->setIconSize(QSize(48,48));

for(int i=3;i<10;i++)
 {
 QString str=QString::number(i);

 tBar->addLockButton("buttonM"+str,"M"+str,QString("MACHINE.runMCode(%1)").arg(str));

 tBar->getButton("buttonM"+str)->setIconFrom(":/data/icons/M"+str+".png");
 tBar->getButton("buttonM"+str)->setShortcut("F"+str);
 }

 tBar->getButton("buttonM6")->setShow(false);

for(int i=1;i<11;i++)
 {
 QString str=QString::number(i);
 tBar->addLockButton("buttonUserFunc"+str,"userFunc"+str);

 tBar->getButton("buttonUserFunc"+str)->setShow(false);
 }

connect(MillMachine,&WLGMachine::changedPossibleManual,tBar,&QToolBar::setEnabled);

tBar->setObjectName("tb1");
addToolBar(tBar);
}

void WLMill::createTBControl()
{
QAction *Action;
QToolBar *TBControl = new QToolBar(tr("tollbar Control"));
QFont font;

font=TBControl->font();
font.setPixelSize(24);

TBControl->setFont(font);

TBControl->setBaseSize(QSize(32,32));
TBControl->setIconSize(QSize(32,32));

Action=TBControl->addAction(QIcon(":/data/icons/play.png"),tr("start GCode (program)"),this,SLOT(onPBStart()));
connect(MillMachine,&WLGMachine::changedReadyRunList,Action,&QAction::setDisabled);

QMenu *menuStart = new QMenu();
menuStart->addAction((tr("start at...")),this,SLOT(onPBStartAt()));
menuStart->addAction((tr("continue...")),this,SLOT(onPBStartContinue()));
Action->setMenu(menuStart);

Action=TBControl->addAction(QIcon(":/data/icons/H.png"),tr("h probe"),MillMachine,[=](){
    MillMachine->runMScript("WLProbeToolTabletDialog()");
    });
connect(MillMachine,&WLGMachine::changedReadyRunList,Action,&QAction::setDisabled);

Action=TBControl->addAction(QIcon(":/data/icons/HT.png"),tr("h tool probe"),MillMachine,[=](){
    MillMachine->runMScript("WLProbeToolHDialog()");
    });

connect(MillMachine,&WLGMachine::changedReadyRunList,Action,&QAction::setDisabled);



TBControl->setObjectName("tb1Control");
addToolBar(TBControl);
}

void WLMill::onPBSetG28()
{
WLGPoint GP;
WLEditPoint EP;
QString nameDrive;

EP.setNameData("X,Y,Z,A,B,C");

QList <double> List;

GP=MillMachine->getGCode()->getG28Position();


EP.setLabel(tr("enter G28 position"));

EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    GP.fromString(EP.getValueStr());
    MillMachine->getGCode()->setG28Position(GP);
    }
    ///emit ChangedHomePosition(Program->GCode.getSC(iSC-1).toM()*EP.getFrame().toM());

}

void WLMill::onPBGetG28()
{
if(QMessageBox::question(this, tr("Question:"),tr("are you sure?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
                   MillMachine->getGCode()->setG28Position(MillMachine->getCurrentPosition(1));
}


void WLMill::onPBRotSC()
{
//if(disButton) return;

WLEnterNum  EnterNum (this);
EnterNum.setLabel("A=");
EnterNum.setSuffix(tr("gr"));

if(EnterNum.exec())
 {
 MillMachine->rotAboutRotPointSC(MillMachine->getGCode()->getActivSC(),EnterNum.getNow());
 }

}


void WLMill::onPBSetP0()
{
WLEditPoint EP;

WLGPoint  GP=MillMachine->getGCode()->getRefPoint0SC(MillMachine->getGCode()->getActivSC(),0);

EP.setNameData("X,Y,Z");
EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    qDebug("update rotPoint");
    GP.fromString(EP.getValueStr());
    MillMachine->getGCode()->setRefPoint0SC(MillMachine->getGCode()->getActivSC(),GP);
    }
};

void WLMill::onPBSetP1()
{
WLEditPoint EP;

WLGPoint  GP=MillMachine->getGCode()->getRefPoint1SC(MillMachine->getGCode()->getActivSC());

EP.setNameData("X,Y,Z");
EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    qDebug("update rotPoint");
    GP.fromString(EP.getValueStr());
    MillMachine->getGCode()->setRefPoint1SC(MillMachine->getGCode()->getActivSC(),GP);
}
}

void WLMill::onPBStart()
{
if(MillMachine->isPause())  MillMachine->setPause(false);
else
if(!MillMachine->isEmptyMotion())  MillMachine->startMov();
else
    {
    MillMachine->runGProgram();
    }
}

void WLMill::onPBStartAt()
{
if(MillMachine->isActiv()) return;

WLEnterNum EnterNum(this);
EnterNum.setMinMaxNow(0,Program->getElementCount(),Program->getActivElement());
EnterNum.setDecimals(0);
EnterNum.setLabel(tr("Which element to start processing?:"));
EnterNum.show();
if(EnterNum.exec())
 {
 MillMachine->runGProgram(EnterNum.getNow());
 }
}

void WLMill::onPBStartContinue()
{
if(MillMachine->isActiv()) return;

if(Program->getLastMovElement()!=0)
  {
  MillMachine->runGProgram(Program->getLastMovElement());
 }
}

void WLMill::onModbusConsole()
{
WLConsoleModbus console(this);

if(MillMachine->getMotionDevice()->getModuleDModbus())
    console.setModuleModbus(MillMachine->getMotionDevice()->getModuleDModbus());

console.setModal(true);
console.show();
console.exec();
}


void WLMill::createMenuBar()
{
MenuBar = new QMenuBar;

QMenu *MFile= new QMenu(tr("File")); 
MFile->addAction(tr("Save program"),Program,SLOT(saveFile()));
MFile->addAction(tr("Save program as"),this,SLOT(onSaveAsProgram()));
MFile->addSeparator();
MFile->addAction(tr("Load program"),this,SLOT(onLoadProgram()));
MFile->addSeparator();

MFile->addAction(tr("Save coordinates"),this,SLOT(onSaveSC()));
MFile->addAction(tr("Load coordinates"),this,SLOT(onLoadSC()));
MFile->addSeparator();

MFile->addAction(tr("Save Tools"),this,SLOT(onSaveTools()));
MFile->addAction(tr("Load Tools"),this,SLOT(onLoadTools()));
MFile->addSeparator();

MFile->addAction(tr("Close"),this,SLOT(close()));

MenuBar->addMenu(MFile);

QMenu *MEdit= new QMenu(tr("Edit")); 
MEdit->addAction(("WLMill"),this,SLOT(onEditWLMill()));
MEdit->addSeparator();
MEdit->addAction((tr("Drive")+" X"),this,SLOT(onEditDriveX()));
MEdit->addAction((tr("Drive")+" Y"),this,SLOT(onEditDriveY()));
MEdit->addAction((tr("Drive")+" Z"),this,SLOT(onEditDriveZ()));

if(MillMachine->getDrive("A"))
   MEdit->addAction((tr("Drive")+" A"),this,SLOT(onEditDriveA()));

if(MillMachine->getDrive("B"))
   MEdit->addAction((tr("Drive")+" B"),this,SLOT(onEditDriveB()));

MEdit->addSeparator();
MEdit->addAction(tr("MScript"),this,SLOT(onEditCodeMScript()));
MEdit->addAction(tr("LScript"),this,SLOT(onEditCodeLScript()));
MEdit->addSeparator();
//connect(MillMachine,&WLMillMachine::changedPossibleManual,Action,&QAction::setEnabled);

if(MillMachine->getMPG())
  MEdit->addAction("MPG",this,SLOT(onEditMPG()));

MEdit->addAction(tr("Device"),this,SLOT(onEditDevice()));
MEdit->addAction(("GModel"),this,SLOT(onEditGModel()));

QMenu *MView= new QMenu(tr("View"));
MView->addAction(tr("Colors"),this,SLOT(onSetColors()));
MView->addSeparator();
MView->addAction(tr("set  State 1"),this,SLOT(restoreState1()));
MView->addAction(tr("set  State 2"),this,SLOT(restoreState2()));
MView->addSeparator();
MView->addAction(tr("save State 1"),this,SLOT(saveState1()));
MView->addAction(tr("save State 2"),this,SLOT(saveState2()));
MenuBar->addMenu(MView);

QMenu *MOther= new QMenu(tr("Other"));

if(MillMachine->getMotionDevice()->getModuleDModbus())
     MOther->addAction(tr("Modbus term"),this,SLOT(onModbusConsole()));


//MEdit->addAction(("WLMotion"),this,SLOT(onL

connect(MillMachine,&WLGMachine::changedEnable,MEdit,&QMenu::setDisabled);

MenuBar->addMenu(MEdit);

if(!MOther->isEmpty())
   MenuBar->addMenu(MOther);

QMenu *MHelp= new QMenu(tr("Help"));
MHelp->addAction("WLMill.pdf",this,[=](){QDesktopServices::openUrl(QUrl("http://wldev.ru/data/doc/WLMill.pdf"));});
MHelp->addAction("WLMill-Script.pdf",this,[=](){QDesktopServices::openUrl(QUrl("http://wldev.ru/data/doc/WLMill-Script.pdf"));});
MHelp->addAction(tr("Device"),this,[=](){QDesktopServices::openUrl(QUrl("http://wldev.ru/data/doc/"+MillMachine->getMotionDevice()->getNameDevice()+".pdf"));});
MHelp->addAction(tr("Dir"),this,[=](){QDesktopServices::openUrl(configGMPath);});
MHelp->addSeparator();

MHelp->addAction(tr("save debug file"),this,SLOT(onSaveDebugFile()));///??
MHelp->addAction(tr("clear debug file"),WLLog::getInstance(),SLOT(clearDebug()));///??
MHelp->addSeparator();

MHelp->addAction(tr("about"),this,SLOT(about()));
MHelp->addAction(tr("Qt"),qApp,SLOT(aboutQt()));///??
MHelp->addSeparator();

MenuBar->addMenu(MHelp);

//connect(MillMachine,SIGNAL(ChangedEnableMoving(bool)),MenuBar,SLOT(setDisabled(bool)));

MenuBar->setObjectName("menubar");
setMenuBar(MenuBar);
}



void WLMill::createDockProgram()
{
dockProgram=new QDockWidget(this);

ProgramWidget = new WLGProgramWidget(Program,this);

//connect(ProgramWidget,SIGNAL(changed()),this,SLOT(setStarChangedProgram()));
//connect(ProgramWidget,&WLGProgramWidget::changed,this,&WLMill::sendProSLOT(sendProgramToShow()));

connect(ProgramWidget,&WLGProgramWidget::pressOpenFile,this,&WLMill::onLoadProgram);

connect(VisualWidget,&WLVisualWidget::changedEditElement,ProgramWidget,&WLGProgramWidget::setEditElement);

connect(ProgramWidget,&WLGProgramWidget::changedEditElement,VisualWidget,&WLVisualWidget::setEditElement);
connect(MillMachine,&WLGMachine::changedReadyRunList,ProgramWidget,&WLGProgramWidget::setEditDisabled);

dockProgram->setWindowTitle(tr("Program"));
dockProgram->setWidget(ProgramWidget);

dockProgram->setObjectName("DProgram");
dockProgram->setFeatures(QDockWidget::DockWidgetFloatable
                        |QDockWidget::DockWidgetMovable
                        |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::RightDockWidgetArea,dockProgram);
}

void WLMill::createDockPosition()
{   
dockPosition=new QDockWidget(this);

dockPosition->setWindowTitle(tr("Positions"));
dockPosition->setObjectName("DPosition");

PositionWidget=new WLPositionWidget(MillMachine,Program,dockPosition);

PositionWidget->addTopWidget(MessManager);

dockPosition->setWidget(PositionWidget);
dockPosition->setFeatures(QDockWidget::DockWidgetFloatable
                         |QDockWidget::DockWidgetMovable
                         |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::RightDockWidgetArea,dockPosition);
}

void WLMill::createTabTools()
{
ToolWidget = new WLDataWidget(this);

WLGToolsTableModel *toolsModel=new WLGToolsTableModel(MillMachine->getGCode(),ToolWidget);

connect(Program,&WLGProgram::changedToolList,toolsModel,&WLGDataTableModel::setSelectList);

ToolWidget ->setModel(toolsModel);

WLTBarTool *tBarTools = new WLTBarTool(MScript,this);

MScript->addObject(tBarTools,"TOOLBARTOOLS");
MScript->addBeforeInitScript("TOOLBARTOOLS.removeButtons();");

connect(tBarTools,&WLTBarScript::runScript,this,[=](QString txt){MScript->runScript(txt);});
tBarTools->setIconSize(QSize(48,48));

connect(MillMachine,&WLGMachine::changedPossibleManual,tBarTools,&QToolBar::setEnabled);

tBarTools->setObjectName("tbtool");

ToolWidget->addToolBar(tBarTools);

ToolWidget->show();

tabWidget->addTab(ToolWidget,"Tools");
}

void WLMill::createTabSC()
{
SCWidget = new WLDataWidget(this);

WLGSCTableModel *scModel=new WLGSCTableModel(MillMachine->getGCode(),SCWidget);

connect(Program,&WLGProgram::changedSCList,scModel,&WLGDataTableModel::setSelectList);

SCWidget ->setModel(scModel);

WLTBarData *tBarSC = new WLTBarData(MScript,this);

MScript->addObject(tBarSC,"TOOLBARSC");
MScript->addBeforeInitScript("TOOLBARSC.removeButtons();");

connect(tBarSC,&WLTBarScript::runScript,this,[=](QString txt){MScript->runScript(txt);});
tBarSC->setIconSize(QSize(48,48));

connect(MillMachine,&WLGMachine::changedPossibleManual,tBarSC,&QToolBar::setEnabled);

tBarSC->setObjectName("tbsc");

SCWidget->addToolBar(tBarSC);

SCWidget->show();

tabWidget->addTab(SCWidget,"SC");
}

void WLMill::createDockIOPut()
{
qDebug()<<"createDockIOPut";

dockIOPut=new QDockWidget(this);

dockIOPut->setWindowTitle("In/Out");
dockIOPut->setObjectName("DIOPut");

IOWidget =new WLIOWidget();

IOWidget->setDevice(MillMachine->getMotionDevice());

IOWidget->Init();

dockIOPut->setWidget(IOWidget);
dockIOPut->setFeatures(QDockWidget::DockWidgetFloatable
                      |QDockWidget::DockWidgetMovable
                      |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::LeftDockWidgetArea,dockIOPut);
}

void WLMill::createDockHeightMap()
{
qDebug()<<"createDockHeightMap";

dockHeightMap=new QDockWidget(this);

dockHeightMap->setWindowTitle("Height Map");
dockHeightMap->setObjectName("DHeightMap");

HMWidget =new WLHeightMapWidget(MillMachine->getHeightMap(),this);

//IOWidget->setDevice(MillMachine->getMotionDevice());
//IOWidget->Init();

dockHeightMap->setWidget(HMWidget);
dockHeightMap->setFeatures(QDockWidget::DockWidgetFloatable
                          |QDockWidget::DockWidgetMovable
                          |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::RightDockWidgetArea,dockHeightMap);
}

void WLMill::updateEnableMoved(bool en)
{
Q_UNUSED(en)
//TextProgram->setReadOnly(en);
}

void WLMill::createDockConsoleMScript()
{
qDebug()<<"createDockConsoleMScript";

dockConsoleMScript=new QDockWidget(this);

dockConsoleMScript->setWindowTitle("Console MScript");
dockConsoleMScript->setObjectName("consoleMScriptDock");

WLConsoleScript *cScript =new WLConsoleScript(this);

connect(MScript,&WLEVScript::sendConsole,cScript,&WLConsoleScript::addText);

dockConsoleMScript->setWidget(cScript);
dockConsoleMScript->setFeatures(QDockWidget::DockWidgetFloatable
                    |QDockWidget::DockWidgetMovable
                    |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::LeftDockWidgetArea,dockConsoleMScript);

dockConsoleMScript->close();
}

void WLMill::createDockConsoleLScript()
{
qDebug()<<"createDockConsoleLScript";

dockConsoleLScript=new QDockWidget(this);

dockConsoleLScript->setWindowTitle("Console LScript");
dockConsoleLScript->setObjectName("consoleLScriptDock");

WLConsoleScript *cScript =new WLConsoleScript(this);

connect(LScript,&WLEVScript::sendConsole,cScript,&WLConsoleScript::addText);

dockConsoleLScript->setWidget(cScript);
dockConsoleLScript->setFeatures(QDockWidget::DockWidgetFloatable
                               |QDockWidget::DockWidgetMovable
                               |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::LeftDockWidgetArea,dockConsoleLScript);

dockConsoleLScript->close();
}

void WLMill::createDockMPG()
{
if(!MillMachine->getMPG()) return;

qDebug()<<"createDockMPG";

dockMPG=new QDockWidget(this);

dockMPG->setWindowTitle("MPG");
dockMPG->setObjectName("MPGDock");

MPGWidget =new WLMPGWidget(MillMachine->getMPG(),this);

dockMPG->setWidget(MPGWidget);
dockMPG->setFeatures(QDockWidget::DockWidgetFloatable
                    |QDockWidget::DockWidgetMovable
                    |QDockWidget::DockWidgetClosable);

addDockWidget(Qt::LeftDockWidgetArea,dockMPG);
}

void WLMill::setStateButtonGo(bool state)
{
qDebug()<<"setStateButtonGo"<<state;
TButtonGo->setDown(!state);
}

void WLMill::Error(QString name,QString data)
{
QString Pstr;
QTextStream str(&Pstr);
str<<name<<":"<<data;
QMessageBox::information(this, tr("Stop error"),Pstr,QMessageBox::Ok);
}


WLMill::~WLMill()
{
qDebug("WLMill::~WLMill()");
MillMachine->setEnable(false);

while(MScript->isBusy()
    ||LScript->isBusy()) {
QCoreApplication::processEvents();
}

LScript->quit();
LScript->wait();

MScript->quit();
MScript->wait();

if(MillMachine->isReady())
          saveConfigINI();

delete VisualWidget;
delete Program;
delete MillMachine;

qDebug("WLMill END");
}

void WLMill::onEditDrive(QString nameDrive)
{
if(MillMachine->getDrive(nameDrive))
  {
  WLDriveWidget AW(MillMachine->getDrive(nameDrive));
  WLGDriveWidget MDW(MillMachine->getDrive(nameDrive));

  AW.setFminutes(true);
  AW.addTabWidget(&MDW,tr("other")); 

  MDW.setUnit(AW.getUnit());
  connect(&AW,SIGNAL(changedUnit(QString)),&MDW,SLOT(setUnit(QString)));

  AW.show();

  if(AW.exec())
    {
    AW.saveDataDrive();
    MillMachine->updateMainDimXYZ();
    MillMachine->updateGModel();
    VisualWidget->updateViewGModel();
    }

  MillMachine->saveConfig();
  }
}

void WLMill::onEditWLMill()
{
WLEditMillWidget EditMill(MillMachine);
WLSpindleWidget SW(MillMachine->getMotionDevice()->getModuleSpindle()->getSpindle(0));
WLEditGCode EditGCode(MillMachine->getGCode());

EditMill.addTabWidget(&SW,"Spindle");
EditMill.addTabWidget(&EditGCode,"GCode");

EditMill.show();

if(EditMill.exec())
   {
   if(EditMill.getNeedClose())
    {
    QMessageBox::information(this,tr("Attention"),tr("Please restart WLMill"));
    close();
    }
}
}


void WLMill::onSaveSC()
{
if(lastFileSC.isEmpty()) lastFileSC=QCoreApplication::applicationDirPath();

QString fileName = QFileDialog::getSaveFileName(this, tr("Save coordinates"),lastFileSC,"CS (*.csv);;sc (*.scxml)");

if(!fileName.isEmpty()){
QFileInfo fi(fileName);

if(fi.suffix()==("scxml"))
  {
  QFile FileXML(lastFileSC=fileName);
  if(FileXML.open(QIODevice::WriteOnly))
  {
  QXmlStreamWriter stream(&FileXML);

  stream.setAutoFormatting(true);

  stream.setCodec(QTextCodec::codecForName("Windows-1251"));
  stream.writeStartDocument("1.0");

  stream.writeStartElement("WLMill-SC");

   for(int i=0;i<sizeSC;i++)
   {
   stream.writeStartElement("SC");
   stream.writeAttribute("i",QString::number(i));

   stream.writeAttribute("GPoint",MillMachine->getGCode()->getOffsetSC(i).toString());
   stream.writeAttribute("refPoint0",MillMachine->getGCode()->getRefPoint0SC(i).toString());
   stream.writeAttribute("refPoint1",MillMachine->getGCode()->getRefPoint1SC(i).toString());
   stream.writeEndElement();
   }

   stream.writeStartElement("author");
   stream.writeAttribute("Name","BocharovSergey");
   stream.writeAttribute("e-mail","bs_info@mail.ru");
   stream.writeAttribute("phone","+79139025883");
   stream.writeEndElement();

  stream.writeEndElement();

  stream.writeEndDocument();

  FileXML.close();
  }
 }
 else if(fi.suffix()==("csv")){
  MillMachine->getGCode()->writeSCFile(fileName);
 }
 }
}

void WLMill::onEditCodeMScript()
{
WLEditText EditText;
WLMCodeSH  codeSH(EditText.getDocument());

EditText.setText(MScript->getCode());
EditText.setLabel(tr("Edit")+"MScript:");

EditText.show();

if(EditText.exec())  {
  MScript->setBaseCode(EditText.getText());
  MillMachine->saveMScript(EditText.getText());
  }
}

void WLMill::onEditCodeLScript()
{
WLEditText EditText;
WLMCodeSH  codeSH(EditText.getDocument());

EditText.setText(LScript->getCode());
EditText.setLabel(tr("Edit")+"LScript:");

EditText.show();

if(EditText.exec()) {
  LScript->setBaseCode(EditText.getText());
  MillMachine->saveLScript(EditText.getText());
  }
}

void WLMill::onLoadSC()
{
if(MillMachine->isActiv())  return;

QString fileName = QFileDialog::getOpenFileName(this, tr("Download coordinates"),lastFileSC,("CS (*.csv);;sc (*.scxml)"));

if(!fileName.isEmpty())
{
QFileInfo fi(fileName);

if(fi.suffix()==("scxml")){
    QFile FileXML(lastFileSC=fileName);

    QXmlStreamReader stream;

    qDebug()<<"load ConfigXML";
    //bool ret=0;

    if(FileXML.open(QIODevice::ReadOnly))
      {
     qDebug()<<"open ConfigXML";
      stream.setDevice(&FileXML);

      while(!stream.atEnd())
       {
       if(stream.readNext()==QXmlStreamReader::StartElement)
       if(stream.name()=="WLMill-SC")
        {
        //Program->loadFile(stream.attributes().value("LastProgram").toString());

         while(!stream.atEnd())
         {
            stream.readNextStartElement();
            qDebug()<<"findElement "<<stream.name()<<" "<<stream.tokenString();
            if(stream.name()=="WLMill-SC") break;
            if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

            if(stream.name()=="SC")
             {
             qDebug()<<"loadSC";

             int i=0;
             i=stream.attributes().value("i").toString().toInt();

             WLGPoint Gp;

             Gp.fromString(stream.attributes().value("GPoint").toString());
             MillMachine->getGCode()->setOffsetSC(i,Gp);

             Gp.fromString(stream.attributes().value("refPoint0").toString());
             MillMachine->getGCode()->setRefPoint0SC(i,Gp);

             Gp.fromString(stream.attributes().value("refPoint1").toString());
             MillMachine->getGCode()->setRefPoint1SC(i,Gp);
             continue;
             }
         }
        }
       }
      }
    }
    else if (fi.suffix()==("csv")) {
    MillMachine->getGCode()->readSCFile(fileName);
    }
}

MillMachine->saveConfig();
}

void WLMill::onSaveTools()
{
if(MillMachine->isActiv())  return;

QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tools"),toolsFile,("Tools (*.csv);;all type(*.*)"));

if(!fileName.isEmpty())
  MillMachine->getGCode()->writeToolFile(fileName);

}

void WLMill::onLoadTools()
{
if(MillMachine->isActiv())  return;

QString fileName = QFileDialog::getOpenFileName(this, tr("Load Tools"),toolsFile,("Tools (*.csv);;all type(*.*)"));

if(!fileName.isEmpty())
    MillMachine->getGCode()->readToolFile(fileName);
}

void WLMill::onSaveDebugFile()
{
QString dir=QFileDialog::getExistingDirectory(this, tr("Save File Device"),WLLog::getDirDebug());
WLEnterString EnterString(this);

EnterString.setLabel(tr("Enter comment:"));

if(!dir.isEmpty()){

EnterString.show();
EnterString.exec();

WLLog::saveDebugFile(dir,"title:"+windowTitle(),EnterString.getString());
}
}

void WLMill::onLoadProgram()
{
if(MillMachine->isActiv())  return;

QString fileName = QFileDialog::getOpenFileName(this, tr("Load program"),Program->getNameFile(),("g-code (*.ncc *.nc *.tap);;all type(*.*)"));

if(!fileName.isEmpty())
      {
      if(Program->loadFile(fileName,true)) {
                Program->setLastMovElement(0);
                }

      }
//updateTitleDockProgram(1);
}

void WLMill::onSaveAsProgram()
{
QString fileName = QFileDialog::getSaveFileName(this, tr("Save program as"),Program->getNameFile(),("g-code (*.ncc *.nc);;all type(*.*)"));
if(!fileName.isEmpty())
     Program->saveFile(fileName); 
//updateTitleDockProgram(1);
}

void WLMill::UpdateTimes()
{
	TimeNow=QTime::currentTime();
	//ui.label_Time->setText(TimeNow.toString("hh:mm"));
    TimeEnd=TimeNow;
}


/*
void WLMill::moveCursorTextProgram()
{
//QStringList ListKadr;
int pos;

QTextCursor TextCursor=TextProgram->textCursor();
pos=TextCursor.position();
//qDebug("blo=%d",pos);
if(pos>=0) 
    {
	qDebug()<<"Pos="<<pos;
    emit ChangedEditElementPos(pos);	
	}
}
*/
/*
void WLMill::setCursorToEditElement(int l)
{
qDebug()<<"set edit line"<<l<<Program->getElementTraj(l).ipos
	<<Program->getElementTraj(l).iline
	<<Program->getElementTraj(Program->getElementTraj(l).iline).ipos;
/*
//pos-=size;*/
/*
if(!fEditProgram)
{
TextProgram->setFocus();
QTextCursor TextCursor=TextProgram->textCursor();

TextCursor.setPosition(Program->getElementTraj(l).ipos);
//qDebug()<<"setPosET"<<Program->getElementTraj(l).ipos;
TextCursor.select(QTextCursor::LineUnderCursor);

TextProgram->blockSignals(true);
TextProgram->setTextCursor(TextCursor);
TextProgram->blockSignals(false);
}
*/
/*
QStringList ListKadr;

int pos,size;
qDebug("set edit line=%d",l);

ListKadr=TextProgram->toPlainText().split("\n");
pos=0;
for(int i=0;i<ListKadr.size();i++)
  {
  if(i==l)
   {
   //pos-=size;
   TextProgram->setFocus();
   QTextCursor TextCursor=TextProgram->textCursor();
   
   TextCursor.setPosition(pos);

   TextCursor.select(QTextCursor::LineUnderCursor);

   TextProgram->blockSignals(true);
   TextProgram->setTextCursor(TextCursor);
   TextProgram->blockSignals(false);
   break;
   }
  pos+=size=ListKadr[i].size()+1;
  }

}
*/

void WLMill::closeEvent (QCloseEvent * event)
{   
switch(QMessageBox::question(this, tr("Confirmation:"),
       tr("Do you really want to exit the program?"), 
	   QMessageBox::Yes|QMessageBox::No))
	    {
		
        case QMessageBox::Yes:     MillMachine->setEnable(false);

                                   saveDataState();
                                   qDebug()<<"WLMill close";
			                       break;

        default:                   event->ignore();
			                       return;
		}

}


void WLMill::leaveEvent ( QEvent * event )
{
Q_UNUSED(event)
    //Machine->Pause(1);
}

#ifdef DEF_QML
QQuickWidget *WLMill::createQuickWidget(QString file)
{
QQuickWidget *view  = new QQuickWidget(this);

view->setSource(QUrl::fromLocalFile(file));

if (view->status() == QQuickWidget::Error)
 {
 MessManager->setMessage("WLMill","Error file QML:"+file,1);
 qDebug()<<"Error createQuickWidget:"<<file;

 view->deleteLater();
 return nullptr;
 }

QQmlContext *context = view->engine()->rootContext();

context->setContextProperty("view", view);
context->setContextProperty("MSCRIPT", MScript);
context->setContextProperty("MACHINE", MillMachine);
context->setContextProperty("FILE",new WLFile(view));

return view;
}
#endif

void WLMill::runQML(QString file)
{
runQMLFile(_qmlPath+file);
}

void WLMill::runQMLFile(QString file)
{
#ifdef DEF_QML
QQuickWidget *view  = createQuickWidget(file);

if(view)
 {
 view->setWindowFlag(Qt::WindowType::Dialog);
 view->setResizeMode(QQuickWidget::SizeRootObjectToView);
 view->setWindowModality(Qt::ApplicationModal);

 view->show(); 

 connect(view,&QWidget::close,view,&QWidget::deleteLater);
 }
#else
qDebug()<<"no runQMLFile"<<file;
#endif
}

void WLMill::addTabQML(QString file)
{
addTabQMLFile(_qmlPath+file);
}

void WLMill::addTabQMLFile(QString file)
{
#ifdef DEF_QML
QFileInfo FI(file);

for (int i=0;i<tabWidget->count();i++) {
if(tabWidget->tabText(i)==FI.baseName())
     return;
}

QQuickWidget *view  = createQuickWidget(file);

if(view) { 
 view->setResizeMode(QQuickWidget::SizeRootObjectToView);
 view->setWindowModality(Qt::ApplicationModal);

 view->setFocusPolicy(Qt::WheelFocus);

 tabWidget->addTab(view,FI.baseName());
 }
#else
qDebug()<<"no addQMLFile"<<file;
#endif
}

void WLMill::readyMachine()
{	
qDebug()<<"WLMill::readyMachine() <<<";

createMenuBar();

createTabTools();
createTabSC();

createDockIOPut();
createDockPosition();
createDockMPG();
createDockHeightMap();

createDockConsoleMScript();
createDockConsoleLScript();

if(QFileInfo(iniconfigWLMill).exists()) {
   loadConfigINI();
   }else{
   loadConfigXML();
   }

loadDataState();

connect(MillMachine->getMotionDevice(),SIGNAL(changedVersion(quint32)),SLOT(updateTitle()));
connect(this,SIGNAL(changedLife()),SLOT(updateTitle()));

updateTitle();

if(MillMachine->getMotionDevice()->getUID96().isEmpty())
    QTimer::singleShot(1000,this,SLOT(onEditDevice()));

setEnabled(true);

//MScript->initCode();
//LScript->initCode();
VisualWidget->updateViewGModel();

#ifdef DEF_PLOT
if(MillMachine->getMotionDevice()->getModule(WLModule::typeMOscp)){
 oscp = new WLOscpWidget(MillMachine->getMotionDevice()->getModuleOscp(),this);
 tabWidget->addTab(oscp,tr("Osciloscope"));
 }
#endif

qDebug()<<"WLMill::readyMachine() >>>";
}

void WLMill::onGoHome()
{
qDebug()<<"onGOHome";
MillMachine->runGCode("G28 Z0");
MillMachine->runGCode("G28 X0 Y0");
MillMachine->runGCode("G28 A0 B0 C0");
}

void WLMill::restoreState1()
{
restoreState(m_state1);
}

void WLMill::restoreState2()
{
restoreState(m_state2);
}

void WLMill::saveState1()
{
m_state1=saveState();
}

void WLMill::saveState2()
{
m_state2=saveState();
}

void WLMill::onEditDevice()
{
WLDeviceInfo DevInfo;

if(MillMachine->getMotionDevice()->isConnect())
   DevInfo=MillMachine->getMotionDevice()->getInfo();

WLDeviceWidget DW(DevInfo,this);

DW.show();

if(DW.exec())
 {
 qDebug()<<"changed Device";
 DevInfo=DW.getDeviceInfo();

 if(!DevInfo.name.isEmpty())
  {
  MillMachine->getMotionDevice()->setInfo(DevInfo);
  QMessageBox::information(this,tr("Attention"),tr("Please restart WLMill"));
  close();
  }
 }
}

void WLMill::onEditGModel()
{
WLGModelWidget GMW(MillMachine->getGModel(),this);

GMW.show();

if(GMW.exec())
 {
 MillMachine->updateGModel();
 VisualWidget->updateViewGModel();
 qDebug()<<"updateGModel";
 }
}

void WLMill::onEditMPG()
{
if(!MillMachine->getMotionDevice()->getModuleMPG()) return;

WLEditMPGWidget EditMPG(MillMachine->getMotionDevice()->getModuleMPG()->getMPG(0)
                         ,MillMachine->getMotionDevice()->getModuleIOPut()
                         ,MillMachine->getMotionDevice()->getModuleEncoder()
                         ,this);

EditMPG.show();

if(EditMPG.exec())
  {
  EditMPG.saveData();

  if(MPGWidget) QTimer::singleShot(200,MPGWidget,SLOT(update()));
  }
}

void WLMill::onSetColors()
{
QColorDialog Dialog(this);
QColor lastColor=VisualWidget->getClearColor();

Dialog.setCurrentColor(lastColor);
Dialog.setModal(true);
Dialog.show();

connect(&Dialog,SIGNAL(currentColorChanged(QColor)),VisualWidget,SLOT(setClearColor(QColor)));

if(Dialog.exec()) {
 VisualWidget->setClearColor(Dialog.currentColor());
 }else {
 VisualWidget->setClearColor(lastColor);
 }

}

void WLMill::loadDataState()
{
QSettings settings(configGMPath+"state",QSettings::IniFormat);
settings.setIniCodec("UTF-8");
restoreGeometry(settings.value("geometry").toByteArray());
restoreState(settings.value("windowState").toByteArray());
m_state1=settings.value("state1").toByteArray();
m_state2=settings.value("state2").toByteArray();
}

bool WLMill::loadConfigINI()
{
QSettings setting(iniconfigWLMill,QSettings::IniFormat);

setLifeM(setting.value("minuteLife",0).toUInt());

Program->setMaxShowPoints(setting.value("View/maxShowPoint",250000).toInt());
VisualWidget->setZoomDir(setting.value("View/zoomDir",0).toBool());

QColor color;
color.setNamedColor(setting.value("View/clearColor",VisualWidget->getClearColor().name()).toString());
VisualWidget->setClearColor(color);

Program->setLastMovElement(setting.value("Program/iLastElement",0).toUInt());
Program->loadFile(setting.value("Program/file","").toString(),true);

ToolWidget->setHeadersTable(setting.value("Tools/showColumn","").toString());
SCWidget->setHeadersTable(setting.value("SC/showColumn","").toString());
//lastProg=(m_setting.value("view/zoomDir",0).toBool());

return true;
}

void WLMill::saveConfigINI()
{
QSettings setting(iniconfigWLMill,QSettings::IniFormat);

setting.setValue("minuteLife",getLifeM());

setting.setValue("View/maxShowPoint",Program->maxShowPoints());
setting.setValue("View/zoomDir",VisualWidget->zoomDir());
setting.setValue("View/clearColor",VisualWidget->getClearColor().name());

setting.setValue("Program/file",Program->getNameFile());
setting.setValue("Program/iLastElement",Program->getLastMovElement());

setting.setValue("Tools/showColumn",ToolWidget->getHeaderTable().join(","));
setting.setValue("SC/showColumn",SCWidget->getHeaderTable().join(","));
}

void WLMill::saveDataState()
{
qDebug()<<"WLMill::saveDataState()";
if(!MillMachine->isReady()) return;

QSettings settings(configGMPath+"state",QSettings::IniFormat);
settings.setIniCodec("UTF-8");
settings.setValue("geometry", saveGeometry());
settings.setValue("windowState", saveState());
settings.setValue("state1",m_state1);
settings.setValue("state2",m_state2);
}

void WLMill::updateTitle()
{
QString title;
quint32 verWLMill=VERSION_YEAR*10000+VERSION_MONTH*100+VERSION_DAY;

title=QString("WLMill %1h ver=%2").arg((float)getLifeM()/60.0,0,'f',1).arg(verWLMill);

#ifdef enableDemo
 title+=QString(" (demo)");
#endif

WLMotion *MD=MillMachine->getMotionDevice();

 if(MillMachine
  &&MD)
 {
 if(MD->isReady())
  title+=" / "+MD->getNameDevice()+" ver="+QString::number(MD->getVersion());
else
  title+=QString(" /noDevice ("+MD->getUID96()+")");
 }

setWindowTitle(title);
}




void WLMill::showMessage(QString name,QString data,int code)
{
QString Pstr;
QTextStream str(&Pstr);
str<<name<<": "<<data<<tr(", code=")<<code;

//if(Pstr!=lastError)
    {
	if(code!=0)
		QMessageBox::information(this, tr("Stop due to an error:"),Pstr,QMessageBox::Ok);
	else
		QMessageBox::information(this, tr("Stop: "),Pstr,QMessageBox::Ok);
    }

}

/*

void WLMill::saveConfigXML()
{
QFile FileXML(configWLMill);

if(!MillMachine->isReady()) return;

if(FileXML.open(QIODevice::WriteOnly))
{
QXmlStreamWriter stream(&FileXML);

stream.setAutoFormatting(true);

stream.setCodec(QTextCodec::codecForName("Windows-1251"));
stream.writeStartDocument("1.0");

stream.writeStartElement("WhiteLineMillConfig");

 stream.writeAttribute("maxShowPoints",QString::number(Program->maxShowPoints()));
 stream.writeAttribute("zoomDir",QString::number(VisualWidget->zoomDir()));
 stream.writeAttribute("LastProgram",Program->getNameFile());
 stream.writeAttribute("LastElementProgram",QString::number(Program->getLastMovElement()));
 stream.writeAttribute("LastSC",lastFileSC);


 stream.writeStartElement("Colors");
 stream.writeAttribute("clear",VisualWidget->getClearColor().name());
 stream.writeEndElement();

 stream.writeStartElement("Operating"); 
 stream.writeAttribute("Minute",QString::number(getLifeM(),'g',36));	 
 stream.writeEndElement();


 stream.writeStartElement("author");
 stream.writeAttribute("Name","BocharovSergey");
 stream.writeAttribute("e-mail","wldev@mail.ru");
 stream.writeAttribute("phone","+79139025883");
 stream.writeEndElement();

stream.writeEndElement();

stream.writeEndDocument();
qDebug()<<"Write XML";

 FileXML.close();
}
else
qDebug()<<"no Write XML";
}
*/
bool WLMill::loadConfigXML()
{
QFile FileXML(configWLMill);
QXmlStreamReader stream;
qDebug()<<"load ConfigXML";
bool ret=0;

QString lastProg;

if(FileXML.open(QIODevice::ReadOnly))
  {
  qDebug()<<"open ConfigXML";
  stream.setDevice(&FileXML);

  while(!stream.atEnd())
   {
   if(stream.readNext()==QXmlStreamReader::StartElement)
   if(stream.name()=="WhiteLineMillConfig")
    {
    qDebug()<<"read ConfigXML";
	ret=1;
	
    if(!stream.attributes().value("maxShowPoints").isEmpty()) {
     Program->setMaxShowPoints(stream.attributes().value("maxShowPoints").toLongLong());
     }

    if(!stream.attributes().value("zoomDir").isEmpty()) {
     VisualWidget->setZoomDir(stream.attributes().value("zoomDir").toInt());
     }

	lastProg=stream.attributes().value("LastProgram").toString();
	Program->setLastMovElement(stream.attributes().value("LastElementProgram").toString().toLong());

	lastFileSC=(stream.attributes().value("LastSC").toString());

	 while(!stream.atEnd())
	 { 
       stream.readNextStartElement();
		qDebug()<<"findElement "<<stream.name()<<" "<<stream.tokenString();
	    if(stream.name()=="WhiteLineMillConfig") break;
		if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

		if(stream.name()=="Operating")
		  {
          setLifeM(stream.attributes().value("Minute").toString().toULong());
          continue;
          }

        if(stream.name()=="Colors")
          {
          QColor color;
          if(!stream.attributes().value("clear").isEmpty())
               {
               color.setNamedColor(stream.attributes().value("clear").toString());
               VisualWidget->setClearColor(color);
               }
          continue;
          }

		if(stream.name()=="Name")
		  {
		  Program->loadFile(stream.readElementText(),true);
		  continue;
	      }

	 }
	
    }
   }

  Program->loadFile(lastProg,true);
  }
  else
    ret=0;


return ret;
}


void WLMill::about()
{
QMessageBox::about(this,("WLMill"),
		   tr(" WLMill <br>"
  		      " http://wldev.ru <br>"
			  " wldev@mail.ru <br>"
              "<br>WhiteLine</b> <br>"
#ifdef enableExportKuka
			  "buyer:"buyer
#endif
			  ));

}


void WLMill::help()
{
    about();
}

void WLMill::placeVisualizer()
{
//if(MillControl)    MillControl->move(2,0);
//if(PositionWidget) PositionWidget->move(VisualWidget->width()-PositionWidget->width()-2,0);
}

void WLMill::resizeEvent(QResizeEvent *event)
{
Q_UNUSED(event);
    placeVisualizer();
}

void WLMill::keyPressEvent(QKeyEvent *event)
{
qDebug()<<"WLMill::keyPressEvent"<<event->text()<<event->key();

if(event->modifiers()==Qt::ControlModifier)
    emit changedStateKeyControl(event->key(),true);
}

void WLMill::keyReleaseEvent(QKeyEvent *event)
{
qDebug()<<"WLMill::keyReleaseEvent"<<event->text();

if(event->modifiers()==Qt::ControlModifier)
    emit changedStateKeyControl(event->key(),false);
}

void WLMill::showEvent(QShowEvent *event)
{
Q_UNUSED(event);
    placeVisualizer();
}


