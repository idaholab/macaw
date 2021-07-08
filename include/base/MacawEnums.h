#pragma once

#include "MooseEnum.h"

MooseEnum getParticleTypeEnum();
MooseEnum getTallyEstimatorEnum();

namespace particles
{
enum ParticleEnum
{
  neutron,
  photon,
  electron,
  positron
};
}

namespace estimators
{
enum TallyEstimatorEnum
{
  ANALOG,
  TRACKLENGTH,
  COLLISION
};
}

/*
MooseEnum
getFilterEnum()
{
  return MooseEnum("azimuthal cell cellborn cellfrom cellinstance distribcell
   delayedgroup evergyfunction energy collision energyout legendre material mesh
    meshsurface mu particle polar surface spatiallegendre sphericalharmonics
     universe zernike zernikeradial");
}


MooseEnum scores("")
MooseEnum filters("")
MooseEnum nuclides("")
MooseEnum estimators("")
MooseEnum type("")
//MooseEnum particle("")
MooseEnum reaction_type("")

namespace scores
{
  // Enumeration of possible tally scores in OpenMCTally
  enum TallyScoreEnum
  {

  }
}

namespace filters
{
  // Enumeration of possible tally filters in OpenMCTally
  enum TallyFilterEnum
  {

  }
}

namespace Estimators
{
  // Enumeration of possible tally Estimators in OpenMCTally
  enum TallyEstimatorEnum
  {

  }
}
*/
