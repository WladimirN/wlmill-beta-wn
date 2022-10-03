#include "wlgprogram.h"
#include <math.h>
#include <QTextStream>
#include <QRegExp>

const QString StartProg="start_program";
const QString EndProg  ="end_program";

#define M_PI 3.14159265358979323846

WLGProgram::WLGProgram(QObject *parent)
    : QObject(parent)
{
m_colorF=WL3DPointf(1,0,0);
m_colorG=WL3DPointf(0,0,1);
m_colorBl=WL3DPointf(0,1,0);

m_buildShow=0;
m_build=0;

m_maxShowPoints=DEF_maxShowPoints;

iActivElement=0;
iLastMovElement=0;

threadProg = new QThread;
threadProg->start(QThread::IdlePriority);
moveToThread(threadProg);	

m_buildElement=0;

time=0;
activ=0;

m_showGCode=nullptr;

m_totalKadr=0;
}

WLGProgram::~WLGProgram()
{
qDebug()<<"~WLGProgram()";

stopBuildShow();

QFile::remove(QCoreApplication::applicationDirPath()+"//prog.bkp");

threadProg->quit();
threadProg->wait();
}


void WLGProgram::setTextProgram(QString txt)
{
QFile nFile(m_fileName);

nFile.open(QIODevice::WriteOnly);
nFile.write(QTextCodec::codecForName("Windows-1251")->fromUnicode(txt));;
nFile.close();

reloadFile(true);
}


bool WLGProgram::loadFile(QString file,bool build)
{
qDebug()<<"WLGProgram::loadFile"<<file;

bool ret=false;

if(!QFileInfo::exists(file))
  {
  qDebug()<<"no file:"<<file;
  return ret;
  }

stopBuildShow();

QMutexLocker loker(&MutexShowBuild);

char buf;

WLElementGProgram EP;

EP.offsetInFile=0;
EP.offsetPoint=0;

Mutex.lock();

m_fileName=file;

indexData.clear();
m_toolList.clear();
m_scList.clear();

if(File.isOpen()) File.close();

QFile::remove(QCoreApplication::applicationDirPath()+"//prog.bkp");
QFile srcFile(m_fileName);

File.setFileName(QCoreApplication::applicationDirPath()+"//prog.bkp");

if(srcFile.open(QIODevice::ReadOnly)
    &&File.open(QIODevice::WriteOnly)){

while(!srcFile.atEnd())
    {
    srcFile.getChar(&buf);

    if(buf=='\r') continue;

    File.putChar(buf);

    if(buf=='\n')
       {
       EP.offsetInFile=File.pos();
       indexData+=EP;       
       }
    }

EP.offsetInFile=File.pos();
indexData+=EP;

File.close();
}

if(File.open(QIODevice::ReadOnly)){
 ret=true;
 }

Mutex.unlock();

WLGCode GCode;

for(int i=0;i<getElementCount();i++)
    {
    QString str=getTextElement(i);
    GCode.loadStr(str);

    if(GCode.getSC()!=0 && m_scList.indexOf(GCode.getSC())==-1)
       m_scList+=GCode.getSC();

    if(GCode.getT()!=0 && m_toolList.indexOf(GCode.getT())==-1)
       m_toolList+=GCode.getT();
    }

qDebug()<<"End check program"<<" T"<<m_toolList.size()<<" SC"<<m_scList.size();

changedToolList(m_toolList);
changedSCList(m_scList);


if(build) QTimer::singleShot(0,this,SLOT(updateShowTraj()));

emit changedProgram();
emit changedTrajSize(getElementCount());

return ret;
}


void WLGProgram::saveFile(QString file)
{ 
File.copy(file);
}


void WLGProgram::calcTime()
{
emit changedTime(++time);
}

long WLGProgram::getNKadr(QString data)
{
if(data.isNull()) return 0;

QTextStream str(&data,QIODevice::ReadOnly);

QString buf;

str>>buf;

return buf.remove("N").toLong();
}


void WLGProgram::buildShowTraj(WLGCodeData GCodeData)
{
if(!m_buildShow) return;

GCodeData.lastGPoint.x=qQNaN();
GCodeData.lastGPoint.y=qQNaN();
GCodeData.lastGPoint.z=qQNaN();

QMutexLocker locker(&MutexShowBuild);
WLGCode GCode(GCodeData);
QList <WLElementTraj> curListTraj;
QList <WL6DPoint> Points;

WLShowPointProgram Point;

WL3DPointf color;

int istart=-1;

bool fast=false;
bool ok=true;
bool ende=false;

bool ch_delta=true;
int iLast=0;

QString txt;
//GCode.reset();

MutexShowPoint.lock();
showPoints.clear();
MutexShowPoint.unlock();

showPoints.reserve(m_maxShowPoints+1000);

minShowPoint=maxShowPoint=WL3DPointf();

GCode.loadStr(GCode.getStrRunProgram());

emit startedBuildShow();
emit changedShowProgress(0);

for(qint32 index=0;(index<indexData.size())
                 &&(showPoints.size()<m_maxShowPoints)
                 &&(m_buildShow&&ok);index++)
  {    
  txt=getTextElement(index);

  ende= index == (indexData.size()-1);

  if(!translate(txt,curListTraj,&GCode,index)) {
      sendMessage(getName(),QString("error G code in line %1: %2").arg(index).arg(txt),-1);
      break;
      }

  indexData[index].offsetPoint=0;

  if(index%10000==0) emit changedShowProgress(50+((float)index*50)/indexData.size());

  MutexShowPoint.lock();

  if(showPoints.isEmpty()
    &&!curListTraj.isEmpty()
    &&curListTraj.first().isLine()) fast=curListTraj.first().isFast();

  for(int j=0;(ende && curListTraj.size()>0)
                     ||curListTraj.size()>1;)
    {
    WLElementTraj ETraj = curListTraj.takeFirst();

    Point.select=ETraj.index;

    m_GModel.setOffsetFrame(GCode.getOffsetSC(GCode.getActivSC()).to3D());

    if(istart!=-1)
      Points=ETraj.calcPoints(&ok,&m_GModel,1);
    else
      Points.clear();

    Point.color = m_colorG;

    if(ETraj.isLine())
       Point.color = ETraj.isFast()? m_colorF : m_colorG;

    ch_delta=!ch_delta;

     if((fast!=ETraj.isFast())
      ||Points.isEmpty())
           {
           if(!showPoints.isEmpty())
		     {
             Point.pos=showPoints.last().pos;
             showPoints+=Point;
		     }
            fast=ETraj.isFast();
            }

     for(;j<Points.size()
       &&(showPoints.size()<m_maxShowPoints);j++)
        {
        Point.pos=Points[j].to3Df();
        showPoints+=Point;
        }
     j=1;

    indexData[ETraj.index].offsetPoint=showPoints.size();
    }

    if(istart==-1)
	   {
       if(GCode.data()->lastGPoint.isValid())
                  {
                  istart=0;//showPoints.size()-1;
                  for(int j=0;j<istart;j++)
                          showPoints[j].pos=showPoints[istart].pos;
                  }
	   }
    else
    if(!showPoints.isEmpty())
    {
    if(iLast==0) maxShowPoint=minShowPoint=showPoints[iLast++].pos;

    for (;iLast<showPoints.size();iLast++)
     {
     if(maxShowPoint.x<showPoints[iLast].pos.x)
         maxShowPoint.x=showPoints[iLast].pos.x;
     else
         if (minShowPoint.x>showPoints[iLast].pos.x)
             minShowPoint.x=showPoints[iLast].pos.x;

     if(maxShowPoint.y<showPoints[iLast].pos.y)
         maxShowPoint.y=showPoints[iLast].pos.y;
     else
         if (minShowPoint.y>showPoints[iLast].pos.y)
             minShowPoint.y=showPoints[iLast].pos.y;

     if(maxShowPoint.z<showPoints[iLast].pos.z)
         maxShowPoint.z=showPoints[iLast].pos.z;
     else
         if (minShowPoint.z>showPoints[iLast].pos.z)
             minShowPoint.z=showPoints[iLast].pos.z;
     }

    }
    MutexShowPoint.unlock();
  }


qDebug()<<maxShowPoint.to3D().toString();
qDebug()<<minShowPoint.to3D().toString();

emit changedShowTraj();
emit changedShowProgress(100);

qDebug()<<"end buildShowTraj"<<showPoints.size()<<m_maxShowPoints;

}

QString WLGProgram::getTextProgram()
{
QMutexLocker locker(&Mutex);
File.seek(0);
return QTextCodec::codecForName("Windows-1251")->toUnicode(File.readAll());
}

QString  WLGProgram::getTextElement(qint32 index)
{
QString ret="";

QMutexLocker locker(&Mutex);

if(0<=index&&index<indexData.size())
 {
 if(index==0)
     {
     File.seek(0);
     ret=QTextCodec::codecForName("Windows-1251")->toUnicode(File.read(indexData[index].offsetInFile));
     }
   else
     {
     File.seek(indexData[index-1].offsetInFile);
     ret=QTextCodec::codecForName("Windows-1251")->toUnicode(File.read(indexData[index].offsetInFile-indexData[index-1].offsetInFile));
     }
  }

return ret;
}

bool WLGProgram::translate(QString dataStr,QList <WLElementTraj> &curListTraj,WLGCode *GCode,qint32 _index,bool GCodeOnly)
{
Q_UNUSED(GCodeOnly)
if(dataStr.isNull()) return  true;

WLElementTraj ElementTraj;

QString com,data,buf;
data=dataStr.toUpper();

double lastS=GCode->getValue('S');

bool ok=true;;

//qDebug()<<dataStr;

int iLastSC=GCode->getActivSC();
int iLastToolOfst=GCode->getOfstTool();

QList <int> MList;

int lastT=GCode->getT();

MList=GCode->loadStr(data);

if(GCode->getValue('F')<0) return false;

GCode->verifyG51();
GCode->verifyG43();

ElementTraj.useHMap=true;

if(GCode->isGCode(53))
{
ElementTraj.useHMap=false;
GCode->resetGCode(53);
GCode->data()->curGPoint=GCode->getPointG53(GCode->data()->lastGPoint);
}
else
 {
 ElementTraj.useHMap=true;
 GCode->movPointToActivSC(iLastSC,GCode->data()->lastGPoint);
 GCode->data()->curGPoint=GCode->getPointGCode(GCode->data()->lastGPoint);
 }

ElementTraj.index=_index;
ElementTraj.str+=dataStr;

ElementTraj.setF(GCode->getValue('F'));
ElementTraj.setS(GCode->getValue('S'));

if(!data.contains("G"))
{
#ifdef GCODE_TURN
if(lastT!=GCode->getT())
  ElementTraj.setScript(GCode->getPointActivSC(GCode->data()->curGPoint),QString("changeTool(%1,%2)").arg(GCode->getT()).arg(lastT));
break;
#endif

if(GCode->isValidValue('P'))
   GCode->setOffsetTool(GCode->getValue('P'));
}

/* добавляет пустой элемент при изменении S
if(lastS!=(GCode->getValue('S'))){
 ElementTraj.setDelay(GCode->getPointActivSC(GCode->data()->curGPoint),0);
 if(ok) curListTraj+=ElementTraj;
 }
*/

if(GCode->isGCode(64)) //устанавливаем тип перемещения
   {
   ElementTraj.setSmooth(GCode->getG64P(),GCode->getG64Q());
   }
else
   {
   ElementTraj.setStopMode(GCode->isStopMode());
   }

if(GCode->isGCode(111)) {
  ElementTraj.setPreBacklash();
  }

if(GCode->isValidValue('R') //Если R для круга то перещитываем в I и J
 &&(GCode->isGCode(2)||GCode->isGCode(3))
 &&GCode->isGCode(80)){
                      ok=GCode->calcCenterPointR(GCode->data()->lastGPoint
                                                ,GCode->data()->curGPoint);
                      }

if(!ok) return ok;

if(GCode->isValidGCode("G4")) //dwell pause
{    
GCode->resetGCode(4);

 if(GCode->isValidValue('P'))
   {
   ElementTraj.setDelay(GCode->getPointActivSC(GCode->data()->curGPoint),GCode->getG4Pms());
   if(ok) curListTraj+=ElementTraj;
   }
}
else if(GCode->isGCode(28))
      {
      ElementTraj.F=-1;

      ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)  //middle
                         ,GCode->getPointActivSC(GCode->data()->curGPoint));

      if(ok) curListTraj+=ElementTraj;

      GCode->data()->lastGPoint=GCode->data()->curGPoint;

      GCode->data()->curGPoint=GCode->getPointG28(GCode->data()->lastGPoint);

      ok=ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)  //end
                            ,GCode->getPointActivSC(GCode->data()->curGPoint));

      if(ok) curListTraj+=ElementTraj;

      GCode->resetGCode(28);
      }
      else if(((GCode->isGCode(81)  //drill
              ||GCode->isGCode(83))
              &&
                (GCode->isValidValue('X')
               ||GCode->isValidValue('Y')
               ||GCode->isValidValue('Z')

               ||GCode->isValidValue('A')
               ||GCode->isValidValue('B')
               ||GCode->isValidValue('C'))))
             {
             ElementTraj.setStopMode(GCode->isStopMode());

             ok=calcDrill(ElementTraj,curListTraj,GCode->data()->lastGPoint,GCode);

             GCode->data()->curGPoint=GCode->data()->lastGPoint;//т.к. меняет last в конце
             }
             else if(GCode->isValidValue('I') //circ
                   ||GCode->isValidValue('J')
                   ||GCode->isValidValue('K'))
                    {
                    if(GCode->data()->lastGPoint.x==qInf()
                     ||GCode->data()->lastGPoint.y==qInf()
                     ||GCode->data()->lastGPoint.z==qInf())
                    {
                    ok=false;
                    }
                    else {
                    if(GCode->isGCode(0)) ElementTraj.F=-1;

                    ok=convertArc(ElementTraj,curListTraj,GCode);
                    }
                   }
                   else if(GCode->isValidValue('X') //line
                         ||GCode->isValidValue('Y')
                         ||GCode->isValidValue('Z')

                         ||GCode->isValidValue('A')
                         ||GCode->isValidValue('B')
                         ||GCode->isValidValue('C'))
                           {
                           ElementTraj.data.line.G93 = GCode->isGCode(93);

                           if(GCode->isGCode(0)) ElementTraj.F=-1;

                           ok=convertLine(ElementTraj,curListTraj,GCode);
                           }
                           else if(lastS!=ElementTraj.S){
                                ElementTraj.setDelay(GCode->getPointActivSC(GCode->data()->curGPoint),0);
                                curListTraj+=ElementTraj;
                                }
                                else if(MList.isEmpty()){
                                     curListTraj+=ElementTraj;
                                     }

GCode->data()->lastGPoint=GCode->data()->curGPoint;

if(ok){
 foreach(int MCode,MList){ 
 ElementTraj.setScript(GCode->getPointActivSC(GCode->data()->curGPoint),QString("M%1()").arg(MCode));
 curListTraj.append(ElementTraj);
 }
}

return ok;
}

bool WLGProgram::calcDrill(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGPoint &lastGPoint,WLGCode *GCode)
{	
if(ElementTraj.F<=0) return false;

WLGPoint Point;

float F=ElementTraj.F;

Point=GCode->getPointGCode(lastGPoint);

if(GCode->isInitDrillPlane())  GCode->setDrillPlane(lastGPoint.z);

ElementTraj.type=WLElementTraj::line;
ElementTraj.setFast();            //??????
ElementTraj.data.line.startPoint=GCode->getPointActivSC(lastGPoint);
ElementTraj.data.line.endPoint=GCode->getPointActivSC(GCode->getPointGCode(lastGPoint));

ElementTraj.data.line.endPoint.z=ElementTraj.data.line.startPoint.z;

//qDebug()<<"Z-="<<ElementTraj.endPoint.z;

QString str=ElementTraj.str;
curListTraj+=ElementTraj;

ElementTraj.setFast();            //Быстро опускаемся
ElementTraj.str=str+"//fast plane R";
ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

Point=GCode->getPointGCode(lastGPoint);
Point.z=GCode->getValue('R')+GCode->getHToolOfst();

ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;

curListTraj+=ElementTraj;

if(GCode->isGCode(81))               //простой цикл
  {
  ElementTraj.setF(F);            //сверлим
  ElementTraj.str=str+"//drill to Z";
  ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

  Point=GCode->getPointGCode(lastGPoint);
  Point.z=GCode->getValue('Z')+GCode->getHToolOfst();

  ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;

  curListTraj+=ElementTraj;

  if(GCode->getDrillPms()!=0) //задержка внизу сверления
     {
     WLElementTraj ElementP=ElementTraj;
     ElementP.setDelay(ElementTraj.data.line.endPoint,GCode->getDrillPms());
     curListTraj+=ElementTraj;
     }

  ElementTraj.setFast();              //подъём
  ElementTraj.str=str+"//back R";
  ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

  Point=GCode->getPointGCode(lastGPoint);

  if(GCode->isGCode(99))
     Point.z=GCode->getValue('R')+GCode->getHToolOfst();
    else //98
     Point.z=GCode->getDrillPlane();

  ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;

  curListTraj+=ElementTraj;

  lastGPoint=GCode->getPointGCode(lastGPoint);
  lastGPoint.z=Point.z;
  return true;
  }
else
if(GCode->isGCode(83))               //с подъёмами
  {
  if(GCode->getValue('Q')==0) {return false;}

  float dist=GCode->getValue('R')-GCode->getValue('Z');
  int   n=dist/GCode->getValue('Q');
  float step=dist/n;
/*
  ElementTraj.setF(F);            //сверлим
  ElementTraj.str=str+"//drill to Z";
  ElementTraj.startPoint=ElementTraj.endPoint;

  Point=GCode->getPointGCode(lastGPoint);
  Point.z=GCode->getValue('R')+GCode->getHofst();
*/
  for(int i=0;i<n;i++)
   {
   if(i!=0){ //быстрое опускание к сверлению
           ElementTraj.setFast();
           ElementTraj.str=str+"//back to drill";
           ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

           Point=GCode->getPointGCode(lastGPoint);
           Point.z=GCode->getValue('R')+GCode->getHToolOfst();
           Point.z-=step*(i+1-1);

           if(GCode->getOffsetBackLongDrill()>0)
            {
            if((Point.z+GCode->getOffsetBackLongDrill())<=GCode->getValue('R'))
                 {
                 Point.z+=GCode->getOffsetBackLongDrill();
                 ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;
                 curListTraj+=ElementTraj;
                 }
            }
            else {
                 ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;
                 curListTraj+=ElementTraj;
                 }
           }

   ElementTraj.setF(F);            //сверлим
   ElementTraj.str=str+"//drill to Z";
   ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

   Point=GCode->getPointGCode(lastGPoint);
   Point.z=GCode->getValue('R')+GCode->getHToolOfst();

   Point.z-=step*(i+1);

   ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;
   curListTraj+=ElementTraj;

   if(GCode->getDrillPms()!=0) //задержка внизу траектории
      {
      WLElementTraj ElementP=ElementTraj;
      ElementP.setDelay(ElementTraj.data.line.endPoint,GCode->getDrillPms());
      curListTraj+=ElementTraj;
      }
   
   if((i+1)<n)
   {
   ElementTraj.setFast();            //подъём
   ElementTraj.str=str+"//back R";
   ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

   Point=GCode->getPointGCode(lastGPoint);
   Point.z=GCode->getValue('R')+GCode->getHToolOfst();

   ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;
   curListTraj+=ElementTraj;
   }
   }


  if(GCode->getDrillPms()>0){
    WLElementTraj   EDelay;
    EDelay=ElementTraj;
    EDelay.str=str+"//dwell P";
    EDelay.setDelay(ElementTraj.data.line.endPoint,GCode->getDrillPms());
    curListTraj+=EDelay;
    }

  ElementTraj.setFast();              //подъём окончательный
  ElementTraj.str=str+"//back R";
  ElementTraj.data.line.startPoint=ElementTraj.data.line.endPoint;

  Point=GCode->getPointGCode(lastGPoint);

  if(GCode->isGCode(99))
     Point.z=GCode->getValue('R')+GCode->getHToolOfst();
    else //98
     Point.z=GCode->getDrillPlane();

  ElementTraj.data.line.endPoint.z=GCode->getPointActivSC(Point).z;

  curListTraj+=ElementTraj;

  lastGPoint=GCode->getPointGCode(lastGPoint);
  lastGPoint.z=Point.z;
  return true;
  }


return false;
}

void WLGProgram::clear()
{
QFile::remove(QCoreApplication::applicationDirPath()+"//clear.ncc");
QFile clearFile(QCoreApplication::applicationDirPath()+"//clear.ncc");

if(clearFile.open(QIODevice::WriteOnly))
{
clearFile.write("// WLMill");
clearFile.close();

loadFile(clearFile.fileName(),true);
}

}


#define LEFT   1
#define RIGHT -1
#define TOLERANCE_CONCAVE_CORNER 0.05
#define TOLERANCE_EQUAL 0.0001

#define CENTER_ARC_RADIUS_TOLERANCE_MM (2 * 0.01 * M_SQRT2)
#define MIN_CENTER_ARC_RADIUS_TOLERANCE_MM 0.001

#ifndef M_PIl
#define M_PIl		3.1415926535897932384626433832795029L  /* pi */
#endif

#ifndef M_PI_2l
#define M_PI_2l        1.570796326794896619231321691639751442L /* pi/2 */
#endif
#ifndef M_PI
#define M_PI		3.1415926535897932384626433832795029   /* pi */
#endif

#define TOOL_INSIDE_ARC(side, turn) (((side)==LEFT&&(turn))||((side)==RIGHT&&(!turn)))

template<class T>
T SQ(T a) { return a*a; }

bool WLGProgram::convertLine(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
bool ok;

if(ElementTraj.F<=0
&&!GCode->isGCode(0)) return false;

if((!GCode->isGCode(41)
  &&!GCode->isGCode(42))
   ||GCode->getCompToolRadius()==0){

  WLGPoint startGPoint;

  startGPoint=GCode->data()->lastGPoint;

  if(!GCode->data()->cutter_comp_firstmove) {
     startGPoint.x=GCode->data()->curPoint.x;
     startGPoint.y=GCode->data()->curPoint.y;
     }

  ok=ElementTraj.setLine(GCode->getPointActivSC(startGPoint)
                        ,GCode->getPointActivSC(GCode->data()->curGPoint));

  if(ok) curListTraj+=ElementTraj;

  GCode->data()->curPoint=GCode->data()->curGPoint.to3D();
  GCode->data()->endPoint=GCode->data()->lastGPoint.to3D();

  GCode->data()->cutter_comp_firstmove=true;

  return ok;
  }
  else if(GCode->data()->cutter_comp_firstmove||curListTraj.isEmpty()){
       return  convert_straight_comp1(ElementTraj,curListTraj,GCode);
       }
       else if(curListTraj.isEmpty()){
            qDebug()<<"convert_straight_comp2 - empty List";
            return false;
            }
            else{
            return  convert_straight_comp2(ElementTraj,curListTraj,GCode);
            }
}

bool WLGProgram::convert_straight_comp1(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
    qDebug()<<"WLGProgram::convert_straight_comp1";
    double alpha;
    double distance;
  //double radius = settings->cutter_comp_radius; /* always will be positive */
    double radius = GCode->getCompToolRadius(); /* always will be positive */
    double end_x, end_y;

    int side = GCode->getCompSide();

    double cx, cy, cz;

    cx=GCode->data()->curPoint.x;
    cy=GCode->data()->curPoint.y;
    cz=GCode->data()->curPoint.z;

    double px, py, pz;

    px=GCode->data()->curGPoint.x;
    py=GCode->data()->curGPoint.y;
    pz=GCode->data()->curGPoint.z;

    distance = hypot((px - cx), (py - cy));

    if((side != LEFT) && (side != RIGHT)) {
      qDebug()<<"error side G41/42";
      return false;
      }

    if(distance <= radius){
      qDebug()<<"Length of cutter compensation entry move is not greater than the tool radius";
      //return false;
      }

    alpha = atan2(py - cy, px - cx) + (side == LEFT ? M_PIl/2. : -M_PIl/2.);

    end_x = (px + (radius * cos(alpha)));
    end_y = (py + (radius * sin(alpha)));

    // with these moves we don't need to record the direction vector.
    // they cannot get reversed because they are guaranteed to be long
    // enough.

    GCode->data()->endPoint.x=cx;
    GCode->data()->endPoint.y=cy;

    WLGPoint endGPoint=GCode->data()->curGPoint;

    endGPoint.x=end_x;
    endGPoint.y=end_y;

    GCode->data()->endPoint.x=end_x;
    GCode->data()->endPoint.y=end_y;

    if (GCode->isGCode(0)) {
        ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)
                           ,GCode->getPointActivSC(endGPoint));

        curListTraj+=ElementTraj;
    }
    else if (GCode->isGCode(1)) {
        ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)
                           ,GCode->getPointActivSC(endGPoint));
        curListTraj+=ElementTraj;
    } else{
        qDebug()<<"error line comp1";
        return false;
        }

    GCode->data()->cutter_comp_firstmove = false;

    GCode->data()->curPoint.x=end_x;
    GCode->data()->curPoint.y=end_y;
    GCode->data()->curPoint.z=pz;

    /*
    comp_set_current(settings, end_x, end_y, pz);
    settings->AA_current = AA_end;
    settings->BB_current = BB_end;
    settings->CC_current = CC_end;
    settings->u_current = u_end;
    settings->v_current = v_end;
    settings->w_current = w_end;
    comp_set_programmed(settings, px, py, pz);
    */
    return true;
}

bool WLGProgram::convert_straight_comp2(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
qDebug()<<"WLGProgram::convert_straight_comp2";
double alpha;
double beta;
double end_x, end_y, end_z;                 /* x-coordinate of actual end point */
double gamma;
double mid_x, mid_y;                 /* x-coordinate of end of added arc, if needed */
double radius;
int side;
double small = TOLERANCE_CONCAVE_CORNER;      /* radians, testing corners */
double opx = 0, opy = 0, opz = 0;      /* old programmed beginning point */
double theta;
double cx, cy, cz;
double px, py, pz;
int concave;

cx=end_x=GCode->data()->curPoint.x;
cy=end_y=GCode->data()->curPoint.y;
cz=end_z=GCode->data()->curPoint.z;

px=GCode->data()->curGPoint.x;
py=GCode->data()->curGPoint.y;
pz=GCode->data()->curGPoint.z;

opx=GCode->data()->lastGPoint.x;
opy=GCode->data()->lastGPoint.y;
opz=GCode->data()->lastGPoint.z;

if ((py == opy) && (px == opx)) {     /* no XY motion */
    if (GCode->isGCode(0)) {
        ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)
                              ,GCode->getPointActivSC(GCode->data()->curGPoint));

        curListTraj+=ElementTraj;
    } else if (GCode->isGCode(1)) {
        ElementTraj.setLine(GCode->getPointActivSC(GCode->data()->lastGPoint)
                              ,GCode->getPointActivSC(GCode->data()->curGPoint));

        curListTraj+=ElementTraj;
    } else{
       qDebug()<<"error no G0/G1";
       return false;
    }
    // end already filled out, above
} else {
    // some XY motion
    side = GCode->getCompSide();;
    radius = GCode->getCompToolRadius();      /* will always be positive */
    theta = atan2(cy - opy, cx - opx);
    alpha = atan2(py - opy, px - opx);

    if (side == LEFT) {
        if (theta < alpha)
            theta = (theta + (2 * M_PIl));
        beta = ((theta - alpha) - M_PI_2l);
        gamma = M_PI_2l;
    } else if (side == RIGHT) {
        if (alpha < theta)
            alpha = (alpha + (2 * M_PIl));
        beta = ((alpha - theta) - M_PI_2l);
        gamma = -M_PI_2l;
    } else {
      qDebug()<<"error side G41/42";
      return false;
      }
    end_x = (px + (radius * cos(alpha + gamma)));
    end_y = (py + (radius * sin(alpha + gamma)));
    mid_x = (opx + (radius * cos(alpha + gamma)));
    mid_y = (opy + (radius * sin(alpha + gamma)));

    if ((beta < -small) || (beta > (M_PIl + small))) {
        concave = 1;
    } else if (beta > (M_PIl - small) &&
                (!curListTraj.empty() && curListTraj.back().type == WLElementTraj::arc &&
                ((side == RIGHT &&  curListTraj.back().data.arc.CCW) ||
                 (side == LEFT  && !curListTraj.back().data.arc.CCW)))) {
        // this is an "h" shape, tool on right, going right to left
        // over the hemispherical round part, then up next to the
        // vertical part (or, the mirror case).  there are two ways
        // to stay to the "right", either loop down and around, or
        // stay above and right.  we're forcing above and right.
        concave = 1;
    } else {
        concave = 0;
        mid_x = (opx + (radius * cos(alpha + gamma)));
        mid_y = (opy + (radius * sin(alpha + gamma)));
    }

    if (!concave && (beta > small)) {       /* ARC NEEDED */
        //CHP(move_endpoint_and_flush(settings, cx, cy));
        GCode->data()->endPoint.x=GCode->data()->curPoint.x;
        GCode->data()->endPoint.y=GCode->data()->curPoint.y;

        if(GCode->isGCode(1)) {
            //enqueue_ARC_FEED(settings, block->line_number,
            //                 0.0, // doesn't matter, since we will not move this arc's endpoint
            //                 mid_x, mid_y, opx, opy,
            //                 ((side == LEFT) ? -1 : 1), cz,
            //                 AA_end, BB_end, CC_end, u_end, v_end, w_end);
            //dequeue_canons(settings);
            //set_endpoint(mid_x, mid_y);

            WLGPoint startGPoint=GCode->data()->lastGPoint;
            startGPoint.x=GCode->data()->curPoint.x;
            startGPoint.y=GCode->data()->curPoint.y;

            WLGPoint centerGPoint=startGPoint;
            centerGPoint.x=opx;
            centerGPoint.y=opy;

            WLGPoint endGPoint=GCode->data()->lastGPoint;
            endGPoint.x=mid_x;
            endGPoint.y=mid_y;            

            ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                              ,GCode->getPointActivSC(centerGPoint)
                              ,GCode->getPointActivSC(endGPoint)
                              ,side!=LEFT,GCode->getPlane());

            curListTraj+=ElementTraj;

            GCode->data()->endPoint.y=mid_y;
            GCode->data()->endPoint.x=mid_x;

        } else if(GCode->isGCode(0)) {
            // we can't go around the corner because there is no
            // arc traverse.  but, if we do this anyway, at least
            // most of our rapid will be parallel to the original
            // programmed one.  if nothing else, this will look a
            // little less confusing in the preview.
            //
            //enqueue_STRAIGHT_TRAVERSE(settings, block->line_number,
            //                          0.0, 0.0, 0.0,
            //                          mid_x, mid_y, cz,
            //                          AA_end, BB_end, CC_end,
            //                          u_end, v_end, w_end);
            //dequeue_canons(settings);
            //set_endpoint(mid_x, mid_y);

            WLGPoint startGPoint;
            startGPoint=GCode->data()->lastGPoint;
            startGPoint.x=GCode->data()->curPoint.x;
            startGPoint.y=GCode->data()->curPoint.y;

            WLGPoint endGPoint=GCode->data()->curGPoint;
            endGPoint.x=mid_x;
            endGPoint.y=mid_y;

            ElementTraj.setLine(GCode->getPointActivSC(startGPoint)
                               ,GCode->getPointActivSC(endGPoint));

            curListTraj+=ElementTraj;

            GCode->data()->endPoint.y=mid_y;
            GCode->data()->endPoint.x=mid_x;
        } else {
            qDebug()<<"error not G0/G1";
            return false;
          }
    } else if (concave) {
        //if (qc().front().type != QARC_FEED) {
          if (curListTraj.back().type!=WLElementTraj::arc) { // line->line
            // line->line
            double retreat;
            // half the angle of the inside corner
            double halfcorner = (beta + M_PIl) / 2.0;

            if(halfcorner == 0.0){
                qDebug()<<"Zero degree inside corner is invalid for cutter compensation";
                return false;
                }

            retreat = radius / tan(halfcorner);
            // move back along the compensated path
            // this should replace the endpoint of the previous move
            mid_x = cx + retreat * cos(theta + gamma);
            mid_y = cy + retreat * sin(theta + gamma);
            // we actually want to move the previous line's endpoint here.  That's the same as
            // discarding that line and doing this one instead.

            WLElementTraj ETraj = curListTraj.takeLast();
            WLGPoint startGPoint = GCode->getPointActivSC(ETraj.data.line.startPoint,true);
            WLGPoint endGPoint   = GCode->getPointActivSC(ETraj.data.line.endPoint,true);

            endGPoint.x=mid_x;
            endGPoint.y=mid_y;

            ETraj.setLine(GCode->getPointActivSC(startGPoint),
                          GCode->getPointActivSC(endGPoint));

            curListTraj+=ETraj;

            //GCode->data()->endPoint.y=mid_y;//update last point
            //GCode->data()->endPoint.x=mid_x;
            //CHP(move_endpoint_and_flush(settings, mid_x, mid_y));

        } else {
            // arc->line
            // beware: the arc we saved is the compensated one.
            WLElementCirc prev = curListTraj.back().data.arc;

            prev.startPoint  = GCode->getPointActivSC(prev.startPoint,true);
            prev.centerPoint = GCode->getPointActivSC(prev.centerPoint,true);
            prev.endPoint    = GCode->getPointActivSC(prev.endPoint,true);

            double oldrad = hypot(prev.centerPoint.y - prev.endPoint.y, prev.centerPoint.x - prev.endPoint.x);
            double oldrad_uncomp;

            // new line's direction
            double base_dir = atan2(py - opy, px - opx);
            double theta;
            double phi;

            theta = (prev.CCW) ? base_dir + M_PI_2l : base_dir - M_PI_2l;
            phi = atan2(prev.centerPoint.y - opy, prev.centerPoint.x - opx);

            if TOOL_INSIDE_ARC(side, prev.CCW) {
                oldrad_uncomp = oldrad + radius;
            } else {
                oldrad_uncomp = oldrad - radius;
            }

            double alpha = theta - phi;
            // distance to old arc center perpendicular to the new line
            double d = oldrad_uncomp * cos(alpha);
            double d2;
            double angle_from_center;

            if TOOL_INSIDE_ARC(side, prev.CCW) {
                d2 = d - radius;
                double l = d2/oldrad;
                if(l > 1.0 || l < -1.0){
                    qDebug()<<"Arc to straight motion makes a corner the compensated tool can't fit in without gouging";
                    return false;
                    }
                if(prev.CCW)
                    angle_from_center = - acos(l) + theta + M_PIl;
                else
                    angle_from_center = acos(l) + theta + M_PIl;
            } else {
                d2 = d + radius;
                double l = d2/oldrad;
                if(l > 1.0 || l < -1.0){
                    qDebug()<<"Arc to straight motion makes a corner the compensated tool can't fit in without gouging";
                    return false;
                    }
                if(prev.CCW)
                    angle_from_center = acos(l) + theta + M_PIl;
                else
                    angle_from_center = - acos(l) + theta + M_PIl;
            }
            mid_x = prev.centerPoint.x + oldrad * cos(angle_from_center);
            mid_y = prev.centerPoint.y + oldrad * sin(angle_from_center);

            WLElementTraj ETraj = curListTraj.takeLast();

            WLGPoint startGPoint = prev.startPoint;
            WLGPoint centerGPoint= prev.centerPoint;
            WLGPoint endGPoint   = prev.endPoint;

            endGPoint.x=mid_x;
            endGPoint.y=mid_y;

            ETraj.setArc(GCode->getPointActivSC(startGPoint)
                        ,GCode->getPointActivSC(centerGPoint)
                        ,GCode->getPointActivSC(endGPoint)
                        ,ETraj.data.arc.CCW,ETraj.data.arc.plane);

            curListTraj+=ETraj;

            //заменяем предыдущий
            //CHP(move_endpoint_and_flush(settings, mid_x, mid_y));
        }
    } else {
        // no arc needed, also not concave (colinear lines or tangent arc->line)
        //dequeue_canons(settings);
        //set_endpoint(cx, cy);
        GCode->data()->endPoint.y=cx;
        GCode->data()->endPoint.x=cy;
    }
//    (move == G_0? enqueue_STRAIGHT_TRAVERSE : enqueue_STRAIGHT_FEED)
//        (settings, block->line_number,
//         px - opx, py - opy, pz - opz,
//         end_x, end_y, pz,
//         AA_end, BB_end, CC_end,
//         u_end, v_end, w_end);

    WLGPoint startGPoint;
    startGPoint  =GCode->data()->lastGPoint;
    startGPoint.x=mid_x;
    startGPoint.y=mid_y;

    WLGPoint endGPoint=GCode->data()->curGPoint;
    endGPoint.x=end_x;
    endGPoint.y=end_y;


    ElementTraj.setLine(GCode->getPointActivSC(startGPoint)
                       ,GCode->getPointActivSC(endGPoint));

    curListTraj+=ElementTraj;
}

GCode->data()->curPoint.x=end_x;
GCode->data()->curPoint.y=end_y;
GCode->data()->curPoint.z=pz;

//comp_set_current(settings, end_x, end_y, pz);
//settings->AA_current = AA_end;
//settings->BB_current = BB_end;
//settings->CC_current = CC_end;
//settings->u_current = u_end;
//settings->v_current = v_end;
//settings->w_current = w_end;
//comp_set_programmed(settings, px, py, pz);
//return INTERP_OK;
return true;
}


bool WLGProgram::convertArc(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
if(ElementTraj.F<=0) return false;

bool ok;

if((!GCode->isGCode(41)
  &&!GCode->isGCode(42))
    ||GCode->getCompToolRadius()<=0){

  WLGPoint startGPoint;

  startGPoint=GCode->data()->lastGPoint;

  if(!GCode->data()->cutter_comp_firstmove) {
     startGPoint.x=GCode->data()->curPoint.x;
     startGPoint.y=GCode->data()->curPoint.y;
     }

  ok=ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                       ,GCode->getPointActivSC(GCode->getPointIJK(GCode->data()->lastGPoint))
                       ,GCode->getPointActivSC(GCode->data()->curGPoint)
                       ,GCode->isGCode(3)
                       ,GCode->getPlane());

  if(ok) curListTraj+=ElementTraj;

  GCode->data()->curPoint=GCode->data()->curGPoint.to3D();
  GCode->data()->endPoint=GCode->data()->lastGPoint.to3D();

  GCode->data()->cutter_comp_firstmove=true;

  return ok;
  }
  else if(GCode->data()->cutter_comp_firstmove||curListTraj.isEmpty()){
       return  convert_arc_comp1(ElementTraj,curListTraj,GCode);
       }
       else if(curListTraj.isEmpty()){
             qDebug()<<"convert_arc_comp2 - empty List";
             return false;
             }
             else{
             return  convert_arc_comp2(ElementTraj,curListTraj,GCode);
             }
}


bool WLGProgram::convert_arc_comp1(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
double center_x, center_y;
double gamma;                 /* direction of perpendicular to arc at end */
int side;                     /* offset side - right or left              */
double tool_radius;
int turn;                     /* 1 for counterclockwise, -1 for clockwise */
double cx, cy, cz; // current
int plane = GCode->getPlane();
double end_x, end_y, end_z;

side = GCode->getCompSide();
tool_radius = GCode->getCompToolRadius();   /* always is positive */

//double spiral_abs_tolerance = (settings->length_units == CANON_UNITS_INCHES) ? settings->center_arc_radius_tolerance_inch : settings->center_arc_radius_tolerance_mm;
//double radius_tolerance = (settings->length_units == CANON_UNITS_INCHES) ? RADIUS_TOLERANCE_INCH : RADIUS_TOLERANCE_MM;

//comp_get_current(settings, &cx, &cy, &cz);

cx = GCode->data()->curPoint.x;
cy = GCode->data()->curPoint.y;
cz = GCode->data()->curPoint.z;

end_x = GCode->data()->endPoint.x;
end_y = GCode->data()->endPoint.y;
end_z = GCode->data()->endPoint.z;

if(hypot((end_x - cx), (end_y - cy)) <= tool_radius){
     qDebug()<<"Radius of cutter compensation entry arc is not greater than the tool radius";
    // return false;
     }

if(!ElementTraj.setArc(GCode->getPointActivSC(GCode->data()->lastGPoint)
                      ,GCode->getPointActivSC(GCode->getPointIJK(GCode->data()->lastGPoint))
                      ,GCode->getPointActivSC(GCode->data()->curGPoint)
                      ,GCode->isGCode(3),GCode->getPlane())){
  qDebug()<<"error point Arc";
  return false;
  }

/* проверка точек
if (block->r_flag) {
    CHP(arc_data_comp_r(move, plane, side, tool_radius, cx, cy, end_x, end_y,
                        block->r_number, block->p_flag? round_to_int(block->p_number): 1,
                        &center_x, &center_y, &turn, radius_tolerance));
} else {
    CHP(arc_data_comp_ijk(move, plane, side, tool_radius, cx, cy, end_x, end_y,
                          (settings->ijk_distance_mode == MODE_ABSOLUTE),
                          offset_x, offset_y, block->p_flag? round_to_int(block->p_number): 1,
                          &center_x, &center_y, &turn, radius_tolerance, spiral_abs_tolerance, SPIRAL_RELATIVE_TOLERANCE));
}
разворот!
inverse_time_rate_arc(cx, cy, cz, center_x, center_y,
                      turn, end_x, end_y, end_z, block, settings);
*/

// the tool will end up in gamma direction from the programmed arc endpoint
if TOOL_INSIDE_ARC(side, turn) {
    // tool inside the arc: ends up toward the center
    gamma = atan2((center_y - end_y), (center_x - end_x));
} else {
    // outside: away from the center
    gamma = atan2((end_y - center_y), (end_x - center_x));
}

GCode->data()->cutter_comp_firstmove = false;

//comp_set_programmed(settings, end_x, end_y, end_z);

GCode->data()->endPoint.x=end_x;
GCode->data()->endPoint.y=end_y;

// move endpoint to the compensated position.  This changes the radius and center.
end_x += tool_radius * cos(gamma);
end_y += tool_radius * sin(gamma);

/* To find the new center:
   imagine a right triangle ABC with A being the endpoint of the
   compensated arc, B being the center of the compensated arc, C being
   the midpoint between start and end of the compensated arc. AB_ang
   is the direction of A->B.  A_ang is the angle of the triangle
   itself.  We need to find a new center for the compensated arc
   (point B). */

double b_len = hypot(cy - end_y, cx - end_x) / 2.0;
double AB_ang = atan2(center_y - end_y, center_x - end_x);
double A_ang = atan2(cy - end_y, cx - end_x) - AB_ang;

//CHKS((fabs(cos(A_ang)) < TOLERANCE_EQUAL), NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP);
if(fabs(cos(A_ang)) < TOLERANCE_EQUAL){
   qDebug()<<"NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP";
   return false;
   }

double c_len = b_len/cos(A_ang);

// center of the arc is c_len from end in direction AB
center_x = end_x + c_len * cos(AB_ang);
center_y = end_y + c_len * sin(AB_ang);

double spiral_abs_tolerance=CENTER_ARC_RADIUS_TOLERANCE_MM;
/* center to endpoint distances matched before - they still should. */
if(fabs(hypot(center_x-end_x,center_y-end_y) -
        hypot(center_x-cx,center_y-cy)) > spiral_abs_tolerance){
  qDebug()<<"NCE_BUG_IN_TOOL_RADIUS_COMP";
  return false;
  }


// need this move for lathes to move the tool origin first.  otherwise, the arc isn't an arc.
/* для токарного
if (settings->cutter_comp_orientation != 0 && settings->cutter_comp_orientation != 9) {
    enqueue_STRAIGHT_FEED(settings, block->line_number,
                          0, 0, 0,
                          cx, cy, cz,
                          AA_end, BB_end, CC_end, u_end, v_end, w_end);
    set_endpoint(cx, cy);
}
*/

WLGPoint startGPoint;

startGPoint=GCode->data()->lastGPoint;
startGPoint.x=cx;
startGPoint.x=cy;

WLGPoint centerGPoint;

centerGPoint=GCode->getPointIJK(GCode->data()->lastGPoint);
centerGPoint.x=center_x;
centerGPoint.y=center_y;

WLGPoint endGPoint;

endGPoint=GCode->data()->curGPoint;
endGPoint.x=end_x;
endGPoint.y=end_y;

ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                  ,GCode->getPointActivSC(centerGPoint)
                  ,GCode->getPointActivSC(endGPoint)
                  ,GCode->isGCode(3),GCode->getPlane());
/*
enqueue_ARC_FEED(settings, block->line_number,
                 find_turn(cx, cy, center_x, center_y, turn, end_x, end_y),
                 end_x, end_y, center_x, center_y, turn, end_z,
                 AA_end, BB_end, CC_end, u_end, v_end, w_end);
*/
GCode->data()->curPoint.x=end_x;
GCode->data()->curPoint.y=end_y;
GCode->data()->curPoint.z=end_z;
/*
comp_set_current(settings, end_x, end_y, end_z);
settings->AA_current = AA_end;
settings->BB_current = BB_end;
settings->CC_current = CC_end;
settings->u_current = u_end;
settings->v_current = v_end;
settings->w_current = w_end;
*/
return true;
}

bool WLGProgram::convert_arc_comp2(WLElementTraj ElementTraj,QList <WLElementTraj> &curListTraj,WLGCode *GCode)
{
double alpha;                 /* direction of tangent to start of arc */
double arc_radius;
double beta;                  /* angle between two tangents above */
double centerx, centery;      /* center of arc */
double delta;                 /* direction of radius from start of arc to center of arc */
double gamma;                 /* direction of perpendicular to arc at end */
double midx, midy;
int side;
double small = TOLERANCE_CONCAVE_CORNER;      /* angle for testing corners */
double opx = 0, opy = 0, opz = 0;
double theta;                 /* direction of tangent to last cut */
double tool_radius;
int turn = GCode->isGCode(3);                     /* number of full or partial circles CCW */
int plane = GCode->getPlane();
double cx, cy, cz;
double new_end_x, new_end_y;
double end_x,end_y,end_z;

//double spiral_abs_tolerance = (settings->length_units == CANON_UNITS_INCHES) ? settings->center_arc_radius_tolerance_inch : settings->center_arc_radius_tolerance_mm;
//double radius_tolerance = (settings->length_units == CANON_UNITS_INCHES) ? RADIUS_TOLERANCE_INCH : RADIUS_TOLERANCE_MM;

/* find basic arc data: center_x, center_y, and turn */

//double px, py, pz; end x y z

cx=GCode->data()->curPoint.x;
cy=GCode->data()->curPoint.y;
cz=GCode->data()->curPoint.z;

opx=GCode->data()->lastGPoint.x;
opy=GCode->data()->lastGPoint.y;
opz=GCode->data()->lastGPoint.z;

end_x=GCode->data()->curGPoint.x;
end_y=GCode->data()->curGPoint.y;
end_z=GCode->data()->curGPoint.z;

WLGPoint centerGPoint;
centerGPoint=GCode->getPointIJK(GCode->data()->lastGPoint);

centerx=centerGPoint.x;
centery=centerGPoint.y;

//comp_get_programmed(settings, &opx, &opy, &opz);
//comp_get_current(settings, &cx, &cy, &cz);

if(!ElementTraj.setArc(GCode->getPointActivSC(GCode->data()->lastGPoint)
                      ,GCode->getPointActivSC(GCode->getPointIJK(GCode->data()->lastGPoint))
                      ,GCode->getPointActivSC(GCode->data()->curGPoint)
                      ,GCode->isGCode(3),GCode->getPlane())){
  qDebug()<<"error point Arc";
  return false;
  }

/*
проверка точек
if (block->r_flag) {
    CHP(arc_data_r(move, plane, opx, opy, end_x, end_y,
                   block->r_number, block->p_flag? round_to_int(block->p_number): 1,
                   &centerx, &centery, &turn, radius_tolerance));
} else {
    CHP(arc_data_ijk(move, plane,
                     opx, opy, end_x, end_y,
                     (settings->ijk_distance_mode == MODE_ABSOLUTE),
                     offset_x, offset_y, block->p_flag? round_to_int(block->p_number): 1,
                     &centerx, &centery, &turn, radius_tolerance, spiral_abs_tolerance, SPIRAL_RELATIVE_TOLERANCE));
}
разворот!
inverse_time_rate_arc(opx, opy, opz, centerx, centery,
                      turn, end_x, end_y, end_z, block, settings);
*/
side = GCode->getCompSide();
tool_radius = GCode->getCompToolRadius();   /* always is positive */
arc_radius = hypot((centerx - end_x), (centery - end_y));
theta = atan2(cy - opy, cx - opx);
theta = (side == LEFT) ? (theta - M_PI_2l) : (theta + M_PI_2l);
delta = atan2(centery - opy, centerx - opx);
alpha = (GCode->isGCode(3)) ? (delta - M_PI_2l) : (delta + M_PI_2l);
beta = (side == LEFT) ? (theta - alpha) : (alpha - theta);

// normalize beta -90 to +270?
beta = (beta > (1.5 * M_PIl)) ? (beta - (2 * M_PIl)) : (beta < -M_PI_2l) ? (beta + (2 * M_PIl)) : beta;

if (((side == LEFT) && (GCode->isGCode(3))) || ((side == RIGHT) && (GCode->isGCode(2)))) {
    // we are cutting inside the arc
    gamma = atan2((centery - end_y), (centerx - end_x));
    if(arc_radius <= tool_radius){
        qDebug()<<"NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP";
        return  false;
        }
} else {
    gamma = atan2((end_y - centery), (end_x - centerx));
    delta = (delta + M_PIl);
}

// move arc endpoint to the compensated position
new_end_x = end_x + tool_radius * cos(gamma);
new_end_y = end_y + tool_radius * sin(gamma);

if (beta < -small ||
    beta > M_PIl + small ||
    // special detection for convex corner on tangent arc->arc (like atop the middle of "m" shape)
    // or tangent line->arc (atop "h" shape)
    (fabs(beta - M_PIl) < small && !TOOL_INSIDE_ARC(side, turn))
    ) {
    // concave
    if (curListTraj.back().type!=WLElementTraj::arc) {
        // line->arc
        double cy = arc_radius * sin(beta - M_PI_2l);
        double toward_nominal;
        double dist_from_center;
        double angle_from_center;

        if TOOL_INSIDE_ARC(side, turn) {
            // tool is inside the arc
            dist_from_center = arc_radius - tool_radius;
            toward_nominal = cy + tool_radius;
            double l = toward_nominal / dist_from_center;
            if(l > 1.0 || l < -1.0){
              qDebug()<<"Arc move in concave corner cannot be reached by the tool without gouging";
              return false;
              }
            if(turn > 0) {
                angle_from_center = theta + asin(l);
            } else {
                angle_from_center = theta - asin(l);
            }
        } else {
            dist_from_center = arc_radius + tool_radius;
            toward_nominal = cy - tool_radius;
            double l = toward_nominal / dist_from_center;
            if(l > 1.0 || l < -1.0){
              qDebug()<<"Arc move in concave corner cannot be reached by the tool without gouging";
              return false;
              }
            if(turn > 0) {
                angle_from_center = theta + M_PIl - asin(l);
            } else {
                angle_from_center = theta + M_PIl + asin(l);
            }
        }

        midx = centerx + dist_from_center * cos(angle_from_center);
        midy = centery + dist_from_center * sin(angle_from_center);

        WLElementTraj ETraj = curListTraj.takeLast();

        WLGPoint startGPoint = GCode->getPointActivSC(ETraj.data.line.startPoint,true);
        WLGPoint endGPoint   = GCode->getPointActivSC(ETraj.data.line.endPoint,true);

        endGPoint.x=midx;
        endGPoint.y=midy;

        ETraj.setLine(GCode->getPointActivSC(startGPoint)
                     ,GCode->getPointActivSC(endGPoint));

        curListTraj+=ETraj;
        //CHP(move_endpoint_and_flush(settings, midx, midy));
       }
       else {
        // arc->arc
        WLElementCirc prev = curListTraj.back().data.arc;

        prev.startPoint  = GCode->getPointActivSC(prev.startPoint,true);
        prev.centerPoint = GCode->getPointActivSC(prev.centerPoint,true);
        prev.endPoint    = GCode->getPointActivSC(prev.endPoint,true);

        double oldrad = hypot(prev.centerPoint.y - prev.endPoint.y
                             ,prev.centerPoint.x - prev.endPoint.x);
        double newrad;
        if TOOL_INSIDE_ARC(side, turn) {
            newrad = arc_radius - tool_radius;
        } else {
            newrad = arc_radius + tool_radius;
        }

        double arc_cc, pullback, cc_dir, a;
        arc_cc = hypot(prev.centerPoint.y - centery, prev.centerPoint.x - centerx);

        if(oldrad == 0 || arc_cc == 0){
            qDebug()<<"Arc to arc motion is invalid because the arcs have the same center";
            return false;
            }

        a = (SQ(oldrad) + SQ(arc_cc) - SQ(newrad)) / (2 * oldrad * arc_cc);

        if(a > 1.0 || a < -1.0){
            qDebug()<<"Arc to arc motion makes a corner the compensated tool can't fit in without gouging";
            return false;
            }

        pullback = acos(a);
        cc_dir = atan2(centery - prev.centerPoint.y, centerx - prev.centerPoint.x);

        double dir;
        if TOOL_INSIDE_ARC(side, prev.CCW) {
            if(turn > 0)
                dir = cc_dir + pullback;
            else
                dir = cc_dir - pullback;
        } else {
            if(turn > 0)
                dir = cc_dir - pullback;
            else
                dir = cc_dir + pullback;
        }

        midx = prev.centerPoint.x + oldrad * cos(dir);
        midy = prev.centerPoint.y + oldrad * sin(dir);

        WLElementTraj ETraj = curListTraj.takeLast();

        WLGPoint startGPoint = prev.startPoint;
        WLGPoint centerGPoint= prev.centerPoint;
        WLGPoint endGPoint   = prev.endPoint;

        endGPoint.x=midx;
        endGPoint.y=midy;

        ETraj.setArc(GCode->getPointActivSC(startGPoint)
                    ,GCode->getPointActivSC(centerGPoint)
                    ,GCode->getPointActivSC(endGPoint)
                    ,ETraj.data.arc.CCW,ETraj.data.arc.plane);

        curListTraj+=ETraj;
        //CHP(move_endpoint_and_flush(settings, midx, midy));
    }
    WLGPoint startGPoint;
    startGPoint=GCode->data()->lastGPoint;
    startGPoint.x=midx;
    startGPoint.y=midy;

    centerGPoint.x=centerx;
    centerGPoint.y=centery;

    WLGPoint endGPoint;
    endGPoint=GCode->data()->curGPoint;
    endGPoint.x=new_end_x;
    endGPoint.y=new_end_y;

    ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                      ,GCode->getPointActivSC(centerGPoint)
                      ,GCode->getPointActivSC(endGPoint)
                      ,GCode->isGCode(3),GCode->getPlane());

    curListTraj+=ElementTraj;
    //enqueue_ARC_FEED(settings, block->line_number,
    //                 find_turn(opx, opy, centerx, centery, turn, end_x, end_y),
    //                 new_end_x, new_end_y, centerx, centery, turn, end_z,
    //                 AA_end, BB_end, CC_end, u, v, w);
} else if (beta > small) {           /* convex, two arcs needed */
    midx = opx + tool_radius * cos(delta);
    midy = opy + tool_radius * sin(delta);
    //dequeue_canons(settings);
    WLGPoint startGPoint=GCode->data()->lastGPoint;
    startGPoint.x=cx;
    startGPoint.y=cy;

    centerGPoint.x=opx;
    centerGPoint.y=opy;

    WLGPoint endGPoint=GCode->data()->lastGPoint;///????
    endGPoint.x=midx;
    endGPoint.y=midy;


    ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                      ,GCode->getPointActivSC(centerGPoint)
                      ,GCode->getPointActivSC(endGPoint)
                      ,side!=LEFT,GCode->getPlane());

    curListTraj+=ElementTraj;

    //enqueue_ARC_FEED(settings, block->line_number,
    //                 0.0, // doesn't matter since we won't move this arc's endpoint
    //                 midx, midy, opx, opy, ((side == LEFT) ? -1 : 1),
    //                 cz,
    //                 AA_end, BB_end, CC_end, u, v, w);
    //dequeue_canons(settings);

    GCode->data()->endPoint.x=midx;
    GCode->data()->endPoint.y=midy;

    //set_endpoint(midx, midy);
    startGPoint=GCode->data()->lastGPoint;
    startGPoint.x=midx;
    startGPoint.y=midy;

    centerGPoint.x=centerx;
    centerGPoint.y=centery;

    endGPoint=GCode->data()->curGPoint;
    endGPoint.x=new_end_x;
    endGPoint.y=new_end_y;

    ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                      ,GCode->getPointActivSC(centerGPoint)
                      ,GCode->getPointActivSC(endGPoint)
                      ,GCode->isGCode(3),GCode->getPlane());

    curListTraj+=ElementTraj;
//  enqueue_ARC_FEED(settings, block->line_number,
//                   find_turn(opx, opy, centerx, centery, turn, end_x, end_y),
//                   new_end_x, new_end_y, centerx, centery, turn, end_z,
//                   AA_end, BB_end, CC_end, u, v, w);
} else {                      /* convex, one arc needed */
    //dequeue_canons(settings);
    GCode->data()->endPoint.x=cx;
    GCode->data()->endPoint.y=cy;
    //set_endpoint(cx, cy);

    WLGPoint startGPoint;
    startGPoint=GCode->data()->lastGPoint;
    startGPoint.x=cx;
    startGPoint.y=cy;

    WLGPoint endGPoint=GCode->data()->curGPoint;
    endGPoint.x=new_end_x;
    endGPoint.y=new_end_y;

    centerGPoint.x=centerx;
    centerGPoint.y=centery;

    ElementTraj.setArc(GCode->getPointActivSC(startGPoint)
                      ,GCode->getPointActivSC(centerGPoint)
                      ,GCode->getPointActivSC(endGPoint)
                      ,GCode->isGCode(3),GCode->getPlane());

    curListTraj+=ElementTraj;
  //enqueue_ARC_FEED(settings, block->line_number,
  //                 find_turn(opx, opy, centerx, centery, turn, end_x, end_y),
  //                 new_end_x, new_end_y, centerx, centery, turn, end_z,
  //                 AA_end, BB_end, CC_end, u, v, w);
}

GCode->data()->endPoint.x=end_x;
GCode->data()->endPoint.y=end_y;
GCode->data()->endPoint.z=end_z;

GCode->data()->curPoint.x=new_end_x;
GCode->data()->curPoint.y=new_end_y;
GCode->data()->curPoint.z=end_z;
//comp_set_programmed(settings, end_x, end_y, end_z);
//comp_set_current(settings, new_end_x, new_end_y, end_z);
//settings->AA_current = AA_end;
//settings->BB_current = BB_end;
//settings->CC_current = CC_end;
//settings->u_current = u;
//settings->v_current = v;
//settings->w_current = w;

return true;
}
