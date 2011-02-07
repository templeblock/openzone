/*
 *  MD3StaticModel.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "stable.hpp"

#include "client/Model.hpp"

namespace oz
{
namespace client
{

  class MD3;

  class MD3StaticModel : public Model
  {
    protected:

      MD3* md3;

      virtual ~MD3StaticModel();

    public:

      static Pool<MD3StaticModel, 256> pool;

      static Model* create( const Object* obj );

      virtual void draw( const Model* parent );

    OZ_STATIC_POOL_ALLOC( pool )

  };

}
}