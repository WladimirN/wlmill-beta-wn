#include "wlelementtraj.h"
#include <math.h>
#include <QDebug>

//#define M_PI       3.14159265358979323846

WLElementTraj::WLElementTraj()
{
reset();
}

WLElementTraj::~WLElementTraj()
{

}

void WLElementTraj::reset(bool all)
{
index=0;

movDistanceIJ=movDistanceK=movDistanceXYZ=0;

str.clear();

type=empty;
data.empty.point=WLGPoint();

if(all){
    m_stopMode=false;
    }
}


bool WLElementTraj::setULine(WLGPoint _startPoint,WLGPoint _midPoint,WLGPoint _endPoint)
{
if(_startPoint!=_midPoint
 &&_endPoint!=_midPoint
 &&(qAbs(qAbs((_startPoint.to3D()-_midPoint.to3D()).r())
        -qAbs((_endPoint.to3D()  -_midPoint.to3D()).r()))<=ULINE_MAXERRORPOINTR)){
 type=uline;
 data.uline.startPoint=_startPoint;
 data.uline.midPoint=_midPoint;
 data.uline.endPoint=_endPoint;
 return true;
 }
 else{
 qDebug()<<"Erorr setULine dR="<<qAbs(qAbs((_startPoint.to3D()-_midPoint.to3D()).r())
                                     -qAbs((_endPoint.to3D()  -_midPoint.to3D()).r()));
 type=empty;
 data.empty.point=_endPoint;
 return false;
 }
}

bool WLElementTraj::setDelay(WLGPoint _Point,quint32 ms)
{
if(ms!=0){
    type=WLElementTraj::delay;
    data.delay.time=ms;
    data.delay.point=_Point;
    return true;
    }
    else{
    type=WLElementTraj::empty;
    data.empty.point=_Point;
    }

return false;
}

bool WLElementTraj::setMCode(WLGPoint _Point, int _MCode)
{
qDebug()<<"WLElementTraj::setMCode"<<_MCode;

if(_MCode>0){
 type=WLElementTraj::mcode;
 data.mcode.MCode=_MCode;
 data.mcode.point=_Point;
 return true;
 }
 else {
 type=WLElementTraj::empty;
 data.empty.point=_Point;
 }

return false;
}


QList <WL6DPoint> WLElementTraj::calcMCodePoints(bool *ok,WLGModel *GModel)
{
QList <WL6DPoint> Points;

Points+=GModel->getFrame(data.mcode.point).to6D();

if(ok)
    *ok=true;

return Points;
}


QList<WL6DPoint> WLElementTraj::calcPoints(bool *ok, WLGModel *GModel, double delta)
{
QList<WL6DPoint> List;

    switch (type)
    {
    case line:  List=calcLinePoints(ok,GModel,delta);  break;
    case arc:   List=calcArcPoints(ok,GModel,delta);break;
    case uline: List=calcULinePoints(ok,GModel,delta); break;
    case mcode: List=calcMCodePoints(ok,GModel);  break;
    default: break;
    }

return List;
}

WLGPoint WLElementTraj::getStartPoint()
{
 switch (type)
 {
 case line:  return data.line.startPoint;
 case arc:   return data.arc.startPoint;
 case uline: return data.uline.startPoint;
 case mcode: return data.mcode.point;
 case delay: return data.delay.point;
 case empty: return data.empty.point;
 }
}

WLGPoint WLElementTraj::getEndPoint()
{
 switch (type)
 {
 case line:  return data.line.endPoint;
 case arc:   return data.arc.endPoint;
 case uline: return data.uline.endPoint;
 case mcode: return data.mcode.point;
 case delay: return data.delay.point;
 case empty: return data.empty.point;
 }
}

void WLElementTraj::setStartPoint(WLGPoint point)
{
 switch (type)
 {
 case line:   data.line.startPoint=point; break;
 case arc:     data.arc.startPoint=point; break;
 case uline: data.uline.startPoint=point; break;
 case mcode:      data.mcode.point=point; break;
 case delay:      data.delay.point=point; break;
 case empty:      data.empty.point=point; break;
 }
}

void WLElementTraj::setEndPoint(WLGPoint point)
{
 switch (type)
 {
 case line:   data.line.endPoint=point; break;
 case arc:     data.arc.endPoint=point; break;
 case uline: data.uline.endPoint=point; break;
 case mcode:    data.mcode.point=point; break;
 case delay:    data.delay.point=point; break;
 case empty:    data.empty.point=point; break;
 }
}


bool WLElementTraj::setLine(WLGPoint _startPoint,WLGPoint _endPoint)
{
data.line.startPoint=_startPoint;
data.line.endPoint  =_endPoint;

if(_startPoint!=_endPoint){
    type=WLElementTraj::line;
    }
    else{
    type=WLElementTraj::empty;
    data.empty.point=_endPoint;
    }

return true;
}

QList <WL6DPoint> WLElementTraj::calcLinePoints(bool *ok,WLGModel *GModel,double delta)
{
QList <WL6DPoint> Points;

if(!isLine()
 ||!data.line.startPoint.isValid()
 ||!data.line.endPoint.isValid()){

return Points;
}

    movDistanceXYZ=(GModel->getFrame(data.line.endPoint).to3D()
                -GModel->getFrame(data.line.startPoint).to3D()).r();


if(delta==0.0f||movDistanceXYZ==0.0f)
 {
 Points+=GModel->getFrame(data.line.startPoint).to6D();
 Points+=GModel->getFrame(data.line.endPoint).to6D();
 }
else {
 double n=5;
 double k;
 double dist;
 //double distXYZ;
 //double distA,distB,distC;

 GModel->addRotaryPosition(data.line.startPoint,data.line.endPoint);

 WLGPoint deltaG=(data.line.endPoint-data.line.startPoint)/n;

 if(GModel->getA().rotary
   &&deltaG.a>delta) {n*=deltaG.a;deltaG=deltaG/deltaG.a;}

 if(GModel->getB().rotary
   &&deltaG.b>delta) {n*=deltaG.b;deltaG=deltaG/deltaG.b;}

 if(GModel->getC().rotary
   &&deltaG.c>delta) {n*=deltaG.c;deltaG=deltaG/deltaG.c;}

 //qDebug()<<"deltaG"<<deltaG.toString();

 dist=(GModel->getFrame(data.line.startPoint+deltaG).to3D()-GModel->getFrame(data.line.startPoint).to3D()).r();

 //qDebug()<<"dist="<<dist;

 k=(double)n*dist/delta;

 n=(int)k+1;

 deltaG=(data.line.endPoint-data.line.startPoint)/(double)n;

 //qDebug()<<"deltaG"<<deltaG.toString()<<"n"<<n;

 for(int i=0;i<=n;i++){
    Points+=GModel->getFrame(data.line.startPoint+deltaG*(double)i).to6D();
    }
}

startV=endV=(data.line.endPoint-data.line.startPoint).normalize();//bug

if(ok) *ok=true;

return Points;
}

QList <WLElementTraj> WLElementTraj::calcModelPoints(bool *ok, WLGModel *GModel, double delta)
{
QList <WLElementTraj> retList;
WL6DPoint lastP,curP;
WLGPoint lastGP,curGP;

double dist;
double dF;
double dFxyz;
double kF;

if(!isLine()
  ||isFast()
  ||data.line.G93) //60/F
    {
    retList+=*this;
    return retList;
    }

if(delta<=0.0f) delta=0.5;

int n=5;

WLGPoint deltaG=(data.line.endPoint-data.line.startPoint)/(double)n;

lastGP=data.line.startPoint;
curGP=data.line.startPoint+deltaG;
dist=(GModel->getFrame(curGP).to3D()-GModel->getFrame(lastGP).to3D()).r();

kF=(float)n*dist/delta;

n=kF+1;

deltaG=(data.line.endPoint-data.line.startPoint)/(double)n;

dFxyz=deltaG.x*deltaG.x
     +deltaG.y*deltaG.y
     +deltaG.z*deltaG.z;

dF=dFxyz
  +deltaG.a*deltaG.a
  +deltaG.b*deltaG.b
  +deltaG.c*deltaG.c;

kF= dFxyz!=0.0 ? sqrt(dF/dFxyz) : 1;

if(kF==1.0)
    {
    retList+=*this;
    return retList;
    }

dF=sqrt(dF);
dFxyz=sqrt(dFxyz);

curGP=data.line.startPoint;
curP=GModel->getFrame(curGP).to6D();

WLElementTraj ET=*this;

for(int i=1;i<=n;i++)
    {
    lastP=curP;
    lastGP=curGP;

    curGP=data.line.startPoint+deltaG*(double)i;
    curP=GModel->getFrame(curGP).to6D();

    dist=(curP.to3D()-lastP.to3D()).r();
    //qDebug()<<"curP"<<curP.toString();
    //qDebug()<<"lastP"<<lastP.toString();
    ET.setLine(lastGP,curGP);

    if(this->F>0)
         ET.F=(dF/(dist/this->F));

    //qDebug()<<"kF"<<kF<<"dF"<<dF<<"dFxyz"<<dFxyz<<"dist"<<dist<<"n"<<n<<"F="<<ET.getF()<<this->getF();

    if(ET.F!=this->F
     ||retList.isEmpty())
      {
      retList+=ET;
      }
    }


if(ok) *ok=true;

return retList;
}

QList <WL6DPoint> WLElementTraj::calcULinePoints(bool *ok,WLGModel *GModel,double delta)
{
Q_UNUSED(GModel)
Q_UNUSED(delta)
QList <WL6DPoint> Points;

if(!isULine())
    {
    return Points;
    }

float ax[3],ay[3],az[3];
float dt;
WL3DPoint P,startPoint,midPoint,endPoint;

QMatrix3x3 T;
T(0,0)= 1; T(0,1)= 0; T(0,2)= 0;
T(1,0)=-3; T(1,1)= 4; T(1,2)=-1;
T(2,0)= 2; T(2,1)=-4; T(2,2)= 2;

startPoint=data.uline.startPoint.to3D();
  midPoint=data.uline.midPoint.to3D();
  endPoint=data.uline.endPoint.to3D();

ax[0]=startPoint.x*T(0,0)+midPoint.x*T(0,1)+endPoint.x*T(0,2);
ax[1]=startPoint.x*T(1,0)+midPoint.x*T(1,1)+endPoint.x*T(1,2);
ax[2]=startPoint.x*T(2,0)+midPoint.x*T(2,1)+endPoint.x*T(2,2);

ay[0]=startPoint.y*T(0,0)+midPoint.y*T(0,1)+endPoint.y*T(0,2);
ay[1]=startPoint.y*T(1,0)+midPoint.y*T(1,1)+endPoint.y*T(1,2);
ay[2]=startPoint.y*T(2,0)+midPoint.y*T(2,1)+endPoint.y*T(2,2);

az[0]=startPoint.z*T(0,0)+midPoint.z*T(0,1)+endPoint.z*T(0,2);
az[1]=startPoint.z*T(1,0)+midPoint.z*T(1,1)+endPoint.z*T(1,2);
az[2]=startPoint.z*T(2,0)+midPoint.z*T(2,1)+endPoint.z*T(2,2);

dt=1.0f/20;

Points+=startPoint;

for(float d=dt;d<1;d+=dt)
 {
 P.x=ax[0]+ax[1]*d+ax[2]*d*d;
 P.y=ay[0]+ay[1]*d+ay[2]*d*d;
 P.z=az[0]+az[1]*d+az[2]*d*d;

 Points+=P;
 }

Points+=endPoint;

movDistanceXYZ=0;

for(int i=1;i<Points.size();i++)
 {
 movDistanceXYZ+=(Points[i]-Points[i-1]).to3D().r();
 }

startV.x=ax[1];
startV.y=ay[1];
startV.z=az[1];
startV=startV.normalize();

endV.x=2*ax[2]+ax[1];
endV.y=2*ay[2]+ay[1];
endV.z=2*az[2]+az[1];
endV=endV.normalize();


if(ok) *ok=true;

return Points;
}
/*
void WLElementTraj::calcGLElement()
{
if(GLElemnt==0) GLElemnt=glGenLists(1);

if(GLElemnt!=0)
{
glNewList(GLElemnt,GL_COMPILE);

glBegin(GL_LINE_STRIP);

for(int i=0;i<Points.size();i++)
   {
   glVertex3f(Points[i].x
             ,Points[i].y
             ,Points[i].z);
   }
glEnd();
}

glEndList();
}
*/
bool WLElementTraj::setArc(WLGPoint _startPoint,WLGPoint _centerPoint,WLGPoint _endPoint,bool CCW,int _plane)
{/*
if(_startPoint==_endPoint)
    {
    qDebug()<<"error point circ (startPoint=endPoint)";
    return false;
    }
*/
WL3DPoint sP=WLGCode::convertPlane(_startPoint,_plane,true).to3D();
WL3DPoint eP=WLGCode::convertPlane(_endPoint,_plane,true).to3D();
WL3DPoint cP=WLGCode::convertPlane(_centerPoint,_plane,true).to3D();

double R1=hypot(sP.x-cP.x,sP.y-cP.y);
double R2=hypot(eP.x-cP.x,eP.y-cP.y);

if(R1>(R2*1.02)||R1<(R2*0.92))
      {
      qDebug()<<"error circ radius(2%)"<<R1<<R2;
      return false;
      }
      else {
      type=arc;
      data.arc.R=(R1+R2)/2.0;
      data.arc.startPoint=_startPoint;
      data.arc.endPoint=_endPoint;
      data.arc.centerPoint=_centerPoint;
      data.arc.CCW=CCW;
      data.arc.plane=_plane;
      return true;
      }

}

QList <WL6DPoint> WLElementTraj::calcArcPoints(bool *ok,WLGModel *GModel,double delta)
{
Q_UNUSED(GModel);

QList <WL6DPoint> Points;

if(!isCirc()
 ||!data.arc.startPoint.isValid()
 ||!data.arc.centerPoint.isValid()
 ||!data.arc.endPoint.isValid())
    {
    return Points;
    }

double dl= delta < 1 ? 1: delta;
double R,R1;
double A_st;
double A_en;
double A_now;
double Ioffset;
double Joffset;

//qDebug()<<" StartP x:"<<startPoint.x<<" y:"<<startPoint.y<<" z:"<<startPoint.z;
//qDebug()<<"   EndP x:"<<endPoint.x<<" y:"<<endPoint.y<<" z:"<<endPoint.z;
//qDebug()<<"Centert x:"<<centerPoint.x<<" y:"<<centerPoint.y<<" z:"<<centerPoint.z;

//centerPoint.z=startPoint.z;???
WLGPoint  startPoint=WLGCode::convertPlane(data.arc.startPoint,data.arc.plane,true);
WLGPoint centerPoint=WLGCode::convertPlane(data.arc.centerPoint,data.arc.plane,true);
WLGPoint    endPoint=WLGCode::convertPlane(data.arc.endPoint,data.arc.plane,true);

Ioffset=startPoint.x-centerPoint.x;
Joffset=startPoint.y-centerPoint.y;

R =hypot(Ioffset,Joffset);

Ioffset=endPoint.x-centerPoint.x;
Joffset=endPoint.y-centerPoint.y;

R1=hypot(Ioffset,Joffset);
/*
double dR;

if(R1!=0&&R!=0)
  {
  if(R>R1) dR=R/R1; else dR=R1/R;
  }
else
  dR=2;
  */
if(R==0.0||R1==0.0)
{
//qDebug()<<"error radius"<<R<<R1;
if(ok!=NULL) *ok=false;
return Points;
}

data.arc.R=(R+R1)/2;

A_st=startPoint.to3D().getAxy(centerPoint.to3D());
A_en=endPoint.to3D().getAxy(centerPoint.to3D());

if((data.arc.CCW)&&(A_en<=A_st))  A_en+=2*M_PI;
if((!data.arc.CCW)&&(A_en>=A_st)) A_en-=2*M_PI;

movDistanceIJ=qAbs((A_en-A_st)*R);
movDistanceK =qAbs(endPoint.z-startPoint.z);
movDistanceXYZ=hypot(movDistanceIJ,movDistanceK);

int n=1;

if(R<10) dl=R/10;

n=qAbs(A_en-A_st)*R/dl+1;

const double da=(A_en-A_st)/n;
const double dz=(endPoint.z-startPoint.z)/n;

data.arc.spiral = dz!=0;

WL6DPoint Point;

Point=startPoint.to3D();

Points.clear();

//qDebug("A_st=%f A_e=%f dA=%f n=%d R=%f dl=%f",A_st,A_en,da,n,R,dl);

//qDebug("Start Point  %f:%f:%f",Point.x,Point.y,Point.z);
/*
midPoint=centerPoint;
midPoint.x+=R*cos(A_st+(A_en-A_st)/2);
midPoint.y+=R*sin(A_st+(A_en-A_st)/2);
midPoint.z=(endPoint.z+startPoint.z)/2;
*/
A_now=A_st;

int i;

for(i=0;;i++)
 {
 Point=centerPoint.to3D();
 Point.x+=R*cos(A_now);
 Point.y+=R*sin(A_now);
 Point.z=startPoint.z+dz*i;

 Points+=Point;

//qDebug("Point  %f:%f:%f",Point.x,Point.y,Point.z);
  A_now+=da;

 if((data.arc.CCW&&A_now>=A_en)
 ||(!data.arc.CCW&&A_now<=A_en)) break;

 //qDebug("A_now %f",A_now);
 }


//qDebug("i=%d",i);

//qDebug("endPoint  %f:%f:%f",endPoint.x,endPoint.y,endPoint.z);
Points+=endPoint.to3D();

 WL3DPoint N(0,0,data.arc.CCW ?-1:1);

 WL3DPoint sV=(startPoint.to3D()-centerPoint.to3D());
 WL3DPoint eV=(endPoint.to3D()  -centerPoint.to3D());

 sV.z=0;
 sV=sV.normalize();
 sV=sV*N;

 eV.z=0;
 eV=eV.normalize();
 eV=eV*N;

 if(data.arc.spiral)
  {
  float k=movDistanceIJ/movDistanceXYZ;

  sV.x*=k;
  sV.y*=k;
  sV.z =(endPoint.to3D().z-startPoint.to3D().z)/movDistanceXYZ;

  eV.x*=k;
  eV.y*=k;
  eV.z = startV.z;
  }

 startV.x=sV.x;
 startV.y=sV.y;
 startV.z=sV.z;

 endV.x=eV.x;
 endV.y=eV.y;
 endV.z=eV.z;

 startV=startV.normalize();
 endV=endV.normalize();

//qDebug("End Point  %f:%f:%f",endPoint.x,endPoint.y,endPoint.z);

//startPoint=WLGCode::convertPlane(startPoint,plane,false);
//centerPoint=WLGCode::convertPlane(centerPoint,plane,false);
//endPoint=WLGCode::convertPlane(endPoint,plane,false);
//
startV=WLGCode::convertPlane(startV,data.arc.plane,false);
  endV=WLGCode::convertPlane(  endV,data.arc.plane,false);


GModel->addRotaryPosition(startPoint,endPoint);

WLGPoint deltaG=(endPoint-startPoint)/Points.size();
//qDebug()<<"delta G circ"<<deltaG.toString();

for(int i=0;i<Points.size();i++)
{
WLGPoint GP;

GP=startPoint+deltaG*i;

GP.x=Points[i].x;
GP.y=Points[i].y;
GP.z=Points[i].z;

GP=WLGCode::convertPlane(GP,data.arc.plane,false);

Points[i]=GModel->getFrame(GP).to6D();

/*
Points[i].x=GP.x;
Points[i].y=GP.y;
Points[i].z=GP.z;
*/
}

if(ok) *ok=true;
return Points;
}

void WLElementTraj::removeEmpty(QList<WLElementTraj> &Traj)
{
for(int i=0;i<Traj.size();i++)
    if(Traj[i].isEmpty()) Traj.removeAt(i--);
}

bool WLElementTraj::detectMCode(QList<WLElementTraj> &Traj)
{
for(int i=0;i<Traj.size();i++)
    if(Traj[i].isMCode()){
     return true;
     }

return false;
}

int WLElementTraj::simpliTrajectory(QList<WLElementTraj> &simpliTraj
                                   ,QList<WLElementTraj> baseTraj
                                   ,float simpliDist
                                   ,bool oneSimpli,float simpliAngle,int Ar,int Br,int Cr)
{
QList <int> indexs;
WL6DPoint A,B,O;

float dist; //расстояние

WL3DPoint vZA,vZO;//вектор Z

simpliTraj.clear();
indexs.clear();

int i,j;

if(baseTraj.size()==1)
  {
  simpliTraj=baseTraj;
  return 1;
  }

for(i=0;i<baseTraj.size();i++)
{
indexs+=i; //индексы подходящих элементов

if(baseTraj[i].isLine()) //если нет M комманд
{
if(indexs.size()==1)
  {
  A=baseTraj[indexs.first()].data.line.startPoint.to6D();//первая точка текущей пачки
  vZA.fromV(A.toM(Ar,Br,Cr).column(2)); //берем вектор ориентации Z
  }

if(indexs.size()>=2)
 {
 if(simpliAngle!=0.0f) //по углу
    {
    //KUKA
    vZO.fromV(baseTraj[indexs.last()].data.line.endPoint.to6D().toM(Ar,Br,Cr).column(2));
    if((calcAngleGrd(vZA,vZO)>simpliAngle)&&(!baseTraj[i].isFast())) goto endpack;

    //if(baseTraj[indexs.first()].data.line.startPoint.a!=baseTraj[indexs.last()].data.line.endPoint.a
    // ||baseTraj[indexs.first()].data.line.startPoint.b!=baseTraj[indexs.last()].data.line.endPoint.b
    // ||baseTraj[indexs.first()].data.line.startPoint.c!=baseTraj[indexs.last()].data.line.endPoint.c) goto endpack;
    }

  if(!baseTraj[indexs.first()].isEqFS(baseTraj[indexs.last()])) goto endpack;

  if((calcACos(baseTraj[indexs.last()].data.line.endPoint.to3D()-baseTraj[indexs.size()-2].data.line.endPoint.to3D(),
            (baseTraj[indexs.size()-2].data.line.endPoint.to3D()-baseTraj[indexs.first()].data.line.startPoint.to3D()))<(-0.707)))goto endpack;

  B=baseTraj[indexs.last()].data.line.endPoint.to6D();//последняя точка текущей пачки

  if(baseTraj[i].isLine())
  for(j=0;j<(indexs.size()-1);j++) //по дистанции
     {
     O=baseTraj[indexs[j]].data.line.endPoint.to6D(); //перебор
     dist=fabs((O.to3D()-A.to3D()).r()*calcAngleRad((O.to3D()-A.to3D()),(B.to3D()-A.to3D()))); //находим расстояние от общей прямой до всех точек

     //qDebug()<<"distSimpl"<<dist<<qMax(simpliDist,baseTraj[i].getG64Q());
     if(baseTraj[i].getSmoothQ()==0.0
      ||dist>(qMax(simpliDist,(float)baseTraj[i].getSmoothQ()))) //если нельзя дальше считать
         {
         endpack:
         i--;
         indexs.removeLast();
         goto endspack;
         }
     }
  }
}
else
 {
 endspack:
  //qDebug()<<"endspack:";
  if(!indexs.isEmpty()) //если 2 и больше
  {
  simpliTraj+=baseTraj[indexs.first()]; //добавляем первый

  if(indexs.size()>1)  //если есть сглаженные то добавляем его
   {
   if(!oneSimpli) {
    simpliTraj+=baseTraj[indexs.last()];  //добавляем последний, который не прошёл фильтр
    simpliTraj[simpliTraj.size()-2].setEndPoint(baseTraj[indexs.last()].getStartPoint()); //соединяем их
    }
   else {
    simpliTraj[simpliTraj.size()-1].setEndPoint(baseTraj[indexs.last()].getStartPoint()); //соединяем их
    return i-1;
    }

   }

  indexs.clear();
  }

 if(oneSimpli) return i;
 }
}

oneSimpli=true; //чтобы вышел из цикла
goto endspack;
}


void WLElementTraj::updateFS(QList<WLElementTraj> &Traj)
{
float curF=1;
for(int i=0;i<Traj.size();i++)
  {
  if(Traj[i].F==0)
      Traj[i].F=curF;
      else
        if(Traj[i].F>=0)
            curF=Traj[i].F;
  }

float curS=1;
for(int i=0;i<Traj.size();i++)
  {
  if(Traj[i].S==0)
      Traj[i].S=curS;
      else
        if(Traj[i].S>=0)
            curF=Traj[i].S;

 // qDebug()<<"F="<<Traj[i].speedF;
  }

}

void WLElementTraj::calcPoints(QList<WLElementTraj> &Traj, WLGModel *GModel, double delta)
{
bool ok;

for(int i=0;i<Traj.size();i++){
  Traj[i].calcPoints(&ok,GModel);
  }
}

inline bool WLElementTraj::isEqFS(WLElementTraj ET)
{
return (F==ET.F)
     &&(S==ET.S);
}

QString WLElementTraj::toString()
{
QString ret;
ret=    "i"+QString::number(index)+
    " type"+QString::number(type)+
      " S:"+QString::number(S)+
      " F:"+QString::number(F);

return ret;
}
