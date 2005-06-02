�e�tina pro GNUBG
*****************

Jak nainstalovat?
-----------------
Pokud ho snad je�t� nem�te, budete opravdu pot�ebovat GNU Backgammon.
Odkaz na sta�en� najdete na str�nk�ch na adrese http://www.gnubg.org/,
nemus�te se b�t, jedn� se o voln� software ���en� zdarma pod GNU licenc�.

Pot� rozbalte arch�v s �esk�m p�ekladem do adres��e, do kter�ho jste
GNU Backgammon nainstalovali. (T�m v n�m vznikne adres�� LOCALE.)

Spus�te GNU Backgammon.

Pokud provozujete �esk� opera�n� syst�m, je dost mo�n�, �e program ji�
b�� v �e�tin�. Pokud ne, zvolte nab�dku Settings, v n� polo�ku Options.
V okn�, kter� se t�m otev�e, zvolte posledn� z�lo�ku Other. Na t�to
z�lo�ce dole je polo�ka Language. Z nab�dky vyberte Czech. Pot� potvr�te
stisknut�m OK. Ulo�te nastaven� pomoc� polo�ky Save settings v nab�dce
Settings (pokud nastaven� neulo��te, nebude to fungovat!). Odejd�te
z GNU Backgammonu a spus�te ho znovu. Te� u� by m�l b�t ur�it� �esky.

U��vejte si hru v �e�tin�!


Probl�my?
---------
Ot�zka:  V m�m programu v nab�dce jazyk� �e�tina chyb�! Co m�m d�lat?
Odpov��: M�te dv� mo�nosti: Bu� si st�hn�te novou verzi programu (na
str�nk�ch http://www.gnubg.org/ najd�te odkaz na "Daily builds", odkud
si m��ete st�hnout ka�d� den aktu�ln� v�vojovou verzi), nebo m��ete
jazyk nastavit ru�n�: V nab�dce Edit vyberte Enter command a do
nab�dnut�ho ��dku zadejte "set lang cs" (bez uvozovek). Pot� ulo�te
nastaven� a restartujte gnubg podle p�edchoz�ho n�vodu. V�echno by m�lo
fungovat.

Ot�zka: P�i experimentov�n� jsem si nastavil n�jak� ��len� jazyk a te�
ni�emu nerozum�m. Jak tam m�m vr�tit �e�tinu?
Odpov��: V adres��i programu GNU Backgammon je soubor .gnubgautorc. Ten
otev�ete v n�jak�m textov�m editoru a najd�te v n�m text "set lang" (bez
uvozovek). Bu� m��ete cel� ��dek vymazat, nebo nastavit jazykov� k�d
na jazyk, kter� chcete pou��vat (k�d �e�tiny je cs).

Ot�zka: Skoro v�echno je �esky, ale ob�as se objev� i n�co anglicky. Kde
je probl�m?
Odpov��: Syst�m GNU gettext, pomoc� kter�ho je lokalizace GNU Backgammonu
provedena, funguje tak, �e pokud nen� nalezen p�eklad pro dan� term�n,
je pou�ita p�vodn� (anglick�) verze. Tak�e, pokud se v�m ob�as objev�
n�co anglicky, znamen� to, �e v p�ekladu n�co chyb�. Dal�� informace
viz n�sleduj�c� kapitolka.


Chyby v p�ekladu
----------------
Pokud najdete n�jakou chybu v p�ekladu (jako t�eba p�eklep, pravopisnou
chybu, chybn� p�elo�en� term�n, atd.), rozhodn� mi o tom dejte v�d�t!
Moje e-mailov� adresa je mormegil@centrum.cz a pokud u mailu pou�ijete
n�jak� smyslupln� p�edm�t, ve kter�m se objev� "p�eklad gnubg", bude to
fajn.

Ale budu r�d, pokud p�ed nahl�en�m chyby budete p�em��let. Pokud jste si
pr�v� st�hli nejnov�j�� verzi gnubg, ve kter� se objevily nov� texty, tak
byste se rozhodn� nem�li divit, �e dosud nejsou p�elo�eny. Obecn�, pokud
se n�kde objev� p�vodn� anglick� text, je to bu� proto, �e se objevil
v nov� verzi, pro kterou dosud nem�te p�eklad, nebo proto, �e je to chyba
v p�vodn�m gnubg. D�ky mechanismu p�ekladu se _nem��e_ st�t, �e bych
zapomn�l p�elo�it jednu v�tu. Proto pros�m m�jte strpen�, na odstran�n�
takov�ch probl�m� se pracuje. P�esto, pokud se v�m zd�, �e p�eklad dan�ho
text u� chyb� dost dlouho (anebo je na n�jak�m z��dka pou��van�m m�st�),
dejte mi v�d�t, je mo�n�, �e jsem si t� chyby dosud ani nev�iml.

Pokud se v�m nezd� n�jak� odborn� term�n (t�eba proto, �e jste zvykl� na
p�vodn� anglick�), neznamen� to je�t�, �e je jeho p�eklad chybn�. Pokud mi
navrhnete lep�� term�n, budu ho zva�ovat, ale st�nosti typu "m� se v�c l�b�
anglick� term�n" budu ignorovat (jako odpov�� si p�edstavte "tak pou��vejte
anglickou verzi").


Nov� verze
----------
Jeliko� GNU Backgammon se st�le bou�liv� vyv�j�, pr�b�n� v n�m p�ib�vaj�
(a m�n� se) texty. Proto i p�eklad se mus� vyv�jet. Nov� verze p�ekladu
by se m�ly ���it spole�n� s GNU Backgammonem, ale budu se sna�it udr�ovat
v�dy posledn� verzi na http://mormegil.wz.cz/bg/
Doporu�en�hodn� je v�dy updatovat p�eklad sou�asn� s GNU Backgammonem tak,
abyste pou��vali v�dy tu verzi p�ekladu, kter� souhlas� s programem.


Licence
-------
�esk� p�eklad je ���en podle stejn� licence jako cel� bal�k GNU Backgammon,
tzn. podle GNU licence, co� znamen�, �e ho sm�te voln� kop�rovat.
(Podrobnosti viz text licence.)


Autor
-----
Autor p�ekladu, Petr Kadlec, je k zasti�en� na e-mailov� adrese
mormegil@centrum.cz, na kter� o�ek�v� i va�e p�ipom�nky a informace
o chyb�ch v p�ekladu. Ob�as ho m��ete zastihnout i na ICQ (#68196926).
