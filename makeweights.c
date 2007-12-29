/*
 * makeweights.c
 *
 * by Gary Wong <gary@cs.arizona.edu>, 2000.
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
 * $Id: makeweights.c,v 1.18 2007/12/29 14:32:31 Superfly_Jon Exp $
 */

#include "config.h"

#include <neuralnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "eval.h" /* for WEIGHTS_VERSION */

static void 
usage (char *prog) {
  fprintf (stderr, "Usage: %s [[-f] outputfile [inputfile]]\n"
	  "  outputfile: Output to file instead of stdout\n"
	  "  inputfile: Input from file instead of stdin\n",
	   prog);

  exit (1);
}

extern int main( int argc, char *argv[] )
{
    neuralnet nn;
    char szFileVersion[ 16 ];
    static float ar[ 2 ] = { WEIGHTS_MAGIC_BINARY, WEIGHTS_VERSION_BINARY };
    int c;
    FILE *input = stdin, *output = stdout;

	if (argc > 1)
	{
		int arg = 1;
		if (!StrCaseCmp (argv[1], "-f"))
			arg++;	/* Skip */

		if (argc > arg + 2)
			usage (argv[0]);
		if ((output = fopen (argv[arg], "wb")) == 0)
		{
			perror ("Can't open output file");
			exit (1);
		}
		if (argc == arg + 2)
		{
			if ((input = fopen(argv[arg + 1], "r")) == 0)
			{
				perror ("Can't open input file");
				exit (1);
			}
		}
    }

    setlocale (LC_ALL, "C");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);

    /* generate weights */
    
    if (fscanf(input, "GNU Backgammon %15s\n", szFileVersion ) != 1)
	{
		fprintf( stderr, _("%s: invalid weights file\n"), argv[ 0 ] );
		return EXIT_FAILURE;
    }

    if (StrCaseCmp( szFileVersion, WEIGHTS_VERSION ) )
	{
		fprintf( stderr, _("%s: incorrect weights version\n"
							"(version %s is required, "
							"but these weights are %s)\n" ),
				argv[ 0 ], WEIGHTS_VERSION, szFileVersion );
		return EXIT_FAILURE;
    }
	
    fwrite( ar, sizeof( ar[ 0 ] ), 2, output );

    for( c = 0; !feof(input); c++ )
	{
		if (NeuralNetLoad( &nn, input ) == -1)
		{
			fprintf(stderr, "Failed to load neural net!");
			return EXIT_FAILURE;
		}
		if( NeuralNetSaveBinary( &nn, output ) == -1 )
		{
			fprintf(stderr, "Failed to save neural net!");
			return EXIT_FAILURE;
		}
	}

    fprintf( stderr, _("%d nets converted\n"), c );

    return EXIT_SUCCESS;

}
