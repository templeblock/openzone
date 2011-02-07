/*
 *  Struct.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "matrix/Struct.hpp"

#include "matrix/Timer.hpp"
#include "matrix/Synapse.hpp"
#include "matrix/Collider.hpp"
#include "matrix/Physics.hpp"

namespace oz
{

  Vector<Object*> Struct::Entity::overlappingObjs;

  const float Struct::DAMAGE_THRESHOLD = 400.0f;

  Pool<Struct, 256> Struct::pool;

  const Mat44 Struct::rotations[] =
  {
    Mat44::ID,
    Mat44::rotZ(  Math::PI_2 ),
    Mat44::rotZ(  Math::PI ),
    Mat44::rotZ( -Math::PI_2 )
  };

  const Mat44 Struct::invRotations[] =
  {
    Mat44::ID,
    Mat44::rotZ( -Math::PI_2 ),
    Mat44::rotZ(  Math::PI ),
    Mat44::rotZ(  Math::PI_2 )
  };

  void Struct::Entity::updateIgnoring()
  {
    switch( state ) {
      case CLOSED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = OPENING;
        }
        break;
      }
      case OPENING:
      case OPENING_BLOCKED: {
        ratio += model->ratioInc;
        time += Timer::TICK_TIME;

        if( ratio >= 1.0f ) {
          ratio = 1.0f;
          time = 0.0f;
          state = OPENED;
        }

        offset = ratio * model->move;
        break;
      }
      case OPENED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = CLOSING;
        }
        break;
      }
      case CLOSING:
      case CLOSING_BLOCKED: {
        ratio -= model->ratioInc;
        time += Timer::TICK_TIME;

        if( ratio <= 0.0f ) {
          ratio = 0.0f;
          time = 0.0f;
          state = CLOSED;
        }

        offset = ratio * model->move;
        break;
      }
      default: {
        assert( false );
        break;
      }
    }
  }

  void Struct::Entity::updateBlocking()
  {
    switch( state ) {
      case CLOSED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = OPENING;
        }
        break;
      }
      case OPENING:
      case OPENING_BLOCKED: {
        Vec3 oldOffset = offset;
        float oldRatio = ratio;

        ratio = Math::min( ratio + model->ratioInc, 1.0f );
        offset = ratio * model->move;

        if( collider.overlapsOO( this ) ) {
          ratio = oldRatio;
          offset = oldOffset;
          break;
        }
        else {
          if( ratio == 1.0f ) {
            state = OPENED;
          }

          offset = oldOffset;

          overlappingObjs.clear();
          collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );

          offset = ratio * model->move;

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }
        }
        break;
      }
      case OPENED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = CLOSING;
        }
        break;
      }
      case CLOSING:
      case CLOSING_BLOCKED: {
        Vec3 oldOffset = offset;
        float oldRatio = ratio;

        ratio = Math::max( ratio - model->ratioInc, 0.0f );
        offset = ratio * model->move;

        if( collider.overlapsOO( this ) ) {
          ratio = oldRatio;
          offset = ratio * model->move;
        }
        else {
          if( ratio == 0.0f ) {
            state = CLOSED;
          }

          offset = oldOffset;

          overlappingObjs.clear();
          collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );

          offset = ratio * model->move;

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }
        }
        break;
      }
      default: {
        assert( false );
        break;
      }
    }
  }

  void Struct::Entity::updatePushing()
  {
    switch( state ) {
      case CLOSED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = OPENING;
        }
        break;
      }
      case OPENING:
      case OPENING_BLOCKED: {
        Vec3 oldOffset = offset;
        float oldRatio = ratio;

        ratio = Math::min( ratio + model->ratioInc, 1.0f );
        offset = ratio * model->move;

        overlappingObjs.clear();
        collider.getOverlaps( this, &overlappingObjs, 0.0f * EPSILON );

        if( !overlappingObjs.isEmpty() ) {
          Vec3 momentum = ( model->ratioInc * model->move ) / Timer::TICK_TIME;
          momentum = str->toAbsoluteCS( momentum ) * ( 1.0f + 2.0f * EPSILON );

          if( momentum.z > 0.0f ) {
            momentum.z -= Physics::G_VELOCITY;
          }

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->momentum += momentum;
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }

          offset = oldOffset;
          ratio = oldRatio;
        }
        else {
          if( ratio == 1.0f ) {
            state = OPENED;
          }

          offset = oldOffset;

          overlappingObjs.clear();

          collider.mask = Object::DYNAMIC_BIT;
          collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );
          collider.mask = Object::SOLID_BIT;

          offset = ratio * model->move;

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }
        }
        break;
      }
      case OPENED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = CLOSING;
        }
        break;
      }
      case CLOSING:
      case CLOSING_BLOCKED: {
        Vec3 oldOffset = offset;
        float oldRatio = ratio;

        ratio = Math::max( ratio - model->ratioInc, 0.0f );
        offset = ratio * model->move;

        overlappingObjs.clear();
        collider.getOverlaps( this, &overlappingObjs, 0.0f * EPSILON );

        if( !overlappingObjs.isEmpty() ) {
          Vec3 momentum = ( model->ratioInc * model->move ) / Timer::TICK_TIME;
          momentum = str->toAbsoluteCS( momentum ) * ( 1.0f + 2.0f * EPSILON );

          if( momentum.z > 0.0f ) {
            momentum.z -= Physics::G_VELOCITY;
          }

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->momentum += momentum;
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }

          offset = oldOffset;
          ratio = oldRatio;
        }
        else {
          if( ratio == 0.0f ) {
            state = CLOSED;
          }

          offset = oldOffset;

          overlappingObjs.clear();

          collider.mask = Object::DYNAMIC_BIT;
          collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );
          collider.mask = Object::SOLID_BIT;

          offset = ratio * model->move;

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }
        }
        break;
      }
      default: {
        assert( false );
        break;
      }
    }
  }

  void Struct::Entity::updateCrushing()
  {
    switch( state ) {
      case CLOSED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = OPENING;
        }
        break;
      }
      case OPENING:
      case OPENING_BLOCKED: {
        ratio = Math::min( ratio + model->ratioInc, 1.0f );
        offset = ratio * model->move;

        overlappingObjs.clear();
        collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );

        if( !overlappingObjs.isEmpty() ) {
          Vec3 move = ( model->ratioInc + 2.0f * EPSILON ) * model->move;
          move = str->toAbsoluteCS( move );

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              dyn->momentum = move / Timer::TICK_TIME;
              dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
            }
          }
        }
        else if( ratio == 1.0f ) {
          state = OPENED;
        }
        break;
      }
      case OPENED: {
        time += Timer::TICK_TIME;

        if( time >= model->timeout ) {
          time = 0.0f;
          state = CLOSING;
        }
        break;
      }
      case CLOSING:
      case CLOSING_BLOCKED: {
        ratio = Math::max( ratio - model->ratioInc, 0.0f );
        offset = ratio * model->move;

        overlappingObjs.clear();
        collider.getOverlaps( this, &overlappingObjs, 2.0f * EPSILON );

        if( !overlappingObjs.isEmpty() ) {
          Vec3 move = ( model->ratioInc + 2.0f * EPSILON ) * model->move;
          move = str->toAbsoluteCS( move );

          foreach( obj, overlappingObjs.iter() ) {
            Dynamic* dyn = static_cast<Dynamic*>( *obj );

            if( dyn->flags & Object::DYNAMIC_BIT ) {
              collider.translate( *dyn, move, dyn );

              if( collider.hit.ratio == 1.0f ) {
                dyn->p += move;
                dyn->velocity += move / Timer::TICK_TIME;
                dyn->flags &= ~( Object::DISABLED_BIT | Object::ON_FLOOR_BIT );
              }
              else {
                dyn->destroy();
              }
            }
          }
        }
        else if( ratio == 0.0f ) {
          state = CLOSED;
        }
        break;
      }
      default: {
        assert( false );
        break;
      }
    }
  }

  Struct::Struct( int index_, int bsp_, const Point3& p_, Rotation rot_ ) :
      index( index_ ), bsp( bsp_ ), p( p_ ), rot( rot_ ), life( orbis.bsps[bsp]->life )
  {
    const BSP* bsp = orbis.bsps[this->bsp];

    assert( bsp != null );

    nEntities = bsp->nModels;
    entities = null;

    if( nEntities != 0 ) {
      entities = new Entity[nEntities];
    }
    for( int i = 0; i < nEntities; ++i ) {
      Entity& entity = entities[i];

      entity.model  = &bsp->models[i];
      entity.str    = this;
      entity.offset = Vec3::ZERO;
      entity.state  = Entity::CLOSED;
      entity.ratio  = 0.0f;
      entity.time   = 0.0f;
    }
  }

  Struct::Struct( int index_, int bsp_, InputStream* istream ) :
      index( index_ ), bsp( bsp_ )
  {
    const BSP* bsp = orbis.bsps[this->bsp];

    assert( bsp != null );

    nEntities = bsp->nModels;
    entities = null;

    if( nEntities != 0 ) {
      entities = new Entity[nEntities];
    }
    for( int i = 0; i < nEntities; ++i ) {
      Entity& entity = entities[i];

      entity.model = &bsp->models[i];
      entity.str   = this;
    }

    readFull( istream );
  }

  Struct::~Struct()
  {
    delete[] entities;
  }

  Bounds Struct::toStructCS( const Bounds& bb ) const
  {
    switch( rot ) {
      default: {
        assert( false );
      }
      case Struct::R0: {
        return Bounds( Point3( +bb.mins.x - p.x, +bb.mins.y - p.y, +bb.mins.z - p.z ),
                       Point3( +bb.maxs.x - p.x, +bb.maxs.y - p.y, +bb.maxs.z - p.z ) );
      }
      case Struct::R90: {
        return Bounds( Point3( +bb.mins.y - p.y, -bb.maxs.x + p.x, +bb.mins.z - p.z ),
                       Point3( +bb.maxs.y - p.y, -bb.mins.x + p.x, +bb.maxs.z - p.z ) );
      }
      case Struct::R180: {
        return Bounds( Point3( -bb.maxs.x + p.x, -bb.maxs.y + p.y, +bb.mins.z - p.z ),
                       Point3( -bb.mins.x + p.x, -bb.mins.y + p.y, +bb.maxs.z - p.z ) );
      }
      case Struct::R270: {
        return Bounds( Point3( -bb.maxs.y + p.y, +bb.mins.x - p.x, +bb.mins.z - p.z ),
                       Point3( -bb.mins.y + p.y, +bb.maxs.x - p.x, +bb.maxs.z - p.z ) );
      }
    }
  }

  Bounds Struct::toAbsoluteCS( const Bounds& bb ) const
  {
    switch( rot ) {
      default: {
        assert( false );
      }
      case Struct::R0: {
        return Bounds( p + Vec3( +bb.mins.x, +bb.mins.y, +bb.mins.z ),
                       p + Vec3( +bb.maxs.x, +bb.maxs.y, +bb.maxs.z ) );
      }
      case Struct::R90: {
        return Bounds( p + Vec3( -bb.maxs.y, +bb.mins.x, +bb.mins.z ),
                       p + Vec3( -bb.mins.y, +bb.maxs.x, +bb.maxs.z ) );
      }
      case Struct::R180: {
        return Bounds( p + Vec3( -bb.maxs.x, -bb.maxs.y, +bb.mins.z ),
                       p + Vec3( -bb.mins.x, -bb.mins.y, +bb.maxs.z ) );
      }
      case Struct::R270: {
        return Bounds( p + Vec3( +bb.mins.y, -bb.maxs.x, +bb.mins.z ),
                       p + Vec3( +bb.maxs.y, -bb.mins.x, +bb.maxs.z ) );
      }
    }
  }

  Bounds Struct::rotate( const Bounds& in, Rotation rot )
  {
    Point3 p = in.mins + ( in.maxs - in.mins ) * 0.5f;

    switch( rot ) {
      default: {
        assert( false );
      }
      case Struct::R0: {
        return Bounds( p + Vec3( +in.mins.x, +in.mins.y, +in.mins.z ),
                       p + Vec3( +in.maxs.x, +in.maxs.y, +in.maxs.z ) );
      }
      case Struct::R90: {
        return Bounds( p + Vec3( -in.maxs.y, +in.mins.x, +in.mins.z ),
                       p + Vec3( -in.mins.y, +in.maxs.x, +in.maxs.z ) );
      }
      case Struct::R180: {
        return Bounds( p + Vec3( -in.maxs.x, -in.maxs.y, +in.mins.z ),
                       p + Vec3( -in.mins.x, -in.mins.y, +in.maxs.z ) );
      }
      case Struct::R270: {
        return Bounds( p + Vec3( +in.mins.y, -in.maxs.x, +in.mins.z ),
                       p + Vec3( +in.maxs.y, -in.mins.x, +in.maxs.z ) );
      }
    }
  }

  void Struct::setRotation( const Bounds& in, Rotation rot )
  {
    switch( rot ) {
      default: {
        assert( false );
      }
      case Struct::R0: {
        mins = p + Vec3( +in.mins.x, +in.mins.y, +in.mins.z );
        maxs = p + Vec3( +in.maxs.x, +in.maxs.y, +in.maxs.z );
        break;
      }
      case Struct::R90: {
        mins = p + Vec3( -in.maxs.y, in.mins.x, in.mins.z );
        maxs = p + Vec3( -in.mins.y, in.maxs.x, in.maxs.z );
        break;
      }
      case Struct::R180: {
        mins = p + Vec3( -in.maxs.x, -in.maxs.y, +in.mins.z );
        maxs = p + Vec3( -in.mins.x, -in.mins.y, +in.maxs.z );
        break;
      }
      case Struct::R270: {
        mins = p + Vec3( in.mins.y, -in.maxs.x, in.mins.z );
        maxs = p + Vec3( in.maxs.y, -in.mins.x, in.maxs.z );
        break;
      }
    }
  }

  void Struct::destroy()
  {
    synapse.genParts( 100, p, Vec3::ZERO, 10.0f, Vec3( 0.4f, 0.4f, 0.4f ), 0.1f,
                      1.98f, 0.0f, 2.0f );
    synapse.remove( this );
  }

  void Struct::update()
  {
    for( int i = 0; i < nEntities; ++i ) {
      Entity& entity = entities[i];

      assert( 0.0f <= entity.ratio && entity.ratio <= 1.0f );

      switch( entity.model->mode ) {
        case BSP::Model::IGNORING: {
          entity.updateIgnoring();
          break;
        }
        case BSP::Model::BLOCKING: {
          entity.updateBlocking();
          break;
        }
        case BSP::Model::PUSHING: {
          entity.updatePushing();
          break;
        }
        case BSP::Model::CRUSHING: {
          entity.updateCrushing();
          break;
        }
        default: {
          assert( false );
          break;
        }
      }
    }
  }

  void Struct::readFull( InputStream* istream )
  {
    p    = istream->readPoint3();
    rot  = Rotation( istream->readInt() );
    life = istream->readFloat();

    for( int i = 0; i < nEntities; ++i ) {
      entities[i].offset = istream->readVec3();
      entities[i].state = Entity::State( istream->readInt() );
      entities[i].ratio = istream->readFloat();
      entities[i].time = istream->readFloat();
    }
  }

  void Struct::writeFull( OutputStream* ostream )
  {
    ostream->writePoint3( p );
    ostream->writeInt( rot );
    ostream->writeFloat( life );

    for( int i = 0; i < nEntities; ++i ) {
      ostream->writeVec3( entities[i].offset );
      ostream->writeInt( entities[i].state );
      ostream->writeFloat( entities[i].ratio );
      ostream->writeFloat( entities[i].time );
    }
  }

}