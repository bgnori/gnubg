/*
 * relational.c
 *
 * by Joern Thyssen <jth@gnubg.org>, 2004.
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
 * $Id: relational.c,v 1.6 2004/05/26 12:11:25 Superfly_Jon Exp $
 */

#include <stdio.h>


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include "i18n.h"
#include "relational.h"
#include "backgammon.h"


#if USE_PYTHON
static int fPyLoaded = FALSE;
#endif

static void
LoadDatabasePy( void ) {

#if USE_PYTHON
  char *pch;

  if ( fPyLoaded )
    return;

  fPyLoaded = TRUE;

  pch = g_strdup( "database.py" );
  CommandLoadPython( pch );
  g_free( pch );
#endif /* USE_PYTHON */

}

extern void
CommandRelationalAddMatch( char *sz ) {

#if USE_PYTHON
  PyObject *m, *d, *v, *r;
  int env_id = 0;
  int force = FALSE;
  char *pch;

  if (sz && *sz)
    env_id = ParseNumber( &sz );
  if ( env_id < 0 )
    env_id = 0;

  pch = NextToken( &sz );
  force = pch && *pch && 
    ( !strcasecmp( "on", pch ) || !strcasecmp( "yes", pch ) ||
      !strcasecmp( "true", pch ) );

  /* load database.py */
  LoadDatabasePy();

  /* connect and add match */

  if ( ! ( m = PyImport_AddModule("__main__") ) ) {
    outputl( _("Error importing __main__") );
    return;
  }

  d = PyModule_GetDict( m );

  /* create new object */
  if ( ! ( r = PyRun_String( "relational()", Py_eval_input, d, d ) ) ) {
    PyErr_Print();
    return;
  }
  else if ( r == Py_None ) {
    outputl( _("Error calling relational()") );
    return;
  }

  /* connect to database */
  if ( ! ( v = PyObject_CallMethod( r, "connect", "" ) ) ) {
    PyErr_Print();
    Py_DECREF( r );
    return;
  }
  else if ( v == Py_None ) {
    outputl( _("Error connecting to database") );
    Py_DECREF( r );
    Py_DECREF( v );
    return;
  }
  else 
    Py_DECREF( v );
    
  /* add match to database */

  if ( ! ( v = PyObject_CallMethod( r, "addmatch", "ii", env_id, force)) ) {
    PyErr_Print();
    Py_DECREF( r );
    return;
  }
  else {
    if ( PyInt_Check( v ) ) {
      int l = PyInt_AsLong( v );
      switch( l ) {
      case 0:
        outputl( _("Match succesfully added to database") );
        break;
      case -1:
        outputl( _("Error adding match to database") );
        break;
      case -2:
        outputl( _("No match is in progress") );
        break;
      case -3:
        outputl( _("Match is already in database") );
        break;
      default:
        outputf( _("Unknown return code %d from addmatch"), l );
        break;
      }
    }
    else {
      outputl( _("Hmm, addmatch return non-integer...") );
    }
    Py_DECREF( v );
  }

  /* disconnect */
  if ( ! ( v = PyObject_CallMethod( r, "disconnect", "" ) ) ) {
    Py_DECREF( r );
    PyErr_Print();
    return;
  }
  else {
    Py_DECREF( v );
  }

   Py_DECREF( r );

#else /* USE_PYTHON */
   outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */

}

extern void
CommandRelationalTest( char *sz ) {

#if USE_PYTHON
  PyObject *m, *d, *v, *r;

  /* load database.py */
  LoadDatabasePy();

  /* connect and add match */

  if ( ! ( m = PyImport_AddModule("__main__") ) ) {
    outputl( _("Error importing __main__") );
    return;
  }

  d = PyModule_GetDict( m );

  /* create new object */
  if ( ! ( r = PyRun_String( "relational()", Py_eval_input, d, d ) ) ) {
    PyErr_Print();
    return;
  }
  else if ( r == Py_None ) {
    outputl( _("Error calling relational()") );
    return;
  }

  /* connect to database */
  if ( ! ( v = PyObject_CallMethod( r, "test", "" ) ) ) {
    PyErr_Print();
  }
  else {
    if ( PyInt_Check( v ) ) {
      int l = PyInt_AsLong( v );
      switch( l ) {
      case 0:
        outputl( _("Database test is successful!") );
        break;
      case -1:
        outputl( _("Database connection test failed!\n"
                   "Check that you've created the gnubg database "
                   "and that your database manager is running." ) );
        break;
      case -2:
        outputl( _("Database table check failed!\n"
                   "The table gnubg.match is missing.") );
        break;
      default:
        outputl( _("Database test failed with unknown error!") );
        break;
      }
    }
    else
      outputl( _("Hmm, test returned non-integer") );
    Py_DECREF( v );
  }

  Py_DECREF( r );

#else /* USE_PYTHON */
   outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
    

}

extern void
CommandRelationalHelp( char *sz ) {
  LoadDatabasePy();
  CommandNotImplemented( sz );
}

extern void
CommandRelationalShowEnvironments( char *sz ) {

#if USE_PYTHON
  PyObject *m, *d, *v, *r;

  /* load database.py */
  LoadDatabasePy();

  /* connect and add match */

  if ( ! ( m = PyImport_AddModule("__main__") ) ) {
    outputl( _("Error importing __main__") );
    return;
  }

  d = PyModule_GetDict( m );

  /* create new object */
  if ( ! ( r = PyRun_String( "relational()", Py_eval_input, d, d ) ) ) {
    PyErr_Print();
    return;
  }
  else if ( r == Py_None ) {
    outputl( _("Error calling relational()") );
    return;
  }

  /* connect to database */
  if ( ! ( v = PyObject_CallMethod( r, "connect", "" ) ) ) {
    PyErr_Print();
    Py_DECREF( r );
    return;
  }
  else if ( v == Py_None ) {
    outputl( _("Error connecting to database") );
    Py_DECREF( r );
    Py_DECREF( v );
    return;
  }
  else 
    Py_DECREF( v );
    
  /* list env */
  if ( ! ( v = PyObject_CallMethod( r, "list_environments", "" ) ) ) {
    PyErr_Print();
  }
  else {
    if ( PySequence_Check( v ) ) {
      PyObject *e;
      int i;

      outputf( ("%-10.10s    %-40.40s\n"
                "----------    -------------------------------------\n"),
               _("Env. ID"), _("Place") );

      for ( i = 0; i < PySequence_Size( v ); ++i ) {

        PyObject *e = PySequence_GetItem( v, i );

        if ( !e ) {
          outputf( _("Error getting item no %d\n"), i );
          continue;
        }

        if ( PySequence_Check( e ) ) {
          PyObject *env_id = PySequence_GetItem( e, 0 );
          PyObject *place = PySequence_GetItem( e, 1 );

          outputf( _("%10ld    %-40.40s\n"),
                   PyInt_AsLong( env_id ),
                   PyString_AsString( place ) );

          Py_DECREF( env_id );
          Py_DECREF( place );
        }
        else {
          outputf( _("Item no. %d is not a sequence\n"), i );
          continue;
        }

        Py_DECREF( e );

      }

      outputl( "" );
      
    }
    else
      outputl( _("Hmm, list_environments returned non-tuple") );
    Py_DECREF( v );
  }

  /* disconnect */
  if ( ! ( v = PyObject_CallMethod( r, "disconnect", "" ) ) ) {
    Py_DECREF( r );
    PyErr_Print();
    return;
  }
  else {
    Py_DECREF( v );
  }

   Py_DECREF( r );

#else /* USE_PYTHON */
   outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */

}
