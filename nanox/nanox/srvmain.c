/*
 * Copyright (c) 1999, 2000, 2001, 2003 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.
 * Copyright (c) 1991 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Main module of graphics server.
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>

#define MWINCLUDECOLORS
#include "serv.h"

#ifndef _EMUL_WIN
#include <unistd.h>
#include <sys/time.h>
#endif
/*
 * External definitions defined here.
 */
int scrNumberOfLayers = 1;	/* the number of layers to be used */

GR_WINDOW_ID	cachewindowid;		/* cached window id */
GR_WINDOW_ID    cachepixmapid;         /* cached pixmap id */
GR_GC_ID	cachegcid;		/* cached graphics context id */
GR_WINDOW	*cachewp;		/* cached window pointer */
GR_GC		*cachegcp;		/* cached graphics context */
GR_PIXMAP       *cachepp;               /* cached pixmap */
GR_PIXMAP       *listpp;                /* List of all pixmaps */

/* T.J.Park : Support multiple OSD layers. Make the window tree for each OSD layer. */
GR_WINDOW	*listwp[MAX_OSD_LAYER];		/* list of all windows */
GR_WINDOW	*rootwp[MAX_OSD_LAYER];		/* root window pointer */

GR_GC		*listgcp;		/* list of all gc */
GR_REGION	*listregionp;		/* list of all regions */
GR_FONT		*listfontp;		/* list of all fonts */
#if MW_FEATURE_IMAGES
GR_IMAGE	*listimagep;		/* list of all images */
#endif
GR_CURSOR	*listcursorp;		/* list of all cursors */
GR_CURSOR	*stdcursor;		/* root window cursor */
GR_GC		*curgcp;		/* currently enabled gc */
GR_WINDOW	*clipwp;		/* window clipping is set for */
GR_WINDOW	*focuswp;		/* focus window for keyboard */
GR_WINDOW	*mousewp;		/* window mouse is currently in */
GR_WINDOW	*grabbuttonwp;		/* window grabbed by button */
GR_CURSOR	*curcursor;		/* currently enabled cursor */
GR_COORD	cursorx;		/* current x position of cursor */
GR_COORD	cursory;		/* current y position of cursor */
GR_BUTTON	curbuttons;		/* current state of buttons */
GR_CLIENT	*curclient;		/* client currently executing for */
GR_EVENT_LIST	*eventfree;		/* list of free events */
GR_BOOL		focusfixed;		/* TRUE if focus is fixed on a window */
PMWFONT		stdfont;		/* default font*/
int		escape_quits = TRUE;	/* terminate when pressing ESC */
char		*progname;		/* Name of this program.. */

int		current_fd;		/* the fd of the client talking to */
int		connectcount = 0;	/* number of connections to server */
GR_CLIENT	*root_client;		/* root entry of the client table */
GR_CLIENT	*current_client;	/* the client we are currently talking*/
char		*current_shm_cmds;
int		current_shm_cmds_size;
int		keyb_fd;		/* the keyboard file descriptor */
int		mouse_fd;		/* the mouse file descriptor */


char		*curfunc;		/* the name of the current server func*/
GR_BOOL		screensaver_active;	/* time before screensaver activates */
GR_SELECTIONOWNER selection_owner;	/* the selection owner and typelist */
GR_TIMEOUT	startTicks;		/* ms time server started*/
int		autoportrait = FALSE;	/* auto portrait mode switching*/
MWCOORD		nxres;			/* requested server x resolution*/
MWCOORD		nyres;			/* requested server y resolution*/

#if MW_FEATURE_TIMERS
GR_TIMEOUT	screensaver_delay;	/* time before screensaver activates */
GR_TIMER_ID     cache_timer_id;         /* cached timer ID */
GR_TIMER        *cache_timer;           /* cached timer */
GR_TIMER        *list_timer;            /* list of all timers */
#endif /* MW_FEATURE_TIMERS */
GR_GRABBED_KEY  *list_grabbed_keys = NULL;     /* list of all grabbed keys */

static int	persistent_mode = FALSE;
static int	portraitmode = MWPORTRAIT_NONE;

SERVER_LOCK_DECLARE /* Mutex for all public functions (only if NONETWORK and THREADSAFE) */

#if !NONETWORK
int		un_sock;		/* the server socket descriptor */

static void
usage(void)
{
	printf("Usage: %s [-e] [-p] [-A] [-NLRD] [-x#] [-y#] [-c <fontconfig-file> ...]\n",
		progname);
	exit(1);
}

/*
 * This is the main server loop which initialises the server, services
 * the clients, and shuts the server down when there are no more clients.
 */
int
main(int argc, char *argv[])
{
	int t;

	progname = argv[0];

	t = 1;
	while ( t < argc ) {
		if ( !strcmp("-e",argv[t])) {
			escape_quits = FALSE;
			++t;
			continue;
		}
		if ( !strcmp("-p",argv[t]) ) {
			persistent_mode = TRUE;
			++t;
			continue;
		}
		if ( !strcmp("-A",argv[t]) ) {
			autoportrait = TRUE;
			++t;
			continue;
		}
		if ( !strcmp("-N",argv[t]) ) {
			portraitmode = MWPORTRAIT_NONE;
			++t;
			continue;
		}
		if ( !strcmp("-L",argv[t]) ) {
			portraitmode = MWPORTRAIT_LEFT;
			++t;
			continue;
		}
		if ( !strcmp("-R",argv[t]) ) {
			portraitmode = MWPORTRAIT_RIGHT;
			++t;
			continue;
		}
		if ( !strcmp("-D",argv[t]) ) {
			portraitmode = MWPORTRAIT_DOWN;
			++t;
			continue;
		}
		if ( !strcmp("-x",argv[t]) ) {
			if (++t >= argc)
				usage();
			nxres = atoi(argv[t]);
			++t;
			continue;
		}
		if ( !strcmp("-y",argv[t]) ) {
			if (++t >= argc)
				usage();
			nyres = atoi(argv[t]);
			++t;
			continue;
		}
#if FONTMAPPER
		if ( !strcmp("-c",argv[t]) ) {
			int read_configfile(char *file);

			if (++t >= argc)
				usage();
			read_configfile(argv[t]);
			++t;
			continue;
		}
#endif
		usage();
	}

	/* Attempt to initialise the server*/
	if(GsInitialize() < 0)
		exit(1);

	while(1)
		GsSelect(0L);
	return 0;
}
#endif

void
GsAcceptClientFd(int i)
{
	GR_CLIENT *client, *cl;

	if(!(client = NANOX_MALLOC(sizeof(GR_CLIENT)))) {
		close(i);
		return;
	}

	client->id = i;
	client->eventhead = NULL;
	client->eventtail = NULL;
	/*client->errorevent.type = GR_EVENT_TYPE_NONE;*/
	client->next = NULL;
	client->prev = NULL;
	client->waiting_for_event = FALSE;
	client->shm_cmds = 0;

	if(connectcount++ == 0)
		root_client = client;
	else {
		cl = root_client;
			while(cl->next)
				cl = cl->next;
		client->prev = cl;
		cl->next = client;
	}
}

void GrSetNumOfLayers(int numOfLayers)
{
	scrNumberOfLayers = numOfLayers;
	//printf("GrSetNumOfLayers : scrNumberOfLayers=%d\n", numOfLayers);
}

/*
 * Open a connection from a new client to the server.
 * Returns -1 on failure.
 */
#if (USE_OSADAP > 0)
MWMUTEX gr_server_mutex = 0;
#endif
int
GrOpen(void)
{
#if NONETWORK
	#if (USE_OSADAP > 0)
	gr_server_mutex = OSA_CreateMtxSem("nanoxMtx", SMF_RECURSIVE);
	/* If needed, initialize the server mutex. */
	#endif
	SERVER_LOCK();
	escape_quits = 1;

	/* Client calls this routine once.  We
	 * init everything here
	 */
	if (connectcount <= 0) {
		if(GsInitialize() < 0) {
			SERVER_UNLOCK();
			return -1;
		}
		GsAcceptClientFd(999);
		curclient = root_client;
	}

	SERVER_UNLOCK();
#endif
        return 1;
}

/*
 * Close the current connection to the server.
 */
void
GrClose(void)
{
	SERVER_LOCK();
	GsClose(current_fd);
	SERVER_UNLOCK();
}

/*
 * Drop a specific server connection.
 */
void
GsClose(int fd)
{
	GsDropClient(fd);
	if(!persistent_mode && connectcount == 0)
		GsTerminate();
}

#if NONETWORK
/* client/server GsDropClient is in srvnet.c*/
void
GsDropClient(int fd)
{
	--connectcount;
}
#endif

#if 0
#if UNIX | DOS_DJGPP
#if NONETWORK && defined(HAVESELECT)
/*
 * Register the specified file descriptor to return an event
 * when input is ready.
 */

static int regfdmax = -1;
static fd_set regfdset;

void
GrRegisterInput(int fd)
{
	SERVER_LOCK();
	FD_SET(fd, &regfdset);
	if (fd >= regfdmax) regfdmax = fd + 1;
	SERVER_UNLOCK();
}

void
GrUnregisterInput(int fd)
{
	int i, max;

	SERVER_LOCK();

	/* unregister all inputs if the FD is -1 */
	if (fd == -1) {
		FD_ZERO(&regfdset);
		regfdmax = -1;
		SERVER_UNLOCK();
		return;
	}

	FD_CLR(fd, &regfdset);
	/* recalculate the max file descriptor */
	for (i = 0, max = regfdmax, regfdmax = -1; i < max; i++)
		if (FD_ISSET(i, &regfdset))
			regfdmax = i + 1;

	SERVER_UNLOCK();
}

#endif /* NONETWORK && HAVESELECT */
#endif /* UNIX | DOS_DJGPP*/
#endif

/*
 * This function suspends execution of the program for the specified
 * number of milliseconds.
 */
void
GrDelay(GR_TIMEOUT msecs)
{
#if 0
#if UNIX
	struct timeval timeval;

	timeval.tv_sec = msecs / 1000;
	timeval.tv_usec = (msecs % 1000) * 1000;
	select(0, NULL, NULL, NULL, &timeval);
#endif
#endif
}

#if NONETWORK
void
GrFlush(void)
{
}

void
GrMainLoop(GR_FNCALLBACKEVENT fncb)
{
	GR_EVENT event;

	for(;;) {
		GrGetNextEvent(&event);
		fncb(&event);
	}
}

void
GrReqShmCmds(long shmsize)
{
	/* no action required, no client/server*/
}
#endif

#if 1
//#if MSDOS | _MINIX | WIN32 | VXWORKS | LINUX

void
GsSelect(GR_TIMEOUT timeout)
{
	/* If mouse data present, service it*/
	if(mousedev.Poll())
		while(GsCheckMouseEvent())
			continue;

	/* If keyboard data present, service it*/
	if(kbddev.Poll())
		while(GsCheckKeyboardEvent())
			continue;

}

#elif UNIX && defined(HAVESELECT)

void
GsSelect(GR_TIMEOUT timeout)
{
	fd_set	rfds;
	int 	e;
	int	setsize = 0;
	struct timeval tout;
	struct timeval *to;
#if NONETWORK
	int	fd;
#endif

	/* perform pre-select duties, if any*/
	if(rootwp->psd->PreSelect)
		rootwp->psd->PreSelect(rootwp->psd);

	/* Set up the FDs for use in the main select(): */
	FD_ZERO(&rfds);
	if(mouse_fd >= 0) {
		FD_SET(mouse_fd, &rfds);
		if (mouse_fd > setsize)
			setsize = mouse_fd;
	}
	if(keyb_fd >= 0) {
		FD_SET(keyb_fd, &rfds);
		if (keyb_fd > setsize)
			setsize = keyb_fd;
	}
#if NONETWORK
	/* handle registered input file descriptors*/
	for (fd = 0; fd < regfdmax; fd++) {
		if (!FD_ISSET(fd, &regfdset))
			continue;

		FD_SET(fd, &rfds);
		if (fd > setsize) setsize = fd;
	}
#else /* not NONETWORK */
	/* handle client socket connections*/
	FD_SET(un_sock, &rfds);
	if (un_sock > setsize) setsize = un_sock;
	curclient = root_client;
	while(curclient) {
		if(curclient->waiting_for_event && curclient->eventhead) {
			curclient->waiting_for_event = FALSE;
			GrGetNextEventWrapperFinish(curclient->id);
			return;
		}
		FD_SET(curclient->id, &rfds);
		if(curclient->id > setsize) setsize = curclient->id;
		curclient = curclient->next;
	}
#endif /* NONETWORK */
	/* Set up the timeout for the main select(): */
	if (timeout == (GR_TIMEOUT) -1L) {
		/* poll*/
		tout.tv_sec = 0;
		tout.tv_usec = 0;
		to = &tout;
	} else {
#if MW_FEATURE_TIMERS
		if(GdGetNextTimeout(&tout, timeout) == TRUE)
			to = &tout;
		else
#endif /* MW_FEATURE_TIMERS */
			to = NULL;
	}

	/* Wait for some input on any of the fds in the set or a timeout: */
	if((e = select(setsize+1, &rfds, NULL, NULL, to)) > 0) {
		/* If data is present on the mouse fd, service it: */
		if(mouse_fd >= 0 && FD_ISSET(mouse_fd, &rfds))
			while(GsCheckMouseEvent())
				continue;

		/* If data is present on the keyboard fd, service it: */
		if(keyb_fd >= 0 && FD_ISSET(keyb_fd, &rfds))
			while(GsCheckKeyboardEvent())
				continue;

#if NONETWORK
		/* check for input on registered file descriptors */
		for (fd = 0; fd < regfdmax; fd++) {
			GR_EVENT_FDINPUT *	gp;

			if (!FD_ISSET(fd, &regfdset)  ||  !FD_ISSET(fd, &rfds))
				continue;

			gp =(GR_EVENT_FDINPUT *)GsAllocEvent(curclient);
			if(gp) {
				gp->type = GR_EVENT_TYPE_FDINPUT;
				gp->fd = fd;
			}
		}
#else /* not NONETWORK */

		/* If a client is trying to connect, accept it: */
		if(FD_ISSET(un_sock, &rfds))
			GsAcceptClient();

		/* If a client is sending us a command, handle it: */
		curclient = root_client;
		while(curclient) {
			GR_CLIENT *curclient_next;

			/* curclient may be freed in GsDropClient*/
			curclient_next = curclient->next;
			if(FD_ISSET(curclient->id, &rfds))
				GsHandleClient(curclient->id);
			curclient = curclient_next;
		}
#endif /* NONETWORK */
	}
	else if (e == 0) {
#if NONETWORK
		/*
		 * Timeout has occured.  Currently return
		 * a timeout event regardless of whether
		 * client has selected for it.
		 * Note: this will be changed back to GR_EVENT_TYPE_NONE
		 * for the GrCheckNextEvent/LINK_APP_TO_SERVER case
		 */
#if MW_FEATURE_TIMERS
		if(GdTimeout() == TRUE)
#endif
		{
			GR_EVENT_GENERAL *	gp;
			gp = (GR_EVENT_GENERAL *)GsAllocEvent(curclient);
			if(gp)
				gp->type = GR_EVENT_TYPE_TIMEOUT;
		}
#else /* not NONETWORK */
#if MW_FEATURE_TIMERS
		GdTimeout();
#endif
#endif /* NONETWORK */
	} else
		if(errno != EINTR)
			EPRINTF("Select() call in main failed\n");
}

#if NONETWORK
/*
 * Prepare for the client to call select().  Asks the server to send the next
 * event but does not wait around for it to arrive.  Initializes the
 * specified fd_set structure with the client/server socket descriptor and any
 * previously registered external file descriptors.  Also compares the current
 * contents of maxfd, the client/server socket descriptor, and the previously
 * registered external file descriptors, and returns the highest of them in
 * maxfd.
 *
 * Usually used in conjunction with GrServiceSelect().
 *
 * Note that in a multithreaded client, the application must ensure that
 * no Nano-X calls are made between the calls to GrPrepareSelect() and
 * GrServiceSelect(), else there will be race conditions.
 *
 * @param maxfd  Pointer to a variable which the highest in use fd will be
 *               written to.  Must contain a valid value on input - will only
 *               be overwritten if the new value is higher than the old
 *               value.
 * @param rfdset Pointer to the file descriptor set structure to use.  Must
 *               be valid on input - file descriptors will be added to this
 *               set without clearing the previous contents.
 */
void
GrPrepareSelect(int *maxfd, void *rfdset)
{
	fd_set *rfds = (fd_set *) rfdset;
	int fd;

	SERVER_LOCK();

	/* perform pre-select duties, if any*/
	if(rootwp->psd->PreSelect)
		rootwp->psd->PreSelect(rootwp->psd);

	if(mouse_fd >= 0) {
		FD_SET(mouse_fd, rfds);
		if (mouse_fd > *maxfd)
			*maxfd = mouse_fd;
	}
	if(keyb_fd >= 0) {
		FD_SET(keyb_fd, rfds);
		if (keyb_fd > *maxfd)
			*maxfd = keyb_fd;
	}

	/* handle registered input file descriptors*/
	for (fd = 0; fd < regfdmax; fd++) {
		if (!FD_ISSET(fd, &regfdset))
			continue;

		FD_SET(fd, rfds);
		if (fd > *maxfd)
			*maxfd = fd;
	}

	SERVER_UNLOCK();
}

/*
 * Handles events after the client has done a select() call.
 *
 * Calls the specified callback function is an event has arrived, or if
 * there is data waiting on an external fd specified by GrRegisterInput().
 *
 * Used by GrMainLoop().
 *
 * @param rfdset Pointer to the file descriptor set containing those file
 *               descriptors that are ready for reading.
 * @param fncb   Pointer to the function to call when an event needs handling.
 */
void
GrServiceSelect(void *rfdset, GR_FNCALLBACKEVENT fncb)
{
	fd_set *	rfds = rfdset;
	GR_EVENT_LIST *	elp;
	GR_EVENT 	ev;
	int fd;

	SERVER_LOCK();

	/* If data is present on the mouse fd, service it: */
	if(mouse_fd >= 0 && FD_ISSET(mouse_fd, rfds))
		while(GsCheckMouseEvent())
			continue;

	/* If data is present on the keyboard fd, service it: */
	if(keyb_fd >= 0 && FD_ISSET(keyb_fd, rfds))
		while(GsCheckKeyboardEvent())
			continue;

	/* Dispatch all queued events */
	while((elp = curclient->eventhead) != NULL) {

		ev = elp->event;

		/* Remove first event from queue*/
		curclient->eventhead = elp->next;
		if (curclient->eventtail == elp)
			curclient->eventtail = NULL;

		elp->next = eventfree;
		eventfree = elp;

		fncb(&ev);
	}

	/* check for input on registered file descriptors */
	for (fd = 0; fd < regfdmax; fd++) {
		if (!FD_ISSET(fd, &regfdset) || !FD_ISSET(fd, rfds))
			continue;

		ev.type = GR_EVENT_TYPE_FDINPUT;
		ev.fdinput.fd = fd;
		fncb(&ev);
	}

	SERVER_UNLOCK();
}


#endif /* NONETWORK */

#endif /* UNIX && defined(HAVESELECT)*/

/*
 * Initialize the graphics and mouse devices at startup.
 * Returns nonzero with a message printed if the initialization failed.
 */
int
GsInitialize(void)
{
	GR_CURSOR_ID	cid;
	int layer;

	static MWIMAGEBITS cursorbits[16] = {
	      0xe000, 0x9800, 0x8600, 0x4180,
	      0x4060, 0x2018, 0x2004, 0x107c,
	      0x1020, 0x0910, 0x0988, 0x0544,
	      0x0522, 0x0211, 0x000a, 0x0004
	};
	static MWIMAGEBITS cursormask[16] = {
	      0xe000, 0xf800, 0xfe00, 0x7f80,
	      0x7fe0, 0x3ff8, 0x3ffc, 0x1ffc,
	      0x1fe0, 0x0ff0, 0x0ff8, 0x077c,
	      0x073e, 0x021f, 0x000e, 0x0004
	};

	#if (USE_OSADAP == 0)
	/* If needed, initialize the server mutex. */
	SERVER_LOCK_INIT();
	#endif

	//setbuf(stdout, NULL);	// removed 20070614 chaplin
	//setbuf(stderr, NULL);

	startTicks = GsGetTickCount();

#ifndef MW_NOSIGNALS
	/* catch terminate signal to restore tty state*/
	signal(SIGTERM, (void *)GsTerminate);
#endif

#if MW_FEATURE_TIMERS
	screensaver_delay = 0;
#endif
	screensaver_active = GR_FALSE;

	selection_owner.wid = 0;
	selection_owner.typelist = NULL;

#if !NONETWORK
#ifndef MW_NOSIGNALS
#ifndef _EMUL_WIN
	/* ignore pipe signal, sent when clients exit*/
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
#endif
#endif

	if (GsOpenSocket() < 0) {
		EPRINTF("Cannot bind to named socket\n");
#ifndef _EMUL_WIN
		NANOX_FREE(wp);
#endif
		return -1;
	}
#endif


	if ((keyb_fd = GdOpenKeyboard()) == -1) {
		EPRINTF("Cannot initialise keyboard\n");
		/*GsCloseSocket();*/
		for(layer = 0;layer < scrNumberOfLayers;layer++)
		{
			NANOX_FREE(rootwp[layer]);
		}
		return -1;
	}

	for(layer = 0;layer < scrNumberOfLayers;layer++)
	{
		if (GdOpenScreen(layer) == NULL) {
			EPRINTF("Cannot initialise screen\n");
			/*GsCloseSocket();*/
			GdCloseKeyboard();
			for(layer = 0;layer < scrNumberOfLayers;layer++)
			{
				NANOX_FREE(rootwp[layer]);
			}
			return -1;
		}
		GdSetPortraitMode(scrdev, portraitmode);
	}

	if ((mouse_fd = GdOpenMouse()) == -1) {
		EPRINTF("Cannot initialise mouse\n");
		/*GsCloseSocket();*/
		for(layer = 0;layer < scrNumberOfLayers;layer++)
			GdCloseScreen(scrdev + layer);
		GdCloseKeyboard();
		for(layer = 0;layer < scrNumberOfLayers;layer++)
		{
			NANOX_FREE(rootwp[layer]);
		}
		return -1;
	}

#if 0	// 20080417 stonekim : font_fontfusion.c, font_utf.c ·Î initºÎºÐ ¿Å±è.
#if HAVE_FONTFUSION_SUPPORT
	{
		extern int fontfusion_init(void); /* See font_fontfusion.c */
		if(!fontfusion_init())
			return -1;
	}
#endif
#if HAVE_UTF_SUPPORT
	{
		extern int UTF_Init(void); /* See font_utf.c */
		if(!UTF_Init())
			return -1;
	}
#endif

#if HAVE_UNITYPE_SUPPORT
	{
		extern int UNITYPE_Init(void); /* See font_unitype.c */
		if(!UNITYPE_Init())
			return -1;
	}
#endif
#endif
	/*
	 * Create std font.
	 */
#if 1 /* T.J.Park Note : We don't use any system font. */
	stdfont = 0;
#else
#if (HAVE_BIG5_SUPPORT | HAVE_GB2312_SUPPORT | HAVE_JISX0213_SUPPORT | HAVE_KSC5601_SUPPORT)
	/* system fixed font looks better when mixed with builtin fixed fonts*/
	stdfont = GdCreateFont(scrdev, MWFONT_SYSTEM_FIXED, 0, NULL);
#else
	stdfont = GdCreateFont(scrdev, MWFONT_SYSTEM_VAR, 0, NULL);
#endif
#endif

	/*
	 * Initialize the root window.
	 */
	for(layer = 0;layer < scrNumberOfLayers;layer++)
	{
		GR_WINDOW	*wp;		/* root window */

		wp = (GR_WINDOW *) NANOX_MALLOC(sizeof(GR_WINDOW));
		if (wp == NULL) {
			EPRINTF("Cannot allocate root window\n");
			return -1;
		}
		wp->psd = scrdev + layer;
		wp->id = layer + 1; /* GR_ROOT_WINDOW_ID, GR_ROOT_WINDOW2_ID, ... */
		wp->parent = NULL;	/* changed: was = NULL*/
		wp->owner = NULL;
		wp->children = NULL;
		wp->siblings = NULL;
		wp->next = NULL;
		wp->x = 0;
		wp->y = 0;
		wp->width = wp->psd->xvirtres;
		wp->height = wp->psd->yvirtres;
		wp->layer = layer;
		wp->bordersize = 0;
		wp->background = 0;
		wp->bordercolor = wp->background;
		wp->nopropmask = 0;
		wp->bgpixmap = NULL;
		wp->bgpixmapflags = GR_BACKGROUND_TILE;
		wp->eventclients = NULL;
		wp->cursorid = 0;
		wp->mapped = GR_TRUE;
		wp->realized = GR_TRUE;
		wp->output = GR_TRUE;
		wp->props = 0;
		wp->title = NULL;
		wp->clipregion = NULL;

		listwp[layer] = wp;
		rootwp[layer] = wp;
	}
	listpp = NULL;
	focuswp = rootwp[0];
	mousewp = rootwp[0];
	focusfixed = GR_FALSE;

	/*
	 * Initialize and position the default cursor.
	 */
	curcursor = NULL;
	cursorx = -1;
	cursory = -1;
	GdShowCursor(scrdev);
	GrMoveCursor(scrdev[0].xvirtres / 2, scrdev[0].yvirtres / 2);
	cid = GrNewCursor(16, 16, 0, 0, WHITE, BLACK, cursorbits, cursormask);
	GrSetWindowCursor(GR_ROOT_WINDOW_ID, cid);
	stdcursor = GsFindCursor(cid);

	/*for(layer = 0;layer < scrNumberOfLayers;layer++)
		scrdev[layer].FillRect(&(scrdev[layer]), 0, 0, scrdev[layer].xvirtres-1, scrdev[layer].yvirtres-1,
			GdFindColor(scrdev + layer, rootwp[layer]->background));
	*/ // by jjab
	/*
	 * Tell the mouse driver some things.
	 */
	curbuttons = 0;
	GdRestrictMouse(0, 0, scrdev[0].xvirtres - 1, scrdev[0].yvirtres - 1);
	GdMoveMouse(scrdev[0].xvirtres / 2, scrdev[0].yvirtres / 2);

	/* Force root window screen paint*/
	for(layer = 0;layer < scrNumberOfLayers;layer++)
		GsRedrawScreen(layer);

	/*
	 * Force the cursor to appear on the screen at startup.
	 * (not required with above GsRedrawScreen)
	GdHideCursor(psd);
	GdShowCursor(psd);
	 */

	/*
	 * All done.
	 */
	connectcount = 0;
	return 0;
}

/*
 * Here to close down the server.
 */
void
GsTerminate(void)
{
	int layer;

#if !NONETWORK
	GsCloseSocket();
#endif
	for(layer = 0;layer < scrNumberOfLayers;layer++)
		GdCloseScreen(rootwp[layer]->psd);

	GdCloseMouse();
	GdCloseKeyboard();

	exit(0);
}

/*
 * Return # milliseconds elapsed since start of Microwindows
 * Granularity is 25 msec
 */
GR_TIMEOUT
GsGetTickCount(void)
{
#if 0
#if MSDOS
#include <time.h>
	return (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
#else
#if _MINIX
	struct tms	t;

	return (unsigned long)times(&t) * 16;
#else
#if UNIX
	struct timeval t;

	gettimeofday(&t, NULL);
	return ((t.tv_sec * 1000) + (t.tv_usec / 25000) * 25) - startTicks;
#else
	return 0L;
#endif
#endif
#endif
#else
	return 0L;
#endif
}

void
GrBell(void)
{
	SERVER_LOCK();
	write(2, "\7", 1);
	SERVER_UNLOCK();
}


#if NONETWORK
/* T.J.Park Add : For better synchronization between tasks. */
void GrBeginPaint(void)
{
	SERVER_LOCK();
#ifndef WIN32
	NANOX_ScrLock();
#endif
}

void GrEndPaint(void)
{
#ifndef WIN32
	NANOX_ScrUnlock();
#endif
	SERVER_UNLOCK();

}

void GrServerLock(void)
{
	SERVER_LOCK();
}

void GrServerUnlock(void)
{
	SERVER_UNLOCK();
}
#endif
