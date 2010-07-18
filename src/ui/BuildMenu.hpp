/*
 *  BuildMenu.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "ui/Frame.hpp"

namespace oz
{
namespace client
{
namespace ui
{

  class BuildMenu : public Frame
  {
    protected:

      virtual bool onMouseEvent();
      virtual void onDraw();

    public:

      explicit BuildMenu();

  };

}
}
}
