/*
 * Constants
 */

IMPORT_CONST( "OZ_ORBIS_DIM",                   Orbis::DIM );

IMPORT_CONST( "OZ_NORTH",                       NORTH );
IMPORT_CONST( "OZ_WEST",                        WEST );
IMPORT_CONST( "OZ_SOUTH",                       SOUTH );
IMPORT_CONST( "OZ_EAST",                        EAST );

IMPORT_CONST( "OZ_EVENT_CREATE",                Object::EVENT_CREATE );
IMPORT_CONST( "OZ_EVENT_DESTROY",               Object::EVENT_DESTROY );
IMPORT_CONST( "OZ_EVENT_DAMAGE",                Object::EVENT_DAMAGE );
IMPORT_CONST( "OZ_EVENT_HIT",                   Object::EVENT_HIT );
IMPORT_CONST( "OZ_EVENT_SPLASH",                Object::EVENT_SPLASH );
IMPORT_CONST( "OZ_EVENT_FRICTING",              Object::EVENT_FRICTING );
IMPORT_CONST( "OZ_EVENT_USE",                   Object::EVENT_USE );
IMPORT_CONST( "OZ_EVENT_SHOT",                  Weapon::EVENT_SHOT );
IMPORT_CONST( "OZ_EVENT_SHOT_EMPTY",            Weapon::EVENT_SHOT_EMPTY );
IMPORT_CONST( "OZ_EVENT_HIT_HARD",              Bot::EVENT_HIT_HARD );
IMPORT_CONST( "OZ_EVENT_LAND",                  Bot::EVENT_LAND );
IMPORT_CONST( "OZ_EVENT_JUMP",                  Bot::EVENT_JUMP );
IMPORT_CONST( "OZ_EVENT_FLIP",                  Bot::EVENT_FLIP );
IMPORT_CONST( "OZ_EVENT_DEATH",                 Bot::EVENT_DEATH );
IMPORT_CONST( "OZ_EVENT_STEP",                  Bot::EVENT_STEP );
IMPORT_CONST( "OZ_EVENT_WATERSTEP",             Bot::EVENT_WATERSTEP );
IMPORT_CONST( "OZ_EVENT_SWIM",                  Bot::EVENT_SWIM );
IMPORT_CONST( "OZ_EVENT_ENGINE",                Vehicle::EVENT_ENGINE );
IMPORT_CONST( "OZ_EVENT_NEXT_WEAPON",           Vehicle::EVENT_NEXT_WEAPON );
IMPORT_CONST( "OZ_EVENT_SHOT0",                 Vehicle::EVENT_SHOT0 );
IMPORT_CONST( "OZ_EVENT_SHOT1",                 Vehicle::EVENT_SHOT1 );
IMPORT_CONST( "OZ_EVENT_SHOT2",                 Vehicle::EVENT_SHOT2 );
IMPORT_CONST( "OZ_EVENT_SHOT3",                 Vehicle::EVENT_SHOT3 );
IMPORT_CONST( "OZ_EVENT_SHOT_EMPTY",            Vehicle::EVENT_SHOT_EMPTY );

IMPORT_CONST( "OZ_OBJECT_DYNAMIC_BIT",          Object::DYNAMIC_BIT );
IMPORT_CONST( "OZ_OBJECT_WEAPON_BIT",           Object::WEAPON_BIT );
IMPORT_CONST( "OZ_OBJECT_BOT_BIT",              Object::BOT_BIT );
IMPORT_CONST( "OZ_OBJECT_VEHICLE_BIT",          Object::VEHICLE_BIT );
IMPORT_CONST( "OZ_OBJECT_ITEM_BIT",             Object::ITEM_BIT );
IMPORT_CONST( "OZ_OBJECT_BROWSABLE_BIT",        Object::BROWSABLE_BIT );

IMPORT_CONST( "OZ_OBJECT_LUA_BIT",              Object::LUA_BIT );
IMPORT_CONST( "OZ_OBJECT_DESTROY_FUNC_BIT",     Object::DESTROY_FUNC_BIT );
IMPORT_CONST( "OZ_OBJECT_DAMAGE_FUNC_BIT",      Object::DAMAGE_FUNC_BIT );
IMPORT_CONST( "OZ_OBJECT_HIT_FUNC_BIT",         Object::HIT_FUNC_BIT );
IMPORT_CONST( "OZ_OBJECT_USE_FUNC_BIT",         Object::USE_FUNC_BIT );
IMPORT_CONST( "OZ_OBJECT_UPDATE_FUNC_BIT",      Object::UPDATE_FUNC_BIT );

IMPORT_CONST( "OZ_OBJECT_DEVICE_BIT",           Object::DEVICE_BIT );
IMPORT_CONST( "OZ_OBJECT_IMAGO_BIT",            Object::IMAGO_BIT );
IMPORT_CONST( "OZ_OBJECT_AUDIO_BIT",            Object::AUDIO_BIT );

IMPORT_CONST( "OZ_OBJECT_DESTROYED_BIT",        Object::DESTROYED_BIT );

IMPORT_CONST( "OZ_OBJECT_SOLID_BIT",            Object::SOLID_BIT );
IMPORT_CONST( "OZ_OBJECT_CYLINDER_BIT",         Object::CYLINDER_BIT );
IMPORT_CONST( "OZ_OBJECT_DISABLED_BIT",         Object::DISABLED_BIT );
IMPORT_CONST( "OZ_OBJECT_ENABLE_BIT",           Object::ENABLE_BIT );
IMPORT_CONST( "OZ_OBJECT_FRICTING_BIT",         Object::FRICTING_BIT );
IMPORT_CONST( "OZ_OBJECT_BELOW_BIT",            Object::BELOW_BIT );
IMPORT_CONST( "OZ_OBJECT_ON_FLOOR_BIT",         Object::ON_FLOOR_BIT );
IMPORT_CONST( "OZ_OBJECT_ON_SLICK_BIT",         Object::ON_SLICK_BIT );
IMPORT_CONST( "OZ_OBJECT_IN_LIQUID_BIT",        Object::IN_LIQUID_BIT );
IMPORT_CONST( "OZ_OBJECT_IN_LAVA_BIT",          Object::IN_LAVA_BIT );
IMPORT_CONST( "OZ_OBJECT_ON_LADDER_BIT",        Object::ON_LADDER_BIT );

IMPORT_CONST( "OZ_OBJECT_NO_DRAW_BIT",          Object::NO_DRAW_BIT );
IMPORT_CONST( "OZ_OBJECT_WIDE_CULL_BIT",        Object::WIDE_CULL_BIT );

IMPORT_CONST( "OZ_BOT_DEAD_BIT",                Bot::DEAD_BIT );
IMPORT_CONST( "OZ_BOT_MECHANICAL_BIT",          Bot::MECHANICAL_BIT );
IMPORT_CONST( "OZ_BOT_INCARNATABLE_BIT",        Bot::INCARNATABLE_BIT );
IMPORT_CONST( "OZ_BOT_PLAYER_BIT",              Bot::PLAYER_BIT );

IMPORT_CONST( "OZ_BOT_MOVING_BIT",              Bot::MOVING_BIT );
IMPORT_CONST( "OZ_BOT_CROUCHING_BIT",           Bot::CROUCHING_BIT );
IMPORT_CONST( "OZ_BOT_RUNNING_BIT",             Bot::RUNNING_BIT );
IMPORT_CONST( "OZ_BOT_ATTACKING_BIT",           Bot::ATTACKING_BIT );

IMPORT_CONST( "OZ_BOT_JUMP_SCHED_BIT",          Bot::JUMP_SCHED_BIT );
IMPORT_CONST( "OZ_BOT_GROUNDED_BIT",            Bot::GROUNDED_BIT );
IMPORT_CONST( "OZ_BOT_ON_STAIRS_BIT",           Bot::ON_STAIRS_BIT );
IMPORT_CONST( "OZ_BOT_CLIMBING_BIT",            Bot::CLIMBING_BIT );
IMPORT_CONST( "OZ_BOT_SWIMMING_BIT",            Bot::SWIMMING_BIT );
IMPORT_CONST( "OZ_BOT_SUBMERGED_BIT",           Bot::SUBMERGED_BIT );

IMPORT_CONST( "OZ_BOT_GESTURE0_BIT",            Bot::GESTURE0_BIT );
IMPORT_CONST( "OZ_BOT_GESTURE1_BIT",            Bot::GESTURE1_BIT );
IMPORT_CONST( "OZ_BOT_GESTURE2_BIT",            Bot::GESTURE2_BIT );
IMPORT_CONST( "OZ_BOT_GESTURE3_BIT",            Bot::GESTURE3_BIT );
IMPORT_CONST( "OZ_BOT_GESTURE4_BIT",            Bot::GESTURE4_BIT );

IMPORT_CONST( "OZ_VEHICLE_CREW_VISIBLE_BIT",    Vehicle::CREW_VISIBLE_BIT );
IMPORT_CONST( "OZ_VEHICLE_HAS_EJECT_BIT",       Vehicle::HAS_EJECT_BIT );
IMPORT_CONST( "OZ_VEHICLE_AUTO_EJECT_BIT",      Vehicle::AUTO_EJECT_BIT );
