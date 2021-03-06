/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
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
 * @file client/SMMVehicleImago.cc
 */

#include <client/SMMVehicleImago.hh>

#include <matrix/Vehicle.hh>
#include <client/Context.hh>

namespace oz
{
namespace client
{

Pool<SMMVehicleImago> SMMVehicleImago::pool(64);

Imago* SMMVehicleImago::create(const Object* obj)
{
  hard_assert(obj->flags & Object::VEHICLE_BIT);

  SMMVehicleImago* imago = new SMMVehicleImago(obj);

  imago->model = context.requestModel(obj->clazz->imagoModel);

  return imago;
}

SMMVehicleImago::~SMMVehicleImago()
{
  context.releaseModel(clazz->imagoModel);
}

void SMMVehicleImago::draw(const Imago*)
{
  if (!model->isLoaded()) {
    return;
  }

  const Vehicle*      veh   = static_cast<const Vehicle*>(obj);
  const VehicleClass* clazz = static_cast<const VehicleClass*>(obj->clazz);

  tf.model = Mat4::translation(obj->p - Point::ORIGIN) ^ veh->rot;
  tf.model.rotateX(Math::TAU / -4.0f);

  const Bot* pilot = orbis.obj<const Bot>(veh->pilot);

  if (pilot != nullptr && (veh->state & Vehicle::CREW_VISIBLE_BIT)) {
    tf.push();
    tf.model.translate(clazz->pilotPos);

    context.drawImago(pilot, this);

    tf.pop();
  }

  model->schedule(0, Model::SCENE_QUEUE);
}

}
}
