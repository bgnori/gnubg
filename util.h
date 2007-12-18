/*
 * util.h
 *
 * by Christian Anthon 2007
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: util.h,v 1.6 2007/12/18 21:48:06 Superfly_Jon Exp $
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "stdio.h"

#ifdef WIN32
extern char * getInstallDir( void );
extern void PrintSystemError(const char* message);
#endif

extern void PrintError(const char* message);
extern FILE *GetTemporaryFile(const char *nameTemplate, char **retName);

#endif
