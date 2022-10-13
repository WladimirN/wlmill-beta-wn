#include "wlchoscpwidget.h"
#include "ui_wlchoscpwidget.h"
#include <QMetaEnum>
#include <QSpinBox>
#include "wlmoduleioput.h"
#include "wlelement.h"
#include "wlaxis.h"

WLChOscpWidget::WLChOscpWidget(WLModuleOscp *_module,quint8 _iCh,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLChOscpWidget)
{
    ui->setupUi(this);

    m_module=_module;
    m_iCh=_iCh;

    connect(ui->cbElement,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WLChOscpWidget::updateIElement);

    ui->groupBox->setTitle(tr("Channel")+" "+QString::number(m_iCh));

    connect(m_module,&WLModuleOscp::changedSource,this,&WLChOscpWidget::setSource);

    setSource(m_module->getSource());

    updateElement();

    connect(ui->cbElement,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WLChOscpWidget::updateSource);
    connect(ui->cbTypeData,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&WLChOscpWidget::updateSource);
    connect(ui->sbIndex,QOverload<int>::of(&QSpinBox::valueChanged),this,&WLChOscpWidget::updateSource);

    updateSource();
}

WLChOscpWidget::~WLChOscpWidget()
{
    delete ui;
}

bool WLChOscpWidget::isChecked()
{
return  ui->groupBox->isChecked();
}

void WLChOscpWidget::updateSource()
{
WLElement::typeElement etype=static_cast<WLElement::typeElement>(ui->cbElement->currentData().toInt());
quint8 index=ui->sbIndex->value();
quint8 tdata=ui->cbTypeData->currentData().toInt();

if(ui->groupBox->isChecked())
   m_module->setSourceChannel(m_iCh,etype,index,tdata);
}

void WLChOscpWidget::updateIElement()
{
WLElement::typeElement etype=static_cast<WLElement::typeElement>(ui->cbElement->currentData().toInt());

WLMotion *device=static_cast<WLMotion*>(m_module->getDevice());

if(device)
{
ui->sbIndex->setEnabled(true);
 switch(etype)
 {
 case WLElement::typeEInput: ui->sbIndex->setRange(0,device->getModuleIOPut()->getSizeInputs());
                             break;
 case WLElement::typeEOutput:ui->sbIndex->setRange(0,device->getModuleIOPut()->getSizeOutputs());;
                             break;
 case WLElement::typeEAxis:  ui->sbIndex->setRange(0,device->getModuleAxis()->getSizeAxis());
                             break;

 default: ui->sbIndex->setEnabled(false);
 }
}
updateTypeData();
}

void WLChOscpWidget::updateElement()
{
ui->cbElement->clear();

QList<WLElement::typeElement> utypes;

foreach(WLSrcChOscp esrc, src){
if(utypes.indexOf(esrc.element)==-1)
   utypes<<esrc.element;
}

foreach(WLElement::typeElement type, utypes){
ui->cbElement->addItem(QString::fromUtf8(QMetaEnum::fromType<WLElement::typeElement>().valueToKey(type))
                      ,type);
}

updateIElement();
}

void WLChOscpWidget::updateTypeData()
{
ui->cbTypeData->clear();

QList<int> utdatas;

WLElement::typeElement etype=static_cast<WLModule::typeElement>(ui->cbElement->currentData().toInt());

foreach(WLSrcChOscp esrc, src){
 if(etype==esrc.element
  &&utdatas.indexOf(esrc.typeData)==-1)
    utdatas<<esrc.typeData;
}

foreach(int tdata,utdatas){
QString name;
  switch(etype)
  {
  case WLElement::typeEInput: name=QString::fromUtf8(QMetaEnum::fromType<WLIOPut::typeDataIOPut>().valueToKey(tdata));
                              break;
  case WLElement::typeEOutput:name=QString::fromUtf8(QMetaEnum::fromType<WLIOPut::typeDataIOPut>().valueToKey(tdata));
                              break;
  case WLElement::typeEAxis:  name=QString::fromUtf8(QMetaEnum::fromType<WLAxis::typeDataAxis>().valueToKey(tdata));
                              break;
  default: name="no initial";
  }

ui->cbTypeData->addItem(name,tdata);
}

}

void WLChOscpWidget::setSource(QList<WLSrcChOscp> _src)
{
src=_src;
updateElement();
}
