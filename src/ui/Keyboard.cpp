/*
 *  Keyboard.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "ui/Keyboard.hpp"

namespace oz
{
namespace client
{
namespace ui
{

  Keyboard keyboard;

  void Keyboard::init()
  {
    aSet<ubyte>( keys, 0, SDLK_LAST );
    aSet<ubyte>( oldKeys, 0, SDLK_LAST );
    currKeys = SDL_GetKeyState( null );
  }

  void Keyboard::prepare()
  {
    aCopy( oldKeys, keys, SDLK_LAST );
    aCopy( keys, currKeys, SDLK_LAST );
  }

}
}
}