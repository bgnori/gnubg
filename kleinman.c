/*
 * kleinman.c
 *
 * by �ystein Johansen <oeysteij@online.no> 2000-2002
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
 * $Id: kleinman.c,v 1.5 2006/10/26 18:28:02 Superfly_Jon Exp $
 */

#include "config.h"

#ifdef STANDALONE
#include <stdio.h>
#endif

#include <math.h>

#if !HAVE_ERF
extern double erf( double x );
#endif


extern float
KleinmanCount (int nPipOnRoll, int nPipNotOnRoll)
{
  int nDiff, nSum;
  double rK;

  nDiff = nPipNotOnRoll - nPipOnRoll;
  nSum = nPipNotOnRoll + nPipOnRoll;

  /* Don't use this routine if player on roll is not the favorite. */

  if (nDiff < -4) 
    return -1;

  rK = (double) (nDiff + 4) / (2 * sqrt( nSum - 4 ));

  return 0.5f * (1.0f + (float)erf( rK ));
}

#ifdef STANDALONE
int
main (int argc, char *argv[])
{
  int a, b;
  float rKC;

  if (argc == 3)
    {
      sscanf (argv[1], "%d", &a);
      sscanf (argv[2], "%d", &b);
      rKC = KleinmanCount (a, b);
      if (rKC == -1)
	printf ("Pipcount unsuitable for Kleinman Count.\n");
      else
	printf ("Cubeless Winning Chance: %f\n", rKC);
    }
  else
    {
      printf ("Usage: %s <pip for player on roll> "
              "<pip for player not on roll>\n", argv[0]);
    }
  return 0;
}
#endif
