/*
* timer.c
* by Jon Kinsey, 2003
*
* Accurate win32 timer or default timer
*
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
* $Id: timer.c,v 1.4 2003/08/24 11:12:49 Superfly_Jon Exp $
*/

#include <time.h>

#ifdef WIN32
#include "windows.h"

double perFreq = 0;

int setup_timer()
{
	LARGE_INTEGER freq;
	if (!QueryPerformanceFrequency(&freq))
	{	/* Timer not supported */
		return 0;
	}
	else
	{
		perFreq = ((double)freq.QuadPart) / 1000;
		return 1;
	}
}

double get_time()
{	/* Return elapsed time in milliseconds */
	LARGE_INTEGER time;

	if (!perFreq)
	{
		if (!setup_timer())
			return clock() / 1000.0;
	}

	QueryPerformanceCounter(&time);
	return time.QuadPart / perFreq;
}

#else

double get_time()
{	/* Return elapsed time in milliseconds */
	return 1000 * clock() / CLOCKS_PER_SEC;
}

#endif
