#include "wlgaxislabel.h"

WLGAxisLabel::WLGAxisLabel(QWidget *parent) : QWidget(parent)
{
setMinimumSize(200,20);

setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

setVisible(false);

setToolTip(tr("press for edit"));
}

void WLGAxisLabel::setDrive(WLGDrive *_drive)
{
//if(m_drive)
 m_drive=_drive;

 setVisible(m_drive ? true : false);

//connect()
}

void WLGAxisLabel::setGCode(WLGCode *_gcode)
{
m_gcode=_gcode;
}

void WLGAxisLabel::paintEvent(QPaintEvent *event)
{
QPainter painter(this);
QFont font;
QColor colorF=QColor(Qt::darkGreen);
painter.setPen(QColor(Qt::black));
//painter.drawRect(0,0,this->width()-1,this->height()-1);
int xF=0;
QString name=m_drive ? m_drive->getName() : "?";
QString  pos=m_drive ? QString("%1").arg(m_drive->getAxisPosition(),0,'f',3) : "0000.00";
QString    f=m_drive ? QString("%1").arg(m_drive->getVnow()*60,0,'f',xF) : "0000";

if(m_drive)
 while ((m_drive->getVnow()!=0.0)&&(f.toFloat()==0.0))
 {
 xF++;
 f=QString("%1").arg(m_drive->getVnow()*60,0,'f',xF);
 }

QString ofst;
  float Hofst=0;

if(m_gcode)
  {
  if(name=="X"
   ||name=="Y")
     {
     if(m_gcode->isGCode(41)) {
      f=QString(" +RL%1").arg(m_gcode->getValue('D'));
      }
     else if(m_gcode->isGCode(42)) {
       f=QString(" +RR%1").arg(m_gcode->getValue('D'));
       }
    }

    if(m_drive->isAuto()){
      ofst="auto";
      }
      else{
      ofst=QString("%1").arg(m_GPos,0,'f',3);
      }

   if(name=="Z")
      {
      if(m_gcode->isGCode(43))
        {        
        f=QString(" +H%1").arg(m_gcode->getValue('H'));
        colorF=QColor(Qt::red);
        }
        else if(m_gcode->isGCode(44))
          {
          f=QString(" -H%1").arg(m_gcode->getValue('H'));
          colorF=QColor(Qt::blue);
          }
      }
   }
else
   ofst=pos;

if(isChecked()) ofst.prepend("*");

rName.setX(0);
rName.setY(0);
rName.setHeight(this->height()-1);
rName.setWidth(this->height()*1.05);

if(m_drive->isEnable()){
   painter.setPen(QColor(m_drive->isAuto() ? Qt::green : Qt::black));
   }
   else{
   painter.setPen(Qt::gray);
   }

font=painter.font();
font.setPixelSize(rName.height());

QFontMetrics metric(font);

//rName.setWidth(metric.width(name+":"));

painter.setFont(font);
painter.drawText(rName,Qt::AlignVCenter|Qt::AlignHCenter,name+":");

painter.setPen(Qt::black);
painter.drawRoundedRect(0,0,this->width()-1,this->height()-1,3,3);
//painter.drawText(rF,Qt::AlignVCenter|Qt::AlignRight,"F:");

//painter.drawRect(rName);

rPos.setTopLeft(QPoint(this->width()-this->width()/4,(this->height()-2)/2));
rPos.setBottomRight(QPoint(this->width()-2,this->height()-2));

font.setPixelSize((rName.height())/2);

metric=QFontMetrics(font);

if(metric.width(pos)>rPos.width())
 {
 float k=((float)metric.width(pos)/rPos.width());
 font.setPixelSize(font.pixelSize()/k);
 }


painter.setFont(font);
painter.setPen(QColor(Qt::darkRed));
painter.drawText(rPos,Qt::AlignVCenter|Qt::AlignRight,pos);
//painter.drawRect(rPos);

rF.setBottomLeft(rPos.topLeft());
rF.setTopRight(QPoint(this->width()-2,0));

rOfst.setBottomLeft(rName.bottomRight());
rOfst.setTopRight(rF.topLeft());

painter.setPen(colorF);

painter.drawText(rF,Qt::AlignVCenter|Qt::AlignRight,f);

font.setPixelSize(rName.height()-2);

//metric=QFontMetrics(font);

if(metric.width(ofst)>rPos.width())
 {
 float k=((float)metric.width(ofst)/rPos.width());

 font.setPixelSize(font.pixelSize()/k);
 }


painter.setFont(font);
if(m_drive){
  painter.setPen(m_drive->isTruPosition() ?  QColor(Qt::black) : QColor(Qt::red));
  }else{
  painter.setPen(QColor(Qt::darkRed));
  }

painter.drawText(rOfst,Qt::AlignRight,ofst);

painter.end();


}

void WLGAxisLabel::mousePressEvent(QMouseEvent *event)
{
if(!m_drive) return;

if(rName.contains(event->pos())) {
    emit changedPress(m_drive->getName(),typeName);

}
 else if(rOfst.contains(event->pos())) emit changedPress(m_drive->getName(),typeOfst);
       else if((rF.contains(event->pos()))
             ||(rPos.contains(event->pos()))) emit changedPress(m_drive->getName(),typePos);
}

