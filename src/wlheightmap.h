#ifndef WLHEIGHTMAP_H
#define WLHEIGHTMAP_H

#include <QObject>
#include <QVector>
#include <QVector2D>
#include "math.h"

class Interpolation
{
public:
    static double cubicInterpolate(double p[4], double x)
    {
        return p[1] + 0.5 * x * (p[2] - p[0] + x *(2.0 * p[0] - 5.0 * p[1]
                + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
    }

    static double bicubicInterpolate(double p[4][4], double x, double y)
    {
        double arr[4];
        arr[0] = cubicInterpolate(p[0], x);
        arr[1] = cubicInterpolate(p[1], x);
        arr[2] = cubicInterpolate(p[2], x);
        arr[3] = cubicInterpolate(p[3], x);
        return cubicInterpolate(arr, y);
    }

    static double linearInterpolate(double p[2], double x)
    {
        return p[0]+(p[1]-p[0])*x;
    }

    static double bileniarInterpolate(double p[2][2], double x, double y)
    {
        double arr[2];
        arr[0] = linearInterpolate(p[0], x);
        arr[1] = linearInterpolate(p[1], x);
        return linearInterpolate(arr, y);
    }
};

struct SHMapData
{
QVector <QVector <double>> map;

double interpX=5;
double interpY=5;

double X0=0;
double Y0=0;

double X1=0;
double Y1=0;

double Zshow =0.0;
double Zoffset=0.0;

void clear(){
map.clear();
}

};


class WLHeightMap : public QObject
{
Q_OBJECT

public:
  enum  typeInterpoliation {bicubic,bileniar};

public:

explicit WLHeightMap(QObject *parent = nullptr):QObject(parent) { clear();}

private:

SHMapData data;

bool m_enable   =false;
bool m_show     =false;
bool m_showGrid =false;

bool m_updateShow=true;

WLHeightMap::typeInterpoliation m_typeInterpoliation=bicubic;

signals:

void changed();
void changedElement(int x,int y);

public:

void setData(SHMapData _data) {data=_data; m_updateShow=true;}
SHMapData getData() {return data;}

void clear() {data.clear(); setZShow(0); emit changed();}

double getInterpStepX() {return data.interpX;}
double getInterpStepY() {return data.interpY;}

void setInterpStepX(double val) {if(val>0) data.interpX=val;  m_updateShow=true;}
void setInterpStepY(double val) {if(val>0) data.interpY=val;  m_updateShow=true;}

void setShow(bool _enable) {m_show=_enable;}
bool isShow()              {return m_show;}

void setShowGrid(bool _enable) {m_showGrid=_enable;}
bool  isShowGrid()             {return m_showGrid;}

bool isUpdateShow()  {return m_updateShow;}
void resetUpdateShow() {m_updateShow=false;}

Q_INVOKABLE double getZShow() {return data.Zshow;}
Q_INVOKABLE void   setZShow(double Z0) {data.Zshow=Z0;m_updateShow=true;}

Q_INVOKABLE double getZOffset()  {return data.Zoffset;}
Q_INVOKABLE void   setZOffset(double Z) {data.Zoffset=Z;m_updateShow=true;}

Q_INVOKABLE void setEnable(bool _enable) {m_enable=_enable; m_updateShow=true;}
Q_INVOKABLE bool isEnable()              {return m_enable;}

Q_INVOKABLE void setP0(double _X0,double _Y0){data.X0=_X0; data.Y0=_Y0;  m_updateShow=true;}
Q_INVOKABLE void setP1(double _X1,double _Y1){data.X1=_X1; data.Y1=_Y1;  m_updateShow=true;}

QVector2D getP0(){return QVector2D(data.X0,data.Y0);}
QVector2D getP1(){return QVector2D(data.X1,data.Y1);}

Q_INVOKABLE  int countX() {return data.map.size();}
Q_INVOKABLE  int countY() {return data.map.isEmpty() ? 0 : data.map.first().size();}

Q_INVOKABLE void create(int x,int y)
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

Q_INVOKABLE bool isValid()
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

Q_INVOKABLE void setValue(int ix,int iy,double value)
{
if(!data.map.isEmpty()
  &&ix<countX()
  &&iy<countY()){
 data.map[ix][iy]=value;

 m_updateShow=true;

 emit changedElement(ix,iy);
 }
}

Q_INVOKABLE double getValueGrid(int i,int j)
{
if(i<countX()
 &&j<countY())
   return data.map[i][j];

return 0;
}

Q_INVOKABLE double getValue(double x,double y)
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

void setTypeInterpoliation(const WLHeightMap::typeInterpoliation &typeInterpoliation);
WLHeightMap::typeInterpoliation getTypeInterpoliation() const;
};
#endif // WLHEIGHTMAP_H
