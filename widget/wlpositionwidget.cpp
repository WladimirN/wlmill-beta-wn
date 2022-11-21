#include "wlpositionwidget.h"
#include "wlpidwidget.h"

void movDialogToDesktop(QWidget *widget,QWidget *dialog)
{
QPoint pos=widget->mapToGlobal(QPoint());

pos.setY(pos.y()+widget->height());

dialog->move(pos);

QDesktopWidget *desktopWidget = QApplication::desktop();

QRect screenRect = desktopWidget->geometry();

if(dialog->geometry().right()>screenRect.width())
       {
       pos.setX(pos.x()-(dialog->geometry().right()-screenRect.width()));
       dialog->move(pos);
       }else if(dialog->geometry().left()<0)
               {
               pos.setX(0);
               dialog->move(pos);
               }

if(dialog->geometry().bottom()>screenRect.height())
        {
        pos.setY(pos.y()-(dialog->height()+widget->height()));
        dialog->move(pos);
        }else if(dialog->geometry().top()<0)
                {
                pos.setY(0);
                dialog->move(pos);
                }
}

WLPositionWidget::WLPositionWidget(WLGMachine *_MillMachine,WLGProgram *_Program,QWidget *parent)
    : QWidget(parent)
{
    MillMachine=_MillMachine;
    Program=_Program;

	disButton=false;

	ui.setupUi(this);

    ui.cbExGCode->installEventFilter(this);
    this->installEventFilter(this);

	QTimer *timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(updatePosition()));
    timer->start(100);

    QTimer *timerProgres = new QTimer;
    connect(timerProgres,SIGNAL(timeout()),this,SLOT(updateProgress()));
    timerProgres->start(1000);

    QTimer *timerOnButton = new QTimer;
    connect(timerOnButton,SIGNAL(timeout()),this,SLOT(updateFlashButtons()));
    timerOnButton->start(500);

    setJogDistStr("JOG,5.00,1.00,0.10,0.01");

    m_buttonSize=QSize(32,32);

    connect(ui.pbOnMachine,SIGNAL(toggled(bool)),MillMachine,SLOT(setEnable(bool)));

	ui.pbRotSC->setPopupMode(QToolButton::DelayedPopup);
    connect(ui.pbRotSC,SIGNAL(clicked()),SLOT(onPBRotSC()));

	QMenu *menuPBRot = new QMenu();
	menuPBRot->addAction((tr("set base postion")),this,SLOT(onPBsetP0()));
	menuPBRot->addAction((tr("set verify postion")),this,SLOT(onPBsetP1()));
    menuPBRot->addAction((tr("rotation correction")),this,SLOT(onPBRotSCRef()));
    ui.pbRotSC->setMenu(menuPBRot);	

    connect(ui.cbExGCode->lineEdit(),&QLineEdit::returnPressed,this,&WLPositionWidget::onExGCode);

    connect(ui.pbFindDrivePos,&QToolButton::clicked,[=]()
    {
    if(MillMachine->isEnable()){
       MillMachine->goDrivesFind();
    }
    else {
      MillMachine->setDrivesFinded();
    }

    });

    //connect(ui.pbReset,SIGNAL(clicked()),MillMachine,SLOT(reset()),Qt::DirectConnection);

    connect(MillMachine,&WLGMachine::changedBusy,this,&WLPositionWidget::updateEnableMoved);

    ui.cbExGCode->setToolTip(
                 tr(
                "<b>GCode:</font></b>"
                "<ol>"
                  "<li>G0,G1</li>"
                  "<li>G2,G3 (I,J,K,R) </li>"
                  "<li>G17,G18,G19</li>"
                  "<li>G28 </li>"
                  "<li>G40 G41 G42</li>"
                  "<li>G43 G44 G49</li>"
                  "<li>G51(XYZ scale)</li>"
                  "<li>G53(no modal)</li>"
                  "<li>G54-G59</li>"
                  "<li>G64,G61.1,G64(P,Q)</li>"
                  "<li>G80,G81,G83(Z,R,Q)</li>"
                  "<li>G90,G91</li>"
                  "<li>G93,G94</li>"
                  "<li>G98,G99</li>"
                "</ol>"
                  )
                );

initElementControls();

QTimer *timerFS= new QTimer;

connect(timerFS,SIGNAL(timeout()),SLOT(updateFSLabel()));
timerFS->start(50);

WLMPG *Whell=MillMachine->getMPG();

if(Whell)
 {
 connect(Whell,&WLMPG::changedCurIndexAxis,this,[=](quint8 i){if(gALabelX) gALabelX->setChecked(i==1);
                                                              if(gALabelY) gALabelY->setChecked(i==2);
                                                              if(gALabelZ) gALabelZ->setChecked(i==3);
                                                              if(gALabelA) gALabelA->setChecked(i==4);
                                                              if(gALabelB) gALabelB->setChecked(i==5);});


 if(gALabelX) gALabelX->setChecked(Whell->getCurIndexAxis()==1);
 if(gALabelY) gALabelY->setChecked(Whell->getCurIndexAxis()==2);
 if(gALabelZ) gALabelZ->setChecked(Whell->getCurIndexAxis()==3);
 if(gALabelA) gALabelA->setChecked(Whell->getCurIndexAxis()==4);
 if(gALabelB) gALabelB->setChecked(Whell->getCurIndexAxis()==5);

 //connect(Whell,&WLWhell::changedCurIndexX,this,[=](quint8 i){cbTypeManual->setCurrentIndex(i);});
 //connect(Whell,&WLWhell::changedCurVmode,ui.rbWhellVMode,&QRadioButton::setChecked);
 }

setFocusElement('J');

ui.pbOnMachine->setBaseSize(m_buttonSize);

//ui.labelConnect->setVisible(false);

}

WLPositionWidget::~WLPositionWidget()
{

}

void WLPositionWidget::setJogDistStr(QString str)
{
m_listManDist=str.split(",");
}


void WLPositionWidget::initElementControls()
{
QList <WLGDrive*> list=MillMachine->getGDrives();

QAction *action;
QSizePolicy sizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
QHBoxLayout *horLayout;

QVBoxLayout *verLayoutEL = ui.verLayoutControl;

pbPlusX=pbMinusX=
pbPlusY=pbMinusY=
pbPlusZ=pbMinusZ=
pbPlusA=pbMinusA=
pbPlusB=pbMinusB=
pbPlusC=pbMinusC=new QToolButton;

pbPlusX->setEnabled(false);


gALabelX=new WLGAxisLabel(this);
gALabelY=new WLGAxisLabel(this);
gALabelZ=new WLGAxisLabel(this);
gALabelA=new WLGAxisLabel(this);
gALabelB=new WLGAxisLabel(this);
gALabelC=new WLGAxisLabel(this);

QFont font;

font=this->font();
font.setPointSize(12);


foreach(WLGDrive *mdrive,list)
 {
 WLGAxisLabel *AL=nullptr;

      if(mdrive->getName()=="X")  AL=gALabelX;
 else if(mdrive->getName()=="Y")  AL=gALabelY;
 else if(mdrive->getName()=="Z")  AL=gALabelZ;
 else if(mdrive->getName()=="A")  AL=gALabelA;
 else if(mdrive->getName()=="B")  AL=gALabelB;

 if(AL==nullptr) continue;

 horLayout=new QHBoxLayout(this);

 AL->setDrive(mdrive);
 AL->setGCode(MillMachine->getGCode());;

 connect(AL,SIGNAL(changedPress(QString,int)),this,SLOT(onPushDrive(QString,int)));

 horLayout->addWidget(AL);

 QToolButton *TBM = new QToolButton(this);
 TBM->setIcon(QPixmap(":/data/icons/minus1.png"));
 TBM->setIconSize(m_buttonSize);
 TBM->setAutoRepeat(false);
 TBM->setSizePolicy(sizePolicy);

 connect(TBM,&QToolButton::pressed,this,[=]() {onPBAxis(mdrive->getName(),-1,1);});
 connect(TBM,&QToolButton::released,this,[=](){onPBAxis(mdrive->getName(),-1,0);});

 horLayout->addWidget(TBM);

 QToolButton *TBP = new QToolButton(this);
 TBP->setIcon(QPixmap(":/data/icons/plus2.png"));
 TBP->setIconSize(m_buttonSize);
 TBP->setAutoRepeat(false);
 TBP->setSizePolicy(sizePolicy);

 connect(TBP,&QToolButton::pressed, this,[=](){onPBAxis(mdrive->getName(),1,1);});
 connect(TBP,&QToolButton::released,this,[=](){onPBAxis(mdrive->getName(),1,0);});
 horLayout->addWidget(TBP);

 connect(MillMachine,&WLGMachine::changedEnable,TBP,&QToolButton::setEnabled);
 connect(MillMachine,&WLGMachine::changedEnable,TBM,&QToolButton::setEnabled);

 connect(MillMachine,&WLGMachine::changedPossibleManual,TBP,&QToolButton::setEnabled);
 connect(MillMachine,&WLGMachine::changedPossibleManual,TBM,&QToolButton::setEnabled);

 TBM->setEnabled(MillMachine->isEnable());
 TBP->setEnabled(MillMachine->isEnable());

 TBPDriveList+=TBP;
 TBMDriveList+=TBM;

 if(mdrive->getName()=="X") {pbPlusX=TBP;pbMinusX=TBM;}
 else if(mdrive->getName()=="Y") {pbPlusY=TBP;pbMinusY=TBM;}
 else if(mdrive->getName()=="Z") {pbPlusZ=TBP;pbMinusZ=TBM;}
 else if(mdrive->getName()=="A") {pbPlusA=TBP;pbMinusA=TBM;}
 else if(mdrive->getName()=="B") {pbPlusB=TBP;pbMinusB=TBM;}
 else if(mdrive->getName()=="C") {pbPlusC=TBP;pbMinusC=TBM;}

 verLayoutEL->addLayout(horLayout);
 }

QSizePolicy sizePolicyE = sizePolicy;

sizePolicyE.setHorizontalPolicy(QSizePolicy::Expanding);
//F
horLayout=new QHBoxLayout(this);

pbWL = new QToolButton(this);
pbWL->setFont(font);
pbWL->setIcon(QPixmap(":/data/icons/menu.png"));
//pbReset->setText("");
pbWL->setIconSize(m_buttonSize);
pbWL->setSizePolicy(sizePolicy);
//pbWL->setDisabled(true);

pbWL->setPopupMode(QToolButton::InstantPopup);

QMenu *menuWL = new QMenu();

QMenu *menuProgram = new QMenu(tr("Program"));

menuProgram->addAction((tr("Start")),this,[=](){MillMachine->runGProgram();});

menuProgram->addAction((tr("Start at...")),this,[=](){
    if(MillMachine->isActiv()) return;

      WLEnterNum EnterNum(this);
      EnterNum.setMinMaxNow(0,Program->getElementCount(),Program->getActivElement());
      EnterNum.setDecimals(0);
      EnterNum.setLabel(tr("Which element to start processing?:"));
      EnterNum.show();

      if(EnterNum.exec()){
       MillMachine->runGProgram(EnterNum.getNow());
      }});


menuProgram->addAction((tr("Continue...")),this,[=](){
    if(MillMachine->isActiv()) return;

    if(Program->getLastMovElement()!=0){
      MillMachine->runGProgram(Program->getLastMovElement());
      }});

action=menuWL->addMenu(menuProgram);

connect(MillMachine,&WLGMachine::changedReadyRunList,action,&QAction::setDisabled);

QMenu *menuModeRun = new QMenu(tr("Mode"));

QActionGroup *aGroupMode = new QActionGroup(this);

action=action=menuModeRun->addAction((tr("normal")),[=](){MillMachine->getMotionDevice()->getModulePlanner()->setModeRun(PLANNER_normal);});
aGroupMode->addAction(action);
action->setCheckable(true);
action->setChecked(true);

action=menuModeRun->addAction((tr("one element")),[=](){MillMachine->getMotionDevice()->getModulePlanner()->setModeRun(PLANNER_oneElement);});
aGroupMode->addAction(action);
action->setCheckable(true);

action=menuModeRun->addAction((tr("one block"))  ,[=](){MillMachine->getMotionDevice()->getModulePlanner()->setModeRun(PLANNER_oneBlock);});
aGroupMode->addAction(action);
action->setCheckable(true);

aGroupMode->setExclusive(true);

action=menuWL->addMenu(menuModeRun);

connect(MillMachine,&WLGMachine::changedPossibleEditModeRun,action,&QAction::setEnabled);

QMenu *menuG28 = new QMenu(tr("G28"));
menuG28->addAction((tr("set")),this,SLOT(onPBsetG28()));
menuG28->addAction((tr("get")),this,SLOT(onPBgetG28()));

menuWL->addMenu(menuG28);


QMenu *ignoreInput = new QMenu(tr("Ignore"));

action=ignoreInput->addAction("inPause");
action->setCheckable(true);
connect(action,&QAction::toggled,this,[=](bool en){MillMachine->setIgnoreInPause(en);});

action=ignoreInput->addAction("inStop");
action->setCheckable(true);
connect(action,&QAction::toggled,this,[=](bool en){MillMachine->setIgnoreInStop(en);});

connect(ignoreInput,&QMenu::aboutToShow,[=](){
foreach(QAction *act,ignoreInput->actions())
{
if(act->text()=="inPause")
    act->setChecked(MillMachine->isIngnoreInPause());
else
if(act->text()=="inStop")
    act->setChecked(MillMachine->isIngnoreInStop());
}

});

menuWL->addMenu(ignoreInput);

menuWL->addAction((tr("Reset task")+"(ESC)"),this,[=](){MillMachine->reset();});

QAction *act=menuWL->addAction((tr("Show real Pos.")),[=](){m_showErrorPos=!m_showErrorPos;});
act->setCheckable(true);

pbWL->setMenu(menuWL);

QShortcut *scEscape = new QShortcut(Qt::Key_Escape,this);
connect(scEscape,&QShortcut::activated,this,[=](){MillMachine->reset();});

//connect(pbReset,SIGNAL(clicked()),MillMachine,SLOT(reset()),Qt::DirectConnection);

horLayout->addWidget(pbWL);

labelF = new WLLabel(this);
labelF->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
labelF->setPrefix("F:");
labelF->setDataN(0);
labelF->setFont(font);
labelF->setSizePolicy(sizePolicyE);

horLayout->addWidget(labelF);
/*
QToolButton *TB100 = new QToolButton(this);
TB100->setIcon(QPixmap(":/data/icons/100.png"));
TB100->setIconSize(m_buttonSize);
TB100->setSizePolicy(sizePolicy);
horLayout->addWidget(TB100);
*/
corFper = new WLValueLabel(this);
corFper->setAlignment(Qt::AlignCenter);
corFper->setRange(0,300);
corFper->setValue(100);
corFper->setPrefix("(");
corFper->setSuffix("%)");
corFper->setSizePolicy(sizePolicyE);
corFper->setFont(font);

connect(corFper,&WLValueLabel::valueChanged,MillMachine,&WLGMachine::setPercentF);
connect(MillMachine,&WLGMachine::changedPercentSpeed,corFper,&WLValueLabel::setValue);
MillMachine->setPercentF(100);

horLayout->addWidget(corFper);

//connect(TB100,&QToolButton::clicked,this,[=](){corFper->setValue(100);});

verLayoutEL->addLayout(horLayout);

//horLayout=new QHBoxLayout(this);

QToolButton *TBM = new QToolButton(this);
TBM->setIcon(QPixmap(":/data/icons/minus1.png"));
TBM->setIconSize(m_buttonSize);
TBM->setAutoRepeat(true);
TBM->setSizePolicy(sizePolicy);
//TBM->setMinimumSize(m_sizeButton);
connect(TBM,&QToolButton::clicked,this,&WLPositionWidget::on_pbMinusFper_pressed);
horLayout->addWidget(TBM);

QToolButton *TBP = new QToolButton(this);
TBP->setIcon(QPixmap(":/data/icons/plus2.png"));
TBP->setIconSize(m_buttonSize);
TBP->setAutoRepeat(true);
TBP->setSizePolicy(sizePolicy);
//TBP->setMinimumSize(m_sizeButton);
connect(TBP,&QToolButton::clicked,this,&WLPositionWidget::on_pbPlusFper_pressed);
horLayout->addWidget(TBP);

verLayoutEL->addLayout(horLayout);

//S
horLayout=new QHBoxLayout(this);
pbStart= new QToolButton(this);
pbStart->setIcon(QPixmap(":/data/icons/play.png"));
pbStart->setToolTip(tr("start GCode (program)"));
pbStart->setIconSize(m_buttonSize);
pbStart->setSizePolicy(sizePolicy);

connect(pbStart,&QToolButton::clicked,this,[=](){MillMachine->startMov();});

//connect(pbStart,&QToolButton::clicked,this,[=](){
//        if(MillMachine->isPause())  MillMachine->setPause(false);
//         else  if(!MillMachine->isEmptyMotion())  MillMachine->startMov();});

horLayout->addWidget(pbStart);

labelS = new WLLabel(this);
labelS->setPrefix("S:");
labelS->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
labelS->setDataN(0);
labelS->setFont(font);
labelS->setToolTip(tr("press for edit"));
labelS->setSizePolicy(sizePolicyE);
horLayout->addWidget(labelS);
/*
TB100 = new QToolButton(this);
TB100->setIcon(QPixmap(":/data/icons/100.png"));
TB100->setIconSize(m_buttonSize);
TB100->setSizePolicy(sizePolicy);
//TB100->setMinimumSize(m_sizeButton);
connect(TB100,&QToolButton::clicked,this,[=](){MillMachine->setPercentSOut(100);});
horLayout->addWidget(TB100);
*/
corSper = new WLValueLabel(this);
corSper->setAlignment(Qt::AlignCenter);
corSper->setRange(0.1,300);
//corSper->setSingleStep(0.05);
corSper->setValue(100);
corSper->setPrefix("(");
corSper->setSuffix("%)");
//corSper->setButtonSymbols(QAbstractSpinBox::NoButtons);
corSper->setFont(font);
corSper->setSizePolicy(sizePolicyE);

//connect(corSper,&WLValueLabel::doubleClicked,this,[=](){MillMachine->setPercentSOut(100);});
connect(corSper,&WLValueLabel::valueChanged,MillMachine,&WLGMachine::setPercentS);
connect(MillMachine,&WLGMachine::changedPercentSOut,corSper,&WLValueLabel::setValue);
MillMachine->setPercentS(100);
horLayout->addWidget(corSper);

//connect(TB100,&QToolButton::clicked,this,[=](){corSper->setValue(100);});

verLayoutEL->addLayout(horLayout);
//ui.laElementControl->addLayout(horLayout);

//horLayout=new QHBoxLayout(this);

TBM = new QToolButton(this);
TBM->setIcon(QPixmap(":/data/icons/minus1.png"));
TBM->setIconSize(m_buttonSize);
TBM->setAutoRepeat(true);
TBM->setSizePolicy(sizePolicy);
//TBM->setMinimumSize(m_sizeButton);
connect(TBM,&QToolButton::clicked,this,&WLPositionWidget::on_pbMinusSper_pressed);
horLayout->addWidget(TBM);

TBP = new QToolButton(this);
TBP->setIcon(QPixmap(":/data/icons/plus2.png"));
TBP->setIconSize(m_buttonSize);
TBP->setAutoRepeat(true);
TBP->setSizePolicy(sizePolicy);
//TBP->setMinimumSize(m_sizeButton);
connect(TBP,&QToolButton::clicked,this,&WLPositionWidget::on_pbPlusSper_pressed);
horLayout->addWidget(TBP);

//ui.laElementControl->addLayout(horLayout);
verLayoutEL->addLayout(horLayout);

//JOG
horLayout=new QHBoxLayout(this);

pbPause = new QToolButton(this);
pbPause->setFont(font);
pbPause->setIcon(QPixmap(":/data/icons/pause.png"));
pbPause->setIconSize(m_buttonSize);
pbPause->setSizePolicy(sizePolicy);
//pbPause->setCheckable(true);
//pbPause->setEnabled(false);

QShortcut *scSpace = new QShortcut(Qt::Key_Space,this);

//connect(scSpace,&QShortcut::activated,this,[=](){if(pbPause->isEnabled()) pbPause->setChecked(true);});
connect(scSpace,&QShortcut::activated,this,[=](){pbPause->click();});
horLayout->addWidget(pbPause);

//pbPause->setVisible(false);


pbStop = new QToolButton(this);
pbStop->setFont(font);
pbStop->setIcon(QPixmap(":/data/icons/stop.png"));
pbStop->setIconSize(m_buttonSize);
pbStop->setSizePolicy(sizePolicy);

//connect(pbStop,&QToolButton::pressed,MillMachine,&WLMillMachine::stopMov);

QShortcut *scCtrl = new QShortcut(Qt::Key_Control,this);
connect(scCtrl,&QShortcut::activated,this,[=](){pbStop->click();});

//QShortcut *scSpace = new QShortcut(Qt::Key_Space,this);

//connect(scSpace,&QShortcut::activated,this,[=](){if(pbPause->isEnabled()) pbPause->setChecked(true);});
//connect(scSpace,&QShortcut::activated,this,[=](){pbStop->click();});
//QMenu *menuPause = new QMenu();
//menuPause->addAction((tr("cancel")),this,[=](){MillMachine->reset();});

//pbPause->setMenu(menuPause);

horLayout->addWidget(pbStop);

labelTypeManual = new QLabel(this);
labelTypeManual->setFont(font);
labelTypeManual->setText("JOG");
labelTypeManual->setSizePolicy(sizePolicyE);

horLayout->addWidget(labelTypeManual);

pbFast = new QToolButton(this);
pbFast->setFont(font);
pbFast->setText(tr("fast"));
pbFast->setIconSize(m_buttonSize);
pbFast->setSizePolicy(sizePolicy);

connect(pbFast,&QToolButton::pressed,this,&WLPositionWidget::on_pbFast_pressed);
connect(pbFast,&QToolButton::released,this,&WLPositionWidget::on_pbFast_released);

horLayout->addWidget(pbFast);

sbFman = new QDoubleSpinBox(this);
sbFman->setAlignment(Qt::AlignLeft);
sbFman->setRange(0,100);
sbFman->setValue(10);
sbFman->setPrefix("");
sbFman->setSuffix("%");
sbFman->setButtonSymbols(QAbstractSpinBox::NoButtons);
sbFman->setSizePolicy(sizePolicyE);
sbFman->setFont(font);

connect(sbFman,SIGNAL(valueChanged(double)),MillMachine,SLOT(setPercentManual(double)));
connect(MillMachine,&WLGMachine::changedPercentManual,sbFman,&QDoubleSpinBox::setValue);

horLayout->addWidget(sbFman);

//ui.laElementControl->addLayout(horLayout);

//horLayout=new QHBoxLayout(this);

TBM = new QToolButton(this);
TBM->setIcon(QPixmap(":/data/icons/minus1.png"));
TBM->setIconSize(m_buttonSize);
TBM->setAutoRepeat(true);
TBM->setSizePolicy(sizePolicy);
connect(TBM,&QToolButton::clicked,MillMachine,&WLGMachine::minusPercentManual);
horLayout->addWidget(TBM);


TBP = new QToolButton(this);
TBP->setIcon(QPixmap(":/data/icons/plus2.png"));
TBP->setIconSize(m_buttonSize);
TBP->setAutoRepeat(true);
TBP->setSizePolicy(sizePolicy);
connect(TBP,&QToolButton::clicked,MillMachine,&WLGMachine::plusPercentManual);
horLayout->addWidget(TBP);

verLayoutEL->addLayout(horLayout);

horLayout=new QHBoxLayout(this);

labelActivGCode = new QLabel(this);
labelActivGCode->setFont(font);
labelActivGCode->setText("GCode");
//labelActivGCode->setSizePolicy(sizePolicyE);
labelActivGCode->setWordWrap(true);

horLayout->addWidget(labelActivGCode);

connect(pbPause,&QToolButton::clicked,MillMachine,[=](){
  MillMachine->setPause();
 });

connect(pbStop,&QToolButton::clicked,MillMachine,[=](){
if(MillMachine->isPause()){
    if(QMessageBox::question(this,tr("Question")
                                  ,tr("reset task?")
                                  ,QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    {
    MillMachine->reset();
    }
  }
  else{
  MillMachine->stopMov();
  }
 }
);

QMenu *pbStopMenu = new QMenu;

action=pbStopMenu->addAction("ignore");
action->setCheckable(true);
connect(action,&QAction::toggled,this,[=](bool en){MillMachine->setIgnoreInStop(en);});

connect(pbStopMenu,&QMenu::aboutToShow,[=](){
foreach(QAction *act,pbStopMenu->actions())
  {
  if(act->text()=="ignore")
      act->setChecked(MillMachine->isIngnoreInStop());
  }
});
pbStop->setMenu(pbStopMenu);

QMenu *pbPauseMenu = new QMenu;

action=pbPauseMenu->addAction("ignore");
action->setCheckable(true);
connect(action,&QAction::toggled,this,[=](bool en){MillMachine->setIgnoreInPause(en);});

connect(pbPauseMenu,&QMenu::aboutToShow,[=](){
foreach(QAction *act,pbPauseMenu->actions())
  {
  if(act->text()=="ignore")
      act->setChecked(MillMachine->isIngnoreInPause());
  }
});
pbPause->setMenu(pbPauseMenu);

verLayoutEL->addLayout(horLayout);
}

float WLPositionWidget::calcStepMov()
{
if(m_curIndexListMan==0)
    return 0.0;
 else
    return labelTypeManual->text().toFloat();
}

void WLPositionWidget::setFocusElement(char f)
{
if(corFper->suffix().back()=="*") {corFper->setSuffix(corFper->suffix().chopped(1));}
else if(sbFman->suffix().back()=="*") {sbFman->setSuffix(sbFman->suffix().chopped(1));}
else if(corSper->suffix().back()=="*") {corSper->setSuffix(corSper->suffix().chopped(1));}

if(f=='F') corFper->setSuffix(corFper->suffix()+"*");
else if(f=='S') corSper->setSuffix(corSper->suffix()+"*");
else if(f=='J') sbFman->setSuffix(sbFman->suffix()+"*");

focusElement=f;
}


void WLPositionWidget::keyPressEvent (QKeyEvent * event)
{
if(!event->isAutoRepeat())
    qDebug()<<"WLPositionWidget press"<<event->key();

if(!event->isAutoRepeat())
switch(event->key())
   {
   case Qt::Key_Left:    if(pbMinusX->isEnabled()) {pbMinusX->setDown(true);onPBAxis("X", -1,1);} break;
   case Qt::Key_Right:   if(pbPlusX->isEnabled())  {pbPlusX->setDown(true); onPBAxis("X",  1,1);} break;
   case Qt::Key_Down:    if(pbMinusY->isEnabled()) {pbMinusY->setDown(true);onPBAxis("Y", -1,1);} break;
   case Qt::Key_Up:      if(pbPlusY->isEnabled())  {pbPlusY->setDown(true); onPBAxis("Y",  1,1);} break;
   case Qt::Key_PageDown:if(pbMinusZ->isEnabled()) {pbMinusZ->setDown(true);onPBAxis("Z", -1,1);} break;
   case Qt::Key_PageUp:  if(pbPlusZ->isEnabled())  {pbPlusZ->setDown(true); onPBAxis("Z",  1,1);} break;

   case Qt::Key_Z:       if(pbMinusA->isEnabled()) {pbMinusA->setDown(true);onPBAxis("A", -1,1);} break;
   case Qt::Key_A:       if(pbPlusA->isEnabled())  {pbPlusA->setDown(true); onPBAxis("A",  1,1);} break;

   case Qt::Key_X:       if(pbMinusB->isEnabled()) {pbMinusB->setDown(true);onPBAxis("B", -1,1);} break;
   case Qt::Key_S:       if(pbPlusB->isEnabled())  {pbPlusB->setDown(true); onPBAxis("B",  1,1);} break;

   case Qt::Key_C:       if(pbMinusC->isEnabled()) {pbMinusC->setDown(true);onPBAxis("C", -1,1);} break;
   case Qt::Key_D:       if(pbPlusC->isEnabled())  {pbPlusC->setDown(true); onPBAxis("C",  1,1);} break;
   }

if(!event->isAutoRepeat()){
switch(event->key())
   {
   case Qt::Key_Shift:   pbFast->setDown(true);
                         on_pbFast_pressed();
                         break;

   case Qt::Key_F:       setFocusElement('F'); break;
   case Qt::Key_S:       setFocusElement('S'); break;
   case Qt::Key_J:       setFocusElement('J'); break;
   }
}

switch(event->key())
   {
   case Qt::Key_Plus:    on_pbPlus_pressed();    break;
   case Qt::Key_Minus:   on_pbMinus_pressed();   break;
   }

event->accept();
}

void WLPositionWidget::keyReleaseEvent ( QKeyEvent * event )
{
if(!event->isAutoRepeat())
{
qDebug()<<"WLPositionWidget release"<<event->key();

  switch(event->key())
  {
  case Qt::Key_Left:    if(pbMinusX->isEnabled()) {pbMinusX->setDown(false); onPBAxis("X", 1,0);} break;
  case Qt::Key_Right:   if(pbPlusX->isEnabled())  {pbPlusX->setDown(false);  onPBAxis("X", 1,0);} break;
  case Qt::Key_Down:    if(pbMinusY->isEnabled()) {pbMinusY->setDown(false); onPBAxis("Y", 1,0);} break;
  case Qt::Key_Up:      if(pbPlusY->isEnabled())  {pbPlusY->setDown(false);  onPBAxis("Y", 1,0);} break;
  case Qt::Key_PageDown:if(pbMinusZ->isEnabled()) {pbMinusZ->setDown(false); onPBAxis("Z", 1,0);} break;
  case Qt::Key_PageUp:  if(pbPlusZ->isEnabled())  {pbPlusZ->setDown(false);  onPBAxis("Z", 1,0);} break;


  case Qt::Key_Z:       if(pbMinusA->isEnabled()) {pbMinusA->setDown(false);onPBAxis("A",  1,0);} break;
  case Qt::Key_A:       if(pbPlusA->isEnabled())  {pbPlusA->setDown(false); onPBAxis("A",  1,0);} break;

  case Qt::Key_X:       if(pbMinusB->isEnabled()) {pbMinusB->setDown(false);onPBAxis("B",  1,0);} break;
  case Qt::Key_S:       if(pbPlusB->isEnabled())  {pbPlusB->setDown(false); onPBAxis("B",  1,0);} break;

  case Qt::Key_C:       if(pbMinusC->isEnabled()) {pbMinusC->setDown(false);onPBAxis("C",  1,0);} break;
  case Qt::Key_D:       if(pbPlusC->isEnabled())  {pbPlusC->setDown(false); onPBAxis("C",  1,0);} break;
  }

  switch(event->key())
  {
  case Qt::Key_Shift:   pbFast->setDown(false);
                        on_pbFast_released();
                        break;
  }
}

event->accept();
}

void WLPositionWidget::focusOutEvent(QFocusEvent *event)
{
Q_UNUSED(event);

foreach(QToolButton *TBP,TBPDriveList){
 TBP->setDown(false);
 }

foreach(QToolButton *TBM,TBMDriveList){
 TBM->setDown(false);
 }

QList <WLDrive*> list=MillMachine->getDrives();

foreach (WLDrive *drive,list) {
  if(drive->isManual()) drive->startStop();
  }

if(pbFast->isDown()){
 pbFast->setDown(false);
 on_pbFast_released();
 }

}

void WLPositionWidget::mousePressEvent(QMouseEvent *event)
{
QMenu menu(this);
QAction *act;

if(labelS->geometry().contains(event->pos())) {
 act=menu.addAction(tr("set S correct"),this,SLOT(onSetSCor()));
 act=menu.addAction(tr("clear correct"),this,SLOT(onClearSCorList()));

 menu.exec(labelS->mapToGlobal(pos()));
 }else if (labelTypeManual->geometry().contains(event->pos()))
        {
        if(++m_curIndexListMan==m_listManDist.size())
          {
          m_curIndexListMan=0;
          }

        labelTypeManual->setText(m_listManDist[m_curIndexListMan]);
        }
        else  if (ui.labelInProbe->geometry().contains(event->pos()))
              {
              if(event->button()==Qt::LeftButton)
                  MillMachine->setSafeProbe(!MillMachine->isSafeProbe());

              if(event->button()==Qt::RightButton){
                  act=menu.addAction(tr("autoset"),this,[=](){MillMachine->setAutoSetSafeProbe(!MillMachine->isAutoSetSafeProbe());});
                  act->setCheckable(true);
                  act->setChecked(MillMachine->isAutoSetSafeProbe());

                  menu.exec(ui.labelInProbe->mapToGlobal(pos()));
                 }
              }
return;
}

void WLPositionWidget::updateProgress()
{
QString str;

ui.proBar->setVisible(MillMachine->isRunGProgram());
ui.labelTime->setVisible(MillMachine->isRunGProgram());
ui.cbExGCode->setVisible(!MillMachine->isRunGProgram());

if(!MillMachine->isRunGProgram())  return;

long           made=MillMachine->getIProgram();
long          order=Program->getElementCount();
double timeElement=MillMachine->getTimeElement()/1000;

if(timeElement>0.0)
{
long time_s=timeElement*(order-made+MillMachine->getMotionDevice()->getModulePlanner()->getCountBuf()
                                   +MillMachine->getTrajIProgramSize());

long h,m,s,d;

h=time_s/3600;
time_s-=h*3600;

m=time_s/60;
time_s-=m*60;

s=time_s;

str=(QString(("%1:%2:%3")).arg(h).arg(m).arg(s));

if(h<0)

str+=(QString(("/%1:%2:%3:%4")).arg(d).arg(h).arg(m).arg(s));

ui.labelTime->setText(str);

ui.proBar->setValue((float)(Program->getLastMovElement())
                   /(float)(Program->getElementCount())*100.0);
}
else {
ui.labelTime->setText("?");
ui.proBar->setValue(0);
}
}

void WLPositionWidget::updateFlashButtons()
{
bool ntryPos=false;
static bool flash=false;

ui.pbOnMachine->setStyleSheet(flash
                           &&(!ui.pbOnMachine->isChecked()) ? "background-color: rgb(255, 100, 100);border-style: solid;"
                               :(MillMachine->isRunMScript() ? "background-color: yellow ;border-style: solid;"
                               :(MillMachine->isAuto() ?"background-color: rgb(100, 255, 100); ;border-style: solid;":"border-style: solid;")));

foreach(WLGDrive *MD,MillMachine->getGDrives())
 {
 if(!MD->isTruPosition())   {ntryPos=true; break;}
 }

ui.pbFindDrivePos->setStyleSheet(ntryPos
                               &&flash
                               &&ui.pbOnMachine->isChecked() ? "background-color: rgb(255, 100, 100);border-style: solid;"
                                                                            :"border-style: solid;");

QPixmap pixmap = QPixmap(":/data/icons/play.png");

QBitmap mask = pixmap.createMaskFromColor(Qt::transparent);

pixmap.setMask(mask);

pixmap.fill(Qt::green);

pbStart->setIcon(pixmap);

QString iconStr;

switch(MillMachine->getMotionDevice()->getModulePlanner()->getModeRun()){
 case PLANNER_normal:     iconStr=":/data/icons/play.png";          break;
 case PLANNER_oneElement: iconStr=":/data/icons/playOneElement.png";break;
 case PLANNER_oneBlock:   iconStr=":/data/icons/playOneBlock.png";  break;
 }

pbStart->setIcon(((MillMachine->isActiv()||MillMachine->isRunMScript())
                 &&flash)
                 ? QPixmap(":/data/icons/play_red.png")
                  :QPixmap(iconStr));

pbPause->setIcon(MillMachine->getMotionDevice()->getModulePlanner()->getInput(PLANNER_inPause)->getNow()
                 &&flash
                 ? (MillMachine->isIngnoreInPause() ? QPixmap(":/data/icons/ionX.png") : QPixmap(":/data/icons/ion.png"))
                 : (MillMachine->isIngnoreInPause()&&flash ? QPixmap(":/data/icons/ioffX.png") : QPixmap(":/data/icons/pause.png")));

pbStop->setIcon(MillMachine->getMotionDevice()->getModulePlanner()->getInput(PLANNER_inStop)->getNow()
                &&flash
                ? (MillMachine->isIngnoreInStop() ? QPixmap(":/data/icons/ionX.png") : QPixmap(":/data/icons/ion.png"))
                : (MillMachine->isIngnoreInStop()&&flash ? QPixmap(":/data/icons/ioffX.png") : QPixmap(":/data/icons/stop.png")));

flash=!flash;
}


void WLPositionWidget::onTeachAxis(QString nameDrive)
{
WLGDrive *Drive=MillMachine->getDrive(nameDrive);
if(Drive!=nullptr)
 {
 WLEnterNum  EnterNum (this);
 EnterNum.setLabel(tr("Current position Drive ")+nameDrive+" = ");
 EnterNum.setNow(Drive->getAxisPosition());

 if(EnterNum.exec())
        {
        Drive->setPosition(EnterNum.getNow());
        MillMachine->goDriveTeach(nameDrive);
        }
}
}

void WLPositionWidget::onEditPidAxis(WLAxis *axis)
{
WLPidWidget PW(QString("Axis %1").arg(axis->getIndex()),axis->getPidData(),this);

PW.show();

connect(&PW,&WLPidWidget::changedPidData,[=](WLPidData data){axis->setPidData(data);});

if(PW.exec()) {
  axis->setPidData(PW.getPidData());
  }
}

void WLPositionWidget::onSetSCor()
{
if(QMessageBox::question(this,tr("Question")
                              ,tr("set correct S?")
                              ,QMessageBox::Ok|QMessageBox::Cancel))
{
MillMachine->addCurrentSCor();
corSper->setValue(100);
}
/*
if(MillMachine->isUseCorrectSOut())  {
 MillMachine->addCurrentSCor();
 corSper->setValue(100);
 }
else {
     QMessageBox::information(this,tr("information"),tr("S correct is off, please on in Edit->WLMill"));
     }
*/
}

void WLPositionWidget::onClearSCorList()
{
auto ret=QMessageBox::question(this,tr("Question")
                              ,tr("Delete all data correct S?")
                              ,QMessageBox::Ok|QMessageBox::Cancel);
if(ret==QMessageBox::Ok)
 {
 MillMachine->clearSCorList();
 }
}

void WLPositionWidget::on_pbFast_pressed()
{
pressedManual=sbFman->value();
MillMachine->setPercentManual(100.0);
}

void WLPositionWidget::on_pbFast_released()
{
MillMachine->setPercentManual(pressedManual);
}


void WLPositionWidget::on_pbPlus_pressed()
{
switch(focusElement)
{
case 'F': on_pbPlusFper_pressed(); break;
case 'S': on_pbPlusSper_pressed(); break;
//case 'J': on_pbPlusFman_pressed(); break;
}

}

void WLPositionWidget::on_pbMinus_pressed()
{
switch(focusElement)
{
case 'F': on_pbMinusFper_pressed(); break;
case 'S': on_pbMinusSper_pressed(); break;
//case 'J': on_pbMinusFman_pressed(); break;
}

}

void WLPositionWidget::onPBAxis(QString name, int rot,bool press)
{
qDebug()<<name<<"onPBAxis"<<rot<<press;

if(press)
    {
    double percentManual;

    if(pbFast->isDown())
        percentManual=100.0;
    else
        percentManual=sbFman->value();

    MillMachine->setPercentManual(percentManual);

    MillMachine->goDriveManual(name,percentManual/100.0*rot,calcStepMov());
    }
 else
    {
    if(calcStepMov()==0)
        MillMachine->goDriveManual(name,0,0);
    }
}



void WLPositionWidget::updateFSLabel()
{
labelF->setData(MillMachine->getCurFxyz()*60);
labelS->setData(MillMachine->getCurSOut());
}


void WLPositionWidget::onExGCode()
{    
MillMachine->runGCode(ui.cbExGCode->currentText());
}

void WLPositionWidget::onPBRotSCRef()
{
WL3DPoint refP0(MillMachine->m_GCode.getRotPoint0SC(MillMachine->m_GCode.getActivSC()).to3D());
WL3DPoint curPos(MillMachine->getCurrentPositionActivSC().to3D());
WL3DPoint refP1(MillMachine->m_GCode.getRotPoint1SC(MillMachine->m_GCode.getActivSC()).to3D());

curPos.z=0;

double a=(curPos.getAxy(refP0)-refP1.getAxy(refP0))*180.0/M_PI;

MillMachine->getGCode()->setRotCurSC(MillMachine->getGCode()->getRotCurSC()+a);
}

void WLPositionWidget::updateEnableMoved(bool en)
{
disButton=en;

ui.pbFindDrivePos->setDisabled(en);
ui.pbRotSC->setDisabled(en);

if(MillMachine->isRunGProgram())
    ui.cbExGCode->setDisabled(en);
else
    ui.cbExGCode->setDisabled(false);

QPalette palette=ui.cbExGCode->palette();

palette.setColor(QPalette::Text, MillMachine->isAutoStartGCode() ? Qt::red : Qt::black);

ui.cbExGCode->setPalette(palette);
}

void WLPositionWidget::updatePosition()
{
WLGPoint GP;
WLGPoint GP53;

GP53=MillMachine->getAxisPosition();
GP=MillMachine->getCurrentPositionActivSC();

if(m_showErrorPos)
  GP=GP-MillMachine->getAxisErrorPosition();

QPalette blckPalette,redPalette;

blckPalette.setColor(QPalette::WindowText, Qt::black);
 redPalette.setColor(QPalette::WindowText, Qt::red);

gALabelX->setGPos(MillMachine->getGCode()->isXDiam() ? GP.x*2.0 : GP.x);
gALabelY->setGPos(GP.y);
gALabelZ->setGPos(GP.z-MillMachine->getGCode()->getHToolOfst());
gALabelA->setGPos(GP.a);
gALabelB->setGPos(GP.b);

/*
ui.qwAxisLabelPosX->rootObject()->setProperty("p_data53",QString("%1").arg((GP53.x),0,'f',2));
ui.qwAxisLabelPosY->rootObject()->setProperty("p_data53",QString("%1").arg((GP53.y),0,'f',2));

ui.qwAxisLabelPosX->rootObject()->setProperty("p_data",QString("%1").arg((GP.x),0,'f',2));
ui.qwAxisLabelPosY->rootObject()->setProperty("p_data",QString("%1").arg((GP.y),0,'f',2));

ui.qwAxisLabelPosX->rootObject()->setProperty("p_feed",QString("%1").arg((MillMachine->getDrive("X")->Vnow()*60),0,'f',0));
ui.qwAxisLabelPosY->rootObject()->setProperty("p_feed",QString("%1").arg((MillMachine->getDrive("Y")->Vnow()*60),0,'f',0));

ui.qwAxisLabelPosX->rootObject()->setProperty("p_color",MillMachine->getDrive("X")->isTruPosition()?"black":"red");
ui.qwAxisLabelPosY->rootObject()->setProperty("p_color",MillMachine->getDrive("Y")->isTruPosition()?"black":"red");

if(ui.qwAxisLabelPosZ->isVisible())
{
ui.qwAxisLabelPosZ->rootObject()->setProperty("p_color",MillMachine->getDrive("Z")->isTruPosition()?"black":"red");
ui.qwAxisLabelPosZ->rootObject()->setProperty("p_data",QString("%1").arg((GP.z),0,'f',2));
ui.qwAxisLabelPosZ->rootObject()->setProperty("p_data53",QString("%1").arg((GP53.z),0,'f',2));
ui.qwAxisLabelPosZ->rootObject()->setProperty("p_feed",QString("%1").arg(MillMachine->getDrive("Z")->Vnow()*60,0,'f',0));

}

if(ui.qwAxisLabelPosA->isVisible())
{
ui.qwAxisLabelPosA->rootObject()->setProperty("p_color",MillMachine->getDrive("A")->isTruPosition()?"black":"red");
ui.qwAxisLabelPosA->rootObject()->setProperty("p_data",QString("%1").arg((GP.a),0,'f',2));
ui.qwAxisLabelPosA->rootObject()->setProperty("p_data53",QString("%1").arg((GP53.a),0,'f',2));
ui.qwAxisLabelPosA->rootObject()->setProperty("p_feed",QString("%1").arg(MillMachine->getDrive("A")->Vnow()*60,0,'f',0));

}

if(ui.qwAxisLabelPosB->isVisible())
{
ui.qwAxisLabelPosB->rootObject()->setProperty("p_color",MillMachine->getDrive("B")->isTruPosition()?"black":"red");
ui.qwAxisLabelPosB->rootObject()->setProperty("p_data",QString("%1").arg((GP.b),0,'f',2));
ui.qwAxisLabelPosB->rootObject()->setProperty("p_data53",QString("%1").arg((GP53.b),0,'f',2));
ui.qwAxisLabelPosB->rootObject()->setProperty("p_feed",QString("%1").arg(MillMachine->getDrive("B")->Vnow()*60,0,'f',0));

}*/

gALabelX->update();
gALabelY->update();
gALabelZ->update();
gALabelA->update();
gALabelB->update();

ui.pbRotSC->setText(QString::number(MillMachine->m_GCode.getRotCurSC(),'f',2));

labelActivGCode->setText(MillMachine->m_GCode.getActivGCodeString());


if(MillMachine->motDevice->getModuleConnect())
 ui.labelConnect->setEnabled(MillMachine->motDevice->getModuleConnect()->isConnect());
else
 ui.labelConnect->setEnabled(false);

if(MillMachine->motDevice->getModuleConnect())
 ui.labelConnect->setEnabled(MillMachine->motDevice->getModuleConnect()->isConnect());
else
 ui.labelConnect->setEnabled(false);


WLModulePlanner *ModulePlanner=MillMachine->getMotionDevice()->getModulePlanner();

if(!(MillMachine->isSafeProbe()&&MillMachine->getActSafeProbe()!=WLIOPut::INPUT_actNo)){
   if(ModulePlanner->getInput(PLANNER_inProbe)->getNow())
       ui.labelInProbe->setPixmap(QPixmap(":/data/icons/ionX.png"));
   else
       ui.labelInProbe->setPixmap(QPixmap(":/data/icons/ioffX.png"));
 }
 else{
  if(ModulePlanner->getInput(PLANNER_inProbe)->getNow())
      ui.labelInProbe->setPixmap(QPixmap(":/data/icons/ion.png"));
  else
      ui.labelInProbe->setPixmap(QPixmap(":/data/icons/ioff.png"));
  }

QPalette palette = labelS->palette();
palette.setColor(QPalette::WindowText,MillMachine->isSpindleStop()? Qt::black : Qt::red);
labelS->setPalette(palette);

palette = labelF->palette();
palette.setColor(QPalette::WindowText,ModulePlanner->getStatus()==PLANNER_run ? Qt::red: Qt::black);
labelF->setPalette(palette);

palette = corFper->palette();
palette.setColor(QPalette::WindowText,corFper->value()==100.0 ? Qt::black : corFper->value()>100.0 ? Qt::red : Qt::blue);
corFper->setPalette(palette);

palette = corSper->palette();
palette.setColor(QPalette::WindowText,corSper->value()==100.0 ? Qt::black : corSper->value()>100.0 ? Qt::red : Qt::blue);
corSper->setPalette(palette);


#ifdef DEF_5D
P=MillMachine->getCurrentPosition();
AG5X->setData(P.a,3 );
BG5X->setData(P.b,3 );
CG5X->setData(P.c,3 );
#endif
#ifdef MILLKUKA
WL6EPos EPos =MillMachine->Kuka->getPosition6EPos();
E1->setData(EPos.e1.pos,5);
#endif
}

bool WLPositionWidget::eventFilter(QObject *watched, QEvent *event)
{
if(event->type()==QEvent::KeyPress)
  {
  QKeyEvent *kevent=static_cast<QKeyEvent*>(event);

  if(kevent->key()==Qt::Key_Tab){
    if(ui.cbExGCode==watched)  {
      setFocus(Qt::MouseFocusReason);
      return true;
      }
    }
  }
/*
if(watched==ui.cbExGCode
  &&ui.cbExGCode->currentText().isEmpty())
  {
  QKeyEvent *kevent=static_cast<QKeyEvent*>(event);

  if(kevent->key()==Qt::Key_Left
   ||kevent->key()==Qt::Key_Right
   ||kevent->key()==Qt::Key_Down
   ||kevent->key()==Qt::Key_Up
   ||kevent->key()==Qt::Key_PageDown
   ||kevent->key()==Qt::Key_PageUp){

    if(event->type()==QEvent::KeyPress){
       keyPressEvent(static_cast<QKeyEvent*>(event));
       return true;
       }

    if(event->type()==QEvent::KeyRelease){
       keyReleaseEvent(static_cast<QKeyEvent*>(event));
       return true;
       }
    }
  }
*/
if(event->type()==QEvent::KeyPress)
 {
 qDebug()<<"eventFilter keyPress"<<watched->metaObject()->className();
 }

if(event->type()==QEvent::KeyRelease)
 {
 qDebug()<<"eventFilter keyRelease"<<watched->metaObject()->className();
 }

return QWidget::eventFilter(watched,event);
}

void WLPositionWidget::onPushDrive(QString nameDrive,int type)
{
if(disButton) return;

 WLGCode *GCode=MillMachine->getGCode();

 QWidget *widget=this;

     if(nameDrive=="X") widget=gALabelX;
else if(nameDrive=="Y") widget=gALabelY;
else if(nameDrive=="Z") widget=gALabelZ;
else if(nameDrive=="A") widget=gALabelA;
else if(nameDrive=="B") widget=gALabelB;

if(type==WLGAxisLabel::typeName)
 {
 QMenu menu(this);
 QAction *act;

 QMenu menuPos(this);
 menuPos.setTitle(tr("Position"));

 act=menuPos.addAction(tr("0"));
 connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->setCurPositionSC(nameDrive,0);});

 act=menuPos.addAction(tr("1/2"));
 connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->setCurPositionSC(nameDrive,MillMachine->getCurPositionSC(nameDrive)/2);});

 menu.addMenu(&menuPos);

 if(nameDrive=="Z"){
   if(MillMachine->isUseHPause()){
   act=menu.addAction(tr("set H pause"));
   connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->setHPause(MillMachine->getCurPosition(nameDrive));});
   }

  quint16 H=GCode->getValue('H');

  if((GCode->isGCode(43)
    ||GCode->isGCode(44))
       &&(H>0)){

    act=menu.addAction(tr("calc H tool"));
    connect(act, &QAction::triggered,MillMachine,[=](){

    WLEnterNum  EnterNum (this);

    EnterNum.setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);

    EnterNum.show();

    movDialogToDesktop(widget,&EnterNum);

    EnterNum.setLabel("Enter current Z=");

    EnterNum.setNow(MillMachine->getCurPositionSC(nameDrive));

    EnterNum.selectAll();

    if(EnterNum.exec())  {
        if(GCode->isGCode(43)){
          GCode->setHTool(GCode->getValue('H')
                        ,MillMachine->getCurPositionSC(nameDrive)-EnterNum.getNow()+GCode->getHToolOfst());
          }
          else{
          GCode->setHTool(GCode->getValue('H')
                        ,EnterNum.getNow()-MillMachine->getCurPositionSC(nameDrive)-GCode->getHToolOfst());

          }
        }
    }
    );
    }
  }

 QMenu menuOper(this);
 menuOper.setTitle(tr("Action"));

 act=menuOper.addAction(tr("Find"));
 connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->goDriveFind(nameDrive);});

 act=menuOper.addAction(tr("Reset Find"));
 connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->getDrive(nameDrive)->setTruPosition(false);});

 act=menuOper.addAction(tr("Verify"));
 connect(act, &QAction::triggered,this,[=](){MillMachine->goDriveVerify(nameDrive);});

 act=menuOper.addAction(tr("Teach"));
 connect(act, &QAction::triggered,this,[=](){onTeachAxis(nameDrive);});

 menu.addMenu(&menuOper);

 QMenu menuPid(this);
 menuPid.setTitle(tr("pid"));

 foreach(WLAxis *axis,MillMachine->getDrive(nameDrive)->getAxisList()){
 if(axis->isUsePid()){
     act=menuPid.addAction(QString("Axis %1").arg(axis->getIndex()));
     connect(act, &QAction::triggered,this,[=](){onEditPidAxis(axis);});
     }
 }

 if(!menuPid.isEmpty())
     menu.addMenu(&menuPid);

 //act=menu.addAction(tr("Set Plus  Limit"));
 //act=menu.addAction(tr("Set Minus Limit"));


 act=menu.addAction(tr("Reset alarm"));
 connect(act, &QAction::triggered,MillMachine,[=](){MillMachine->getDrive(nameDrive)->resetAlarm();});

 act=menu.addAction(tr("Enable"));
 act->setCheckable(true);
 act->setChecked(MillMachine->getDrive(nameDrive)->isEnable());
 connect(act, &QAction::toggled,this,[=](bool check){MillMachine->getDrive(nameDrive)->setEnable(check);});



      if(nameDrive=="X") menu.exec(gALabelX->mapToGlobal(pos()));
 else if(nameDrive=="Y") menu.exec(gALabelY->mapToGlobal(pos()));
 else if(nameDrive=="Z") menu.exec(gALabelZ->mapToGlobal(pos()));
 else if(nameDrive=="A") menu.exec(gALabelA->mapToGlobal(pos()));
 else if(nameDrive=="B") menu.exec(gALabelB->mapToGlobal(pos()));

 return;
 }else{
 WLEnterNum  EnterNum (this);

 EnterNum.setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);
 EnterNum.show();

 movDialogToDesktop(widget,&EnterNum);

 if(type==WLGAxisLabel::typePos)
     {
     EnterNum.setLabel(nameDrive+"(G53)=");
     EnterNum.setNow(MillMachine->getDrive(nameDrive)->position());
     }else {
     EnterNum.setLabel(nameDrive+"(G"+QString::number(MillMachine->getGCode()->getGSC())+")=");
     }

 EnterNum.selectAll();

 if(EnterNum.exec())  setPosition(nameDrive,EnterNum.getNow(),type);
 }

}


void WLPositionWidget::setPosition(QString nameDrive,float pos,int type)
{
switch(type)
{
case WLGAxisLabel::typeOfst:  MillMachine->setCurPositionSC(nameDrive,pos);
                              break;

case WLGAxisLabel::typePos:   MillMachine->setCurPosition(nameDrive,pos);
                              break;
}

}

void WLPositionWidget::onPBRotSC()
{
if(disButton) return;

WLEnterNum  EnterNum (this);
EnterNum.setLabel("Rxy=");
EnterNum.setSuffix(tr("gr"));

EnterNum.show();

if(EnterNum.exec()){
 MillMachine->getGCode()->setRotCurSC(EnterNum.getNow());
 }

}

void WLPositionWidget::onPBsetP0()
{
WLEditPoint EP;

WLGPoint  GP=MillMachine->m_GCode.getRotPoint0SC(MillMachine->m_GCode.getActivSC(),0);

EP.setNameData("X,Y,Z");
EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    qDebug("update rotPoint");
    GP.fromString(EP.getValueStr());
    MillMachine->m_GCode.setRotPoint0SC(MillMachine->m_GCode.getActivSC(),GP);
    }
};

void WLPositionWidget::onPBsetP1()
{
WLEditPoint EP;

WLGPoint  GP=MillMachine->m_GCode.getRotPoint1SC(MillMachine->m_GCode.getActivSC());

EP.setNameData("X,Y,Z");
EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    qDebug("update rotPoint");
    GP.fromString(EP.getValueStr());
    MillMachine->m_GCode.setRotPoint1SC(MillMachine->m_GCode.getActivSC(),GP);
    }
};

void WLPositionWidget::onPBsetG28()
{
WLGPoint GP;
WLEditPoint EP;
QString nameDrive;

EP.setNameData("X,Y,Z,A,B,C");

QList <double> List;

GP=MillMachine->m_GCode.getG28Position();


EP.setLabel(tr("enter G28 position"));

EP.setValueStr(GP.toString());

EP.show();

if(EP.exec())
    {
    GP.fromString(EP.getValueStr());
    MillMachine->m_GCode.setG28Position(GP);
    }
	///emit ChangedHomePosition(Program->GCode.getSC(iSC-1).toM()*EP.getFrame().toM());

}

void WLPositionWidget::onPBgetG28()
{
if(QMessageBox::question(this, tr("Question:"),tr("are you sure?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
                   MillMachine->m_GCode.setG28Position(MillMachine->getCurrentPosition(1));
}
