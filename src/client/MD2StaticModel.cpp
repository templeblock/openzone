/*
 *  MD2StaticModel.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "precompiled.h"

#include "MD2StaticModel.h"

#include "Context.h"

namespace oz
{
namespace client
{

  Model *MD2StaticModel::create( const Object *object )
  {
    MD2StaticModel *model = new MD2StaticModel();

    model->list = context.loadMD2StaticModel( object->type->modelPath );
    return model;
  }

  void MD2StaticModel::draw()
  {
    glCallList( list );
  }

}
}