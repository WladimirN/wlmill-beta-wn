#include "wlheightmap.h"


WLHeightMap::typeInterpoliation WLHeightMap::getTypeInterpoliation() const
{
return m_typeInterpoliation;
}

void WLHeightMap::setTypeInterpoliation(const WLHeightMap::typeInterpoliation &typeInterpoliation)
{
if(m_typeInterpoliation != typeInterpoliation)
 {
 m_typeInterpoliation = typeInterpoliation;
 m_updateShow=true;
 }
}
