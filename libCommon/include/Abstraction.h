#ifndef _ABSTRACTION_H
#define _ABSTRACTION_H

/*
 * The Abstraction.h header is meant to be included on either the Windows or
 *  Linux platform.  This will include the correct headers for the detected
 *  platform and typedef the classes to platform-agnostic identifiers.
 *
 * This also includes stdint.h and provides a few 'utility' functions which may
 *  differ in implementation/system calls across the platforms.  These functions
 *  are implemented in WindowsFunctions.cpp and LinuxFunctions.cpp.
 */

#include "AbstractionTypes.h"     // Provides non-class types
#include "AbstractionException.h" // Provides exceptions thrown by libCommon
#include "AbstractionFunctions.h" // Provides common functions which require separate implementations on each platform
#include "AbstractionBasic.h"     // Provides basic objects
#include "AbstractionComplex.h"   // Provides complex objects, using basic objects

#endif
