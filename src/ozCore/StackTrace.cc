/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * @file ozCore/StackTrace.cc
 */

#include "StackTrace.hh"

#include "arrays.hh"
#include "Thread.hh"

#if defined( __GLIBC__ ) || defined( _LIBCPP_VERSION )
# include <cstdlib>
# include <cstring>
# include <cxxabi.h>
# include <execinfo.h>
#endif

namespace oz
{

const int StackTrace::MAX_FRAMES;

#if !defined( __GLIBC__ ) && !defined( _LIBCPP_VERSION )

StackTrace StackTrace::current( int )
{
  return { Thread::name(), 0, {} };
}

char** StackTrace::symbols() const
{
  return nullptr;
}

#else

// Size of output buffer where stack trace output string is generated.
static const int TRACE_BUFFER_SIZE = 4096;

StackTrace StackTrace::current( int nSkippedFrames )
{
  hard_assert( nSkippedFrames >= -1 );

  void* framesBuffer[StackTrace::MAX_FRAMES + 4];
  int nFrames = backtrace( framesBuffer, MAX_FRAMES + 4 );

  StackTrace st;
  st.threadName = Thread::name();
  st.nFrames    = min<int>( nFrames - 1 - nSkippedFrames, MAX_FRAMES );

  aCopy<void*>( st.frames, framesBuffer + 1 + nSkippedFrames, st.nFrames );
  return st;
}

char** StackTrace::symbols() const
{
  char outputBuffer[TRACE_BUFFER_SIZE];

  char** symbols = backtrace_symbols( frames, nFrames );
  if( symbols == nullptr ) {
    return nullptr;
  }

  const char* const outEnd = outputBuffer + TRACE_BUFFER_SIZE;
  char* out = outputBuffer;

  *out = '\0';

  int i;
  for( i = 0; i < nFrames; ++i ) {
    // File.
    char* file = symbols[i];

    // Mangled function name.
    char* func = strrchr( symbols[i], '(' );
    char* end  = nullptr;

    if( func != nullptr ) {
      *func++ = '\0';

      end = strrchr( func, '+' );

      if( end != nullptr ) {
        *end = '\0';
      }
    }

    size_t fileLen = strlen( file );
    if( out + fileLen + 4 > outEnd ) {
      break;
    }

    mCopy( out, file, fileLen );
    out += fileLen;

    *out++ = ':';
    *out++ = ' ';

    if( func == nullptr || func >= end ) {
      *out++ = '?';
    }
    else {
      // Demangle name.
      char* demangled;
      int   status = 0;

      demangled = abi::__cxa_demangle( func, nullptr, nullptr, &status );
      func      = demangled == nullptr ? func : demangled;

      size_t funcLen = strlen( func );

      if( funcLen != 0 && out + funcLen + 1 <= outEnd ) {
        mCopy( out, func, funcLen );
        out += funcLen;
      }
      else {
        *out++ = '?';
      }

      free( demangled );
    }

    *out++ = '\0';
  }

  int nWrittenFrames = i;

  size_t headerSize = size_t( nWrittenFrames ) * sizeof( char* );
  size_t bodySize   = size_t( out - outputBuffer );

  if( headerSize + bodySize == 0 ) {
    free( symbols );
    return nullptr;
  }

  char** niceSymbols = static_cast<char**>( realloc( symbols, headerSize + bodySize ) );

  if( niceSymbols == nullptr ) {
    free( symbols );
    return nullptr;
  }

  mCopy( &niceSymbols[nWrittenFrames], outputBuffer, bodySize );

  char* entry = reinterpret_cast<char*>( &niceSymbols[nWrittenFrames] );
  for( i = 0; i < nWrittenFrames; ++i ) {
    niceSymbols[i] = entry;
    entry += strlen( entry ) + 1;
  }

  return niceSymbols;
}

#endif

}