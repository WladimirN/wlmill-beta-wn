#include "wlgpoint.h"

bool  WLGPoint::isNull()
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

double  WLGPoint::get(QString name,bool *ok)
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

void  WLGPoint::set(QString name,double value)
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

bool  WLGPoint::isValid()
{
return   !qIsNaN(x)&&!qIsInf(x)
       &&!qIsNaN(y)&&!qIsInf(y)
       &&!qIsNaN(z)&&!qIsInf(z)
       &&!qIsNaN(a)&&!qIsInf(a)
       &&!qIsNaN(b)&&!qIsInf(b)
       &&!qIsNaN(c)&&!qIsInf(c);
}

QString  WLGPoint::toString()
{
return
   +"X "+QString::number(x)
   +",Y "+QString::number(y)
   +",Z "+QString::number(z)
   +",A "+QString::number(a)
   +",B "+QString::number(b)
   +",C "+QString::number(c);
}

bool  WLGPoint::fromString(QString str)
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

QString  WLGPoint::toString(bool all)
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
WLGPoint  WLGPoint::normalize()
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

WL3DPoint  WLGPoint::to3D()
{
WL3DPoint ret;

ret.x=x;
ret.y=y;
ret.z=z;

return ret;
}

void  WLGPoint::from6D(WL6DPoint A)
{
x=A.x;
y=A.y;
z=A.z;

a=A.a;
b=A.b;
c=A.c;
}

WL6DPoint  WLGPoint::to6D()
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

double WLGPoint::getR()
{
return sqrt(x*x
           +y*y
           +z*z
           +a*a
           +b*b
           +c*c
           +v*v
           +u*u
           +w*w);
}

bool  WLGPoint::operator==(WLGPoint A)
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

bool  WLGPoint::operator!=(WLGPoint A)
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

WLGPoint  WLGPoint::operator+(WLGPoint A)
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

WLGPoint  WLGPoint::operator-(WLGPoint A)
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

WLGPoint  WLGPoint::operator*(double A)
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


WLGPoint WLGPoint::operator/(double A)
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

