#include "MacawEnums.h"

MooseEnum getParticleTypeEnum()
{
  return MooseEnum("neutron photon electron positron", "neutron");
}

MooseEnum getTallyEstimatorEnum()
{
  return MooseEnum("ANALOG TRACKLENGTH COLLISION", "COLLISION");
}
