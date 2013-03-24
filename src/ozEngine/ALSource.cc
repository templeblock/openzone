/*
 * ozEngine - OpenZone Engine Library.
 *
 * Copyright © 2002-2013 Davorin Učakar
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file ozEngine/ALSource.cc
 */

#include "ALSource.hh"

#include "OpenAL.hh"

namespace oz
{

ALSource::ALSource() :
  sourceId( 0 )
{}

ALSource::~ALSource()
{
  destroy();
}

bool ALSource::create()
{
  alGenSources( 1, &sourceId );

  if( alGetError() != AL_NO_ERROR ) {
    sourceId = 0;
  }
  else {
    // This is not necessary by specification but it seems it's always the case with openalsoft.
    hard_assert( sourceId != 0 );
  }
  return sourceId != 0;
}

void ALSource::destroy()
{
  if( sourceId != 0 ) {
    alDeleteSources( 1, &sourceId );
    sourceId = 0;
  }
}

}