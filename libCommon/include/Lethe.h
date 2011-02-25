#ifndef _LETHE_H
#define _LETHE_H

/*
 * The Lethe.h header is meant to be included on either the Windows or
 *  Linux platform.  This will include the correct headers for the detected
 *  platform and typedef the classes to platform-agnostic identifiers.
 *
 * This also includes stdint.h and provides a few 'utility' functions which may
 *  differ in implementation/system calls across the platforms.  These functions
 *  are implemented in WindowsFunctions.cpp and LinuxFunctions.cpp.
 */

#include "LetheTypes.h"     // Provides non-class types
#include "LetheException.h" // Provides exceptions thrown by libCommon
#include "LetheFunctions.h" // Provides common functions which require separate implementations on each platform
#include "LetheBasic.h"     // Provides basic objects
#include "LetheComplex.h"   // Provides complex objects, using basic objects

#endif
