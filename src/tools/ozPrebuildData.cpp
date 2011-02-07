/*
 *  ozPrebuildData.cpp
 *
 *  Prebuild data
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "matrix/Translator.hpp"
#include "matrix/Orbis.hpp"
#include "matrix/Matrix.hpp"

#include "client/Terra.hpp"
#include "client/BSP.hpp"
#include "client/OBJ.hpp"

#include <sys/stat.h>
#include <SDL_main.h>

#include <csignal>

using namespace oz;

bool Alloc::isLocked = true;

int main( int, char** )
{
  System::catchSignals();

  Alloc::isLocked = false;
  onleave( []() {
    Alloc::isLocked = true;
    Alloc::printLeaks();
  } );

  SDL_Init( SDL_INIT_NOPARACHUTE );

  long startTime = SDL_GetTicks();

  translator.init();
  matrix.init();

  foreach( terra, translator.terras.citer() ) {
    try {
      orbis.terra.prebuild( terra->name );
      client::terra.prebuild();
    }
    catch( const Exception& ) {
    }
  }

  foreach( bsp, translator.bsps.citer() ) {
    try {
      BSP::prebuild( bsp->name );
      client::BSP::prebuild( bsp->name );
    }
    catch( const Exception& e ) {
      log.resetIndent();
      log.println();
      log.printException( e );
      exit( -1 );
    }
  }

  foreach( model, translator.models.citer() ) {
    try {
      struct stat statInfo;

      if( stat( model->path + "/data.obj", &statInfo ) == 0 ) {
        client::OBJ::prebuild( model->name );
      }
    }
    catch( const Exception& ) {
    }
  }

  matrix.free();
  translator.free();

  long endTime = SDL_GetTicks();

  log.println( "Build time: %.2f s", float( endTime - startTime ) / 1000.0f );

  SDL_Quit();

  Alloc::printStatistics();
  return 0;
}