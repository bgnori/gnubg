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
 * $Id: relational.c,v 1.12 2004/11/05 14:46:22 Superfly_Jon Exp $
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

#include "gnubgmodule.h"

#if USE_GTK
#include "gtkgame.h"
#endif

static void
LoadDatabasePy( void ) {

  static int fPyLoaded = FALSE;
  char *pch;

  if ( fPyLoaded )
    return;

  fPyLoaded = TRUE;

  pch = g_strdup( "database.py" );
  CommandLoadPython( pch );
  g_free( pch );

}

static PyObject *Connect()
{
	PyObject *m, *d, *v, *r;

	/* load database.py */
	LoadDatabasePy();

	/* connect to database */
	if (!(m = PyImport_AddModule("__main__")))
	{
		outputl( _("Error importing __main__") );
		return NULL;
	}

	d = PyModule_GetDict(m);

	/* create new object */
	if (!(r = PyRun_String("relational()", Py_eval_input, d, d)))
	{
		PyErr_Print();
		return NULL;
	}
	else if (r == Py_None)
	{
		outputl( _("Error calling relational()") );
		return NULL;
	}

	/* connect to database */
	if (!(v = PyObject_CallMethod(r, "connect", "" )))
	{
		PyErr_Print();
		Py_DECREF(r);
		return NULL;
	}
	else if (v == Py_None)
	{
		outputl( _("Error connecting to database") );
		Py_DECREF(r);
		return NULL;
	}
	else 
	{
		Py_DECREF(v);
		return r;
	}
}

static void Disconnect(PyObject *r)
{
	PyObject *v;
	if (!(v = PyObject_CallMethod(r, "disconnect", "")))
	{
		PyErr_Print();
	}
	else
	{
		Py_DECREF(v);
	}

	Py_DECREF(r);
}
#endif /* USE_PYTHON */

extern int RelationalMatchExists()
{
  int ret = -1;
#if USE_PYTHON
  PyObject *v, *r;

  if (!(r = Connect()))
	  return -1;

  /* Check if match is in database */
  if (!(v = PyObject_CallMethod(r, "is_existing_match", "O", PythonMatchChecksum(0, 0))))
  {
    PyErr_Print();
    Py_DECREF(r);
    return -1;
  }
  else
  {
    if (v == Py_None)
		ret = 0;
	else if (PyInt_Check(v))
		ret = 1;

    Py_DECREF( v );
  }

  Disconnect(r);

#else /* USE_PYTHON */
  outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
  return ret;
}

extern void
CommandRelationalAddMatch( char *sz ) {

#if USE_PYTHON
  PyObject *v, *r;
  char* env;
  int force = FALSE;
  char *pch;

  env = NextToken( &sz );
  if (!env)
	  env = "";

  pch = NextToken( &sz );
  force = pch && *pch && 
    ( !strcasecmp( "on", pch ) || !strcasecmp( "yes", pch ) ||
      !strcasecmp( "true", pch ) );

  if (!(r = Connect()))
	  return;

  /* add match to database */
  if ( ! ( v = PyObject_CallMethod( r, "addmatch", "si", env, force)) ) {
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
      case -4:
        outputl( _("Unknown environment") );
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

  Disconnect(r);

#else /* USE_PYTHON */
  outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */

}

int env_added;	/* Horrid flag to see if next function worked... */

extern void CommandRelationalAddEnvironment(char *sz)
{
#if USE_PYTHON
	PyObject *v, *r;

	env_added = FALSE;

	if (!sz || !*sz)
	{
		outputl( _("You must specify an environment name to add "
		"(see `help relational add environment').") );
		return;
	}

	if (!(r = Connect()))
		return;

	/* add environment to database */
	if (!(v = PyObject_CallMethod(r, "addenv", "s", sz)))
	{
		PyErr_Print();
		Py_DECREF(r);
		return;
	}
	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == -1)
			outputl( _("Error adding environment to database") );
		else if (l == 1)
		{
			outputl( _("Environment succesfully added to database") );
			env_added = TRUE;
		}
		else if (l == -2)
			outputl( _("That environment already exists") );
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void
CommandRelationalTest( char *sz ) {

#if USE_PYTHON
  PyObject *v, *r;

  if (!(r = Connect()))
	  return;

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

  Disconnect(r);

#else /* USE_PYTHON */
  outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */

}

extern void
CommandRelationalHelp( char *sz ) {
#if USE_PYTHON
	LoadDatabasePy();
#endif
  CommandNotImplemented( sz );
}

extern void
CommandRelationalShowEnvironments( char *sz )
{
#if USE_PYTHON
	/* Use the Select command */
	CommandRelationalSelect("place FROM env "
                         "ORDER BY env_id");

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void
CommandRelationalShowDetails( char *sz )
{
#if USE_PYTHON
	PyObject *v, *r;

	char *player_name, *env;

	if (!sz || !*sz || !(player_name = NextToken(&sz)))
	{
		outputl( _("You must specify a player name to list the details for "
		"(see `help relational show details').") );
		return;
	}
	env = NextToken(&sz);
	if (!env)
		env = "";

	r = Connect();

	/* list env */
	if (!(v = PyObject_CallMethod(r, "list_details", "ss", player_name, env)))
	{
		PyErr_Print();
		return;
	}

	if (PySequence_Check(v))
	{
		int i;

		for ( i = 0; i < PySequence_Size( v ); ++i )
		{
			PyObject *e = PySequence_GetItem(v, i);

			if ( !e )
			{
				outputf( _("Error getting item no %d\n"), i );
				continue;
			}

			if (PySequence_Check(e))
			{
				PyObject *detail = PySequence_GetItem(e, 0);
				PyObject *value = PySequence_GetItem(e, 1);

				if (PyInt_Check(value))
					outputf( _("%s: %d\n"),
						PyString_AsString(detail),
						(int)PyInt_AsLong(value));
				else if (PyFloat_Check(value))
					outputf( _("%s: %.2f\n"),
						PyString_AsString(detail),
						PyFloat_AsDouble(value));
				else
					outputf( _("unknown type"));

				Py_DECREF(detail);
				Py_DECREF(value);
			}
			else
			{
				outputf( _("Item no. %d is not a sequence\n"), i );
				continue;
			}
			Py_DECREF( e );
		}
		outputl( "" );
	}
	else
	{
		if (PyInt_Check(v))
		{
			int l = PyInt_AsLong(v);
			if (l == -1)
				outputl( _("Player not in database") );
			else if (l == -4)
				outputl( _("Unknown environment") );
			else
				outputl( _("unknown return value") );
		}
		else
			outputl( _("invalid return type") );
	}

	Py_DECREF( v );

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void
CommandRelationalShowPlayers( char *sz )
{
#if USE_PYTHON
	/* Use the Select command */
	CommandRelationalSelect("person.name AS Player, nick.name AS Nickname, env.place AS env"
		" FROM nick INNER JOIN env INNER JOIN person"
		" ON nick.env_id = env.env_id AND nick.person_id = person.person_id"
		" ORDER BY person.name");

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void CommandRelationalErase(char *sz)
{
#if USE_PYTHON
	PyObject *v, *r;
	char *player_name, *env;

	if (!sz || !*sz || !(player_name = NextToken(&sz)))
	{
		outputl( _("You must specify a player name to remove "
		"(see `help relational erase player').") );
		return;
	}
	env = NextToken(&sz);
	if (!env)
		env = "";

	r = Connect();

	/* remove player */
	if (!(v = PyObject_CallMethod(r, "erase_player", "ss", player_name, env)))
	{
		PyErr_Print();
		return;
	}

	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == -1)
			outputl( _("Player not in database") );
		else if (l == 1)
			outputl( _("player removed from database") );
		else if (l == -4)
			outputl( _("Unknown environment") );
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void RelationalLinkNick(char* nick, char* env, char* player)
{	/* Link nick on env to player */
#if USE_PYTHON
	PyObject *v, *r;

	r = Connect();

	if (!(v = PyObject_CallMethod(r, "link_players", "sss", nick, env, player)))
	{
		PyErr_Print();
		return;
	}

	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == -1)
			outputl( _("Failed to link players") );
		else if (l == 1)
			outputl( _("Players linked successfully") );
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void CommandRelationalRenameEnv(char *sz)
{
#if USE_PYTHON
	PyObject *v, *r;
	char *env_name, *new_name;

	if (!sz || !*sz || !(env_name = NextToken(&sz))
		|| !(new_name = NextToken(&sz)))
	{
		outputl( _("You must specify an environment to rename and "
			"a new name (see `help relational rename environment').") );
		return;
	}

	r = Connect();

	/* rename env */
	if (!(v = PyObject_CallMethod(r, "rename_env", "ss", env_name, new_name)))
	{
		PyErr_Print();
		return;
	}

	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == -1)
			outputl( _("Environment not in database") );
		else if (l == 1)
			outputl( _("Environment renamed") );
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

int env_deleted;	/* Horrid flag to see if next function worked... */

extern void CommandRelationalEraseEnv(char *sz)
{
#if USE_PYTHON
	PyObject *v, *r;

	env_deleted = FALSE;
	if (!sz || !*sz)
	{
		outputl( _("You must specify an environment to remove "
		"(see `help relational erase environment').") );
		return;
	}

	if (fConfirmSave && !GetInputYN( _("Are you sure you want to erase the "
		"environment and all related data?") ))
		return;

	r = Connect();

	/* erase env */
	if (!(v = PyObject_CallMethod(r, "erase_env", "s", sz)))
	{
		PyErr_Print();
		return;
	}

	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == -1)
			outputl( _("Environment not in database") );
		else if (l == -2)
			outputl( _("You must keep at least one environment in the database") );
		else if (l == 1)
		{
			outputl( _("Environment removed from database") );
			env_deleted = TRUE;
		}
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

extern void CommandRelationalEraseAll(char *sz)
{
#if USE_PYTHON
	PyObject *v, *r;

	if( fConfirmSave && !GetInputYN( _("Are you sure you want to erase all "
				       "player records?") ) )
		return;

	r = Connect();

	/* clear database */
	if (!(v = PyObject_CallMethod(r, "erase_all", "")))
	{
		PyErr_Print();
		return;
	}

	if (PyInt_Check(v))
	{
		int l = PyInt_AsLong( v );
		if (l == 1)
			outputl( _("all players removed from database") );
		else
			outputl( _("unknown return value") );
	}
	else
		outputl( _("invalid return (non-integer)") );

	Py_DECREF(v);

	Disconnect(r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}

#if USE_PYTHON

extern void FreeRowset(RowSet* pRow)
{
	int i, j;
	free(pRow->widths);

	for (i = 0; i < pRow->rows; i++)
	{
		for (j = 0; j < pRow->cols; j++)
		{
			free (pRow->data[i][j]);
		}
		free(pRow->data[i]);
	}
	free(pRow->data);

	pRow->cols = pRow->rows = 0;
	pRow->data = NULL;
	pRow->widths = NULL;
}

void MallocRowset(RowSet* pRow, int rows, int cols)
{
	int i;
	pRow->widths = malloc(cols * sizeof(int));
	memset(pRow->widths, 0, cols * sizeof(int));

	pRow->data = malloc(rows * sizeof(char*));
	for (i = 0; i < rows; i++)
	{
		pRow->data[i] = malloc(cols * sizeof(char*));
		memset(pRow->data[i], 0, cols * sizeof(char*));
	}

	pRow->cols = cols;
	pRow->rows = rows;
}

#if USE_PYTHON
extern int UpdateQuery(char *sz)
{
	PyObject *v, *r;

	r = Connect();

	/* run query */
	if (!(v = PyObject_CallMethod(r, "update", "s", sz))
		|| v == Py_None)
	{
		outputl(_("Error running query"));
		PyErr_Print();
		return FALSE;
	}
	Py_DECREF(v);
	Disconnect(r);
	return TRUE;
}

extern int RunQuery(RowSet* pRow, char *sz)
{
	PyObject *v, *r;
	int i, j;

	r = Connect();

	/* enter query */
	if (!(v = PyObject_CallMethod(r, "select", "s", sz))
		|| v == Py_None)
	{
		outputl(_("Error running query"));
		PyErr_Print();
		return FALSE;
	}

	if (!PySequence_Check(v))
	{
		outputl( _("invalid return (non-tuple)") );
		Py_DECREF(v);
		Disconnect(r);
		return FALSE;
	}
		
	i = PySequence_Size(v);
	j = 0;
	if (i > 0)
	{
		PyObject *cols = PySequence_GetItem(v, 0);
		if (!PySequence_Check(cols))
		{
			outputl( _("invalid return (non-tuple)") );
			Py_DECREF(v);
			Disconnect(r);
			return FALSE;
		}
		else
			j = PySequence_Size(cols);
	}

	MallocRowset(pRow, i, j);

	for (i = 0; i < pRow->rows; i++)
	{
		PyObject *e = PySequence_GetItem(v, i);

		if (!e)
		{
			outputf(_("Error getting item no %d\n"), i);
			continue;
		}

		if (PySequence_Check(e))
		{
			int j, size;
			for (j = 0; j < pRow->cols; j++)
			{
				char buf[1024];
				PyObject *e2 = PySequence_GetItem(e, j);

				if (!e2)
				{
					outputf(_("Error getting sub item no (%d, %d)\n"), i, j);
					continue;
				}

				if (PyString_Check(e2))
				{
					strcpy(buf, PyString_AsString(e2));
					size = strlen(buf);
				}
				else if (PyInt_Check(e2))
				{
					sprintf(buf, "%d", (int)PyInt_AsLong(e2));
					size = ((int)log((int)PyInt_AsLong(e2))) + 1;
				}
				else if (PyFloat_Check(e2))
				{
					sprintf(buf, "%.2f", PyFloat_AsDouble(e2));
					size = ((int)log(PyFloat_AsDouble(e2))) + 1 + 3;
				}
				else if (e2 == Py_None)
				{
					strcpy(buf, _("[null]"));
					size = strlen(buf);
				}
				else
				{
					strcpy(buf, _("[unknown type]"));
					size = strlen(buf);
				}

				if (i == 0 || size > pRow->widths[j])
					pRow->widths[j] = size;

				pRow->data[i][j] = malloc(strlen(buf) + 1);
				strcpy(pRow->data[i][j], buf);

				Py_DECREF(e2);
			}
		}
		else
		{
			outputf(_("Item no. %d is not a sequence\n"), i);
		}

		Py_DECREF(e);
	}

	Py_DECREF(v);
	Disconnect(r);

	return TRUE;
}
#endif
#endif

extern void CommandRelationalSelect(char *sz)
{
#if USE_PYTHON
#if !USE_GTK
	int i, j;
#endif
	RowSet r;

	if (!sz || !*sz)
	{
		outputl( _("You must specify a sql query to run').") );
		return;
	}

	if (!RunQuery(&r, sz))
		return;

	if (r.rows == 0)
	{
		outputl(_("No rows found.\n"));
		return;
	}

#if USE_GTK
	GtkShowQuery(&r);
#else
	for (i = 0; i < r.rows; i++)
	{
		if (i == 1)
		{	/* Underline headings */
			char* line, *p;
			int k;
			int totalwidth = 0;
			for (k = 0; k < r.cols; k++)
			{
				totalwidth += r.widths[k] + 1;
				if (k != 0)
					totalwidth += 2;
			}
			line = malloc(totalwidth + 1);
			memset(line, '-', totalwidth);
			p = line;
			for (k = 0; k < r.cols - 1; k++)
			{
				p += r.widths[k];
				p[1] = '|';
				p += 3;
			}
			line[totalwidth] = '\0';
			outputl(line);
			free(line);
		}

		for (j = 0; j < r.cols; j++)
		{
			if (j > 0)
				output(" | ");

			outputf("%*s", r.widths[j], r.data[i][j]);
		}
		outputl("");
	}
#endif
	FreeRowset(&r);

#else /* USE_PYTHON */
	outputl( _("This build was not compiled with support for Python.\n") );
#endif /* !USE_PYTHON */
}
#if USE_PYTHON
extern void RelationalUpdatePlayerDetails(int player_id, const char* newName,
										  const char* newNotes)
{
	char query[1024];
	RowSet r;

	/* Can't change the name to an existing one */
	sprintf(query, "person_id FROM person WHERE name = '%s'", newName);
	if (!RunQuery(&r, query))
	{
		outputerrf( _("Error running database command") );
		return;
	}
	if (r.rows > 1)
		outputerrf( _("Player name already exists.  Use the link button to combine different"
			" nicknames for the same player") );
	else
	{
		sprintf(query, "person SET name = \"%s\", notes = \"%s\" WHERE person_id = %d",
			newName, newNotes, player_id);
		if (!UpdateQuery(query))
			outputerrf( _("Error running database command") );
	}
	FreeRowset(&r);
}
#endif
