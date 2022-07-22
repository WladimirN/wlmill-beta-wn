#include "wlcameraview.h"
#include <QPainter>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>
#include <QImageEncoderSettings>
#include <QSizePolicy>

WLCameraView::WLCameraView(QWidget *parent):QCameraViewfinder (parent)
{
setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
setAspectRatioMode(Qt::KeepAspectRatio);
}

void WLCameraView::paintEvent(QPaintEvent *event)
{
QCamera *camera = static_cast<QCamera*>(mediaObject());
QCameraViewfinderSettings setting=camera->viewfinderSettings();

QCameraViewfinder::paintEvent(event);

QPainter painter(this);

painter.setPen(Qt::green);

float h=height();
float w=width();

float h1=setting.resolution().height();
float w1=setting.resolution().width();

painter.drawLine(0,h/2 ,w  ,h/2);
painter.drawLine(w/2,0 ,w/2,h);


qDebug()<<"resolution"<<setting.resolution().width()<<setting.resolution().height()
                <<setting.pixelAspectRatio().width()<<setting.pixelAspectRatio().height();

qDebug()<<"scale h"<<h/h1;
qDebug()<<"scale w"<<w/w1;
}
