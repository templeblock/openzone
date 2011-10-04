/*
 *  MenuStage.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/MenuStage.hpp"

#include "client/GameStage.hpp"
#include "client/Render.hpp"

#include "client/ui/UI.hpp"

namespace oz
{
namespace client
{

  MenuStage menuStage;

  bool MenuStage::update()
  {
    ui::ui.update();

    return !doExit;
  }

  void MenuStage::present()
  {
    render.draw( Render::DRAW_UI_BIT );
    render.sync();
  }

  void MenuStage::load()
  {
    mainMenu = new ui::MainMenu();

    ui::ui.root->add( mainMenu );
    ui::mouse.doShow = true;
    ui::mouse.buttons = 0;
    ui::mouse.currButtons = 0;

    ui::ui.showLoadingScreen( false );
  }

  void MenuStage::unload()
  {
    ui::ui.root->remove( mainMenu );

    mainMenu = null;
  }

  void MenuStage::init()
  {
    mainMenu = null;
    doExit = false;
  }

  void MenuStage::free()
  {}

}
}