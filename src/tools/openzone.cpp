/*
 *  openzone.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/Main.hpp"

#include <SDL_main.h>

using namespace oz;

bool Alloc::isLocked = true;

int main( int argc, char** argv )
{
  System::catchSignals();

  Alloc::isLocked = false;
  onleave( []() {
    Alloc::isLocked = true;
    Alloc::printLeaks();
  } );

  printf( "OpenZone  Copyright (C) 2002-2011  Davorin Učakar\n"
      "This program comes with ABSOLUTELY NO WARRANTY.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions; See COPYING file for details.\n\n" );

  try {
    client::main.main( &argc, argv );
  }
  catch( const Exception& e ) {
    oz::log.resetIndent();
    oz::log.println();
    oz::log.printException( e );
    oz::log.println();

    if( oz::log.isFile() ) {
      fprintf( stderr, "\nEXCEPTION: %s\n", e.what() );
      fprintf( stderr, "  in %s\n\n", e.function );
      fprintf( stderr, "  at %s:%d\n\n", e.file, e.line );
    }
  }
  catch( const std::exception& e ) {
    oz::log.resetIndent();
    oz::log.println();
    oz::log.println( "EXCEPTION: %s", e.what() );
    oz::log.println();

    if( oz::log.isFile() ) {
      fprintf( stderr, "\nEXCEPTION: %s\n\n", e.what() );
    }
  }

  client::main.shutdown();

  return 0;
}
