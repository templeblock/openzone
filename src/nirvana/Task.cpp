/*
 *  Task.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "nirvana/Task.hpp"

namespace oz
{
namespace nirvana
{

Task::~Task()
{}

void Task::write( OutputStream* ostream ) const
{
  for( auto child : children.citer() ) {
    ostream->writeString( child->type() );
    child->write( ostream );
  }
  ostream->writeInt( flags );
}

}
}
