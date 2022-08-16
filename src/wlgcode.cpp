#include <QFile>
#include <QTextStream>
#include "wl3dpoint.h"
#include "wlgcode.h"

WLGCode::WLGCode()
{
reset();

setGCode(94);
setGCode(98);
setGCode(90);
setGCode(80);
setGCode(17);
setGCode(01);
setGCode(54);
setGCode(61);
setGCode(9);
//setGCode(7); //radius Turn Mode

push();
}

WLGCode::WLGCode(WLGCodeData _data):WLGCode()
{
setData(_data);
}

WLGCode::~WLGCode()
{

}

bool WLGCode::detectMCode(QString gstr)
{
gstr.remove(QRegExp("[(].*[)]"));
gstr.remove(QRegExp("(\\/).*"));
gstr.remove(QRegExp("(\\;).*"));

return gstr.contains(QRegExp("M\\d+"));
}

void WLGCode::removeTool(int ikey)
{
m_data.Tools.takeTool(ikey);
emit changedTools();
}

QList <int> WLGCode::loadStr(QString gstr)
{
QList <int> MList;
int pos = 0;
char a;
QString buf;
quint32 Pms=0;
double d1;
int i1;

gstr.remove(QRegExp("[(].*[)]"));
gstr.remove(QRegExp("(\\/).*"));
gstr.remove(QRegExp("(\\;).*"));

//QRegExp RegExp("[A-Z]{1}[\\s]*[-]?(?:([\\d]+[.]?[\\d]*)|([\\d]?[.]?[\\d]+))");
QRegExp RegExp("[A-Z]{1}[\\s]*[-]?(?:([\\d]+[.]?[\\d]*)|([\\d]?[.]?[\\d]+))(?:([E]([+,-]?[\\d]+))|)");

RegExp.indexIn(gstr);

QTextStream str(&gstr,QIODevice::ReadOnly);

resetValid();

while ((pos = RegExp.indexIn(gstr, pos)) != -1)
{
QString bufStr=gstr.mid(pos,RegExp.matchedLength());

pos += RegExp.matchedLength();

QTextStream str(&bufStr,QIODevice::ReadOnly);

str>>a;

switch(a)
{
 case 'G': str>>buf;
           GValidList+="G"+buf;
           setGCode(buf);
           break;

 case 'P': str>>buf;
           d1=buf.toDouble();

           if(buf.contains(".")) //pause P10 - 10ms, P10. -10000ms
               Pms=d1*1000;
              else
               Pms=d1;

           setValue(a,d1);
           break;
 case 'X':
 case 'Y':
 case 'Z':

 case 'I':
 case 'J':
 case 'K':

//#ifdef DEF_5D
 case 'A':
 case 'B':
 case 'C':
//#endif

 case 'R': 
 case 'Q':
 case 'H':
 case 'D':

 case 'F':
 case 'S':
 case 'T': str>>d1;
           setValue(a,d1);
           break;

 //case 'N':  str>>kadr; //qDebug()<<"N="<<kadr<<a<<f1;
//	        break;
 case 'M': str>>(i1);
           MList+=i1;
//setMCode(i1);
           break;
}

}

if(isValidGCode("G81")
 ||isValidGCode("G83")){
  m_data.drillPms=Pms;
  qDebug()<<"Drill Pms="<<m_data.drillPms;
}


if(isValidGCode("G4")){
  m_data.G4Pms=Pms;
  qDebug()<<"G4 Pms="<<m_data.G4Pms;
}

if(isValidGCode("G64")
&&(isValidValue('P')||isValidValue('Q')))
 {
  setG64PQ(isValidValue('P') ? getValue('P'):0
          ,isValidValue('Q') ? getValue('Q'):0);
 }


return MList;
}

void WLGCode::init()
{
loadStr(m_data.strInit);
}


int WLGCode::getPlane()
{
int plane;  

if(isGCode(19)) plane=19;
else 
if(isGCode(18)) plane=18;
else plane=17;

return plane;
}

QString WLGCode::getActivGCodeString()
{
QString ret;

for(int i=0;i<GCodeSize;i++)    
   if(m_data.GCode[i])
       {
       switch(i)
       {
       case 64: ret+="G64(P"+QString::number(getG64P())
                        +"Q"+QString::number(getG64Q())
                        +") ";
                break;

       case 61: ret+=isStopMode() ? "G61.1 " : "G61 ";

                break;

       default: ret+="G"+QString::number(i)+" ";
       }
       }

if(m_data.G51Scale.x!=1.0
 ||m_data.G51Scale.y!=1.0
 ||m_data.G51Scale.z!=1.0) ret+="\nG51(X"+QString::number(m_data.G51Scale.x)
                                    +" Y"+QString::number(m_data.G51Scale.y)
                                    +" Z"+QString::number(m_data.G51Scale.z)+")";

ret+="\nF"+QString::number(m_data.gF.value);
ret+=" S" +QString::number(m_data.gS.value);
ret+=" T" +QString::number(m_data.gT.value);
ret+=" D" +QString::number(m_data.gD.value);
ret+=" H" +QString::number(m_data.gH.value);

return ret;
}

QStringList WLGCode::getContextGCodeList()
{
QStringList retList;

for(int i=0;i<GCodeSize;i++)
   if(m_data.GCode[i])
      {
       switch(i)
       {
       case 64: retList+=QString("G64 P%1 Q%2").arg(getG64P()).arg(getG64Q());
                break;

       case 61: retList+=isStopMode() ? "G61.1 " : "G61 ";
                break;

       default: retList+="G"+QString::number(i)+" ";
       }
      }

if(m_data.G51Scale.x!=1.0
 ||m_data.G51Scale.y!=1.0
 ||m_data.G51Scale.z!=1.0) retList+=QString("G51 X%1 Y%2 Z%3").arg(m_data.G51Scale.x)
                                                              .arg(m_data.G51Scale.y)
                                                              .arg(m_data.G51Scale.z);

retList+="F"+QString::number(m_data.gF.value);
retList+="S" +QString::number(m_data.gS.value);
retList+="T" +QString::number(m_data.gT.value);
retList+="D" +QString::number(m_data.gD.value);
retList+="H" +QString::number(m_data.gH.value);

return retList;
}


void WLGCode::setDataTool(int ikey,QString key,QVariant value,bool send)
{
WLGTool Tool=getTool(ikey);

bool add=Tool.isEmpty();

Tool.insert(key,value);

setTool(ikey,Tool);

if(send) {
  emit changedTool(ikey);
  }

if(add) {
  emit changedTools();
  }

}

QVariant WLGCode::getDataTool(int ikey, QString key,QVariant defvalue)
{
return m_data.Tools.getValue(ikey,key,defvalue);
}


double WLGCode::getHToolOfst()
{
double ret=0;

quint16 H=(quint16)getValue('H');

if(H>0)
{
 if(isGCode(43)) ret=getDataTool(H,"H",0).toDouble();
   else if(isGCode(44)) ret=-getDataTool(H,"H",0).toDouble();
}
return ret;
}

double WLGCode::getDToolOfst()
{
double ret=0;

quint16 D=(quint16)getValue('D');

if(D!=0){
 ret=getDataTool(D,"D",0).toDouble();
 }

return ret;
}

void WLGCode::verifyG51()
{
if(isGCode(51)) //Scale
  {
  if(isValidValue('X')) m_data.G51Scale.x=getValue('X');
  if(isValidValue('Y')) m_data.G51Scale.y=getValue('Y');
  if(isValidValue('Z')) m_data.G51Scale.z=getValue('Z');

  resetValid();
  resetGCode(51);
}
}

void WLGCode::verifyG43()
{
if((isGCode(43)||isGCode(44))
  &&getValue('H')==0) setGCode("49");
}

void WLGCode::setData(const WLGCodeData &data)
{
m_data = data;
}

WLGCodeData WLGCode::getData() const
{
    return m_data;
}

int WLGCode::getCompSide()
{
int side = isGCode(41) ? 1 //LEFT
                       : (isGCode(42) ? -1 //RIGHT
                                      : 0);

if(getDToolOfst()<0){
 side=-side;
 }

return side;
}

WLGTool const WLGCode::getTool(int ikey)
{
return m_data.Tools.getTool(ikey);
}

void WLGCode::setHTool(int i, float h)
{
qDebug()<<"setHTool"<<i<<h;
setDataTool(i,"H",h);
}

void WLGCode::setDTool(int i, float d)
{
qDebug()<<"setDTool"<<i<<d;
setDataTool(i,"D",d);
}

double WLGCode::getValue(QString name)
{
double ret=0;


if(!name.isEmpty())
{
const char *str=name.toStdString().c_str();

ret=getValue(*str);
}

return ret;
}

double WLGCode::getHTool(int index)
{
return getDataTool(index,"H",0).toDouble();
}

double WLGCode::getDTool(int index)
{
    return getDataTool(index,"D",0).toDouble();
}

void WLGCode::readToolsFile(QString _fileName)
{
getTools()->readFromFile(_fileName);
emit changedTools();
}

void WLGCode::writeToolsFile(QString _fileName)
{
getTools()->writeToFile(_fileName);
}

void WLGCode::writeXMLData(QXmlStreamWriter &stream)
{
stream.writeAttribute("ofstBackLongDrill",QString::number(getOffsetBackLongDrill()));
stream.writeAttribute("strRunProgram",getStrRunProgram());
stream.writeAttribute("strInit",getStrInit());
}

void WLGCode::readXMLData(QXmlStreamReader &stream)
{
setOffsetBackLongDrill(stream.attributes().value("ofstBackLongDrill").toDouble());
setStrRunProgram(stream.attributes().value("strRunProgram").toString());
setStrInit(stream.attributes().value("strInit").toString());
}

void WLGCode::resetValid()
{
m_data.gX.valid=0;
m_data.gY.valid=0;
m_data.gZ.valid=0;

m_data.gI.valid=0;
m_data.gJ.valid=0;
m_data.gK.valid=0;

m_data.gA.valid=0;
m_data.gB.valid=0;
m_data.gC.valid=0;

m_data.gF.valid=0;

m_data.gR.valid=0;
m_data.gP.valid=0;
m_data.gQ.valid=0;

m_data.gS.valid=0;
m_data.gT.valid=0;

m_data.initDrillPlane=false;

GValidList.clear();
}

void WLGCode::resetGCode(int iG)
{
if(iG==-1)
{
for(int i=0;i<GCodeSize;i++)
        m_data.GCode[i]=false;

setGCode(80); //откл 
}
else
{
m_data.GCode[iG]=false;
}
}


int WLGCode::setGCode(QString val)
{
QStringList List=val.split(".");

int code;

if(List.isEmpty()) return -1;

code=List.first().toInt();

switch(code)
   {
   ///**00   
   case 28: m_data.GCode[28]=1;//"Position Zero point";
           break;
   ///**01
   case 0: m_data.GCode[0]=1; //"Position Fast";
           m_data.GCode[1]=0;
           m_data.GCode[2]=0;
           m_data.GCode[3]=0;
           return setGCode(80);

   case 1: m_data.GCode[0]=0;//"Line Interpol";
           m_data.GCode[1]=1;
           m_data.GCode[2]=0;
           m_data.GCode[3]=0;
		   return setGCode(80);           

   case 2: m_data.GCode[0]=0;//"CW";
           m_data.GCode[1]=0;
           m_data.GCode[2]=1;
           m_data.GCode[3]=0;
		   break;

   case 3: m_data.GCode[0]=0;//"CCW";
           m_data.GCode[1]=0;
           m_data.GCode[2]=0;
           m_data.GCode[3]=1;
		   break;

  case 102:
  case 103:m_data.GCode[2]=0;
           m_data.GCode[3]=0;
           break;
//Backlash
  case 9: m_data.GCode[9] =1; //"backlash";
          m_data.GCode[10]=0;
          break;
 case 10: m_data.GCode[9] =0; //"pre backlash";
          m_data.GCode[10]=1;
          break;
   //**03
   case 17:m_data.GCode[17]=1;//"Plane XY";
           m_data.GCode[18]=0;
           m_data.GCode[19]=0;
           m_data.GCode[20]=0;
           break;
   case 18:m_data.GCode[17]=0;//"Plane ZX";
           m_data.GCode[18]=1;
           m_data.GCode[19]=0;
           m_data.GCode[20]=0;
         //  cout<<" no Use";
           break;
   case 19:m_data.GCode[17]=0;//"Plane YZ";
           m_data.GCode[18]=0;
           m_data.GCode[19]=1;
           m_data.GCode[20]=0;
         //  cout<<" no Use";
           break;
   case 20:m_data.GCode[17]=1;//"Plane proizvol";
           m_data.GCode[18]=0;
           m_data.GCode[19]=0;
           m_data.GCode[20]=1;
       //    cout<<" no Use";
           break;
   //**04
   case 21:m_data.GCode[21]=1;//"Razresh kor rab podachi";
           m_data.GCode[22]=0;
           break;
   case 22:m_data.GCode[22]=1;//"Zapret kor rab podachi";
           m_data.GCode[21]=0;
           break;
   //**07
   case 40:m_data.GCode[41]=0;//"Corect R Tool off";
           m_data.GCode[42]=0;
           break;
   case 41:m_data.GCode[41]=1;//"Corect R Tool left";
           m_data.GCode[42]=0;
           break;
   case 42:m_data.GCode[42]=1;//"Corect R Tool right";
           m_data.GCode[41]=0;
           break;
   case 43:m_data.GCode[43]=1;//"Corect Tool H +";
           m_data.GCode[44]=0;
           break;
   case 44:m_data.GCode[44]=1;//"Corect Tool H-";
           m_data.GCode[43]=0;
           break;
   case 49:m_data.GCode[44]=0;//"Corect Tool H Off";
           m_data.GCode[43]=0;
           break;
  //**10
   case 51:m_data.GCode[51]=1;
           break;
   //**10
   case 53:m_data.GCode[53]=1;
           break;
   case 54:
   case 55:
   case 56:
   case 57:
   case 58:
   case 59:
           m_data.GCode[28]=0;
           m_data.GCode[53]=0;

           m_data.GCode[54]=0;
           m_data.GCode[55]=0;
           m_data.GCode[56]=0;
           m_data.GCode[57]=0;
           m_data.GCode[58]=0;
           m_data.GCode[59]=0;
           m_data.GCode[code]=1;

           if(m_data.iSC!=code-53)
            {
            m_data.iSC=code-53;//"Check SK";
            }
           break;
   //--smooth
   case 61: m_data.GCode[61]=1; //No smooth
            m_data.GCode[64]=0;

            if((List.size()>1)&&(List.at(1).toInt()==1))
                m_data.stopMode=true; //stop mode G61.1
            else
                m_data.stopMode=false;
           break;

   case 64:m_data.GCode[61]=0;
           m_data.GCode[64]=1;
           m_data.stopMode=false;
           break;
   //**13
   case 80:m_data.GCode[80]=1;
           m_data.GCode[81]=0; //"Off Drill";
           m_data.GCode[83]=0;
		   break;

   case 81:if((m_data.GCode[83]==0)
            &&(m_data.GCode[81]==0)) m_data.initDrillPlane=true;

           m_data.GCode[81]=1; //"Drill";
           m_data.GCode[83]=0;
           m_data.GCode[80]=0;
           break;

   case 83:if((m_data.GCode[83]==0)
            &&(m_data.GCode[81]==0)) m_data.initDrillPlane=true;

           m_data.GCode[83]=1; //"Long drill";
           m_data.GCode[81]=0;
           m_data.GCode[80]=0;

           break;
   //**14
   case 90:if(List.size()==1)		    
	        {
            m_data.GCode[90]=1; //"Absolut SK";
            m_data.GCode[91]=0;
		    }
		   else
		    {
            if(List[1].toInt()==1) m_data.absIJK=true;
		    }
           break;

   case 91:if(List.size()==1)
		    {
            m_data.GCode[91]=1; //"Increm SK";
            m_data.GCode[90]=0;
		    }
		   else
		    {
            if(List.at(1).toInt()==1) m_data.absIJK=false;
		    }
           break;
   //**17
   case 93:m_data.GCode[93]=1;//"F - time 60sec/F";
           m_data.GCode[94]=0;
           break;
   case 94:m_data.GCode[94]=1;//"F -  vel mm/min;
           m_data.GCode[93]=0;
           break;

   case 98:m_data.GCode[98]=1;//"To Z 80";
           m_data.GCode[99]=0;
           break;
   case 99:m_data.GCode[99]=1;//"To R 80";
           m_data.GCode[98]=0;
           break;
  default: return -1;


   }

return code;
}

bool WLGCode::setValue(char name,double data) 
{
switch(name)
{
  case 'X': m_data.gX.set(data); break;
  case 'Y': m_data.gY.set(data); break;
  case 'Z': m_data.gZ.set(data); break;

  case 'I': m_data.gI.set(data); break;
  case 'J': m_data.gJ.set(data); break;
  case 'K': m_data.gK.set(data); break;

  case 'A': m_data.gA.set(data); break;
  case 'B': m_data.gB.set(data); break;
  case 'C': m_data.gC.set(data); break;
  
  case 'P': m_data.gP.set(data); break;
  case 'Q': m_data.gQ.set(data); break;
  case 'R': m_data.gR.set(data); break;
	  	  
  case 'F': m_data.gF.set(data); break;

  case 'S': m_data.gS.set(data); break;

  case 'T': emit changedTool(m_data.gT.value);
            m_data.gT.set(ceil(fabs(data)));
            emit changedTool(m_data.gT.value);
            break;
  case 'D': m_data.gD.set(ceil(fabs(data))); break;

  case 'H': if(data>=0) {
                m_data.gH.set(ceil(fabs(data)));
                }
           else {              
                return false;
                }

            break;

  default : return false;
}
return true;
}

bool WLGCode::isValidGCode(QString Gx)
{
return GValidList.contains(Gx);
}

WLGPoint WLGCode::getCurPoint()
{
WLGPoint Point=m_data.curGPoint;
return getPointActivSC(Point);
}

WLGPoint WLGCode::convertPlane(WLGPoint Point,int plane,bool front)
{
WLGPoint ret=Point;
if(front)
switch(plane)
{
case 18: ret.x=Point.z;
	     ret.y=Point.x;
	     ret.z=Point.y;
		 break;

case 19: ret.x=Point.y;
	     ret.y=Point.z;
	     ret.z=Point.x;
		 break;
}
else
switch(plane)
{
case 18: ret.z=Point.x;
	     ret.x=Point.y;
	     ret.y=Point.z;
		 break;

case 19: ret.y=Point.x;
	     ret.z=Point.y;
	     ret.x=Point.z;
		 break;
}

return ret;
}

int WLGCode::getActivSC(WLGPoint *P)
{
 if(P!=nullptr)
    *P=isGCode(53) ? WLGPoint():getSC(m_data.iSC);

return isGCode(53) ? 0 : m_data.iSC;
}

double WLGCode::getValue(char name)
{
switch(name)
  {
  case 'X': return m_data.gX.value;
  case 'Y': return m_data.gY.value;
  case 'Z': return m_data.gZ.value;

  case 'I': return m_data.gI.value;
  case 'J': return m_data.gJ.value;
  case 'K': return m_data.gK.value;

  case 'A': return m_data.gA.value;
  case 'B': return m_data.gB.value;
  case 'C': return m_data.gC.value;
  
  case 'P': return m_data.gP.value;
  case 'Q': return m_data.gQ.value;
  case 'R': return m_data.gR.value;
  
  case 'F': return m_data.gF.value;

  case 'S': return m_data.gS.value;

  case 'T': return m_data.gT.value;
  case 'H': return m_data.gH.value;
  case 'D': return m_data.gD.value;

  default : return 0;
 }
}
 
bool WLGCode::isValidValue(char name)
{
switch(name)
 {
  case 'X': return m_data.gX.valid;
  case 'Y': return m_data.gY.valid;
  case 'Z': return m_data.gZ.valid;

  case 'I': return m_data.gI.valid;
  case 'J': return m_data.gJ.valid;
  case 'K': return m_data.gK.valid;

  case 'A': return m_data.gA.valid;
  case 'B': return m_data.gB.valid;
  case 'C': return m_data.gC.valid;

  case 'P': return m_data.gP.valid;
  case 'Q': return m_data.gQ.valid;
  case 'R': return m_data.gR.valid;

  case 'F': return m_data.gF.valid;
  
  case 'S': return m_data.gS.valid;
  case 'T': return m_data.gT.valid;

  case 'H': return m_data.gH.valid;

  default : return false;
 }
}   



WLGPoint WLGCode::getPointGCode(WLGPoint lastPoint,bool scale)
{ 
WLGPoint newPoint=lastPoint;

if(isGCode(91))
	{
    if(isValidValue('X')) newPoint.x+=getValue('X')*(scale ? m_data.G51Scale.x:1.0);
    if(isValidValue('Y')) newPoint.y+=getValue('Y')*(scale ? m_data.G51Scale.y:1.0);
    if(isValidValue('Z')) newPoint.z+=getValue('Z')*(scale ? m_data.G51Scale.z:1.0);

    if(isValidValue('A')) newPoint.a+=getValue('A');
    if(isValidValue('B')) newPoint.b+=getValue('B');
    if(isValidValue('C')) newPoint.c+=getValue('C');
    }
else
    {
    if(isValidValue('X')) newPoint.x=getValue('X')*(scale ? m_data.G51Scale.x:1.0);
    if(isValidValue('Y')) newPoint.y=getValue('Y')*(scale ? m_data.G51Scale.y:1.0);

    if(isValidValue('Z'))
     {
     newPoint.z=getValue('Z')*(scale ? m_data.G51Scale.z:1.0);
     newPoint.z+=getHToolOfst();
     }

    if(isValidValue('A')) newPoint.a=getValue('A');
    if(isValidValue('B')) newPoint.b=getValue('B');
    if(isValidValue('C')) newPoint.c=getValue('C');
    }

return newPoint;
}


WLGPoint WLGCode::getPointSC(int iSC,WLGPoint GPoint,bool back)
{
WLGPoint SC=getSC(iSC);

if(getRefPoint0SC(iSC).a!=0)
{
WLFrame Fr;
WLFrame frP0(getRefPoint0SC(iSC).to3D());

if(back) GPoint=GPoint-SC;

Fr.x=GPoint.x;
Fr.y=GPoint.y;
Fr.z=GPoint.z;

if(back) {
 Fr.fromM(frP0.toM()*getRotMatrix(0,0,-getRefPoint0SC(iSC).a)*frP0.toM().inverted()*Fr.toM());
 }
 else{
 Fr.fromM(frP0.toM()*getRotMatrix(0,0,getRefPoint0SC(iSC).a)*frP0.toM().inverted()*Fr.toM());
 }

GPoint.x=Fr.x;
GPoint.y=Fr.y;
GPoint.z=Fr.z;

if(!back) GPoint=GPoint+SC;

return GPoint;
}
else
 return back? GPoint-SC:GPoint+SC;

}


WLGPoint WLGCode::getPointActivSC(WLGPoint GPoint,bool back)
{
return getPointSC(getActivSC(),GPoint,back);
}

WLGPoint WLGCode::movPointToActivSC(int iLastSC,WLGPoint &lastGPoint)
{      
if(iLastSC!=m_data.iSC)
 {
 lastGPoint=getPointSC(iLastSC,lastGPoint);
 lastGPoint=getPointActivSC(lastGPoint,true);
 }

return lastGPoint;
}


WLGPoint WLGCode::getPointIJK(WLGPoint lastGPoint)
{
WLGPoint newPoint=lastGPoint;

if(m_data.absIJK)
 {
 switch(getPlane())
  {
  case 17: if(isValidValue('I')) newPoint.x=getValue('I')*m_data.G51Scale.x;
           if(isValidValue('J')) newPoint.y=getValue('J')*m_data.G51Scale.y;
           if(isValidValue('K')) newPoint.z=getValue('K')*m_data.G51Scale.z;
           break;

  case 18: if(isValidValue('I')) newPoint.x=getValue('I')*m_data.G51Scale.x;
           if(isValidValue('K')) newPoint.z=getValue('K')*m_data.G51Scale.z;
           if(isValidValue('J')) newPoint.y=getValue('J')*m_data.G51Scale.y;
           break;

  case 19: if(isValidValue('J')) newPoint.y=getValue('J')*m_data.G51Scale.y;
           if(isValidValue('K')) newPoint.z=getValue('K')*m_data.G51Scale.z;
           if(isValidValue('I')) newPoint.x=getValue('I')*m_data.G51Scale.x;
           break;
  }

 }
else
 {
 switch(getPlane())
  {
  case 17: if(isValidValue('I')) newPoint.x+=getValue('I')*m_data.G51Scale.x;
           if(isValidValue('J')) newPoint.y+=getValue('J')*m_data.G51Scale.y;
           if(isValidValue('K')) newPoint.z+=getValue('K')*m_data.G51Scale.z;
           break;

  case 18: if(isValidValue('I')) newPoint.x+=getValue('I')*m_data.G51Scale.x;
           if(isValidValue('K')) newPoint.z+=getValue('K')*m_data.G51Scale.z;
           if(isValidValue('J')) newPoint.y+=getValue('J')*m_data.G51Scale.y;
           break;

  case 19: if(isValidValue('J')) newPoint.y+=getValue('J')*m_data.G51Scale.y;
           if(isValidValue('K')) newPoint.z+=getValue('K')*m_data.G51Scale.z;
           if(isValidValue('I')) newPoint.x+=getValue('I')*m_data.G51Scale.x;
           break;
  }
 }

return newPoint;
}

WLGPoint WLGCode::getPointG28(WLGPoint lastGPoint)
{ 
WLGPoint newPoint=getPointActivSC(lastGPoint);

if(isValidValue('X')) {newPoint.x=m_data.G28Position.x;}
if(isValidValue('Y')) {newPoint.y=m_data.G28Position.y;}
if(isValidValue('Z')) {newPoint.z=m_data.G28Position.z;}
if(isValidValue('A')) {newPoint.a=m_data.G28Position.a;}
if(isValidValue('B')) {newPoint.b=m_data.G28Position.b;}
if(isValidValue('C')) {newPoint.c=m_data.G28Position.c;}
if(isValidValue('U')) {newPoint.u=m_data.G28Position.u;}
if(isValidValue('V')) {newPoint.v=m_data.G28Position.v;}
if(isValidValue('W')) {newPoint.w=m_data.G28Position.w;}

return getPointActivSC(newPoint,true);
}

WLGPoint WLGCode::getPointG53(WLGPoint lastGPoint)
{
WLGPoint newPoint=getPointActivSC(lastGPoint);

newPoint=getPointGCode(newPoint,false);

if(isGCode(90)
 &&isValidValue('Z')) newPoint.z-=getHToolOfst();

return getPointActivSC(newPoint,true);
}



void  WLGCode::rotAboutRotPointSC(int i,float a)
{
m_data.refPoint0SC[i].a=a;

emit changedSK(i);
}



WLGPoint  WLGCode::getSC(int i,bool *ok)
{
if(0<=i&&i<sizeSC) 
  {
  if(ok) *ok=true;
  return m_data.offsetSC[i];
  } 
else 
  {
  if(ok) *ok=false;
  return WLGPoint();
  }
}

WLGPoint  WLGCode::getRefPoint0SC(int i,bool *ok)
{
if(0<=i&&i<sizeSC) 
  {
  if(ok) *ok=true;
  return m_data.refPoint0SC[i];
  } 
else 
  {
  if(ok) *ok=false;
  return WLGPoint();
  }
}

WLGPoint  WLGCode::getRefPoint1SC(int i,bool *ok)
{
if(0<=i&&i<sizeSC) 
  {
  if(ok) *ok=true;
  return m_data.refPoint1SC[i];
  } 
else 
  {
  if(ok) *ok=false;
  return WLGPoint();
  }
}

bool WLGCode::setOffsetSC(int i, WLGPoint P,bool send)
{
if(0<i&&i<sizeSC)
   {
   m_data.offsetSC[i]=P;

   if(send) emit changedSK(m_data.iSC);
   return 1;
   }

return 0;
}

WLGPoint  WLGCode::getOffsetSC(int i,bool *ok)
{
 if(0<=i&&i<sizeSC)
  {
  if(ok) *ok=true;
  return m_data.offsetSC[i];
  } 
else 
  {
  if(ok) *ok=false;
  return WLGPoint();
  }
}
 


bool WLGCode::calcCenterPointR(WLGPoint startGPoint,WLGPoint endGPoint)
{
//qDebug()<<"calcIJfromR";
WL3DPoint startPoint;
WL3DPoint   endPoint;
const int plane=getPlane();

startPoint=WLGCode::convertPlane(startGPoint,plane,true).to3D();
  endPoint=WLGCode::convertPlane(endGPoint,plane,true).to3D();

WL3DPoint O=(endPoint+startPoint)/2;//Находим среднюю точку
WL3DPoint C=(endPoint-O);          //Находим вектор от средней точки к концу
WL3DPoint N,Pr;

if((startPoint.x==endPoint.x)
 &&(startPoint.y==endPoint.y))
   return true; //отбрасываем

if(!isValidValue('R'))
    return false;

double R=getValue('R');

if(R==0.0)
    return false;

if((R<0.0&&isGCode(3))   //+R   0-179.999 gr
 ||(R>0.0&&isGCode(2)))  //-R 180-359.999gr
    N.set(0,0,1);
  else
    N.set(0,0,-1);


double  R1=sqrt(C.x*C.x+C.y*C.y); //находим расстояния от средней точки к концу

if(R1<0)
    return false;

double  Rnq=R*R-R1*R1;//находим размер отвода

if(R1>R
 &&R1<R*1.01)
    Rnq=0.0;

if(Rnq<0)
    return false;

O+=(C*N).normalize()*sqrt(Rnq)-startPoint; //находим разницу от начала к центру если не абс положение;//находим перпендикуляр и прибавляем к средней точке

WLGPoint GIJK=WLGCode::convertPlane(WLGPoint(O),plane,false);

if(m_data.absIJK){
 GIJK=GIJK+startGPoint;
 }

setValue('I',GIJK.x);//устанавливаем величины
setValue('J',GIJK.y);
setValue('K',GIJK.z);

return true;
}

WLGCodeData::WLGCodeData()
{
G51Scale.x=1;
G51Scale.y=1;
G51Scale.z=1;

absIJK=false;

iCurTool=1;

gF.value=200;
gS.value=0;
gT.value=0;
gD.value=0;
}
