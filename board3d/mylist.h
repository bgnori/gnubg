/*
* mylist.h
* by Jon Kinsey, 2003
*
* Simple vector
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
* $Id: mylist.h,v 1.3 2004/03/31 09:51:57 Superfly_Jon Exp $
*/

typedef struct _vector
{
	int eleSize;
	int numElements;
	void* data;
	int curAllocated;
} vector;

extern void VectorInit(vector* l, int eleSize);
extern void VectorAdd(vector* l, void* ele);
extern int VectorSize(vector* l);
extern void* VectorGet(vector* l, int pos);
extern int VectorFind(vector* l, void* ele);
extern void VectorClear(vector* l);
