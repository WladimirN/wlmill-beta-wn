#ifndef WLGPOINT_H
#define WLGPOINT_H
#include "wl6dpoint.h"

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

bool isNull();
double get(QString name,bool *ok=nullptr);

void set(QString name,double value);

bool isValid();

QString toString();
bool fromString(QString str);

QString toString(bool all);
WLGPoint normalize();

WL3DPoint to3D();

void from6D(WL6DPoint A);
WL6DPoint to6D();

bool operator==(WLGPoint A);
bool operator!=(WLGPoint A);

WLGPoint operator+(WLGPoint A);
WLGPoint operator-(WLGPoint A);
WLGPoint operator*(double A);
WLGPoint operator/(double A);

}WLGPoint;

#endif // WLGPOINT_H
