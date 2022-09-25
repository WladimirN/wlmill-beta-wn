#ifndef WLPOSITIONWIDGET_H
#define WLPOSITIONWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QLayout>
#include <QButtonGroup>
#include <QTime>
#include <QSplitter>
#include <QShortcut>
#include <QMessageBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "ui_wlpositionwidget.h"

#include "wllabel.h"
#include "wl3dpoint.h"
#include "wleditpoint.h"
#include "wlgmachine.h"
#include "wlgprogram.h"
#include "wlgaxislabel.h"
#include "wlevscript.h"

void movDialogToDesktop(QWidget *widget,QWidget *dialog);

class WLCorSFDialog: public QDialog
{
Q_OBJECT

public:

explicit WLCorSFDialog(QWidget *parent):QDialog(parent)
    {
    set100 = new QToolButton (this);
    set100->setIcon(QPixmap(":/data/icons/100.png"));
    set100->setIconSize(QSize(32,16));

    slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);
    slider->setMinimumWidth(300);

    QToolButton *tbEnter = new QToolButton(this);
    tbEnter->setText("...");

    connect(tbEnter,&QPushButton::clicked,this,[=](){
    WLEnterNum  EnterNum (this);

    EnterNum.setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);
    EnterNum.setDecimals(m_scaleSlider/10);
    EnterNum.show();

    movDialogToDesktop(tbEnter,&EnterNum);

    EnterNum.setRange(m_min,m_max);
    EnterNum.setNow(m_value);
    EnterNum.selectAll();

    EnterNum.setLabel(tr("cor="));

    EnterNum.selectAll();

    if(EnterNum.exec())  setValue(EnterNum.getNow());
    });
    //slider->setSingleStep(m_scaleSlider);

    setScaleSlider(m_scaleSlider);

    QHBoxLayout *hlayout = new QHBoxLayout;

    hlayout->addWidget(set100);
    hlayout->addWidget(tbEnter);
    hlayout->addWidget(slider);
    setLayout(hlayout);

    connect(set100,&QToolButton::clicked,this,[=](){setValue(100);});
    connect(slider,&QSlider::valueChanged,this,[=](int _value){emit changedValue(m_value=_value/m_scaleSlider);});
    //connect(this,&WLCorSFDialog::changedValue,this,[=](double)QToolButton::clicked,this,[=](){emit changedValue(100);});
    }

void setScaleSlider(double scale)
    {
    m_scaleSlider=scale;

    slider->setRange(m_min*m_scaleSlider,m_max*m_scaleSlider);
    slider->setSingleStep(m_scaleSlider);

    update();
    }

private:
QToolButton *set100=nullptr;
QSlider *slider=nullptr;

double m_min=0;
double m_max=100;
double m_value=0;

double m_scaleSlider=100;

public slots:
void update() {slider->setValue(m_value*m_scaleSlider); }

void setRange(double _min,double _max) {
                                       if(_min<_max){
                                        m_min=_min;
                                        m_max=_max;
                                        setScaleSlider(m_scaleSlider);
                                        }
                                       }

void setValue(double _val) {
                           m_value=qBound(m_min,_val,m_max);
                           emit changedValue(m_value);
                           update();
                           }


signals:
void changedValue(double);

public slots:

 //void setRange()
};

class WLValueLabel: public QLabel
{
Q_OBJECT

public:   WLValueLabel(QWidget *parent):QLabel(parent)
          {

          }

void setRange(double min,double max)  {if(min<max) {m_minimum=min;m_maximum=max; setValue(m_value);}}

void setSuffix(QString _suffix) {m_suffix=_suffix; update();}
void setPrefix(QString _prefix) {m_prefix=_prefix; update();}

QString suffix() {return m_suffix;}
QString prefix() {return m_prefix;}

double value() {return m_value;}
private:
QString m_prefix;
QString m_suffix;

double m_minimum=0;
double m_maximum=100;
double m_value=100;
   int m_prec=1;

signals:

 void doubleClicked();
 void valueChanged(double);

public slots:

 void update() {setText(m_prefix+QString::number(m_value,'f',m_prec)+m_suffix);}

 void setValue(double _val) {_val=qBound(m_minimum,_val,m_maximum);
                             if(m_value!=_val){
                              m_value=qBound(m_minimum,_val,m_maximum);
                              update();
                              emit valueChanged(value());
                              }
                            }


 // QWidget interface
protected:
 void mousePressEvent(QMouseEvent *event)
 {
 // Q_UNUSED(event);
  WLCorSFDialog dialog(this);

  dialog.setWindowFlags(Qt::Popup|Qt::FramelessWindowHint );
  dialog.setRange(m_minimum,m_maximum);
  dialog.setScaleSlider(10);
  dialog.setValue(m_value);

  dialog.show();
  movDialogToDesktop(this,&dialog);

  connect(&dialog,&WLCorSFDialog::changedValue,this,&WLValueLabel::setValue);

  dialog.exec();
  }

 void mouseDoubleClickEvent(QMouseEvent *event){Q_UNUSED(event); emit doubleClicked();}
};

class WLPositionWidget : public QWidget
{
	Q_OBJECT

public:
    WLPositionWidget(WLGMachine *_MillMachine,WLGProgram *_Program,QWidget *parent=0);
	~WLPositionWidget();

	// WLFrame getViewSC() {return MillMachine->GCode.getSC(iSC-1);};
 void setJogDistStr(QString);
 void setFperStr(QString);

 void addTopWidget(QWidget *widget) {ui.topLayout->addWidget(widget);}
// void addLeftWidget(QWidget *widget) {ui.leftLayout->addWidget(widget);}

private:
	Ui::WLPositionWidget ui;

	//int iSC;	
    bool disButton;
	
 WLGMachine *MillMachine;
 WLGProgram  *Program;

 char focusElement='F';

 double pressedManual=10;

signals:
     void changedViewSC(int);
     void changedHomePosition(WLFrame);
     void chnagedGPos(QString name,double pos);

private:
   WLGAxisLabel  *gALabelX=nullptr;
   WLGAxisLabel  *gALabelY=nullptr;
   WLGAxisLabel  *gALabelZ=nullptr;
   WLGAxisLabel  *gALabelA=nullptr;
   WLGAxisLabel  *gALabelB=nullptr;
   WLGAxisLabel  *gALabelC=nullptr;

   QToolButton *pbMinusX=nullptr;
   QToolButton *pbMinusY=nullptr;
   QToolButton *pbMinusZ=nullptr;
   QToolButton *pbMinusA=nullptr;
   QToolButton *pbMinusB=nullptr;
   QToolButton *pbMinusC=nullptr;

   QToolButton *pbPlusX=nullptr;
   QToolButton *pbPlusY=nullptr;
   QToolButton *pbPlusZ=nullptr;
   QToolButton *pbPlusA=nullptr;
   QToolButton *pbPlusB=nullptr;
   QToolButton *pbPlusC=nullptr;

   QList <QToolButton*> TBPDriveList;
   QList <QToolButton*> TBMDriveList;

   WLLabel *labelS;
   WLLabel *labelF;
   QLabel  *labelActivGCode;

   QDoubleSpinBox *sbFman;

   WLValueLabel *corFper;
   WLValueLabel *corSper;

   QToolButton *pbStart;

   QToolButton *pbFast;
   QToolButton *pbPause;
   QToolButton *pbStop;
   QToolButton *pbWL;

   QLabel *labelTypeManual;

   float m_stepSper=2.5;
   float m_stepFper=2.5;

   QStringList m_listManDist;
   int m_curIndexListMan=0;

   QSize m_buttonSize;

   bool m_showErrorPos=false;

private:

    void initElementControls();
    float calcStepMov();

    void setFocusElement(char f);

    void onTeachAxis(QString name);    
    void onEditPidAxis(WLAxis *axis);

    // QWidget interface
protected:

  void keyPressEvent ( QKeyEvent * event );
  void keyReleaseEvent ( QKeyEvent * event );
  void focusOutEvent ( QFocusEvent * event );
  void mousePressEvent(QMouseEvent *event);

private slots: 

     void updateProgress();
     void updateFlashButtons();

     void onSetSCor();
     void onClearSCorList();

     void on_pbFast_pressed();
     void on_pbFast_released();

     void on_pbPlusSper_pressed()  {corSper->setValue(corSper->value()+m_stepSper);}
     void on_pbMinusSper_pressed() {corSper->setValue(corSper->value()-m_stepSper);}

     void on_pbPlusFper_pressed()  {corFper->setValue(corFper->value()+m_stepFper);}
     void on_pbMinusFper_pressed() {corFper->setValue(corFper->value()-m_stepFper);}

     void on_pbPlus_pressed();
     void on_pbMinus_pressed();

     void onPBAxis(QString name,int rot,bool press);

     void updateFSLabel();

     void onExGCode();

     void updateEnableMoved(bool);
	 
     void onPBRotSK();
     void onPBsetG28();
     void onPBgetG28();
	 void onPBsetP0();
	 void onPBsetP1();

     void setPosition(QString nameDrive,float pos,int type);

     void onPBsetX(int type) {onPushDrive("X",type);}
     void onPBsetY(int type) {onPushDrive("Y",type);}
     void onPBsetZ(int type) {onPushDrive("Z",type);}
     void onPBsetA(int type) {onPushDrive("A",type);}
     void onPBsetB(int type) {onPushDrive("B",type);}

     void onPushDrive(QString nameDrive,int type);

public slots:

    void onPBRotSC();

    void setEditDisabled(bool dis)  {disButton=dis;}

    void updatePosition();



    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};


#endif // WLPOSITIONWIDGET_H
