/*
 *  base.h
 *
 *  Common include file.
 *  It is included in "precompiled.h", so include the latter instead of this file for faster
 *  compiling.
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#pragma once

// defining NDEBUG disables assert macro
// #define NDEBUG

#define OZ_USE_REUSEALLOC
#define OZ_USE_POOLALLOC

#ifdef WIN32
# include <boost/typeof/typeof.h>
# define typeof( x ) BOOST_TYPEOF( x )
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>

/*
 * Basic definitons and template functions
 */
#include "common.h"

/*
 * Containers 1
 */
#include "List.h"
#include "DList.h"

#include "Vector.h"
#include "SVector.h"
#include "Sparse.h"

/*
 * Memory managment
 */
#include "ReuseAlloc.h"
#include "PoolAlloc.h"

/*
 * String
 */
#include "String.h"

/*
 * Containers 2
 */
#include "HashIndex.h"
#include "HashString.h"

/*
 * Bitset
 */
#include "Bitset.h"

/*
 * Math
 */
#include "Math.h"

#include "Vec3.h"
#include "Quat.h"
#include "Mat33.h"
#include "Mat44.h"

/*
 * Utilities
 */
#include "LogFile.h"
#include "Exit.h"
#include "Config.h"
