#!/bin/sh
# $Id: credits.sh,v 1.2 2001/03/26 15:35:22 gtw Exp $

contributors=/tmp/cont.$$
credit=/tmp/cred.$$

cat > AUTHORS <<'EOF'
                         GNU Backgammon was written by:

  Joseph Heled         �ystein Johansen         J�rn Thyssen         Gary Wong



         Contributors (of bug reports, comments, and patches) include:

EOF

cat > $contributors <<'EOF'
Eric Augustine
Steve Baedke
Bill Davidson
Eric Groleau
Jeff Haferman
Alain Henry
Jens Hoefkens
Matija Kejzar
Corrin Lakeland
Kevin Maguire
David Montgomery
Dave Neary
Zvezdan Petkovic
Petri Pitk�nen
Sam Pottle
Jared Riley
Frank Worrell
EOF

column -c 72 < $contributors | expand | sed 's/^/    /' >> AUTHORS

cat >> AUTHORS <<'EOF'


  Credit is also due to those who have published information about backgammon
   playing programs (references will appear here later).  GNU Backgammon has
                              borrowed ideas from:

EOF

cat > $credit <<'EOF'
Hans Berliner
Chuck Bower
Rick Janowski
David Montgomery
Brian Sheppard
Gerry Tesauro
Michael Zehr
EOF

column -c 72 < $credit | expand | sed 's/^/    /' >> AUTHORS

cat >> AUTHORS <<'EOF'                            

       
    The manual for GNU Backgammon includes a chapter describing the rules of
      backgammon, written by Tom Keith for his Backgammon Galore web site
                             <http://www.bkgm.com/>.


  Library code from the following authors has been included in GNU Backgammon:

     Ulrich Drepper (an implementation of the public domain MD5 algorithm)
             Bob Jenkins (the ISAAC pseudo random number generator)
     Takuji Nishimura (the Mersenne Twister pseudo random number generator)
                Gerry Tesauro (the "pubeval" position evaluator)


       Please send bug reports for GNU Backgammon to: <bug-gnubg@gnu.org>
EOF

cat > credits.c <<'EOF'
/* Do not modify this file!  It is created automatically by credits.sh.
   Modify credits.sh instead. */

char *aszCredits[] = {
EOF

cat $contributors $credit - <<'EOF' | sort -k 2 | uniq | sed 's/.*/    "&",/' \
    >> credits.c
Tom Keith
EOF

cat >> credits.c <<'EOF'
    0
};
EOF
