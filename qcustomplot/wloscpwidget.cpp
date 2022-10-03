#include "wloscpwidget.h"
#include "ui_wloscpwidget.h"

WLOscpWidget::WLOscpWidget(WLModuleOscp *MOscp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLOscpWidget)
{
    ui->setupUi(this);

    mMOscp=MOscp;

    mGraph1 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 0));
    //mGraph2 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 1));
    //mGraph3 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 0));
    //mGraph4 = ui->plot->addGraph(ui->plot->xAxis, ui->plot->axisRect()->axis(QCPAxis::atRight, 0));

     mGraph1->setPen(QPen(QColor(250, 120, 0)));

     connect(ui->pbRun,&QPushButton::toggled,this,&WLOscpWidget::onPBRun);
     connect(mMOscp,&WLModuleOscp::changedValues,this,&WLOscpWidget::addData,Qt::ConnectionType::QueuedConnection);

     connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
     connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
}

WLOscpWidget::~WLOscpWidget()
{
    delete ui;
}


void WLOscpWidget::addData(double time, QList<double> values)
{
mGraph1->addData(lastTime, values.first());
//mGraph2->addData(time->dataCount(), qCos(mGraph2->dataCount()/50.0)+qSin(mGraph2->dataCount()/50.0/0.4364)*0.15);

ui->plot->xAxis->rescale();

mGraph1->rescaleValueAxis(false, false);

ui->plot->xAxis->setRange(ui->plot->xAxis->range().upper, 3, Qt::AlignRight);

ui->plot->replot();
//qDebug()<<"WLOscpWidget::addData"<<lastTime<<time<<values.first();

lastTime+=time;
}

void WLOscpWidget::onPBRun(bool press)
{
if(press) {
 //mMOscp->setSourceChannel(0,WLModule::typeMIOPut,WLElement::typeEInput,2,(quint8)WLIOPut::dataIOPut_now);
    mMOscp->setSourceChannel(0,WLModule::typeMAxis,WLElement::typeEAxis,1,(quint8)WLAxis::dataAxis_F);
 mMOscp->setRun();
 }
 else{
 mMOscp->setRun(false);
 }
}

void WLOscpWidget::horzScrollBarChanged(int value)
{
if (qAbs(ui->plot->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
  ui->plot->xAxis->setRange(value/100.0, ui->plot->xAxis->range().size(), Qt::AlignCenter);
  ui->plot->replot();
  }
}

void WLOscpWidget::xAxisChanged(QCPRange range)
{
  ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
  ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}
