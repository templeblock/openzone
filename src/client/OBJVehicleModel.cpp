/*
 *  OBJVehicleModel.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING for details.
 */

#include "precompiled.h"

#include "OBJVehicleModel.h"

#include "matrix/Vehicle.h"
#include "matrix/VehicleClass.h"
#include "Context.h"
#include "Camera.h"
#include "Render.h"

namespace oz
{
namespace client
{

  Pool<OBJVehicleModel, 0, 256> OBJVehicleModel::pool;

  Model* OBJVehicleModel::create( const Object* obj )
  {
    assert( obj->flags & Object::VEHICLE_BIT );

    OBJVehicleModel* model = new OBJVehicleModel();

    model->obj  = obj;
    model->list = context.loadOBJ( obj->type->modelName );
    return model;
  }

  OBJVehicleModel::~OBJVehicleModel()
  {
    context.releaseOBJ( obj->type->modelName );
  }

  void OBJVehicleModel::draw( const Model* )
  {
    const Vehicle* veh = static_cast<const Vehicle*>( obj );
    const VehicleClass* clazz = static_cast<const VehicleClass*>( obj->type );

    glMultMatrixf( veh->rot.rotMat44() );

    for( int i = 0; i < Vehicle::CREW_MAX; i++ ) {
      int index = veh->crew[i];

      if( index != -1 && ( index != camera.bot || camera.isExternal ) ) {
        Bot* bot = static_cast<const Bot*>( world.objects[veh->crew[i]] );

        glPushMatrix();
        glTranslatef(  clazz->crewPos[i].x,  clazz->crewPos[i].y,  clazz->crewPos[i].z );
        glRotatef( -bot->h, 0.0f, 0.0f, 1.0f );

        render.drawModel( bot, null );

        glPopMatrix();
      }
    }

    glCallList( list );
  }

}
}
