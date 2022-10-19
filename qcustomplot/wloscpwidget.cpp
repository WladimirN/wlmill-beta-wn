#include "wloscpwidget.h"
#include "ui_wloscpwidget.h"

WLOscpWidget::WLOscpWidget(WLModuleOscp *MOscp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLOscpWidget)
{
    ui->setupUi(this);

    mMOscp=MOscp;


    connect(ui->pbRun,&QPushButton::toggled,this,&WLOscpWidget::onPBRun);
    connect(mMOscp,&WLModuleOscp::changedValues,this,&WLOscpWidget::addData,Qt::ConnectionType::QueuedConnection);

    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

     //ui->plot->setBackground(QBrush(Qt::black));

     for(quint8 i=0;i<mMOscp->getSizeOscp();i++)
     {
     WLChOscpWidget *chw=new WLChOscpWidget(mMOscp,i,this);

     QPointer<QCPGraph> Graph = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, i));
     QPen pen;
     pen.setWidth(3);

     Graph->setPen(pen);

     switch (i) {
       case 0:Graph->setPen(QPen(QColor(250, 120, 0)));  break;
       case 1:Graph->setPen(QPen(QColor(120, 250, 0)));  break;
       case 2:Graph->setPen(QPen(QColor(250, 250, 0)));  break;
       case 3:Graph->setPen(QPen(QColor(120, 120, 250)));break;
       }

     mGraphs<<Graph;
     chWidgets<<chw;

     ui->layoutChannels->addWidget(chw);

     if(i!=0)
         chw->setDisabled(true);
     }

     ui->cBoxPeriod->addItem("50"+tr("ms"),0.05);
     ui->cBoxPeriod->addItem("100"+tr("ms"),0.1);
     ui->cBoxPeriod->addItem("200"+tr("ms"),0.2);
     ui->cBoxPeriod->addItem("500"+tr("ms"),0.5);
     ui->cBoxPeriod->addItem("1000"+tr("ms"),1);
     ui->cBoxPeriod->addItem("2000"+tr("ms"),2);
     ui->cBoxPeriod->addItem("5000"+tr("ms"),5);
     ui->cBoxPeriod->addItem("10000"+tr("ms"),10);

     connect(ui->cBoxPeriod,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
     period=ui->cBoxPeriod->currentData().toDouble();
     });


     ui->cBoxPeriod->setCurrentIndex(3);
}

WLOscpWidget::~WLOscpWidget()
{
    delete ui;
}


void WLOscpWidget::addData(double time, QList<double> values)
{
if(values.size()!=mGraphs.size()
 ||!ui->pbRun->isChecked()){
 //ui->plot->xAxis->rescale();
 //ui->plot->xAxis->setRange(0,ui->plot->xAxis->range().size(), Qt::AlignRight);
 //ui->plot->replot();
 return;
 }

for(int i=0;i<mGraphs.size()&&i<values.size();i++){
  mGraphs[i]->addData(lastTime, values[i]);
  mGraphs[i]->rescaleAxes(true);
  }

bool ok;


//ui->plot->xAxis->rescale();
//ui->plot->yAxis->rescale();

ui->plot->yAxis->setRange(mGraphs.first()->getValueRange(ok));

if(lastTime>period*5)
    ui->plot->xAxis->setRange(ui->plot->xAxis->range().upper, period*5, Qt::AlignRight);
else
    ui->plot->xAxis->setRange(0,period*5);

ui->horizontalScrollBar->setRange(0+period*5*100/2,lastTime*100.0-period*5*100/2); // adjust size of scroll bar slider
ui->horizontalScrollBar->setValue(lastTime*100.0-period*5*100/2);

ui->plot->replot();
//qDebug()<<"WLOscpWidget::addData"<<lastTime<<time<<values.first();

lastTime+=time;

ui->labelTime->setText(tr("Time:")+QString::number(lastTime,'f',3));

if(lastTime>5*60)
  ui->pbRun->click();

}


void WLOscpWidget::onPBRun(bool press)
{
if(press) {
 mMOscp->setRun();

 for(int i=0;i<mGraphs.size();i++){
   mGraphs[i]->setData(QVector <double>(),QVector <double>());
   }

 lastTime=0;
 }
 else{
 mMOscp->setRun(false);
 ui->plot->replot();
 }
}

void WLOscpWidget::horzScrollBarChanged(int value)
{

//if (qAbs(ui->plot->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
  ui->plot->xAxis->setRange(value/100.0, period*5, Qt::AlignCenter);
  ui->plot->replot();
  }

}

void WLOscpWidget::xAxisChanged(QCPRange range)
{
  //qDebug()<<"xAxisChanged(QCPRange range)"<<range.center()*100.0<<range.size()*100.0;
  //ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
  //ui->horizontalScrollBar->setPage(qRound(range.size()*100.0)); // adjust size of scroll bar slider

}
