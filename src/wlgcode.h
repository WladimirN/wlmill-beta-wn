#ifndef WLGCODE_H
#define WLGCODE_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "wl3dpoint.h"
#include "wl6dpoint.h"
#include "wlframe.h"
#include "wlgtool.h"

#define GNO_err  1
#define GNO_use  0
#define GErr    -1

#define GCodeSize 1000
#define sizeSC 7

typedef struct GPar
{
double value;
bool   valid;

GPar()                {value=0;valid=false;}
void set(double _val) {value=_val; valid=true;}

}GPar;

#define GPointNames "X,Y,Z,A,B,C,U,V,W"

typedef struct WLGPoint
{    
double x;
double y;
double z;
double a;
double b;
double c;
double u;
double v;
double w;

WLGPoint() {x=y=z=a=b=c=u=v=w=0;}
WLGPoint(WL6DPoint point) {u=v=w=0; from6D(point); }

bool isNull()
{
return x==0.0
     &&y==0.0
     &&z==0.0
     &&a==0.0
     &&b==0.0
     &&c==0.0
     &&u==0.0
     &&v==0.0
     &&w==0.0;
}

double get(QString name,bool *ok=nullptr)
{
name=name.toUpper();

if(ok!=nullptr) *ok=true;

if(name=="X") return x;
if(name=="Y") return y;
if(name=="Z") return z;
if(name=="A") return a;
if(name=="B") return b;
if(name=="C") return c;
if(name=="U") return u;
if(name=="V") return v;
if(name=="W") return w;

if(ok!=nullptr) *ok=false;
return 0;
}

void set(QString name,double value)
{
name=name.toUpper();

if(name=="X") x=value;
else
if(name=="Y") y=value;
else
if(name=="Z") z=value;
else
if(name=="A") a=value;
else
if(name=="B") b=value;
else
if(name=="C") c=value;
else
if(name=="U") u=value;
else
if(name=="V") v=value;
else
if(name=="W") w=value;
}

bool isValid()
{
return   !qIsNaN(x)&&!qIsInf(x)
       &&!qIsNaN(y)&&!qIsInf(y)
       &&!qIsNaN(z)&&!qIsInf(z)
       &&!qIsNaN(a)&&!qIsInf(a)
       &&!qIsNaN(b)&&!qIsInf(b)
       &&!qIsNaN(c)&&!qIsInf(c);
}

QString toString()
{
return
   +"X "+QString::number(x)
   +",Y "+QString::number(y)
   +",Z "+QString::number(z)
   +",A "+QString::number(a)
   +",B "+QString::number(b)
   +",C "+QString::number(c);
}

bool fromString(QString str)
{
str.remove(QRegExp("[XYZABCUVW]"));
str.remove(QRegExp("[xyzabcuvw]"));

QStringList List=str.split(",");

switch(List.size())
{
case 9: u=List[6].toDouble();
        v=List[7].toDouble();
        w=List[8].toDouble();

case 6: a=List[3].toDouble();
        b=List[4].toDouble();
        c=List[5].toDouble();

case 3: x=List[0].toDouble();
        y=List[1].toDouble();
        z=List[2].toDouble();
        return true;
}

return false;
}

QString toString(bool all)
{
if(all)
return QString("X "+QString::number(x)+","
              +"Y "+QString::number(y)+","
              +"Z "+QString::number(z)+","
              +"A "+QString::number(a)+","
              +"B "+QString::number(b)+","
              +"C "+QString::number(c)+","
              +"U "+QString::number(u)+","
              +"V "+QString::number(v)+","
              +"W "+QString::number(w));
else {
    return QString("X "+QString::number(x)+","
                  +"Y "+QString::number(y)+","
                  +"Z "+QString::number(z));

}
}
WLGPoint normalize()
{
WLGPoint ret;
const double F=sqrt(x*x+y*y+z*z+a*a+b*b+c*c+u*u+v*v+w*w);

ret.x=x/F;
ret.y=y/F;
ret.z=z/F;
ret.a=a/F;
ret.b=b/F;
ret.c=c/F;
ret.u=u/F;
ret.v=v/F;
ret.w=w/F;

return ret;
}

WL3DPoint to3D()
{
WL3DPoint ret;

ret.x=x;
ret.y=y;
ret.z=z;

return ret;
}

void from6D(WL6DPoint A)
{
x=A.x;
y=A.y;
z=A.z;

a=A.a;
b=A.b;
c=A.c;
}

WL6DPoint to6D()
{
WL6DPoint ret;

ret.x=x;
ret.y=y;
ret.z=z;

ret.a=a;
ret.b=b;
ret.c=c;

return ret;
}

bool operator==(WLGPoint A)
{
return x==A.x
     &&y==A.y
     &&z==A.z
     &&a==A.a
     &&b==A.b
     &&c==A.c
     &&u==A.u
     &&v==A.v
     &&w==A.w;
}

bool operator!=(WLGPoint A)
{
return x!=A.x
     ||y!=A.y
     ||z!=A.z
     ||a!=A.a
     ||b!=A.b
     ||c!=A.c
     ||u!=A.u
     ||v!=A.v
     ||w!=A.w;
}

WLGPoint operator+(WLGPoint A)
{
WLGPoint ret;

ret.x=x+A.x;
ret.y=y+A.y;
ret.z=z+A.z;
ret.a=a+A.a;
ret.b=b+A.b;
ret.c=c+A.c;
ret.u=u+A.u;
ret.v=v+A.v;
ret.w=w+A.w;

return ret;
}

WLGPoint operator-(WLGPoint A)
{
WLGPoint ret;

ret.x=x-A.x;
ret.y=y-A.y;
ret.z=z-A.z;
ret.a=a-A.a;
ret.b=b-A.b;
ret.c=c-A.c;
ret.u=u-A.u;
ret.v=v-A.v;
ret.w=w-A.w;

return ret;
}

WLGPoint operator*(double A)
{
WLGPoint ret;

ret.x=x*A;
ret.y=y*A;
ret.z=z*A;
ret.a=a*A;
ret.b=b*A;
ret.c=c*A;
ret.u=u*A;
ret.v=v*A;
ret.w=w*A;

return ret;
}


WLGPoint operator/(double A)
{
WLGPoint ret;

ret.x=x/A;
ret.y=y/A;
ret.z=z/A;
ret.a=a/A;
ret.b=b/A;
ret.c=c/A;
ret.u=u/A;
ret.v=v/A;
ret.w=w/A;

return ret;
}


}WLGPoint;


struct WLGCodeData
{
 WLGPoint    offsetSC[sizeSC]; //Системы координат смещение
 WLGPoint refPoint0SC[sizeSC]; //Вращение опорная точка
 WLGPoint refPoint1SC[sizeSC]; //Вращение проверочная точка

 WLGTools  Tools;

 WLGPoint G43Position;
 WLGPoint G28Position;

 WLGPoint G51Scale;

 WLGPoint  curGPoint;//program!!!
 WLGPoint lastGPoint;//program!!!

 WL3DPoint curPoint;//G41/42
 WL3DPoint endPoint;//G41/42

 int iCurTool=0;

 int iSC=0;

  double drillPlane=0;
 quint32 drillPms=0;
 quint32 G4Pms=0;

 double G64P=0;
 double G64Q=0;

 bool GCode[GCodeSize];

 bool absIJK;
 bool stopMode;
 bool initDrillPlane; 

 QString strRunProgram;
 QString strInit="G64 P0.05 Q0.05";

 double backOffsetLonfDrill=1;

 bool cutter_comp_firstmove=true;

 GPar gX;
 GPar gY;
 GPar gZ;

 GPar gI;
 GPar gJ;
 GPar gK;

 GPar gA;
 GPar gB;
 GPar gC;

 GPar gF;

 GPar gR;
 GPar gP;
 GPar gQ;

 GPar gS;
 GPar gT;
 GPar gD;

 GPar gH;

 WLGCodeData();
};

class WLGCode: public QObject
{
 Q_OBJECT

public:

	enum Code
	 {
     fast_motion=00,
    
	 line       =01,
     circle_cw  =02,
     circle_ccw =03,

	 wait_motion=04,

     plane_xy   =17,
	 plane_zx   =18,
	 plane_yz   =19,
     
	 drill      =81,
	 drill_long =83,
	 drill_off  =80,

	 absolute   =90,
	 incremental=91,

	 plane_drill =98,
	 plane_drillR =99
     };

public:
    WLGCode();
    WLGCode(WLGCodeData data);

    ~WLGCode();

    bool isStopMode() {return m_data.stopMode;}

    QList<int> loadStr(QString gstr);

    double getG64Q() {return m_data.G64Q;}
    double getG64P() {return m_data.G64P;}

    void setG64PQ(float smooth,float simply) {if(smooth>=0&&simply>=0) {m_data.G64P=smooth;m_data.G64Q=simply;}}

    bool isInitDrillPlane()        {return  m_data.initDrillPlane;}
    void setDrillPlane(double pos) {m_data.drillPlane=pos;}
    double getDrillPlane()         {return m_data.drillPlane;}
    quint32 getDrillPms()            {return m_data.drillPms;}
    quint32 getG4Pms()   {return m_data.G4Pms;}

    void init();

   double getValue(char);
     bool isValidValue(char);
     bool setValue(char name,double data);

     bool isValidGCode(QString Gx="G0");

   WLGPoint getCurPoint();

   WLGPoint getPointGCode(WLGPoint lastGPoint,bool scale=true);

   WLGPoint getPointG28(WLGPoint lastGPoint);
   WLGPoint getPointG53(WLGPoint lastGPoint);
   WLGPoint getPointIJK(WLGPoint lastGPoint);
   WLGPoint getPointSC(int iSC,WLGPoint GPoint,bool back=false);
   WLGPoint getPointActivSC(WLGPoint GPoint,bool back=false);

   WLGPoint movPointToActivSC(int iLastSC,WLGPoint &lastGPoint);

   static WLGPoint convertPlane(WLGPoint Point,int plane,bool front);

   int getActivSC(WLGPoint *P=nullptr);
   
   WLGPoint getSC(int i,bool *ok=nullptr);
   WLGPoint getOffsetSC(int i,bool *ok=nullptr);
   WLGPoint getOffsetActivSC(bool *ok=nullptr) {return getOffsetSC(m_data.iSC,ok);}
   WLGPoint getRefPoint0SC(int i,bool *ok=nullptr);
   WLGPoint getRefPoint1SC(int i,bool *ok=nullptr);

   bool setOffsetActivSC(WLGPoint P)    {return setOffsetSC(m_data.iSC,P);}
   bool setOffsetSC(int i,WLGPoint P,bool send=true);
   
   bool setRefPoint0SC(int i,WLGPoint P)  {if(0<i&&i<sizeSC) {m_data.refPoint0SC[i]=P; return 1;} else return 0;}
   bool setRefPoint1SC(int i,WLGPoint P)  {if(0<i&&i<sizeSC) {m_data.refPoint1SC[i]=P; return 1;} else return 0;}

   void rotAboutRotPointSC(int i,float a);

   void setXSC(double X) {setXSC(X,m_data.iSC);}
   void setYSC(double Y) {setXSC(Y,m_data.iSC);}
   void setZSC(double Z) {setXSC(Z,m_data.iSC);}

   void setXSC(double X,int i)       {m_data.offsetSC[i].x=X;}
   void setYSC(double Y,int i)       {m_data.offsetSC[i].y=Y;}
   void setZSC(double Z,int i)       {m_data.offsetSC[i].z=Z;}
													   					 
   void setOffsetASC(double A,int i) {m_data.offsetSC[i].a=A; }

    bool calcCenterPointR(WLGPoint startPoint,WLGPoint endPoint);
	 
	int setGCode(QString val);
    int setGCode(int val) {return setGCode(QString::number(val));}
	
	void resetGCode(int iG=-1);

    void reset(void) {resetValid();resetGCode();}

    WLGPoint getG28Position()        {return m_data.G28Position;}
    void setG28Position(WLGPoint hp) {m_data.G28Position=hp;}

    WLGPoint getG43Position()        {return m_data.G43Position;}
    void setG43Position(WLGPoint hp) {m_data.G43Position=hp;}

	int getPlane();

	QString getActivGCodeString();
    QStringList getContextGCodeList();

    double getHToolOfst();
    double getDToolOfst();
    double getCompToolRadius() {return qAbs(getDToolOfst()/2.0);}

    void verifyG51();
    void verifyG43();

    QString getStrRunProgram() {return m_data.strRunProgram;}
    QString getStrInit()       {return m_data.strInit;}

    void setStrRunProgram(QString str) {m_data.strRunProgram=str;}
    void setStrInit(QString str) {loadStr(m_data.strInit=str);}

    void setOffsetBackLongDrill(double offset=0) {if(offset>=0) m_data.backOffsetLonfDrill=offset;}
    double getOffsetBackLongDrill() {return  m_data.backOffsetLonfDrill;}

    void setData(const WLGCodeData &data);

    WLGCodeData getData() const;

    WLGCodeData *data() {return &m_data;}

    int getCompSide();

    void setTool(int ikey,WLGTool Tool) {m_data.Tools.setTool(ikey,Tool); emit changedTool(ikey);}

    WLGTool const getTool(int ikey);

    QVariant getDataTool(int ikey,QString key,QVariant defvalue);

    WLGTools *getTools() {return &m_data.Tools;}
private:
    void resetValid();

    QStringList GValidList;

public:

    static bool detectMCode(QString gstr);
public:

    Q_INVOKABLE    void removeTool(int ikey);
    Q_INVOKABLE    void setDataTool(int ikey,QString key,QVariant value,bool send=true);
    Q_INVOKABLE double  getDataToolNum(int ikey,QString key,double  defvalue) {return getDataTool(ikey,key,defvalue).toDouble();}
    Q_INVOKABLE QString getDataToolStr(int ikey,QString key,QString defvalue) {return getDataTool(ikey,key,defvalue).toString();}

    Q_INVOKABLE void setHTool(int i,float h);
    Q_INVOKABLE void setDTool(int i,float d);

    Q_INVOKABLE   int getT() {return getValue('T');}
    Q_INVOKABLE  bool setT(int T) {return setValue('T',T);}
    Q_INVOKABLE double getValue(QString name);

    Q_INVOKABLE double getGSC(){return m_data.iSC+53;}

    Q_INVOKABLE double getHTool(int index);
    Q_INVOKABLE double getDTool(int index);

    Q_INVOKABLE  bool isGCode(int i) {return m_data.GCode[i];}

    Q_INVOKABLE  void push() {m_dataStack=m_data;}

    Q_INVOKABLE  void  pop() {memcpy(m_dataStack.offsetSC,m_data.offsetSC,sizeof(WLGPoint)*sizeSC);
                              memcpy(m_dataStack.refPoint0SC,m_data.refPoint0SC,sizeof(WLGPoint)*sizeSC);
                              memcpy(m_dataStack.refPoint1SC,m_data.refPoint1SC,sizeof(WLGPoint)*sizeSC);

                              m_dataStack.Tools=m_data.Tools;
                              m_data=m_dataStack;
                             }

    Q_INVOKABLE void readToolsFile(QString _fileName);
    Q_INVOKABLE void writeToolsFile(QString _fileName);
private:
    WLGCodeData m_data;

    WLGCodeData m_dataStack;

signals:

void changedSK(int);
void changedF();
void changedTool(int);
void changedTools();

public:

virtual void writeXMLData(QXmlStreamWriter &stream);
virtual void  readXMLData(QXmlStreamReader &stream);
};

#endif // WLGCODE_H
