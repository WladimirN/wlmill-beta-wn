#include "wlopengl.h"


WLOpenGL::WLOpenGL()
{
showMatrix.setToIdentity();

showOffset=QVector4D(0,0,0,0);

m_zoom=1;

m_timerMovie = new QTimer;
connect(m_timerMovie,SIGNAL(timeout()),SLOT(updateView()));
m_timerMovie->setInterval(30);

setClearColor(QColor(150,150,150));
}

WLOpenGL::~WLOpenGL()
{

}
void WLOpenGL::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(clearColor.redF(),clearColor.greenF(),clearColor.blueF(),1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    initShaders();
    initGLBuffers();
}

void WLOpenGL::movView(float Xd,float Yd,float Zd)
{
showOffset.setX(showOffset.x()+Xd);
showOffset.setY(showOffset.y()+Yd);
showOffset.setZ(showOffset.y()+Zd);
}

void WLOpenGL::setPointRot(QVector4D pR)
{    
pR.setW(0); //устанавливаем в ноль 

QVector4D P=showMatrix*pR;//находим наши коорд на экран

showOffset+=(showMatrix.column(3)+P)*m_zoom;//двигаем экран на разницу

showMatrix.setColumn(3,QVector4D(-P.x(),-P.y(),-P.z(),1));//устанавливаем 

qDebug()<<"setRotPoint";
}

void WLOpenGL::setRotView()
{
}


void WLOpenGL::resizeGL(int w, int h)
{
m_vport[0]=0;
m_vport[1]=0;
m_vport[2]=w;
m_vport[3]=h;

float wf=w;
float hf=h;

projection.setToIdentity();
projection.ortho(-wf/2,wf/2,-hf/2,hf/2,-50000,50000);
}

void WLOpenGL::zoomView(QPoint MousePos,int delta)
{
if(MousePos.x()<=0||MousePos.x()>=m_vport[2]
 ||MousePos.y()<=0||MousePos.y()>=m_vport[3])
  {
  MousePos.setX(m_vport[2]/2);
  MousePos.setY(m_vport[3]/2);
  }

QVector4D P0(MousePos.x()-m_vport[2]/2-showOffset.x()
           ,-MousePos.y()+m_vport[3]/2-showOffset.y()
		   ,0
		   ,1); //Текущая позиция на экране

QVector4D P1=P0;  

QMatrix4x4 M=showMatrix;
M.scale(m_zoom);
M.setColumn(3,QVector4D(0,0,0,1));//убераем смещение нам нужен только поворот

P0=M.inverted()*P0;   //узнаём позицию в ск отображения

//уменьшаем увеличение
m_zoom-=(delta*m_zoom/1000);
//P0 координата мыши в СК плоского окна новые
M=showMatrix;
M.scale(m_zoom);
M.setColumn(3,QVector4D(0,0,0,1));//убераем смещение нам нужен только поворот

P0=M*P0; //Узнаём новую точку на экране после увелиечния
//смещаем
P1-=P0;
//qDebug()<<P1.x()<<P1.y()<<Zoom;

movView(P1.x(),P1.y());
}

void WLOpenGL::resetView()
{
showMatrix.setToIdentity();
m_zoom=1;
}; 

void WLOpenGL::resetRotView()
{
showMatrix.setColumn(0,QVector4D(1,0,0,0)); 
showMatrix.setColumn(1,QVector4D(0,1,0,0)); 
showMatrix.setColumn(2,QVector4D(0,0,1,0)); 
}; 


void WLOpenGL::startMovie()
{
t=0;
m_timerMovie->start();
}

void WLOpenGL::updateView()
{
t+=0.1;

if(t>=1)   {
m_timerMovie->stop(); t=1;
}

if(m_startOffset!=m_endOffset){
showOffset=m_startOffset+(m_endOffset-m_startOffset)*t;
}

if(m_startZoom!=m_endZoom){
m_zoom=m_startZoom+(m_endZoom-m_startZoom)*t;
}

QQuaternion curQuant=QQuaternion::nlerp(m_startQuant,m_endQuant,t);

QMatrix4x4 M=m_startShowMatrix;

M.setColumn(3,QVector4D(0,0,0,1));

showMatrix=M.inverted()*m_startShowMatrix;//узнаём координаты смещения отн. базовой ск

M=getRotMatrix(0,0);
M.rotate(curQuant);

showMatrix=M*showMatrix;//поварачиваем на квантерион

if(t==3)
 {
 showMatrix=m_startShowMatrix;
 resetRotView();
 showMatrix.rotate(m_endQuant);
 }

update();
}

void WLOpenGL::setView(QMatrix4x4 Fr,QVector4D V,float _Zoom)
{
m_startQuant=MatrixToQuaternion(showMatrix);
m_startQuant.setScalar(-m_startQuant.scalar());

if(_Zoom<=0) _Zoom=m_zoom;

m_startOffset=showOffset;
m_endOffset=V;

m_startZoom=m_zoom;
m_endZoom=_Zoom;

QMatrix4x4 M;

M=m_startShowMatrix=showMatrix;

M.setColumn(3,QVector4D(0,0,0,1));

m_endQuant=MatrixToQuaternion(Fr*M.inverted()*showMatrix);
m_endQuant.setScalar(-m_endQuant.scalar());

startMovie();
}
