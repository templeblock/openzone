/*
 *  VehicleClass.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "stable.hpp"

#include "matrix/DynamicClass.hpp"
#include "matrix/Vehicle.hpp"

namespace oz
{

  class VehicleClass : public DynamicClass
  {
    public:

      int    type;
      int    state;

      float  turnLimitH;
      float  turnLimitV;

      float  enginePitchBias;
      float  enginePitchRatio;
      float  enginePitchLimit;

      int    nWeapons;
      String weaponNames[Vehicle::WEAPONS_MAX];
      String onShot[Vehicle::WEAPONS_MAX];
      int    nShots[Vehicle::WEAPONS_MAX];
      float  shotInterval[Vehicle::WEAPONS_MAX];

      Vec3   pilotPos;
      Quat   pilotRot;

      float  moveMomentum;

      float  hoverHeight;
      float  hoverHeightStiffness;
      float  hoverMomentumStiffness;

      static ObjectClass* init( const Config* config );

      virtual Object* create( int index, const Point3& pos ) const;
      virtual Object* create( int index, InputStream* istream ) const;

  };

}
