/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file client/openzone/openzone.cc
 */

#include "stable.hh"

#include "client/openzone.hh"

#include "client/Client.hh"
#include "client/NaCl.hh"

#ifdef __native_client__

#include <SDL/SDL_nacl.h>

extern "C"
void alSetPpapiInfo( PP_Instance, PPB_GetInterface );

namespace oz
{
namespace client
{

void* MainInstance::mainThreadMain( void* )
{
  int exitCode = EXIT_FAILURE;

  Log::printRaw( "OpenZone  Copyright © 2002-2012 Davorin Učakar\n"
                 "This program comes with ABSOLUTELY NO WARRANTY.\n"
                 "This is free software, and you are welcome to redistribute it\n"
                 "under certain conditions; See COPYING file for details.\n\n" );

  try {
    char  argv0[] = OZ_APPLICATION_NAME;
    char* argv[]  = { argv0 };

    exitCode = client::client.main( 1, argv );
    client::client.shutdown();
  }
  catch( const std::exception& e ) {
    System::error( e );
  }

  Log::verboseMode = true;
  Alloc::printLeaks();
  Log::verboseMode = false;

  if( Alloc::count != 0 ) {
    Log::println( "There are some memory leaks. See '%s' for details.", Log::logFile() );
  }
  return null;
}

MainInstance::MainInstance( PP_Instance instance_ ) :
  pp::Instance( instance_ ), pp::MouseLock( this ), fullscreen( this ), isMouseLocked( false ),
  mainThread( 0 )
{
  System::module   = pp::Module::Get();
  System::instance = this;
  System::core     = pp::Module::Get()->core();
  System::init();

  NaCl::init();

  RequestInputEvents( PP_INPUTEVENT_CLASS_KEYBOARD | PP_INPUTEVENT_CLASS_MOUSE );
}

MainInstance::~MainInstance()
{
  if( mainThread != 0 ) {
    pthread_join( mainThread, null );
    mainThread = null;
  }

  NaCl::free();
}

bool MainInstance::Init( uint32_t, const char**, const char** )
{
  return true;
}

void MainInstance::DidChangeView( const pp::View& view )
{
  int width  = view.GetRect().width();
  int height = view.GetRect().height();

  if( width == NaCl::width && height == NaCl::height ) {
    return;
  }

  NaCl::width  = width;
  NaCl::height = height;

  if( mainThread == 0 ) {
    SDL_NACL_SetInstance( pp_instance(), NaCl::width, NaCl::height );
    alSetPpapiInfo( pp_instance(), System::module->get_browser_interface() );

    pthread_create( &mainThread, null, mainThreadMain, this );
  }
}

void MainInstance::DidChangeView( const pp::Rect&, const pp::Rect& )
{
  PP_NOTREACHED();
}

bool MainInstance::HandleInputEvent( const pp::InputEvent& event )
{
  switch( event.GetType() ) {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN: {
      if( !isMouseLocked && fullscreen.IsFullscreen() ) {
        LockMouse( pp::CompletionCallback( &DidMouseLock, this ) );
      }
      break;
    }
    case PP_INPUTEVENT_TYPE_KEYDOWN: {
      pp::KeyboardInputEvent keyEvent( event );

      if( ( keyEvent.GetKeyCode() == 122 || keyEvent.GetKeyCode() == 13 ) &&
          event.GetModifiers() == 0 )
      {
        if( fullscreen.IsFullscreen() ) {
          fullscreen.SetFullscreen( false );
        }
        else if( fullscreen.SetFullscreen( true ) && !isMouseLocked ) {
          LockMouse( pp::CompletionCallback( &DidMouseLock, this ) );
        }
      }
      break;
    }
    default: {
      break;
    }
  }

  SDL_NACL_PushEvent( event );
  return true;
}

void MainInstance::MouseLockLost()
{
  isMouseLocked = false;
}

void MainInstance::Empty( void*, int )
{}

void MainInstance::DidMouseLock( void* data, int result )
{
  MainInstance* instance = static_cast<MainInstance*>( data );
  instance->isMouseLocked = result == PP_OK;
}

pp::Instance* MainModule::CreateInstance( PP_Instance instance )
{
  return new MainInstance( instance );
}

}
}

namespace pp
{

pp::Module* CreateModule()
{
  return new oz::client::MainModule();
}

}

#else // __native_client__

using namespace oz;

int main( int argc, char** argv )
{
  System::init();

  int exitCode = EXIT_FAILURE;

  printf( "OpenZone  Copyright © 2002-2012 Davorin Učakar\n"
          "This program comes with ABSOLUTELY NO WARRANTY.\n"
          "This is free software, and you are welcome to redistribute it\n"
          "under certain conditions; See COPYING file for details.\n\n" );

  try {
    exitCode = client::client.main( argc, argv );
    client::client.shutdown();
  }
  catch( const std::exception& e ) {
    System::error( e );
  }

  if( Alloc::count != 0 ) {
    Log::println( "There are some memory leaks. See '%s' for details.", Log::logFile() );

    Log::verboseMode = true;
    Alloc::printLeaks();
    Log::verboseMode = false;
  }

  return exitCode;
}

#endif // __native_client__
