#include "wlpamwidget.h"
#include <QDebug>
#include <QTimer>

WLPamWidget::WLPamWidget(dataPad Pad,float Vmax,QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

    curPad=Pad;
	qDebug()<<Pad.toString();
	qDebug()<<"Vmax"<<Vmax;

    //ui.sbVst->setRange(0,Vmax/2);
	//ui.sbVma->setRange(Vmax/2,Vmax);	
	
    ui.sbAac->setRange(0,9999999999);
    ui.sbAde->setRange(0,9999999999);

    ui.cbEditName->insertItems(0,QString("main,mainMinus,manual").split(","));
    ui.cbEditName->lineEdit()->setText(Pad.name);
    //ui.cbSize->addItems(QString("1/sec^2,sec").split(","));

    //connect(ui.cbSize,SIGNAL(currentIndexChanged(int)),SLOT(updateAccelLabel(int)));

	connect(ui.buttonBox,SIGNAL(accepted()),SLOT(onAccept()));
	connect(ui.buttonBox,SIGNAL(rejected()),SLOT(onReject()));

    connect(ui.cbATime,&QCheckBox::toggled   ,this,&WLPamWidget::updateAtime);
    connect(ui.cbFMinutes,&QCheckBox::toggled,this,&WLPamWidget::updateFminutes);

    QTimer *timer=new QTimer;

    connect(timer,SIGNAL(timeout()),SLOT(repaint()));
    timer->start(100);

    setFminutes(false);

    setUnit("1");

    updateFminutes();
    updateAtime();

    setModal(true);

}

WLPamWidget::~WLPamWidget()
{

}

void WLPamWidget::updateVstLabel(double val)
{
ui.sbVma->setRange(val,ui.sbVma->maximum());
}

void WLPamWidget::updateVmaLabel(double val)
{
    ui.sbVst->setRange(0,val);
}

void WLPamWidget::setUnit(QString unit)
{
m_unit=unit;

ui.labelVst->setText("Vst("+m_unit+"/"+(ui.cbFMinutes->isChecked() ? tr("min)"):tr("sec)")));
ui.labelVma->setText("Vma("+m_unit+"/"+(ui.cbFMinutes->isChecked() ? tr("min)"):tr("sec)")));

if(ui.cbATime->isChecked()){
 ui.labelAac->setText("Aac(sec)");
 ui.labelAde->setText("Ade(sec)");
 }
 else {
 ui.labelAac->setText("Aac("+m_unit+"/"+tr("sec^2)"));
 ui.labelAde->setText("Ade("+m_unit+"/"+tr("sec^2)"));
 }
}

void WLPamWidget::updateAccelLabel(int index)
{/*
if(index)
{
ui.sbAac->setValue((ui.sbVma->value()-ui.sbVst->value())/ui.sbAac->value());
ui.sbAde->setValue((ui.sbVma->value()-ui.sbVst->value())/ui.sbAde->value());
}
else
{
if(ui.sbAac->value()==0)
   ui.sbAac->setValue((ui.sbVma->value()-ui.sbVst->value())/0.000001);
else
   ui.sbAac->setValue((ui.sbVma->value()-ui.sbVst->value())/ui.sbAac->value());

if(ui.sbAde->value()==0)
   ui.sbAde->setValue((ui.sbVma->value()-ui.sbVst->value())/0.000001);
else
   ui.sbAde->setValue((ui.sbVma->value()-ui.sbVst->value())/ui.sbAde->value());
}
*/
}

dataPad WLPamWidget::getPad()
{
dataPad Pad;

Pad.Vst= Pad.Vst == 0.0 ? 0.0 :(ui.cbFMinutes->isChecked() ? ui.sbVst->value()/60.0 : ui.sbVst->value());
Pad.Vma= Pad.Vma == 0.0 ? 0.0 :(ui.cbFMinutes->isChecked() ? ui.sbVma->value()/60.0 : ui.sbVma->value());

Pad.Aac= Pad.Aac == 0.0 ? 0.0 :(ui.cbATime->isChecked()    ?  (Pad.Vma-Pad.Vst)/ui.sbAac->value() :  ui.sbAac->value());
Pad.Ade= Pad.Ade == 0.0 ? 0.0 :(ui.cbATime->isChecked()    ? -(Pad.Vma-Pad.Vst)/ui.sbAde->value() : -ui.sbAde->value());

Pad.name=ui.cbEditName->currentText();

return Pad;
}

void WLPamWidget::updateFminutes()
{
if(ui.cbFMinutes->isChecked()){
 ui.sbVst->setValue(curPad.Vst*60.0);
 ui.sbVma->setValue(curPad.Vma*60.0);
 }
 else{
 ui.sbVst->setValue(curPad.Vst);
 ui.sbVma->setValue(curPad.Vma);
 }

setUnit(m_unit);
}

void WLPamWidget::updateAtime()
{
if(ui.cbATime->isChecked()){
  ui.sbAac->setValue(curPad.Aac == 0.0 ? 0.0 : (curPad.Vma-curPad.Vst)/curPad.Aac);
  ui.sbAde->setValue(curPad.Ade == 0.0 ? 0.0 :-(curPad.Vma-curPad.Vst)/curPad.Ade);
  }
  else{
  ui.sbAac->setValue(curPad.Aac);
  ui.sbAde->setValue(-curPad.Ade);
  }

setUnit(m_unit);
}

void WLPamWidget::paintEvent(QPaintEvent *)
{
QPainter painter(this);

painter.setPen(Qt::blue);
painter.setFont(QFont("Arial", 30));

QPen penFocus;
QPen pen;

pen.setWidthF(2);
pen.setColor(QColor(Qt::blue));

penFocus.setWidthF(3);
penFocus.setColor(QColor(Qt::green));

QPoint point,pointLast;
QList <QPoint> pointList;
float kA=1;
float kD=1;

if(ui.sbVst->value()>ui.sbVma->value()) ui.sbVst->setValue(ui.sbVma->value());

if(ui.sbAac->value()==0.0
 ||ui.sbAde->value()==0.0)  {
  kA=ui.sbAac->value()==0.0 ? 0 : 1;
  kD=ui.sbAde->value()==0.0 ? 0 : 1;
  }
  else {
       if(ui.sbAac->value()>ui.sbAde->value()) {
         kA=ui.sbAac->value()==0.0 ? 1 : ui.sbAde->value()/ui.sbAac->value();
         }
         else{
         kD=ui.sbAde->value()==0.0 ? 1: ui.sbAac->value()/ui.sbAde->value();
         }

       if(ui.cbATime->isChecked()) {
         float K;
         K=kA;
         kA=kD;
         kD=K;
         }
        }

if(ui.cbEditName->hasFocus()) pen=penFocus;

painter.setPen(pen);

pointLast.setX(0);
pointLast.setY(ui.sbVst->pos().y()-10);
point=pointLast;

point.setX(ui.sbVst->pos().x()+ui.sbVst->size().width()
           +(1-kA)*ui.sbAac->size().width());

painter.drawLine(pointLast,point);
pointLast=point;
point=pointLast;

if(ui.sbVst->value()!=0.0)
 {
 painter.setPen(ui.sbVst->hasFocus() ? penFocus:pen);
 point.setY(ui.sbVst->pos().y()-10-(ui.sbVst->pos().y()-20)*(ui.sbVst->value()/ui.sbVma->value()));
 painter.drawLine(pointLast,point);
 pointLast=point;
 }

point.setX(ui.sbAac->pos().x()+ui.sbAac->size().width());
point.setY(10);
painter.setPen(ui.sbAac->hasFocus() ? penFocus:pen);
painter.drawLine(pointLast,point);
pointLast=point;

point.setX(ui.sbVma->pos().x()+ui.sbVma->size().width());
painter.setPen(ui.sbVma->hasFocus() ? penFocus:pen);
painter.drawLine(pointLast,point);
pointLast=point;

point.setX(ui.sbAde->pos().x()+ui.sbAac->size().width()
          -(1-kD)*ui.sbAac->size().width());

if(ui.sbVst->value()!=0.0)
  point.setY(ui.sbVst->pos().y()-10-(ui.sbVst->pos().y()-20)*(ui.sbVst->value()/ui.sbVma->value()));
else
  point.setY(ui.sbVst->pos().y()-10);

painter.setPen(ui.sbAde->hasFocus() ? penFocus:pen);
painter.drawLine(pointLast,point);
pointLast=point;

if(ui.sbVst->value()!=0.0)
  {
  painter.setPen(ui.sbVst->hasFocus() ? penFocus:pen);
  point.setY(ui.sbVst->pos().y()-10);
  painter.drawLine(pointLast,point);
  pointLast=point;
  }

painter.setPen(pen);
point.setX(this->size().width());
painter.drawLine(pointLast,point);

}

