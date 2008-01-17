/*
 * backgammon.h
 *
 * by Gary Wong <gtw@gnu.org>, 1999, 2000, 2001, 2002.
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
 * $Id: backgammon.h,v 1.364 2008/01/17 22:28:03 Superfly_Jon Exp $
 */

#ifndef _BACKGAMMON_H_
#define _BACKGAMMON_H_

#include <sys/types.h>
#include "gnubg-types.h"
#include <stdarg.h>
#include "analysis.h"
#include "eval.h"

#define MAX_CUBE ( 1 << 12 )
#define MAX_NAME_LEN 32
#define VERSION_STRING "GNU Backgammon " VERSION
#define GNUBG_CHARSET "UTF-8"


typedef struct _command {
	/* Command name (NULL indicates end of list) */
	const char *sz;
	/* Command handler; NULL to use default subcommand handler */
	void (*pf) (char *);
	/* Documentation; NULL for abbreviations */
	const char *szHelp;
	const char *szUsage;
	/* List of subcommands (NULL if none) */
	struct _command *pc;
} command;

typedef enum _playertype {
	PLAYER_HUMAN, PLAYER_GNU, PLAYER_EXTERNAL
} playertype;

typedef struct _player {
	/* For all player types: */
	char szName[MAX_NAME_LEN];
	playertype pt;
	/* For PLAYER_GNU: */
	evalsetup esChequer;
	evalsetup esCube;
	movefilter aamf[MAX_FILTER_PLIES][MAX_FILTER_PLIES];
	int h;
	/* For PLAYER_EXTERNAL: */
	char *szSocket;
} player;

typedef enum _movetype {
	MOVE_GAMEINFO,
	MOVE_NORMAL,
	MOVE_DOUBLE,
	MOVE_TAKE,
	MOVE_DROP,
	MOVE_RESIGN,
	MOVE_SETBOARD,
	MOVE_SETDICE,
	MOVE_SETCUBEVAL,
    MOVE_SETCUBEPOS
} movetype;

typedef struct _movegameinfo {

	/* ordinal number of the game within a match */
	int i;
	/* match length */
	int nMatch;
	/* match score BEFORE the game */
	int anScore[2];
	/* the Crawford rule applies during this match */
	int fCrawford;
	/* this is the Crawford game */
	int fCrawfordGame;
	int fJacoby;
	/* who won (-1 = unfinished) */
	int fWinner;
	/* how many points were scored by the winner */
	int nPoints;
	/* the game was ended by resignation */
	int fResigned;
	/* how many automatic doubles were rolled */
	int nAutoDoubles;
	/* Type of game */
	bgvariation bgv;
	/* Cube used in game */
	int fCubeUse;
	statcontext sc;
} xmovegameinfo;

typedef struct cubedecisiondata {
	float aarOutput[2][NUM_ROLLOUT_OUTPUTS];
	float aarStdDev[2][NUM_ROLLOUT_OUTPUTS];
	evalsetup esDouble;
} cubedecisiondata;

typedef struct _movenormal {
	/* Move made. */
	int anMove[8];
	/* index into the movelist of the move that was made */
	unsigned int iMove;
	skilltype stMove;
} xmovenormal;

typedef struct _moveresign {
	int nResigned;
	evalsetup esResign;
	float arResign[NUM_ROLLOUT_OUTPUTS];
	skilltype stResign;
	skilltype stAccept;
} xmoveresign;

typedef struct _movesetboard {
	unsigned char auchKey[10];	/* always stored as if player 0 was on roll */
} xmovesetboard;

typedef struct _movesetcubeval {
	int nCube;
} xmovesetcubeval;

typedef struct _movesetcubepos {
	int fCubeOwner;
} xmovesetcubepos;

typedef struct _moverecord {
	/* 
	 * Common variables
	 */
	/* type of the move */
	movetype mt;
	/* annotation */
	char *sz;
	/* move record is for player */
	int fPlayer;
	/* luck analysis (shared between MOVE_SETDICE and MOVE_NORMAL) */
	/* dice rolled */
	unsigned int anDice[2];
	/* classification of luck */
	lucktype lt;
	/* magnitude of luck */
	float rLuck;		/* ERR_VAL means unknown */
	/* move analysis (shared between MOVE_SETDICE and MOVE_NORMAL) */
	/* evaluation setup for move analysis */
	evalsetup esChequer;
	/* evaluation of the moves */
	movelist ml;
	/* cube analysis (shared between MOVE_NORMAL and MOVE_DOUBLE) */
	/* 0 in match play, even numbers are doubles, raccoons 
	   odd numbers are beavers, aardvarken, etc. */
	int nAnimals;
	/* the evaluation and settings */
	cubedecisiondata *CubeDecPtr;
	cubedecisiondata CubeDec;
	/* skill for the cube decision */
	skilltype stCube;
	/* "private" data */
	xmovegameinfo g;	/* game information */
	xmovenormal n;		/* chequerplay move */
	xmoveresign r;		/* resignation */
	xmovesetboard sb;	/* setting up board */
	xmovesetcubeval scv;	/* setting cube */
	xmovesetcubepos scp;	/* setting cube owner */
} moverecord;

typedef enum {
	GAME_NONE, GAME_PLAYING, GAME_OVER, GAME_RESIGNED, GAME_DROP
} gamestate;

typedef struct {
	TanBoard anBoard;
	unsigned int anDice[2];	/* (0,0) for unrolled dice */
	int fTurn;		/* who makes the next decision */
	int fResigned;
	int fResignationDeclined;
	int fDoubled;
	int cGames;
	int fMove;		/* player on roll */
	int fCubeOwner;
	int fCrawford;
	int fPostCrawford;
	int nMatchTo;
	int anScore[2];
	int nCube;
	unsigned int cBeavers;
	bgvariation bgv;
	int fCubeUse;
	int fJacoby;
	gamestate gs;
} matchstate;

typedef struct _matchinfo {	/* SGF match information */
	char *pchRating[2];
	char *pchEvent;
	char *pchRound;
	char *pchPlace;
	char *pchAnnotator;
	char *pchComment;	/* malloc()ed, or NULL if unknown */
	uint nYear;
	uint nMonth;
	uint nDay;		/* 0 for nYear means date unknown */
} matchinfo;

typedef struct _storedmoves {
	movelist ml;
	matchstate ms;
} storedmoves;

typedef struct _storedcube {
	float aarOutput[2][NUM_ROLLOUT_OUTPUTS];
	float aarStdDev[2][NUM_ROLLOUT_OUTPUTS];
	evalsetup es;
	matchstate ms;
} storedcube;

/* There is a global storedmoves struct to maintain the list of moves
   for "=n" notation (e.g. "hint", "rollout =1 =2 =4").
   Anything that _writes_ stored moves ("hint", "show moves", "add move")
   should free the old dynamic move list first (sm.ml.amMoves), if it is
   non-NULL.
   Anything that _reads_ stored moves should check that the move is still
   valid (i.e. auchKey matches the current board and anDice matches the
   current dice). */
extern storedmoves sm;

/*
 * Store cube analysis
 *
 */

extern storedcube sc;

/*  List of moverecords representing the current game. One of the elements in
    lMatch.
    Typically the last game in the match).
*/
extern listOLD *plGame;

/* Current move inside plGame (typically the most recently
   one played, but "previous" and "next" commands navigate back and forth).
*/
extern listOLD *plLastMove;

/* The current match.
  A list of games. Each game is a list of moverecords.
  Note that the first list element is empty. The first game is in
  lMatch.plNext->p. Same is true for games.
*/
extern listOLD lMatch;

extern char *aszCopying[];
extern char *aszGameResult[];
extern char *aszLuckType[];
extern char *aszLuckTypeAbbr[];
extern char *aszLuckTypeCommand[];
extern char *aszSkillType[];
extern char *aszSkillTypeAbbr[];
extern char *aszSkillTypeCommand[];
extern char *aszWarranty[];
extern char *default_export_folder;
extern char *default_import_folder;
extern char *default_sgf_folder;
extern char *log_file_name;
extern char *szCurrentFileName;
extern char *szCurrentFolder;
extern char szDefaultPrompt[];
extern char *szLang;
extern char *szPrompt;
extern const char *szHomeDirectory;
extern evalcontext ecLuck;
extern evalsetup esAnalysisChequer;
extern evalsetup esAnalysisCube;
extern evalsetup esEvalChequer;
extern evalsetup esEvalCube;
extern float arLuckLevel[N_LUCKS];
extern float arSkillLevel[N_SKILLS];
extern float rEvalsPerSec;
extern float rRatingOffset;
extern int fAnalyseCube;
extern int fAnalyseDice;
extern int fAnalyseMove;
extern int fAutoBearoff;
extern int fAutoCrawford;
extern int fAutoGame;
extern int fAutoMove;
extern int fAutoRoll;
extern int fCheat;
extern int fComputing;
extern int fConfirm;
extern int fConfirmSave;
extern int fCubeEqualChequer;
extern int fPlayersAreSame;
extern int fTruncEqualPlayer0;
extern int fCubeUse;
extern int fDisplay;
extern int fFullScreen;
extern int fGotoFirstGame;
extern int fInvertMET;
extern int fJacoby;
extern int fNextTurn;
extern int fOutputRawboard;
extern int fRecord;
extern int fShowProgress;
extern int fStyledGamelist;
extern int fTutor;
extern int fTutorAnalysis;
extern int fTutorChequer;
extern int fTutorCube;
extern int log_rollouts;
extern int nThreadPriority;
extern int nToolbarStyle;
extern int nTutorSkillCurrent;
#if USE_BOARD3D
extern int fSync;
extern int fResetSync;
#endif
extern matchinfo mi;
extern matchstate ms;
extern ConstTanBoard msBoard(void);
extern movefilter aamfAnalysis[MAX_FILTER_PLIES][MAX_FILTER_PLIES];
extern movefilter aamfEval[MAX_FILTER_PLIES][MAX_FILTER_PLIES];
extern player ap[2];
extern rolloutcontext rcRollout;
extern skilltype TutorSkill;
extern statcontext scMatch;
extern unsigned int afCheatRoll[2];
extern unsigned int cAnalysisMoves;
extern unsigned int cAutoDoubles;
extern unsigned int nBeavers;
extern unsigned int nDefaultLength;
extern void *rngctxRollout;

extern command acAnnotateMove[];
extern command acSet[];
extern command acSetAnalysisPlayer[];
extern command acSetCheatPlayer[];
extern command acSetEvalParam[];
extern command acSetEvaluation[];
extern command acSetExportParameters[];
extern command acSetGeometryValues[];
extern command acSetPlayer[];
extern command acSetRNG[];
extern command acSetRollout[];
extern command acSetRolloutJsd[];
extern command acSetRolloutLate[];
extern command acSetRolloutLatePlayer[];
extern command acSetRolloutLimit[];
extern command acSetRolloutPlayer[];
extern command acSetTruncation[];
extern command acTop[];
extern command cFilename;
extern command cOnOff;

/* Temporarily ignore TTY/GUI input. */
extern void SuspendInput(void);
/* Resume input (must match a previous SuspendInput). */
extern void ResumeInput(void);
/* Write a string to stdout/status bar/popup window */
extern void output(const char *sz);
/* Write a string to stdout/status bar/popup window, and append \n */
extern void outputl(const char *sz);
/* Write a character to stdout/status bar/popup window */
extern void outputc(const char ch);
/* Write a string to stdout/status bar/popup window, printf style */
extern void outputf(const char *sz, ...)
    __attribute__ ((format(printf, 1, 2)));
/* Write a string to stdout/status bar/popup window, vprintf style */
extern void outputv(const char *sz, va_list val)
    __attribute__ ((format(printf, 1, 0)));
/* Write an error message, perror() style */
extern void outputerr(const char *sz);
/* Write an error message, fprintf() style */
extern void outputerrf(const char *sz, ...)
    __attribute__ ((format(printf, 1, 2)));
/* Write an error message, vfprintf() style */
extern void outputerrv(const char *sz, va_list val)
    __attribute__ ((format(printf, 1, 0)));
/* Signifies that all output for the current command is complete */
extern void outputx(void);
/* Temporarily disable outputx() calls */
extern void outputpostpone(void);
/* Re-enable outputx() calls */
extern void outputresume(void);
/* Signifies that subsequent output is for a new command */
extern void outputnew(void);
/* Disable output */
extern void outputoff(void);
/* Enable output */
extern void outputon(void);
/* Like strncpy, except it does the right thing */
extern char *strcpyn(char *szDest, const char *szSrc, int cch);

extern char *CheckCommand(char *sz, command * ac);
extern char *FormatMoveHint(char *sz, const matchstate * pms, movelist * pml,
			    int i, int fRankKnown, int fDetailProb,
			    int fShowParameters);
extern char *FormatPrompt(void);
extern char *GetBuildInfoString(void);
extern char *GetInput(char *szPrompt);
extern char *GetLuckAnalysis(const matchstate * pms, float rLuck);
extern char *GetMoveString(moverecord * pmr, int *pPlayer);
extern double get_time(void);
extern char *locale_from_utf8(const char *sz);
extern char *locale_to_utf8(const char *sz);
extern char *NextToken(char **ppch);
extern char *NextTokenGeneral(char **ppch, const char *szTokens);
extern char *SetupLanguage(char *newLangCode);
extern command *FindHelpCommand(command * pcBase, char *sz,
				char *pchCommand, char *pchUsage);
extern double ParseReal(char **ppch);
extern int AnalyzeMove(moverecord * pmr, matchstate * pms,
		       const listOLD * plGame, statcontext * psc,
		       const evalsetup * pesChequer, evalsetup * pesCube,
		       movefilter aamf[MAX_FILTER_PLIES][MAX_FILTER_PLIES],
		       const int afAnalysePlayers[2], float *doubleError);
extern int CompareNames(char *sz0, char *sz1);
extern int confirmOverwrite(const char *sz, const int f);
extern int EPC(const TanBoard anBoard, float *arEPC, float *arMu,
	       float *arSigma, int *pfSource, const int fOnlyBearoff);
extern int EvalCmp(const evalcontext *, const evalcontext *, const int);
extern int getFinalScore(int *anScore);
extern int GetInputYN(char *szPrompt);
extern int GiveAdvice(skilltype Skill);
extern int InternalCommandNext(int fMarkedMoves, int n);
extern int NextTurn(int fPlayNext);
extern int ParseKeyValue(char **ppch, char *apch[2]);
extern int ParseNumber(char **ppch);
extern int ParsePlayer(char *sz);
extern int ParsePosition(TanBoard an, char **ppch, char *pchDesc);
extern int SetToggle(char *szName, int *pf, char *sz, char *szOn,
		     char *szOff);
extern moverecord *getCurrentMoveRecord(int *pfHistory);
extern moverecord *LinkToDouble(moverecord * pmr);
extern moverecord *NewMoveRecord(void);
extern RETSIGTYPE HandleInterrupt(int idSignal);
extern void AddGame(moverecord * pmr);
extern void AddMoveRecord(void *pmr);
extern void ApplyMoveRecord(matchstate * pms, const listOLD * plGame,
			    const moverecord * pmr);
extern void CalculateBoard(void);
extern void CancelCubeAction(void);
extern void ChangeGame(listOLD * plGameNew);
extern void ClearMatch(void);
extern void ClearMoveRecord(void);
extern void DisectPath(const char *path, const char *extension, char **name,
		       char **folder);
extern void FixMatchState(matchstate * pms, const moverecord * pmr);
extern void FreeMatch(void);
extern void GetMatchStateCubeInfo(cubeinfo * pci, const matchstate * pms);
extern void HandleCommand(char *sz, command * ac);
extern void InitBoard(TanBoard anBoard, const bgvariation bgv);
extern void InvalidateStoredCube(void);
extern void InvalidateStoredMoves(void);
extern void PortableSignal(int nSignal, RETSIGTYPE(*p) (int),
			   psighandler * pOld, int fRestart);
extern void PortableSignalRestore(int nSignal, psighandler * p);
extern void PrintCheatRoll(const int fPlayer, const int n);
extern void ProgressEnd(void);
extern void ProgressStart(char *sz);
extern void ProgressStartValue(char *sz, int iMax);
extern void ProgressValueAdd(int iValue);
extern void ProgressValue(int iValue);
extern void PromptForExit(void);
extern void Prompt(void);
extern void ResetInterrupt(void);
extern void SaveRolloutSettings(FILE * pf, char *sz, rolloutcontext * prc);
extern void setDefaultFileName(char *sz);
extern void SetMatchDate(matchinfo * pmi);
extern void SetMatchID(const char *szMatchID);
extern void SetMatchInfo(char **ppch, char *sz, char *szMessage);
extern void SetMoveRecord(void *pmr);
extern void show_8912(TanBoard anBoard, char *sz);
extern void show_bearoff(TanBoard an, char *sz);
extern void ShowBoard(void);
extern void show_keith(TanBoard an, char *sz);
extern void show_kleinman(TanBoard an, char *sz);
extern void show_thorp(TanBoard an, char *sz);
extern void TextToClipboard(const char *sz);
extern void UpdateSettings(void);
extern void UpdateSetting(void *p);
extern void UpdateStoredCube(float aarOutput[2][NUM_ROLLOUT_OUTPUTS],
			     float aarStdDev[2][NUM_ROLLOUT_OUTPUTS],
			     const evalsetup * pes,
			     const matchstate * pms);
extern void UpdateStoredMoves(const movelist * pml,
			      const matchstate * pms);

extern void CommandAccept(char *);
extern void CommandAgree(char *);
extern void CommandAnalyseClearGame(char *);
extern void CommandAnalyseClearMatch(char *);
extern void CommandAnalyseClearMove(char *);
extern void CommandAnalyseGame(char *);
extern void CommandAnalyseMatch(char *);
extern void CommandAnalyseMove(char *);
extern void CommandAnalyseSession(char *);
extern void CommandAnnotateAccept(char *);
extern void CommandAnnotateAddComment(char *);
extern void CommandAnnotateBad(char *);
extern void CommandAnnotateClearComment(char *);
extern void CommandAnnotateClearLuck(char *);
extern void CommandAnnotateClearSkill(char *);
extern void CommandAnnotateCube(char *);
extern void CommandAnnotateDouble(char *);
extern void CommandAnnotateDoubtful(char *);
extern void CommandAnnotateDrop(char *);
extern void CommandAnnotateGood(char *);
extern void CommandAnnotateInteresting(char *);
extern void CommandAnnotateLucky(char *);
extern void CommandAnnotateMove(char *);
extern void CommandAnnotateReject(char *);
extern void CommandAnnotateResign(char *);
extern void CommandAnnotateUnlucky(char *);
extern void CommandAnnotateVeryBad(char *);
extern void CommandAnnotateVeryGood(char *);
extern void CommandAnnotateVeryLucky(char *);
extern void CommandAnnotateVeryUnlucky(char *);
extern void CommandCalibrate(char *sz);
extern void CommandClearHint(char *);
extern void CommandCopy(char *);
extern void CommandDecline(char *);
extern void CommandDiceRolls(char *);
extern void CommandDouble(char *);
extern void CommandDrop(char *);
extern void CommandEq2MWC(char *);
extern void CommandEval(char *);
extern void CommandExportGameEquityEvolution(char *);
extern void CommandExportGameGam(char *);
extern void CommandExportGameHtml(char *);
extern void CommandExportGameLaTeX(char *);
extern void CommandExportGamePDF(char *);
extern void CommandExportGameText(char *);
extern void CommandExportHTMLImages(char *);
extern void CommandExportMatchEquityEvolution(char *);
extern void CommandExportMatchHtml(char *);
extern void CommandExportMatchLaTeX(char *);
extern void CommandExportMatchMat(char *);
extern void CommandExportMatchPDF(char *);
extern void CommandExportMatchSGG(char *);
extern void CommandExportMatchText(char *);
extern void CommandExportMatchTMG(char *);
extern void CommandExportPositionEPS(char *);
extern void CommandExportPositionGammOnLine(char *);
extern void CommandExportPositionGOL2Clipboard(char *);
extern void CommandExportPositionHtml(char *);
extern void CommandExportPositionJF(char *);
extern void CommandExportPositionPDF(char *sz);
extern void CommandExportPositionPNG(char *);
extern void CommandExportPositionPS(char *sz);
extern void CommandExportPositionSnowieTxt(char *);
extern void CommandExportPositionSVG(char *sz);
extern void CommandExportPositionText(char *);
extern void CommandExternal(char *);
extern void CommandFirstGame(char *);
extern void CommandFirstMove(char *);
extern void CommandHelp(char *);
extern void CommandHint(char *);
extern void CommandHistory(char *);
extern void CommandImportAuto(char *);
extern void CommandImportBGRoom(char *);
extern void CommandImportBKG(char *);
extern void CommandImportEmpire(char *);
extern void CommandImportJF(char *);
extern void CommandImportMat(char *);
extern void CommandImportOldmoves(char *);
extern void CommandImportParty(char *);
extern void CommandImportSGG(char *);
extern void CommandImportSnowieTxt(char *);
extern void CommandImportTMG(char *);
extern void CommandListGame(char *);
extern void CommandListMatch(char *);
extern void CommandLoadCommands(char *);
extern void CommandLoadPython(char *);
extern void CommandMove(char *);
extern void CommandMWC2Eq(char *);
extern void CommandNewGame(char *);
extern void CommandNewMatch(char *);
extern void CommandNewSession(char *);
extern void CommandNewWeights(char *);
extern void CommandNext(char *);
extern void CommandNotImplemented(char *);
extern void CommandPlay(char *);
extern void CommandPrevious(char *);
extern void CommandQuit(char *);
extern void CommandRecordAddGame(char *);
extern void CommandRecordAddMatch(char *);
extern void CommandRecordAddSession(char *);
extern void CommandRecordEraseAll(char *);
extern void CommandRecordErase(char *);
extern void CommandRecordShow(char *);
extern void CommandRedouble(char *);
extern void CommandReject(char *);
extern void CommandRelationalAddMatch(char *);
extern void CommandRelationalEraseAll(char *);
extern void CommandRelationalErase(char *);
extern void CommandRelationalHelp(char *);
extern void CommandRelationalSelect(char *);
extern void CommandRelationalShowDetails(char *);
extern void CommandRelationalShowPlayers(char *);
extern void CommandRelationalTest(char *);
extern void CommandResign(char *);
extern void CommandRoll(char *);
extern void CommandRollout(char *);
extern void CommandSetAnalysisChequerplay(char *);
extern void CommandSetAnalysisCube(char *);
extern void CommandSetAnalysisCubedecision(char *);
extern void CommandSetAnalysisLimit(char *);
extern void CommandSetAnalysisLuckAnalysis(char *);
extern void CommandSetAnalysisLuck(char *);
extern void CommandSetAnalysisMoveFilter(char *);
extern void CommandSetAnalysisMoves(char *);
extern void CommandSetAnalysisPlayerAnalyse(char *);
extern void CommandSetAnalysisPlayer(char *);
extern void CommandSetAnalysisThresholdBad(char *);
extern void CommandSetAnalysisThresholdDoubtful(char *);
extern void CommandSetAnalysisThresholdGood(char *);
extern void CommandSetAnalysisThresholdInteresting(char *);
extern void CommandSetAnalysisThresholdLucky(char *);
extern void CommandSetAnalysisThresholdUnlucky(char *);
extern void CommandSetAnalysisThresholdVeryBad(char *);
extern void CommandSetAnalysisThresholdVeryGood(char *);
extern void CommandSetAnalysisThresholdVeryLucky(char *);
extern void CommandSetAnalysisThresholdVeryUnlucky(char *);
extern void CommandSetAnalysisWindows(char *);
extern void CommandSetAnnotation(char *);
extern void CommandSetAppearance(char *);
extern void CommandSetAutoAnalysis(char *);
extern void CommandSetAutoBearoff(char *);
extern void CommandSetAutoCrawford(char *);
extern void CommandSetAutoDoubles(char *);
extern void CommandSetAutoGame(char *);
extern void CommandSetAutoMove(char *);
extern void CommandSetAutoRoll(char *);
extern void CommandSetBeavers(char *);
extern void CommandSetBoard(char *);
extern void CommandSetBrowser(char *);
extern void CommandSetCache(char *);
extern void CommandSetCalibration(char *);
extern void CommandSetCheatEnable(char *);
extern void CommandSetCheatPlayer(char *);
extern void CommandSetCheatPlayerRoll(char *);
extern void CommandSetClockwise(char *);
extern void CommandSetCommandWindow(char *);
extern void CommandSetConfirmNew(char *);
extern void CommandSetConfirmSave(char *);
extern void CommandSetCrawford(char *);
extern void CommandSetCubeCentre(char *);
extern void CommandSetCubeEfficiencyContact(char *);
extern void CommandSetCubeEfficiencyCrashed(char *);
extern void CommandSetCubeEfficiencyOS(char *);
extern void CommandSetCubeEfficiencyRaceCoefficient(char *);
extern void CommandSetCubeEfficiencyRaceFactor(char *);
extern void CommandSetCubeEfficiencyRaceMax(char *);
extern void CommandSetCubeEfficiencyRaceMin(char *);
extern void CommandSetCubeOwner(char *);
extern void CommandSetCubeUse(char *);
extern void CommandSetCubeValue(char *);
extern void CommandSetDelay(char *);
extern void CommandSetDice(char *);
extern void CommandSetDisplay(char *);
extern void CommandSetDisplayPanels(char *);
extern void CommandSetDockPanels(char *);
extern void CommandSetEgyptian(char *);
extern void CommandSetEvalCandidates(char *);
extern void CommandSetEvalChequerplay(char *);
extern void CommandSetEvalCubedecision(char *);
extern void CommandSetEvalCubeful(char *);
extern void CommandSetEvalDeterministic(char *);
extern void CommandSetEvalMoveFilter(char *);
extern void CommandSetEvalNoise(char *);
extern void CommandSetEvalNoOnePlyPrune(char *);
extern void CommandSetEvalParamEvaluation(char *);
extern void CommandSetEvalParamRollout(char *);
extern void CommandSetEvalParamType(char *);
extern void CommandSetEvalPlies(char *);
extern void CommandSetEvalPrune(char *);
extern void CommandSetEvalReduced(char *);
extern void CommandSetEvalTolerance(char *);
extern void CommandSetEvaluation(char *);
extern void CommandSetExportCubeDisplayActual(char *);
extern void CommandSetExportCubeDisplayBad(char *);
extern void CommandSetExportCubeDisplayClose(char *);
extern void CommandSetExportCubeDisplayDoubtful(char *);
extern void CommandSetExportCubeDisplayGood(char *);
extern void CommandSetExportCubeDisplayInteresting(char *);
extern void CommandSetExportCubeDisplayMissed(char *);
extern void CommandSetExportCubeDisplayUnmarked(char *);
extern void CommandSetExportCubeDisplayVeryBad(char *);
extern void CommandSetExportCubeDisplayVeryGood(char *);
extern void CommandSetExportCubeParameters(char *);
extern void CommandSetExportCubeProb(char *);
extern void CommandSetExportFolder(char *sz);
extern void CommandSetExportFormat(char *sz);
extern void CommandSetExportHTMLCSSExternal(char *);
extern void CommandSetExportHTMLCSSHead(char *);
extern void CommandSetExportHTMLCSSInline(char *);
extern void CommandSetExportHTMLPictureURL(char *);
extern void CommandSetExportHtmlSize(char *);
extern void CommandSetExportHTMLTypeBBS(char *);
extern void CommandSetExportHTMLTypeFibs2html(char *);
extern void CommandSetExportHTMLTypeGNU(char *);
extern void CommandSetExportIncludeAnalysis(char *);
extern void CommandSetExportIncludeAnnotations(char *);
extern void CommandSetExportIncludeLegend(char *);
extern void CommandSetExportIncludeMatchInfo(char *);
extern void CommandSetExportIncludeStatistics(char *);
extern void CommandSetExportMovesDisplayBad(char *);
extern void CommandSetExportMovesDisplayDoubtful(char *);
extern void CommandSetExportMovesDisplayGood(char *);
extern void CommandSetExportMovesDisplayInteresting(char *);
extern void CommandSetExportMovesDisplayUnmarked(char *);
extern void CommandSetExportMovesDisplayVeryBad(char *);
extern void CommandSetExportMovesDisplayVeryGood(char *);
extern void CommandSetExportMovesNumber(char *);
extern void CommandSetExportMovesParameters(char *);
extern void CommandSetExportMovesProb(char *);
extern void CommandSetExportParametersEvaluation(char *);
extern void CommandSetExportParametersRollout(char *);
extern void CommandSetExportPNGSize(char *);
extern void CommandSetExportShowBoard(char *);
extern void CommandSetExportShowPlayer(char *);
extern void CommandSetFullScreen(char *);
extern void CommandSetGameList(char *);
extern void CommandSetGeometryAnalysis(char *);
extern void CommandSetGeometryCommand(char *);
extern void CommandSetGeometryGame(char *);
extern void CommandSetGeometryHeight(char *);
extern void CommandSetGeometryHint(char *);
extern void CommandSetGeometryMain(char *);
extern void CommandSetGeometryMax(char *);
extern void CommandSetGeometryMessage(char *);
extern void CommandSetGeometryPosX(char *);
extern void CommandSetGeometryPosY(char *);
extern void CommandSetGeometryTheory(char *);
extern void CommandSetGeometryWidth(char *);
extern void CommandSetGotoFirstGame(char *);
extern void CommandSetGUIAnimationBlink(char *);
extern void CommandSetGUIAnimationNone(char *);
extern void CommandSetGUIAnimationSlide(char *);
extern void CommandSetGUIAnimationSpeed(char *);
extern void CommandSetGUIBeep(char *);
extern void CommandSetGUIDiceArea(char *);
extern void CommandSetGUIDragTargetHelp(char *);
extern void CommandSetGUIHighDieFirst(char *);
extern void CommandSetGUIIllegal(char *);
extern void CommandSetGUIMoveListDetail(char *);
extern void CommandSetGUIShowEPCs(char *);
extern void CommandSetGUIShowIDs(char *);
extern void CommandSetGUIShowPips(char *);
extern void CommandSetGUIUseStatsPanel(char *);
extern void CommandSetGUIWindowPositions(char *);
extern void CommandSetImportFolder(char *sz);
extern void CommandSetImportFormat(char *sz);
extern void CommandSetInvertMatchEquityTable(char *);
extern void CommandSetJacoby(char *);
extern void CommandSetLang(char *);
extern void CommandSetMatchAnnotator(char *);
extern void CommandSetMatchComment(char *);
extern void CommandSetMatchDate(char *);
extern void CommandSetMatchEvent(char *);
extern void CommandSetMatchID(char *);
extern void CommandSetMatchLength(char *);
extern void CommandSetMatchPlace(char *);
extern void CommandSetMatchRating(char *);
extern void CommandSetMatchRound(char *);
extern void CommandSetMessage(char *);
extern void CommandSetMET(char *);
extern void CommandSetMoveFilter(char *);
extern void CommandSetOutputDigits(char *);
extern void CommandSetOutputErrorRateFactor(char *);
extern void CommandSetOutputMatchPC(char *);
extern void CommandSetOutputMWC(char *);
extern void CommandSetOutputRawboard(char *);
extern void CommandSetOutputWinPC(char *);
extern void CommandSetPanelWidth(char *);
extern void CommandSetPathBKG(char *);
extern void CommandSetPathEPS(char *);
extern void CommandSetPathGam(char *);
extern void CommandSetPathHTML(char *);
extern void CommandSetPathLaTeX(char *);
extern void CommandSetPathMat(char *);
extern void CommandSetPathMET(char *);
extern void CommandSetPathOldMoves(char *);
extern void CommandSetPathPDF(char *);
extern void CommandSetPathPNG(char *);
extern void CommandSetPathPos(char *);
extern void CommandSetPathSGF(char *);
extern void CommandSetPathSGG(char *);
extern void CommandSetPathSnowieTxt(char *);
extern void CommandSetPathText(char *);
extern void CommandSetPathTMG(char *);
extern void CommandSetPlayer(char *);
extern void CommandSetPlayerChequerplay(char *);
extern void CommandSetPlayerCubedecision(char *);
extern void CommandSetPlayerExternal(char *);
extern void CommandSetPlayerGNU(char *);
extern void CommandSetPlayerHuman(char *);
extern void CommandSetPlayerMoveFilter(char *);
extern void CommandSetPlayerName(char *);
extern void CommandSetPlayerPlies(char *);
extern void CommandSetPlayerPubeval(char *);
extern void CommandSetPostCrawford(char *);
extern void CommandSetPriorityAboveNormal(char *);
extern void CommandSetPriorityBelowNormal(char *);
extern void CommandSetPriorityHighest(char *);
extern void CommandSetPriorityIdle(char *);
extern void CommandSetPriorityNice(char *);
extern void CommandSetPriorityNormal(char *);
extern void CommandSetPriorityTimeCritical(char *);
extern void CommandSetPrompt(char *);
extern void CommandSetRatingOffset(char *);
extern void CommandSetRecord(char *);
extern void CommandSetRNGAnsi(char *);
extern void CommandSetRNGBBS(char *);
extern void CommandSetRNGBsd(char *);
extern void CommandSetRNG(char *);
extern void CommandSetRNGFile(char *);
extern void CommandSetRNGIsaac(char *);
extern void CommandSetRNGManual(char *);
extern void CommandSetRNGMD5(char *);
extern void CommandSetRNGMersenne(char *);
extern void CommandSetRNGRandomDotOrg(char *);
extern void CommandSetRNGUser(char *);
extern void CommandSetRolloutBearoffTruncationExact(char *);
extern void CommandSetRolloutBearoffTruncationOS(char *);
extern void CommandSetRollout(char *);
extern void CommandSetRolloutChequerplay(char *);
extern void CommandSetRolloutCubedecision(char *);
extern void CommandSetRolloutCubeEqualChequer(char *);
extern void CommandSetRolloutCubeful(char *);
extern void CommandSetRolloutInitial(char *);
extern void CommandSetRolloutJsd(char *);
extern void CommandSetRolloutJsdEnable(char *);
extern void CommandSetRolloutJsdLimit(char *);
extern void CommandSetRolloutJsdMinGames(char *);
extern void CommandSetRolloutJsdMoveEnable(char *);
extern void CommandSetRolloutLate(char *);
extern void CommandSetRolloutLateChequerplay(char *);
extern void CommandSetRolloutLateCubedecision(char *);
extern void CommandSetRolloutLateEnable(char *);
extern void CommandSetRolloutLateMoveFilter(char *);
extern void CommandSetRolloutLatePlayer(char *);
extern void CommandSetRolloutLatePlies(char *);
extern void CommandSetRolloutLimit(char *);
extern void CommandSetRolloutLimitEnable(char *);
extern void CommandSetRolloutLimitMinGames(char *);
extern void CommandSetRolloutLogEnable(char *);
extern void CommandSetRolloutLogFile(char *);
extern void CommandSetRolloutMaxError(char *);
extern void CommandSetRolloutMoveFilter(char *);
extern void CommandSetRolloutPlayer(char *);
extern void CommandSetRolloutPlayerChequerplay(char *);
extern void CommandSetRolloutPlayerCubedecision(char *);
extern void CommandSetRolloutPlayerLateChequerplay(char *);
extern void CommandSetRolloutPlayerLateCubedecision(char *);
extern void CommandSetRolloutPlayerLateMoveFilter(char *);
extern void CommandSetRolloutPlayerMoveFilter(char *);
extern void CommandSetRolloutPlayersAreSame(char *);
extern void CommandSetRolloutRNG(char *);
extern void CommandSetRolloutRotate(char *);
extern void CommandSetRolloutSeed(char *);
extern void CommandSetRolloutTrials(char *);
extern void CommandSetRolloutTruncation(char *);
extern void CommandSetRolloutTruncationChequer(char *);
extern void CommandSetRolloutTruncationCube(char *);
extern void CommandSetRolloutTruncationEnable(char *);
extern void CommandSetRolloutTruncationEqualPlayer0(char *);
extern void CommandSetRolloutTruncationPlies(char *);
extern void CommandSetRolloutVarRedn(char *);
extern void CommandSetScore(char *);
extern void CommandSetSeed(char *);
extern void CommandSetSGFFolder(char *sz);
extern void CommandSetSoundEnable(char *);
extern void CommandSetSoundSoundAgree(char *);
extern void CommandSetSoundSoundAnalysisFinished(char *);
extern void CommandSetSoundSoundBotDance(char *);
extern void CommandSetSoundSoundBotWinGame(char *);
extern void CommandSetSoundSoundBotWinMatch(char *);
extern void CommandSetSoundSoundChequer(char *);
extern void CommandSetSoundSoundDouble(char *);
extern void CommandSetSoundSoundDrop(char *);
extern void CommandSetSoundSoundExit(char *);
extern void CommandSetSoundSoundHumanDance(char *);
extern void CommandSetSoundSoundHumanWinGame(char *);
extern void CommandSetSoundSoundHumanWinMatch(char *);
extern void CommandSetSoundSoundMove(char *);
extern void CommandSetSoundSoundRedouble(char *);
extern void CommandSetSoundSoundResign(char *);
extern void CommandSetSoundSoundRoll(char *);
extern void CommandSetSoundSoundStart(char *);
extern void CommandSetSoundSoundTake(char *);
extern void CommandSetSoundSystemCommand(char *);
extern void CommandSetStyledGameList(char *);
extern void CommandSetTheoryWindow(char *);
extern void CommandSetToolbar(char *);
extern void CommandSetThreads(char *);
extern void CommandSetTurn(char *);
extern void CommandSetTutorChequer(char *);
extern void CommandSetTutorCube(char *);
extern void CommandSetTutorEval(char *sz);
extern void CommandSetTutorMode(char *);
extern void CommandSetTutorSkillBad(char *sz);
extern void CommandSetTutorSkillDoubtful(char *sz);
extern void CommandSetTutorSkillVeryBad(char *sz);
extern void CommandSetVariation1ChequerHypergammon(char *sz);
extern void CommandSetVariation2ChequerHypergammon(char *sz);
extern void CommandSetVariation3ChequerHypergammon(char *sz);
extern void CommandSetVariationNackgammon(char *sz);
extern void CommandSetVariationStandard(char *sz);
extern void CommandSetVsync3d(char * sz);
extern void CommandSetWarning(char *);
extern void CommandShow8912(char *);
extern void CommandShowAnalysis(char *);
extern void CommandShowAutomatic(char *);
extern void CommandShowBearoff(char *);
extern void CommandShowBeavers(char *);
extern void CommandShowBoard(char *);
extern void CommandShowBrowser(char *);
extern void CommandShowBuildInfo(char *);
extern void CommandShowCache(char *);
extern void CommandShowCalibration(char *);
extern void CommandShowCheat(char *);
extern void CommandShowClockwise(char *);
extern void CommandShowCommands(char *);
extern void CommandShowConfirm(char *);
extern void CommandShowCopying(char *);
extern void CommandShowCrawford(char *);
extern void CommandShowCredits(char *);
extern void CommandShowCube(char *);
extern void CommandShowCubeEfficiency(char *);
extern void CommandShowDelay(char *);
extern void CommandShowDice(char *);
extern void CommandShowDisplay(char *);
extern void CommandShowDisplayPanels(char *);
extern void CommandShowEgyptian(char *);
extern void CommandShowEngine(char *);
extern void CommandShowEPC(char *);
extern void CommandShowEvaluation(char *);
extern void CommandShowExport(char *);
extern void CommandShowFullBoard(char *);
extern void CommandShowGammonValues(char *);
extern void CommandShowGeometry(char *);
extern void CommandShowJacoby(char *);
extern void CommandShowKeith(char *);
extern void CommandShowKleinman(char *);
extern void CommandShowLang(char *);
extern void CommandShowManualAbout(char *);
extern void CommandShowManualWeb(char *);
extern void CommandShowMarketWindow(char *);
extern void CommandShowMatchEquityTable(char *);
extern void CommandShowMatchInfo(char *);
extern void CommandShowMatchLength(char *);
extern void CommandShowMatchResult(char *);
extern void CommandShowOneChequer(char *);
extern void CommandShowOneSidedRollout(char *);
extern void CommandShowOutput(char *);
extern void CommandShowPath(char *);
extern void CommandShowPipCount(char *);
extern void CommandShowPlayer(char *);
extern void CommandShowPostCrawford(char *);
extern void CommandShowPrompt(char *);
extern void CommandShowRNG(char *);
extern void CommandShowRollout(char *);
extern void CommandShowRolls(char *);
extern void CommandShowScore(char *);
extern void CommandShowScoreSheet(char *);
extern void CommandShowSeed(char *);
extern void CommandShowSound(char *);
extern void CommandShowStatisticsGame(char *);
extern void CommandShowStatisticsMatch(char *);
extern void CommandShowStatisticsSession(char *);
extern void CommandShowTemperatureMap(char *);
extern void CommandShowThorp(char *);
extern void CommandShowThreads(char *);
extern void CommandShowTurn(char *);
extern void CommandShowTutor(char *);
extern void CommandShowVariation(char *);
extern void CommandShowVersion(char *);
extern void CommandShowWarning(char *);
extern void CommandShowWarranty(char *);
extern void CommandSwapPlayers(char *);
extern void CommandTake(char *);
#endif
