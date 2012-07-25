/*
 *  Network.h
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

namespace oz
{
namespace client
{

  class Network
  {
    private:

      TCPsocket socket;

    public:

      bool connect();
      void disconnect();

      void update();

  };

  extern Network network;

}
}