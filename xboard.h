/*
 * xboard.h
 *
 * by Gary Wong, 1997-1999
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
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
 * $Id: xboard.h,v 1.4 2005/10/29 15:41:03 Superfly_Jon Exp $
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#if USE_EXT
#include <ext.h>
#endif

extern extwindowclass ewcBoard;

extern int BoardSet( extwindow *pewnd, char *sz );

#endif
