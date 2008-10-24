/*
 * relational.h
 *
 * by Joern Thyssen <jth@gnubg.org>, 2004.
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
 * $Id: relational.h,v 1.12 2008/10/24 15:09:08 c_anthon Exp $
 */

#ifndef _RELATIONAL_H_
#define _RELATIONAL_H_

#include <dbprovider.h>
#include <stddef.h>
#include <sys/types.h>

#define DB_VERSION 1


extern int RelationalUpdatePlayerDetails(const char* oldName, const char* newName, const char* newNotes);
extern float Ratio(float a, int b);

#endif /* _RELATIONAL_H_ */
