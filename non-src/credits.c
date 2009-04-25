/* Do not modify this file!  It is created automatically by credits.sh.
   Modify credits.sh instead. 
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
*/

#include "config.h"
#include <glib/gi18n.h>
#include "credits.h"

const char aszCOPYRIGHT[] = N_("Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 by Gary Wong.\n"
                               "Copyright (C) 2009 by Gary Wong and the AUTHORS; for details type `show version'.");

credEntry ceAuthors[] = {
  {"Joseph Heled", 0},
  {"Oystein Johansen", 0},
  {"Jonathan Kinsey", 0},
  {"David Montgomery", 0},
  {"Jim Segrave", 0},
  {"Joern Thyssen", 0},
  {"Gary Wong", 0},
  {"Christian Anthon", 0},
  {0, 0}
};

credEntry ceContrib[] = {
  {"Olivier Baur", 0},
  {"Holger Bochnig", 0},
  {"Nis Joergensen", 0},
  {"Petr Kadlec", 0},
  {"Kaoru Takahashi", 0},
  {"Stein Kulseth", 0},
  {"Mike Petch", 0},
  {"Rod Roark", 0},
  {"Ingo Macherius", 0},
  {0, 0}
};

credEntry ceSupport[] = {
  {"Oystein Johansen", N_("Web Pages") },
  {"Achim Mueller", N_("Manual") },
  {"Nardy Pillards", N_("Web Pages") },
  {"Albert Silver", N_("Tutorial") },
  {0, 0}
};

credEntry ceTranslations[] = {
  {"Petr Kadlec", N_("Czech") },
  {"Joern Thyssen", N_("Danish") },
  {"Olivier Baur", N_("French") },
  {"Achim Mueller", N_("German") },
  {"Hlynur Sigurgislason", N_("Icelandic") },
  {"Renzo Campagna", N_("Italian") },
  {"Yoshito Takeuchi", N_("Japanese") },
  {"Akif Dinc", N_("Turkish") },
  {"Dmitri I GOULIAEV", N_("Russian") },
  {0, 0}
};

credEntry ceCredits[] = {
  {"Fotis", 0},
  {"Misja Alma", 0},
  {"Kazuaki Asai", 0},
  {"Eric Augustine", 0},
  {"Steve Baedke", 0},
  {"Erik Barfoed", 0},
  {"Ron Barry", 0},
  {"Hans Berliner", 0},
  {"Chuck Bower", 0},
  {"Nick Bratby", 0},
  {"Adrian Bunk", 0},
  {"Craig Campbell", 0},
  {"John Chuang", 0},
  {"Tristan Colgate", 0},
  {"Olivier Croisille", 0},
  {"Ned Cross", 0},
  {"Ian Curtis", 0},
  {"Bill Davidson", 0},
  {"Giulio De Marco", 0},
  {"Michael Depreli", 0},
  {"Alberta di Silvio", 0},
  {"Ian Dunstan", 0},
  {"Max Durbano", 0},
  {"Christopher D. Yep", 0},
  {"Peter Eberhard", 0},
  {"Robert Eberlein", 0},
  {"James F. Kibler", 0},
  {"Kennedy Fraser", 0},
  {"Ric Gerace", 0},
  {"Michel Grimminck", 0},
  {"Eric Groleau", 0},
  {"Jeff Haferman", 0},
  {"Alain Henry", 0},
  {"Jens Hoefkens", 0},
  {"Casey Hopkins", 0},
  {"Martin Janke", 0},
  {"Rick Janowski", 0},
  {"Nis Jorgensen", 0},
  {"Morten Juul Henriksen", 0},
  {"Neil Kazaross", 0},
  {"Mathias Kegelmann", 0},
  {"Matija Kejzar", 0},
  {"Johnny Kirk", 0},
  {"Gerhard Knop", 0},
  {"Robert Konigsberg", 0},
  {"Martin Krainer", 0},
  {"Elias Kritikos", 0},
  {"Corrin Lakeland", 0},
  {"Tim Laursen", 0},
  {"Eli Liang", 0},
  {"Ege Lundgren", 0},
  {"Kevin Maguire", 0},
  {"Massimiliano Maini", 0},
  {"Alix Martin", 0},
  {"John Marttila", 0},
  {"William Maslen", 0},
  {"Joachim Matussek", 0},
  {"Thomas Meyer", 0},
  {"Daniel Murphy", 0},
  {"Magnar Naustdalslid", 0},
  {"Dave Neary", 0},
  {"Rolf Nielsen", 0},
  {"Mirori Orange", 0},
  {"Peter Orum", 0},
  {"Roy Passfield", 0},
  {"Thomas Patrick", 0},
  {"Billie Patterson", 0},
  {"Zvezdan Petkovic", 0},
  {"Petri Pitkanen", 0},
  {"Sam Pottle", 0},
  {"Henrik Ravn", 0},
  {"James Rech", 0},
  {"Jared Riley", 0},
  {"Klaus Rindholt", 0},
  {"Oliver Riordan", 0},
  {"Alex Romosan", 0},
  {"Hans-Juergen Schaefer", 0},
  {"Steve Schreiber", 0},
  {"Martin Schrode", 0},
  {"Hugh Sconyers", 0},
  {"Paul Selick", 0},
  {"Sho Sengoku", 0},
  {"Ian Shaw", 0},
  {"Brian Sheppard", 0},
  {"Peter Sochovsky", 0},
  {"Mark Spencer", 0},
  {"Scott Steiner", 0},
  {"Maik Stiebler", 0},
  {"W. Stroop (Rob)", 0},
  {"Yoshito Takeuchi", 0},
  {"Gerry Tesauro", 0},
  {"Jacques Thiriat", 0},
  {"Malene Thyssen", 0},
  {"Claes Tornberg", 0},
  {"Stefan van den Berg", 0},
  {"Bert Van Kerckhove", 0},
  {"Sander van Rijnswou", 0},
  {"Robert-Jan Veldhuizen", 0},
  {"Morten Wang", 0},
  {"Jeff White", 0},
  {"JP White", 0},
  {"Mike Whitton", 0},
  {"Chris Wilson", 0},
  {"Kit Woolsey", 0},
  {"Frank Worrell", 0},
  {"Anders Zachrison", 0},
  {"Douglas Zare", 0},
  {"Michael Zehr", 0},
  {0, 0}
};

credits creditList[] =
{
	{N_("AUTHORS"), ceAuthors},
	{N_("Code Contributors"), ceContrib},
	{N_("Translations"), ceTranslations},
	{N_("Support"), ceSupport},
	{0, 0}
};

const char aszAUTHORS[] =
"                         GNU Backgammon was written by:\n"
"\n"
"    Joseph Heled            David Montgomery        Gary Wong\n"
"    Oystein Johansen        Jim Segrave             Christian Anthon\n"
"    Jonathan Kinsey         Joern Thyssen\n"
" \n"
"                                   Support by:\n"
"\n"
"     Oystein Johansen (Web Pages)    Nardy Pillards (Web Pages)\n"
"     Achim Mueller (Manual)          Albert Silver (Tutorial)\n"
"\n"
"                         Contributors of code include:\n"
"\n"
"    Olivier Baur    Petr Kadlec     Mike Petch\n"
"    Holger Bochnig  Kaoru Takahashi Rod Roark\n"
"    Nis Joergensen  Stein Kulseth   Ingo Macherius\n"
"\n"
"                            Translations by:\n"
"\n"
"     Petr Kadlec (Czech)\n"
"     Joern Thyssen (Danish)\n"
"     Olivier Baur (French)\n"
"     Achim Mueller (German)\n"
"     Hlynur Sigurgislason (Icelandic)\n"
"     Renzo Campagna (Italian)\n"
"     Yoshito Takeuchi (Japanese)\n"
"     Akif Dinc (Turkish)\n"
"     Dmitri I GOULIAEV (Russian)\n"
"\n"
"             Contributors (of bug reports and suggestions) include:\n"
"\n"
"    Misja Alma              Mathias Kegelmann       Jared Riley\n"
"    Kazuaki Asai            Matija Kejzar           Klaus Rindholt\n"
"    Eric Augustine          Bert Van Kerckhove      Oliver Riordan\n"
"    Erik Barfoed            James F. Kibler         Alex Romosan\n"
"    Ron Barry               Johnny Kirk             Hans-Juergen Schaefer\n"
"    Steve Baedke            Gerhard Knop            Steve Schreiber\n"
"    Stefan van den Berg     Robert Konigsberg       Hugh Sconyers\n"
"    Chuck Bower             Martin Krainer          Martin Schrode\n"
"    Adrian Bunk             Elias Kritikos          Paul Selick\n"
"    Nick Bratby             Corrin Lakeland         Sho Sengoku\n"
"    Craig Campbell          Tim Laursen             Ian Shaw\n"
"    John Chuang             Eli Liang               Alberta di Silvio\n"
"    Tristan Colgate         Ege Lundgren            Peter Sochovsky\n"
"    Olivier Croisille       Kevin Maguire           Mark Spencer\n"
"    Ned Cross               Massimiliano Maini      Scott Steiner\n"
"    Ian Curtis              Giulio De Marco         Maik Stiebler\n"
"    Bill Davidson           John Marttila           W. Stroop (Rob)\n"
"    Michael Depreli         Alix Martin             Yoshito Takeuchi\n"
"    Ian Dunstan             William Maslen          Jacques Thiriat\n"
"    Max Durbano             Joachim Matussek        Malene Thyssen\n"
"    Peter Eberhard          Thomas Meyer            Claes Tornberg\n"
"    Robert Eberlein         Daniel Murphy           Sander van Rijnswou\n"
"    Fotis                   Magnar Naustdalslid     Robert-Jan Veldhuizen\n"
"    Kennedy Fraser          Dave Neary              Morten Wang\n"
"    Ric Gerace              Rolf Nielsen            Jeff White\n"
"    Michel Grimminck        Mirori Orange           JP White\n"
"    Eric Groleau            Peter Orum              Mike Whitton\n"
"    Jeff Haferman           Roy Passfield           Chris Wilson\n"
"    Morten Juul Henriksen   Thomas Patrick          Kit Woolsey\n"
"    Alain Henry             Billie Patterson        Frank Worrell\n"
"    Jens Hoefkens           Zvezdan Petkovic        Christopher D. Yep\n"
"    Casey Hopkins           Petri Pitkanen          Anders Zachrison\n"
"    Martin Janke            Sam Pottle              Douglas Zare\n"
"    Nis Jorgensen           Henrik Ravn\n"
"    Neil Kazaross           James Rech\n"
"\n"
"\n"
"  Credit is also due to those who have published information about backgammon\n"
"   playing programs (references will appear here later).  GNU Backgammon has\n"
"                              borrowed ideas from:\n"
"\n"
"    Hans Berliner   Rick Janowski   Gerry Tesauro   Douglas Zare\n"
"    Chuck Bower     Brian Sheppard  Morten Wang     Michael Zehr\n"
"\n"
"       \n"
"    The manual for GNU Backgammon includes a chapter describing the rules of\n"
"      backgammon, written by Tom Keith for his Backgammon Galore web site\n"
"                             <http://www.bkgm.com/>.\n"
"\n"
"\n"
"  Library code from the following authors has been included in GNU Backgammon:\n"
"\n"
"     Ulrich Drepper (an implementation of the public domain MD5 algorithm)\n"
"             Bob Jenkins (the ISAAC pseudo random number generator)\n"
"       Takuji Nishimura and Makoto Matsumoto (the Mersenne Twister PRNG)\n"
"                Gerry Tesauro (the \"pubeval\" position evaluator)\n"
"             Claes Tornberg (the mec match equity table generator)\n"
"\n"
"\n"
"      If you feel that you're not given credits for your contributions to\n"
"         GNU Backgammon please write to one of the developers.\n"
"\n"
"\n"
"       Please send bug reports for GNU Backgammon to: <bug-gnubg@gnu.org>\n"
;
