/*
 *  FloraManager.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

namespace oz
{

  class FloraManager
  {
    private:

      // plants/m2
      static const float DENSITY = 0.03;
      // plants/m2/s
      static const float GROWTH =  0.0001;

      Vector<int> plants;

      int number;
      int growth;

      void addTree( float x, float y );

    public:

      void seed();
      void clear();
      void update();

  };

  extern FloraManager floraManager;

}
