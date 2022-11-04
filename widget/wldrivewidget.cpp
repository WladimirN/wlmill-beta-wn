#include "wldrivewidget.h"
#include "wlenternum.h"

WLDriveWidget::WLDriveWidget(WLDrive *_Drive,QWidget *parent)
	: QDialog(parent)
{	
    m_Drive=_Drive;

    m_ddim=m_Drive->getDriveDim();

    ui.setupUi(this);

    ui.lineEditAxis->setText(m_Drive->getIndexModuleAxisStr());

    m_PamTab = new WLPamListWidget(m_Drive,this);

    connect(this,SIGNAL(changedUnit(QString)),m_PamTab,SLOT(setUnit(QString)));

    addTabWidget(m_PamTab,tr("motion parametrs"));

    ui.cbTypeDim->addItems(QString(tr("step on size,one step size,ratio A/B")).split(","));
    ui.cbTypeDim->setCurrentIndex(static_cast<int>(m_ddim.type));

    ui.sbDimA->setValue(m_ddim.A);
    ui.sbDimB->setValue(m_ddim.B);

    updateCBDimm(static_cast<int>(m_ddim.type));

    connect(ui.cbTypeDim,SIGNAL(activated(int)),SLOT(updateCBDimm(int)));

    ui.sbPLIM->setValue(m_Drive->maxPosition());
    ui.sbMLIM->setValue(m_Drive->minPosition());

    ui.comboBoxLogicFind->addItem(tr("no Find"),WLDrive::noFind);
    ui.comboBoxLogicFind->addItem(tr("only ORG"),WLDrive::onlyORG);
    ui.comboBoxLogicFind->addItem(tr("only PORG"),WLDrive::onlyPORG);
    ui.comboBoxLogicFind->addItem(tr("only MORG"),WLDrive::onlyMORG);
    ui.comboBoxLogicFind->addItem(tr("only PORG back"),WLDrive::onlyPORGHome);
    ui.comboBoxLogicFind->addItem(tr("only MORG back"),WLDrive::onlyMORGHome);
    ui.comboBoxLogicFind->addItem(tr("only PEL"),WLDrive::onlyPEL);
    ui.comboBoxLogicFind->addItem(tr("only MEL"),WLDrive::onlyMEL);
    ui.comboBoxLogicFind->addItem(tr("only PEL back"),WLDrive::onlyPELHome);
    ui.comboBoxLogicFind->addItem(tr("only MEL back"),WLDrive::onlyMORGHome);

    connect(ui.comboBoxLogicFind,SIGNAL(currentIndexChanged(int)),SLOT(updateFindLogic(int)));

    ui.comboBoxLogicFind->setCurrentIndex(0);

    for(int i=0;i<ui.comboBoxLogicFind->count();i++) {
      WLDrive::typeLogiFind curType=m_Drive->getLogicFindPos();

      if(ui.comboBoxLogicFind->itemData(i).toInt()==curType){
         ui.comboBoxLogicFind->setCurrentIndex(i);
         break;
         }
      }

    connect(ui.pbCorrectStepSize,&QPushButton::clicked,this,&WLDriveWidget::onCorrectStepSize);

    ui.sbOrgSize->setValue(m_Drive->getORGSize());

    ui.sbBackFindPosition->setValue(m_Drive->getHomePosition());
    ui.sbOrgPosition->setValue(m_Drive->getOrgPosition());

    //ui.sbVfind->setRange(0.0001,Drive->Pad->getData("main").Vma);
    connect(ui.sbVfind1,SIGNAL(valueChanged(double)),SLOT(updateLabelSDDist()));
    connect(ui.tabWidget,SIGNAL(currentChanged(int)),SLOT(updateLabelSDDist()));

    ui.sbVfind1->setValue(m_Drive->getVFind1());
    ui.sbVfind2->setValue(m_Drive->getVFind2());

    connect(ui.cbTypeDrive,SIGNAL(currentIndexChanged(int)),this,SLOT(updateUnit()));

    ui.cbTypeDrive->setCurrentIndex(m_Drive->getType());

    ui.sbBackDist->setValue(m_Drive->getBackDistFind());
    ui.gbLimit->setChecked(!m_Drive->isInfinity());

	connect(ui.pbVerError,SIGNAL(clicked()),SLOT(onVerifyError()));

    connect(ui.pbApplyAxis,&QPushButton::clicked,this,&WLDriveWidget::updateTabAxis);
    connect(ui.lineEditAxis,&QLineEdit::textChanged,[=](){ui.pbApplyAxis->setEnabled(true);});

    connect(ui.sbBackDist,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,&WLDriveWidget::updateF2);

    setModal(true);

    setWindowTitle(windowTitle()+" "+m_Drive->getName());

    setWindowTitle(tr("Edit Drive: ")+m_Drive->getName());

    updateF2();
    updateFindLogic(m_Drive->getLogicFindPos());
    updateUnit();
    updateTabAxis();
}

WLDriveWidget::~WLDriveWidget()
{

}


void WLDriveWidget::updateLabelSDDist()
{
double V=m_Fminutes ? ui.sbVfind1->value() / 60.0 : ui.sbVfind1->value();

dataPad Pad=m_Drive->pad()->getData("main");
double ScurveMs = axisWidgetList.isEmpty() ? 0 : axisWidgetList.first()->geDelaytSCurveMs();

ui.labelSDDist->setText(QString(tr("%1")+m_unit).arg(((Pad.Vst-V)/Pad.Ade+ScurveMs*1000)*(V+Pad.Vst)/2,0,'f',2));
}

QString WLDriveWidget::verifyError()
{
QString str;

foreach(WLAxisWidget *AW,axisWidgetList)
{
if(!AW->isUniqueInputs())
    str+=tr("input numbers are not unique")+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";

if(!AW->isUniqueOutputs())
    str+=tr("output numbers are not unique")+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";

}

if(ui.sbMLIM->value()>=ui.sbPLIM->value()
 &&ui.gbLimit->isChecked())
 str+=tr("invalid movement limits")+"\n";
else
 {
 if((ui.sbBackFindPosition->value()<ui.sbMLIM->value()
   ||ui.sbBackFindPosition->value()>ui.sbPLIM->value())&&(ui.comboBoxLogicFind->currentIndex()>3))
  str+=tr("invalid base position")+"\n";
 }

WLDrive::typeLogiFind curTypeLF=static_cast<WLDrive::typeLogiFind>(ui.comboBoxLogicFind->currentData().toInt());

const QString noAction=tr("no sensor installed action");
const QString noInput=tr("no sensor installed");

switch(curTypeLF)
{
case WLDrive::onlyPORG:
case WLDrive::onlyMORG:
case WLDrive::onlyPORGHome:
case WLDrive::onlyMORGHome:
           foreach(WLAxisWidget *AW,axisWidgetList)
               {
               if(AW->getActInORG()==WLIOPut::INPUT_actNo)
                   str+=tr("no sensor installed action")+" (inORG)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";
               }



case  WLDrive::onlyORG:  //ORG
         foreach(WLAxisWidget *AW,axisWidgetList)
             {
             if(AW->getIndexInORG()<2)
                 str+=tr("no sensor installed to search")+" (inORG)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";
             }

           if(ui.sbOrgSize->value()==0&&curTypeLF==WLDrive::onlyORG)
           str+=tr("not specified size ORG")+"\n";
           break;


case WLDrive::onlyPEL:    //PEL
case WLDrive::onlyPELHome:
       foreach(WLAxisWidget *AW,axisWidgetList)
         {
         if(AW->getIndexInPEL()<2)
             str+=tr("no sensor installed to search")+" (inPEL)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";

         if(AW->getActInPEL()==WLIOPut::INPUT_actNo)
             str+=tr("no sensor installed action")+" (inPEL)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";
         }

        break;

case WLDrive::onlyMEL:     //MEL
case WLDrive::onlyMELHome:
       foreach(WLAxisWidget *AW,axisWidgetList)
        {
        if(AW->getIndexInMEL()<2)
            str+=tr("no sensor installed to search")+" (inMEL)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";

        if(AW->getActInMEL()==WLIOPut::INPUT_actNo)
            str+=tr("no sensor installed action")+" (inMEL)"+QString("(Axis-%1").arg(AW->getAxis()->getIndex())+")\n";
        }

        break;
}

if(curTypeLF!=WLDrive::noFind&&ui.sbVfind1->value()<=0)
    str+=tr("search speed not set")+" V1"+"\n";

if((curTypeLF==WLDrive::onlyPEL
  ||curTypeLF==WLDrive::onlyMEL
  ||curTypeLF==WLDrive::onlyPELHome
  ||curTypeLF==WLDrive::onlyMELHome
  ||curTypeLF==WLDrive::onlyPORG
  ||curTypeLF==WLDrive::onlyMORG
  ||curTypeLF==WLDrive::onlyPORGHome
  ||curTypeLF==WLDrive::onlyMORGHome)
    &&
   ui.sbVfind1->value()<=0)
str+=tr("search speed not set")+"\n";

return str;
}

void WLDriveWidget::setFminutes(bool Fminutes)
{
if(m_Fminutes!=Fminutes) {
    m_Fminutes=Fminutes;

    if(m_Fminutes) {
    ui.sbVfind1->setValue(m_Drive->getVFind1()*60.0);
    ui.sbVfind2->setValue(m_Drive->getVFind2()*60.0);
    }else {
    ui.sbVfind1->setValue(m_Drive->getVFind1());
    ui.sbVfind2->setValue(m_Drive->getVFind2());
    }

m_PamTab->setFminutes(Fminutes);

foreach(WLAxisWidget *AW,axisWidgetList){
  AW->setFminutes(Fminutes);
  }

}

ui.sbVfind1->setSuffix(m_unit+"/"+(m_Fminutes? tr("m"):tr("s")));
ui.sbVfind2->setSuffix(m_unit+"/"+(m_Fminutes? tr("m"):tr("s")));
}

void WLDriveWidget::onVerifyError()
{
QString str=verifyError();

if(str.isEmpty()) str=tr("No error!!!");

QMessageBox::information(this, tr("Verify error"),str,QMessageBox::Ok);
}

void WLDriveWidget::onCorrectStepSize()
{
double R,T;
WLEnterNum EN(this);
EN.setMinMaxNow(0.1,999999,90);
EN.setLabel(tr("Enter real distance(measure)"));

EN.show();
if(EN.exec())
  {
  R=EN.getNow();

  EN.setLabel(tr("Enter calc distance(display)"));
  EN.show();
   if(EN.exec())
   {
   T=EN.getNow();

   m_ddim.set(static_cast<WLDriveDim::typeDim>(ui.cbTypeDim->currentIndex())
             ,ui.sbDimA->value()
             ,ui.sbDimB->value());

   switch(ui.cbTypeDim->currentIndex())
   {
   case WLDriveDim::typeDim::ratio:        ui.sbDimA->setValue(R/T*ui.sbDimA->value());break;
   case WLDriveDim::typeDim::oneStepSize:  ui.sbDimA->setValue(R/T*ui.sbDimA->value());break;
   case WLDriveDim::typeDim::stepPerSize:  ui.sbDimA->setValue(T/R*ui.sbDimA->value());break;
   }

   }
  }
}

void WLDriveWidget::accept()
{
uint8_t iA=0;

if(!verifyError().isEmpty())
    onVerifyError();
 else
  {
  saveDataDrive();

  for(int i=1;i<ui.tabWidget->count();i++)    {
    QDialog *Dialog=static_cast<QDialog*>(ui.tabWidget->widget(i));

    //qDebug()<<Dialog->metaObject()->className();

    if(Dialog->metaObject()->className()==(tr("WLAxisWidget")))
       {
       m_Drive->setOffsetAxis(iA,static_cast<WLAxisWidget*>(Dialog)->getOffset());
       iA++;

       (static_cast<WLAxisWidget*>(Dialog))->setStepSize(m_Drive->getDriveDim().value);
       }

    Dialog->accept();
    }

  QDialog::accept();
}
}

void WLDriveWidget::keyPressEvent(QKeyEvent *event)
{
event->accept();
}

void WLDriveWidget::updateCBDimm(int index)
{
m_ddim.set(m_ddim.type,ui.sbDimA->value(),ui.sbDimB->value());

WLDriveDim::typeDim tdim=static_cast<WLDriveDim::typeDim>(index);

if(tdim!=m_ddim.type)
switch(tdim)
{
case WLDriveDim::typeDim::stepPerSize: m_ddim.set(tdim,1/m_ddim.valueReal);         break;
case WLDriveDim::typeDim::oneStepSize: m_ddim.set(tdim,m_ddim.valueReal);           break;
case WLDriveDim::typeDim::ratio:       m_ddim.set(tdim,m_ddim.valueReal*1000,1000); break;
}

ui.sbDimB->setVisible(tdim==WLDriveDim::typeDim::ratio);

ui.sbDimA->setValue(m_ddim.A);
ui.sbDimB->setValue(m_ddim.B);
}

void WLDriveWidget::updateCBTypePulse(int index)
{

}

void WLDriveWidget::updateFindLogic(int index)
{
WLDrive::typeLogiFind type=static_cast<WLDrive::typeLogiFind>(ui.comboBoxLogicFind->currentData().toInt());

if(type==WLDrive::onlyORG
 ||type==WLDrive::onlyORGHome
 ||type==WLDrive::onlyPORG
 ||type==WLDrive::onlyMORG
 ||type==WLDrive::onlyPORGHome
 ||type==WLDrive::onlyMORGHome)
    ui.sbOrgSize->setEnabled(true);
else
    ui.sbOrgSize->setEnabled(false);


if(type==WLDrive::onlyPORGHome
 ||type==WLDrive::onlyMORGHome
 ||type==WLDrive::onlyPELHome
 ||type==WLDrive::onlyMELHome)
     ui.sbBackFindPosition->setEnabled(true);
   else
     ui.sbBackFindPosition->setEnabled(false);

ui.sbVfind1->setEnabled(type!=WLDrive::noFind);

ui.sbVfind2->setEnabled(type!=WLDrive::noFind
                      &&type!=WLDrive::onlyORG
                      &&type!=WLDrive::onlyORGHome);

ui.sbBackDist->setEnabled(type!=WLDrive::noFind
                        &&type!=WLDrive::onlyORG
                        &&type!=WLDrive::onlyORGHome);

switch(type)
{
case WLDrive::noFind:  ui.labelOrgPosition->setText(tr("position"));break;

case WLDrive::onlyORG:
case WLDrive::onlyORGHome:
case WLDrive::onlyPORG:
case WLDrive::onlyMORG:
case WLDrive::onlyPORGHome:
case WLDrive::onlyMORGHome: ui.labelOrgPosition->setText("inORG "+tr("position"));break;

case WLDrive::onlyPEL:
case WLDrive::onlyPELHome: ui.labelOrgPosition->setText("inPEL "+tr("position"));break;

case WLDrive::onlyMEL:
case WLDrive::onlyMELHome: ui.labelOrgPosition->setText("inMEL "+tr("position"));break;
}


}

void WLDriveWidget::saveDataDrive()
{
m_Drive->setKGear(1);

m_Drive->setDimension(static_cast<WLDriveDim::typeDim>(ui.cbTypeDim->currentIndex())
                     ,ui.sbDimA->value()
                     ,ui.sbDimB->value());

m_Drive->setLogicFindPos(static_cast<WLDrive::typeLogiFind>(ui.comboBoxLogicFind->currentData().toInt()));

m_Drive->setORGSize(ui.sbOrgSize->value());
m_Drive->setHomePosition(ui.sbBackFindPosition->value());
m_Drive->setOrgPosition(ui.sbOrgPosition->value());

m_Drive->setVFind1(m_Fminutes ? ui.sbVfind1->value() / 60 : ui.sbVfind1->value());
m_Drive->setVFind2(m_Fminutes ? ui.sbVfind2->value() / 60 : ui.sbVfind2->value());
m_Drive->setBackDistFind(ui.sbBackDist->value());


m_Drive->setType(static_cast<WLDrive::typeDrive>(ui.cbTypeDrive->currentIndex()));
m_Drive->setInfinity(!ui.gbLimit->isChecked());
m_Drive->setMinMaxPosition(ui.sbMLIM->value(),ui.sbPLIM->value());
}

void WLDriveWidget::updateF2()
{
ui.sbVfind2->setEnabled(ui.sbBackDist->value()>0.0);
}

void WLDriveWidget::updateUnit()
{
if(ui.cbTypeDrive->currentIndex()==0)
 {
 m_unit=tr(" mm");
 }
else
 {
 m_unit=tr(" gr");
 }

ui.sbBackFindPosition->setSuffix(m_unit);
ui.sbOrgPosition->setSuffix(m_unit);
ui.sbOrgSize->setSuffix(m_unit);
ui.sbMLIM->setSuffix(m_unit);
ui.sbPLIM->setSuffix(m_unit);

setFminutes(m_Fminutes);

updateLabelSDDist();

emit changedUnit(m_unit);
}

void WLDriveWidget::updateTabAxis()
{
foreach(WLAxisWidget *tw,axisWidgetList) {
 ui.tabWidget->removeTab(ui.tabWidget->indexOf(tw));
 }

axisWidgetList.clear();

m_Drive->setIndexModuleAxisStr(ui.lineEditAxis->text());

WLDriveDim dim;

dim.set(static_cast<WLDriveDim::typeDim>(ui.cbTypeDim->currentIndex())
        ,ui.sbDimA->value()
        ,ui.sbDimB->value());

for(uint8_t i=0;i<m_Drive->getAxisList().size();i++)
{
WLAxisWidget *AW=new WLAxisWidget(m_Drive->getAxisList().at(i),i!=0,m_Drive->getOffsetAxis(i),this);

AW->setStepSize(dim.value);
AW->setUnit(m_unit);
AW->setFminutes(m_Fminutes);

insertTabWidget(1+i,AW,("Axis-")+QString::number(m_Drive->getAxisList().at(i)->getIndex()));

connect(this,&WLDriveWidget::changedUnit,AW,&WLAxisWidget::setUnit);

axisWidgetList+=AW;
}

ui.pbApplyAxis->setDisabled(true);
}
