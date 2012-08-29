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
 * @file tools/ozSineSample.cc
 *
 * Generate sine wave sample, used for `src/oz/bellSample.hh`.
 */

#include <oz/oz.hh>

#include <climits>
#include <cstdlib>

static const int   RATE      = 11025;
static const int   SAMPLES   = 3087;
static const int   COLS      = 20;
static const float FREQUENCY = 800.0f;

using namespace oz;

int main()
{
  Log::printRaw(
    "/*\n"
    " * Sine wave at %g Hz with square root attenuation, lasting %g s.\n"
    " * U8 mono %d Hz, %d samples.\n"
    " *\n"
    " * This file was generated by ozSineSample.\n"
    " */\n\n",
    FREQUENCY, float( SAMPLES ) / float( RATE ), RATE, SAMPLES );

  for( int i = 0; i < SAMPLES; ++i ) {
    float amplitude = Math::sqrt( float( SAMPLES - 1 - i ) / float( SAMPLES - 1 ) );
    float value     = amplitude * Math::sin( float( i ) / RATE * FREQUENCY * Math::TAU );
    ubyte sample    = ubyte( float( UCHAR_MAX / 2 ) + float( SCHAR_MAX ) * value + 0.5f );

    Log::printRaw( "%3d", sample );

    if( i != SAMPLES - 1 ) {
      Log::printRaw( "," );

      if( i % COLS == COLS - 1 ) {
        Log::println();
      }
      else {
        Log::printRaw( " " );
      }
    }
  }
  Log::println();

  return EXIT_SUCCESS;
}
