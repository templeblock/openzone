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
 * @file client/NaClDownloader.cc
 */

#ifdef __native_client__

#include "stable.hh"

#include "client/NaClDownloader.hh"

#include "client/NaCl.hh"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/url_loader.h>

namespace oz
{
namespace client
{

void NaClDownloader::readCallback( void* data, int result )
{
  NaClDownloader* downloader = static_cast<NaClDownloader*>( data );

  if( result < 0 ) {
    downloader->buffer.dealloc();
  }
  else {
    int length = downloader->buffer.length() - 4096 + result;

    downloader->buffer.reset();
    downloader->buffer.forward( length );

    if( result != 0 ) {
      pp::URLLoader* loader = downloader->loader;
      int ret = loader->ReadResponseBody( downloader->buffer.forward( 4096 ), 4096,
                                          pp::CompletionCallback( readCallback, downloader ) );
      if( ret == PP_OK_COMPLETIONPENDING ) {
        return;
      }
    }
  }

  downloader->semaphore.post();
}

void NaClDownloader::beginCallback( void* data, int result )
{
  NaClDownloader* downloader = static_cast<NaClDownloader*>( data );

  if( result == PP_OK ) {
    pp::URLLoader* loader = downloader->loader;
    int ret = loader->ReadResponseBody( downloader->buffer.forward( 4096 ), 4096,
                                        pp::CompletionCallback( readCallback, downloader ) );
    if( ret == PP_OK_COMPLETIONPENDING ) {
      return;
    }
  }

  downloader->buffer.dealloc();
  downloader->semaphore.post();
}

NaClDownloader::~NaClDownloader()
{
  if( semaphore.isValid() ) {
    semaphore.destroy();
  }
}

bool NaClDownloader::isComplete() const
{
  return semaphore.counter() == 1;
}

BufferStream NaClDownloader::take()
{
  semaphore.wait();
  semaphore.destroy();

  return static_cast<BufferStream&&>( buffer );
}

void NaClDownloader::begin( const char* _url )
{
  url = _url;

  semaphore.init();

  OZ_MAIN_CALL( this, {
    pp::URLRequestInfo request( System::instance );
    request.SetProperty( PP_URLREQUESTPROPERTY_URL, pp::Var( _this->url ) );

    _this->loader = new pp::URLLoader( System::instance );
    if( _this->loader == null ) {
      _this->semaphore.post();
      return;
    }

    int ret = _this->loader->Open( request, pp::CompletionCallback( beginCallback, _this ) );
    if( ret != PP_OK_COMPLETIONPENDING ) {
      delete _this->loader;
      _this->semaphore.post();
      return;
    }
  } )
}

}
}

#endif
