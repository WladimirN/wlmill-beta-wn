#include "wlheightmap.h"


WLHeightMap::typeInterpoliation WLHeightMap::getTypeInterpoliation() const
{
    return m_typeInterpoliation;
}

void WLHeightMap::addHeighMapPoints(QList<WLElementTraj> &Traj)
{
QList<WLElementTraj> newTraj;

if(!isValid()
 ||!isEnable()) return;

while(!Traj.isEmpty())
{
WLElementTraj ET=Traj.takeFirst();

if(ET.useHMap){

switch(ET.type)
{
case WLElementTraj::line: {
                          double interpolationStepX = getInterpStepX();
                          double interpolationStepY = getInterpStepY();

                          QVector3D stXY(ET.data.line.startPoint.x
                                        ,ET.data.line.startPoint.y
                                        ,ET.data.line.startPoint.z);

                          QVector3D enXY(ET.data.line.endPoint.x
                                        ,ET.data.line.endPoint.y
                                        ,ET.data.line.endPoint.z);

                          QVector3D vec=enXY-stXY;
                          double length;

                          if(qIsNaN(vec.length()))
                             {
                             ET.data.line.startPoint.z+=getValue(ET.data.line.startPoint.x
                                                                            ,ET.data.line.startPoint.y);

                             ET.data.line.endPoint.z+=getValue(ET.data.line.endPoint.x
                                                                           ,ET.data.line.endPoint.y);
                             newTraj+=ET;
                             break;
                             }
                             else {
                             if (fabs(vec.x()) / fabs(vec.y()) < interpolationStepX / interpolationStepY)
                                 length = interpolationStepY / (vec.y() / vec.length());
                             else
                                 length = interpolationStepX / (vec.x() / vec.length());

                             length = fabs(length);

                             QVector3D seg = vec.normalized() * length;
                             int count = trunc(vec.length() / length);
                             seg = vec / count;

                             if (count == 0)
                                {
                                ET.data.line.startPoint.z+=getValue(ET.data.line.startPoint.x
                                                                   ,ET.data.line.startPoint.y);

                                ET.data.line.endPoint.z+=getValue(ET.data.line.endPoint.x
                                                                 ,ET.data.line.endPoint.y);
                                newTraj+=ET;
                                break;
                                }

                             for (int i = 0; i < count; i++) {

                                 WLElementTraj segLine=ET;

                                 segLine.data.line.startPoint.x+=seg.x()*i;
                                 segLine.data.line.startPoint.y+=seg.y()*i;
                                 segLine.data.line.startPoint.z+=seg.z()*i;

                                 segLine.data.line.endPoint=segLine.data.line.startPoint;

                                 segLine.data.line.startPoint.z+=getValue(segLine.data.line.startPoint.x
                                                                         ,segLine.data.line.startPoint.y);

                                 if(i==count-1)
                                 {
                                 segLine.data.line.endPoint.x=ET.data.line.endPoint.x;
                                 segLine.data.line.endPoint.y=ET.data.line.endPoint.y;
                                 segLine.data.line.endPoint.z=ET.data.line.endPoint.z;
                                 }
                                 else{
                                 segLine.data.line.endPoint.x+=seg.x();
                                 segLine.data.line.endPoint.y+=seg.y();
                                 segLine.data.line.endPoint.z+=seg.z();
                                 }

                                 segLine.data.line.endPoint.z+=getValue(segLine.data.line.endPoint.x
                                                                       ,segLine.data.line.endPoint.y);


                                 newTraj+=segLine;
                                 }
                             }

                          }
                          break;


case WLElementTraj::arc:{
                        double interpolationStep = qMin(getInterpStepX(),getInterpStepY());

                        if(ET.data.arc.R/2.0<interpolationStep
                         ||ET.data.arc.plane!=17)
                          {
                          ET.data.arc.startPoint.z+=getValue(ET.data.arc.startPoint.x
                                                            ,ET.data.arc.startPoint.y);

                          ET.data.arc.endPoint.z+=getValue(ET.data.arc.endPoint.x
                                                          ,ET.data.arc.endPoint.y);
                          newTraj+=ET;
                          }
                          else {
                           WLElementTraj segArc=ET;

                           double A_st=ET.data.arc.startPoint.to3D().getAxy(ET.data.arc.centerPoint.to3D());
                           double A_en=ET.data.arc.endPoint.to3D().getAxy(ET.data.arc.centerPoint.to3D());

                           if((ET.data.arc.CCW)&&(A_en<=A_st))  A_en+=2.0*M_PI;
                           if((!ET.data.arc.CCW)&&(A_en>=A_st)) A_en-=2.0*M_PI;

                           double lenght=qAbs((A_en-A_st)*ET.data.arc.R);

                           int count=trunc(lenght/interpolationStep)+1;

                           double dA=(A_en-A_st)/count;
                           double A=A_st;

                           for (int i = 0;i<count; i++) {

                               if(i!=0){
                               segArc.data.arc.startPoint.x=segArc.data.arc.endPoint.x;
                               segArc.data.arc.startPoint.y=segArc.data.arc.endPoint.y;
                               segArc.data.arc.startPoint.z=segArc.data.arc.endPoint.z;
                               }
                               else{
                               segArc.data.arc.startPoint.z+=getValue(segArc.data.arc.startPoint.x
                                                                     ,segArc.data.arc.startPoint.y);
                               }

                               A+=dA;

                               if(i==count-1)
                                {
                                A=A_en;

                                segArc.data.arc.endPoint.x=ET.data.arc.endPoint.x;
                                segArc.data.arc.endPoint.y=ET.data.arc.endPoint.y;
                                }
                                else{
                                segArc.data.arc.endPoint.x=segArc.data.arc.centerPoint.x;
                                segArc.data.arc.endPoint.y=segArc.data.arc.centerPoint.y;

                                segArc.data.arc.endPoint.x+=segArc.data.arc.R*cos(A);
                                segArc.data.arc.endPoint.y+=segArc.data.arc.R*sin(A);
                                }

                               segArc.data.arc.endPoint.z=ET.data.arc.endPoint.z;
                               segArc.data.arc.endPoint.z+=getValue(segArc.data.arc.endPoint.x
                                                                   ,segArc.data.arc.endPoint.y);

                               newTraj+=segArc;

                               if((ET.data.arc.CCW&&A>=A_en)
                               ||(!ET.data.arc.CCW&&A<=A_en)) break;
                               }

                          }
                         }
                        break;
case WLElementTraj::delay:
                            ET.data.delay.point.z+=getValue(ET.data.delay.point.x
                                                           ,ET.data.delay.point.y);
                            newTraj+=ET;
                            break;

default: newTraj+=ET;
}
}
else
  newTraj+=ET;
}


Traj=newTraj;
}


void WLHeightMap::create(int x, int y)
{
    clear();

    for(int i=0;i<x;i++)
    {
        data.map+=QVector<double>();
        for(int j=0;j<y;j++)
            data.map[i]+=qQNaN();
    }

    emit changed();
}

bool WLHeightMap::isValid()
{
if(data.map.isEmpty())
    return false;


for(int i=0;i<countX();i++)
{
    for(int j=0;j<countY();j++)
        if(qIsNaN(data.map[i][j])) return false;
}

return true;
}

void WLHeightMap::setValue(int ix, int iy, double value)
{
if(!data.map.isEmpty()
        &&ix<countX()
        &&iy<countY()){
    data.map[ix][iy]=value;

    m_updateShow=true;

    emit changedElement(ix,iy);
}
}

double WLHeightMap::getValueGrid(int i, int j)
{
if(i<countX()
        &&j<countY())
    return data.map[i][j];

return 0;
}

double WLHeightMap::getValue(double x, double y)
{
if(!isValid()
 ||!isEnable())   return 0.0;
// Setup grid
int gridPointsX = countX();
int gridPointsY = countY();

double gridStepX = gridPointsX > 1 ? (data.X1-data.X0) / (gridPointsX - 1) : 0;
double gridStepY = gridPointsY > 1 ? (data.Y1-data.Y0) / (gridPointsY - 1) : 0;

// Get 16 points
x -= data.X0;
y -= data.Y0;

int ix[4];
int iy[4];

ix[1] = floor(x / gridStepX);
ix[2] =  ceil(x / gridStepX);

ix[0] = ix[1]-1;
ix[3] = ix[2]+1;

iy[1] = floor(y / gridStepY);
iy[2] =  ceil(y / gridStepY);

iy[0] = iy[1]-1;
iy[3] = iy[2]+1;

for(int i=0;i<4;i++)
    ix[i]=qBound(0,ix[i],countX()-1);
for(int i=0;i<4;i++)
    iy[i]=qBound(0,iy[i],countY()-1);

// Interpolate
switch(m_typeInterpoliation)
{
case WLHeightMap::bicubic:{
    double p[4][4];

    p[0][0] = data.map[ix[0]][iy[0]];
    p[0][1] = data.map[ix[1]][iy[0]];
    p[0][2] = data.map[ix[2]][iy[0]];
    p[0][3] = data.map[ix[3]][iy[0]];

    p[1][0] = data.map[ix[0]][iy[1]];
    p[1][1] = data.map[ix[1]][iy[1]];
    p[1][2] = data.map[ix[2]][iy[1]];
    p[1][3] = data.map[ix[3]][iy[1]];

    p[2][0] = data.map[ix[0]][iy[2]];
    p[2][1] = data.map[ix[1]][iy[2]];
    p[2][2] = data.map[ix[2]][iy[2]];
    p[2][3] = data.map[ix[3]][iy[2]];

    p[3][0] = data.map[ix[0]][iy[3]];
    p[3][1] = data.map[ix[1]][iy[3]];
    p[3][2] = data.map[ix[2]][iy[3]];
    p[3][3] = data.map[ix[3]][iy[3]];

    return Interpolation::bicubicInterpolate(p, x/gridStepX-ix[1], y/gridStepY-iy[1])-getZOffset();
}

case WLHeightMap::bileniar:
{
    double p[2][2];

    p[0][0] = data.map[ix[1]][iy[1]];
    p[0][1] = data.map[ix[2]][iy[1]];

    p[1][0] = data.map[ix[1]][iy[2]];
    p[1][1] = data.map[ix[2]][iy[2]];

    return Interpolation::bileniarInterpolate(p, x/gridStepX-ix[1], y/gridStepY-iy[1])-getZOffset();
}
default: return 0.0;
}
}

void WLHeightMap::setTypeInterpoliation(const WLHeightMap::typeInterpoliation &typeInterpoliation)
{
    if(m_typeInterpoliation != typeInterpoliation)
    {
        m_typeInterpoliation = typeInterpoliation;
        m_updateShow=true;
    }
}
