/*
 * procunits.h
 *
 * by Olivier Baur <olivier.baur@noos.fr>, 2003
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
 * $Id: procunits.h,v 1.1.2.6 2003/07/09 18:55:03 hb Exp $
 */

#ifndef _PROCUNITS_H_
#define _PROCUNITS_H_ 1

#include <pthread.h>

#if WIN32
#include <windows.h>
#else
#include <netinet/in.h>
#endif  /* #if WIN32 */

#if USE_GTK
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif  /* #if USE_GTK */

#include "lib/hash.h"
#include "eval.h"
#include "rollout.h"

#include "threadglobals.h"

#define PU_DEBUG 0

#define RPU_MSG_VERSION 0x0100

/*  --------------------------------------------------
    processing unit task definition 
    --------------------------------------------------
*/

typedef enum { pu_task_rollout_bearoff, pu_task_rollout_basiccubeful }
pu_task_rollout_type;

typedef struct {
    int		nTruncate;
    int		nTrials;
    bgvariation	bgv;
} pu_task_rollout_bearoff_data;

typedef struct {
    int			cci;
    cubeinfo		*aci;
    int			*afCubeDecTop;
    rolloutstat		*aaarStatistics;
} pu_task_rollout_basiccubeful_data;

typedef struct {
    pu_task_rollout_type	type;		/* basic cubeful / bearoff */
    
    int				***aanBoardEval;
    float			**aar;
    
    union {
        pu_task_rollout_bearoff_data		bearoff;
        pu_task_rollout_basiccubeful_data 	basiccubeful;
    } specificdata;
    
    rolloutcontext		rc;
    int 			seed;
    int				iTurn;
    int				iGame;
} pu_task_rollout_data;

typedef struct {
    /* for future use */
} pu_task_eval_data;

typedef struct {
    
    
} pu_task_analysis_data;

typedef struct {
    int		masterId;			/* id of master host */
    int		taskId;				/* unique id assigned by master host */
    int		src_taskId;			/* id used on remote master hoster */
} pu_task_id;

typedef enum { pu_task_none = 0, pu_task_info = 1, pu_task_rollout = 2, 
                pu_task_eval = 4, pu_task_analysis = 8 } 
pu_task_type; /* use powers of 2 so types can be ORed in a mask */

typedef enum { pu_task_todo = 1, pu_task_inprogress, pu_task_done } 
pu_task_status;

typedef struct {
    pu_task_type	type;		/* eval / rollout */
    pu_task_status	status;		/* to do / in progress / done */
    pu_task_id		task_id;
    int			procunit_id;	/* -1 if handled by no one; use id instead of procunit* in case procunit removed from list */
    clock_t		timeCreated;
    clock_t		timeStarted;
    union _taskdata {
        pu_task_rollout_data	rollout;
        pu_task_eval_data	eval;
        pu_task_analysis_data	analysis;
    } taskdata;
} pu_task;


/*  --------------------------------------------------
    processing unit definition 
    --------------------------------------------------
*/

typedef struct {
    float	avgSpeed;
    int		total;
    int		totalDone;
    int		totalFailed;
} pu_stats;

typedef struct {
    pthread_t	thread;
} pu_info_local;

#define MAX_HOSTNAME 256
typedef struct {
    pthread_t		thread;
    int			fInterrupt;		/* interrupt all rpu processing */
    int			fStop;			/* stop/disconnect rpu; if set, set fInterrupt too */
    int			fTasksAvailable;	/* set if task(s) have been assigned to rpu */
    pthread_mutex_t 	mutexTasksAvailable;
    pthread_cond_t	condTasksAvailable;
    struct sockaddr_in	inAddress;
    char		hostName[MAX_HOSTNAME];
} pu_info_remote;

typedef union {
    pu_info_local	local;
    pu_info_remote	remote;
} pu_info;

typedef enum { pu_type_none = 0, pu_type_local, pu_type_remote } 
pu_type;

typedef enum { pu_stat_none = 0, pu_stat_na, pu_stat_ready, pu_stat_busy, pu_stat_deactivated,
                pu_stat_connecting } 
pu_status;

typedef struct _procunit {
   
    struct _procunit 	*next;		/* next in chained list */
    int			procunit_id;	/* starts from 1... */
    
    /* processing unit characteristics */
    pu_type		type;		/* local / remote */
    pu_status		status;		/* ready / busy */
    pu_info		info;		/* local thread or remote socket */
    pu_task_type	taskMask;	/* eg, pu_task_rollout+pu_task_eval */
    int			maxTasks;	/* 1 for local procunits */

    /* processing unit stats */
    pu_stats		rolloutStats;
    pu_stats		evalStats;
    pu_stats		analysisStats;

    pthread_mutex_t 	mutexStatusChanged;
    pthread_cond_t	condStatusChanged;
    
} procunit;

/*  --------------------------------------------------
    messages definitions 
    --------------------------------------------------
*/

typedef struct {
    int		len;		/* len of whole structure */
    pu_task_type type;		/* rollout, eval or analysis */
    char	data[0];
} rpu_jobtask;

typedef struct {		
    int		len;		/* len of whole structure */
    int		cJobTasks;
    rpu_jobtask	data[0];	/* concatenation of rpu_jobtask structs */
} rpu_job;

typedef struct {
    char	message[0];	/* greeting string */
} rpu_hello;

typedef struct {
} rpu_close;

typedef struct {
} rpu_getinfo;

typedef struct {
} rpu_hostinfo;

typedef struct {
    rpu_hostinfo hostInfo;
    int		cProcunits;
    procunit	procunitInfo[0];	/* not all fields have meaning (eg, next...) */
} rpu_info;

typedef enum { mmHello = 1, mmGetInfo, mmInfo, mmDoJob, mmTaskResult,
                mmNeuralNet, mmMET, mmClose } pu_message_type;

typedef struct {
    int			len;		/* len of whole structure */
    int			version;	/* for backward compatibility */
    pu_message_type	type;		/* getinfo, job, neuralnet, met, or close */
    union {
        rpu_hello	hello;
        rpu_close	close;
        rpu_getinfo	getinfo;
        rpu_info	info;
        rpu_job		dojob;
        rpu_jobtask	taskresult;
    }			data;
} rpu_message;

/*  --------------------------------------------------
    slave structures
    --------------------------------------------------
*/

typedef enum { pu_mode_master, pu_mode_slave } pu_mode;

typedef struct {
    int		rcvd;
    int		sent;
    int		done;
    int 	failed;
} rpu_stats;

typedef struct {
    rpu_stats	rollout;
    rpu_stats	eval;
    rpu_stats	analysis;
} rpu_slavestats;

/*  --------------------------------------------------
    shared globals 
    --------------------------------------------------
*/

extern pthread_mutex_t	mutexTaskListAccess;

extern pthread_mutex_t 	mutexResultsAvailable;
extern pthread_cond_t	condResultsAvailable;
extern int		gResultsAvailable;

extern pthread_mutex_t 	mutexCPUAccess;
    
extern rpu_slavestats 	gSlaveStats;


/*  --------------------------------------------------
    prototypes 
    --------------------------------------------------
*/

extern void InitProcessingUnits (void);
extern int IsMainThread (void);
extern pu_mode GetProcessingUnitsMode (void);
extern void PrintProcessingUnitList (void);

extern void InitTasks (void);
extern void PrintTaskList (void);

extern pu_task *CreateTask (pu_task_type type, int fDetached);
extern void MarkTaskDone (pu_task *pt, procunit *ppu);
extern void FreeTask (pu_task *pt);

extern void TaskEngine_Init (void);
extern void TaskEngine_Shutdown (void);
extern int TaskEngine_Full (void);
extern int TaskEngine_Empty (void);
extern pu_task * TaskEngine_GetCompletedTask (void);

extern void Slave_UpdateStatus (void);

extern void CommandProcunitsAddLocal( char *sz ) ;
extern void CommandProcunitsAddRemote( char *sz ) ;
extern void CommandProcunitsRemove ( char *sz ) ;
extern void CommandProcunitsSlave ( char *sz ) ;
extern void CommandProcunitsMaster ( char *sz ) ;
extern void CommandProcunitsStart ( char *sz ) ;
extern void CommandProcunitsStop ( char *sz ) ;

extern void CommandShowProcunitsInfo ( char *sz ) ;
extern void CommandShowProcunitsStats ( char *sz ) ;
extern void CommandShowProcunitsRemoteMask ( char *sz ) ;
extern void CommandShowProcunitsRemotePort ( char *sz ) ;
extern void CommandShowProcunitsRemoteNotifListenEnabled (char *sz);
extern void CommandShowProcunitsRemoteNotifListenPort (char *sz);
extern void CommandShowProcunitsRemoteNotifSendMethod (char *sz);
extern void CommandShowProcunitsRemoteNotifSendPort (char *sz);
extern void CommandShowProcunitsRemoteNotifSendDelay (char *sz);

extern void CommandSetProcunitsEnabled ( char *sz ) ;
extern void CommandSetProcunitsRemoteMask ( char *sz ) ;
extern void CommandSetProcunitsRemotePort ( char *sz ) ;
extern void CommandSetProcunitsRemoteMode ( char *sz ) ;
extern void CommandSetProcunitsRemoteQueue ( char *sz ) ;
extern void CommandSetProcunitsRemoteNotifListenEnabled ( char *sz );
extern void CommandSetProcunitsRemoteNotifListenPort ( char *sz );
extern void CommandSetProcunitsRemoteNotifSendMethodNone ( char *sz );
extern void CommandSetProcunitsRemoteNotifSendMethodBroadcast ( char *sz );
extern void CommandSetProcunitsRemoteNotifSendMethodHost ( char *sz );
extern void CommandSetProcunitsRemoteNotifSendPort ( char *sz );
extern void CommandSetProcunitsRemoteNotifSendDelay ( char *sz );

#if USE_GTK
void GTK_Procunit_Slave (gpointer *p, guint n, GtkWidget *pw);
void GTK_Procunit_Master (gpointer *p, guint n, GtkWidget *pw);
void GTK_Procunit_Options (gpointer *p, guint n, GtkWidget *pw);
#endif  /* #if USE_GTK */

#endif  /* #ifndef _PROCUNITS_H_ */