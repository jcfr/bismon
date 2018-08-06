                              // file web_ONIONBM.c
/***
    BISMON 
    Copyright © 2018 Basile Starynkevitch (working at CEA, LIST, France)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/
#include <onion/onion.h>
#include "bismon.h"
#include "web_ONIONBM.const.h"
#include "_login_ONIONBM.h"

#define WEBSESSION_EXPIRATION_DELAY 4000.0
extern void run_onionweb_BM (int nbjobs);
static onion *myonion_BM;
// the command pipe contains bytes, each considered as a different message
static int cmdpipe_rd_BM = -1, cmdpipe_wr_BM = -1;

extern void add_defer_command_onion_BM (void);

//////////////////////////////////////////////////////////////////////////
/// For process queue running processes; similar to gtkrunprocarr_BM
/// in newgui_GTKBM.c stuff is added into onionrunprocarr_BM &
/// onionrunpro_list_BM by any thread doing queue_process_BM. Stuff is
/// removed from them only by plain_event_loop_BM which would also
/// apply the closures.
struct onionproc_stBM
{
  pid_t rp_pid;
  int rp_outpipe;
  const stringval_tyBM *rp_dirstrv;
  const node_tyBM *rp_cmdnodv;
  const closure_tyBM *rp_closv;
  objectval_tyBM *rp_bufob;
} onionrunprocarr_BM[MAXNBWORKJOBS_BM];

/// queued process commands, of nodes (dir, cmd, clos); for processes
/// which are not yet in the array above...
struct listtop_stBM *onionrunpro_list_BM;

// lock for the structures above (both onionrunprocarr_BM & onionrunpro_list_BM)
pthread_mutex_t onionrunpro_mtx_BM = PTHREAD_MUTEX_INITIALIZER;

static volatile atomic_bool onionlooprunning_BM;

// the lock above should be set when calling:
static void
fork_onion_process_at_slot_BM (int slotpos,
                               const stringval_tyBM * dirstrarg,
                               const node_tyBM * cmdnodarg,
                               const closure_tyBM * endclosarg,
                               struct stackframe_stBM *stkf);
static void lockonion_runpro_mtx_at_BM (int lineno);
static void unlockonion_runpro_mtx_at_BM (int lineno);

static void plain_event_loop_BM (void);

// handle signals thu signafd(2)
static void read_sigterm_BM (int sigfd);
static void read_sigquit_BM (int sigfd);
static void read_sigchld_BM (int sigfd);
// handle the command pipe
static void read_commandpipe_BM (void);

static void register_onion_thread_stack_BM (struct stackframe_stBM *);
static void unregister_onion_thread_stack_BM (struct stackframe_stBM *);
static void perhaps_suspend_for_gc_onion_thread_stack_BM (struct
                                                          stackframe_stBM *);

/*** Our websession cookies are something like
     n<rank>r<rand1>t<rand2>o<oid> where <rank> is the websess_rank,
     <rand1> and <rand2> are random integers, <oid> is the websession
     object id
 ***/

#define BISMONION_WEBSESS_SUFLEN 40


void
lockonion_runpro_mtx_at_BM (int lineno __attribute__ ((unused)))
{
#if 0
  // too verbose, so not needed
  DBGPRINTFAT_BM (__FILE__, lineno, "lockonion_runpro_mtx_BM thrid=%ld",
                  (long) gettid_BM ());
#endif
  pthread_mutex_lock (&onionrunpro_mtx_BM);
}                               /* end lockonion_runpro_mtx_at_BM */


void
unlockonion_runpro_mtx_at_BM (int lineno __attribute__ ((unused)))
{
#if 0
  DBGPRINTFAT_BM (__FILE__, lineno, "unlockonion_runpro_mtx_BM thrid=%ld",
                  (long) gettid_BM ());
#endif
  pthread_mutex_unlock (&onionrunpro_mtx_BM);
}                               /* end lockonion_runpro_mtx_at_BM */

void
log_begin_message_BM (void)
{
  static long logcnt;
  logcnt++;
  double now = clocktime_BM (CLOCK_REALTIME);
  time_t nowt = (time_t) floor (now);
  double nowfrac = now - (double) nowt;
  ASSERT_BM (nowfrac >= 0.0 && nowfrac < 1.0);
  struct tm nowtm = {
  };
  localtime_r (&nowt, &nowtm);
  char nowtibuf[40];
  memset (nowtibuf, 0, sizeof (nowtibuf));
  strftime (nowtibuf, sizeof (nowtibuf), "%T %Z", &nowtm);
  char nowfracbuf[8];
  memset (nowfracbuf, 0, sizeof (nowfracbuf));
  snprintf (nowfracbuf, sizeof (nowfracbuf), "%.2f", nowfrac);
  char nowcntbuf[16];
  memset (nowcntbuf, 0, sizeof (nowcntbuf));
  snprintf (nowcntbuf, sizeof (nowcntbuf), " #%ld", logcnt);
  char logmbuf[80];
  memset (logmbuf, 0, sizeof (logmbuf));
  snprintf (logmbuf, sizeof (logmbuf), "%s%s%s", nowtibuf,
            nowfracbuf + 1, nowcntbuf);
  DBGBACKTRACEPRINTF_BM ("web log_begin_message_BM logmbuf %s", logmbuf);
  FATAL_BM ("log_begin_message_BM unimplemented in web_ONIONBM: %s", logmbuf);
#warning log_begin_message_BM unimplemented in web_ONIONBM
}                               /* end log_begin_message_BM */

void
log_end_message_BM (void)
{
  FATAL_BM ("log_end_message_BM unimplemented in web_ONIONBM");
#warning log_end_message_BM  unimplemented in web_ONIONBM
}                               /* end log_end_message_BM */

void
log_object_message_BM (const objectval_tyBM * obj)
{
  FATAL_BM ("log_object_message_BM unimplemented in web_ONIONBM obj %s",
            objectdbg_BM (obj));
#warning log_object_message_BM  unimplemented in web_ONIONBM
}                               /* end log_object_message_BM */

void
log_puts_message_BM (const char *msg)
{
  if (!msg || !msg[0])
    return;
  FATAL_BM ("log_puts_message_BM unimplemented in web_ONIONBM msg %s", msg);
#warning log_puts_message_BM  unimplemented in web_ONIONBM
}                               /* end log_puts_message_BM */

void
log_printf_message_BM (const char *fmt, ...)
{
  char smallbuf[64];
  memset (smallbuf, 0, sizeof (smallbuf));
  va_list args;
  char *buf = smallbuf;
  va_start (args, fmt);
  int ln = vsnprintf (smallbuf, sizeof (smallbuf), fmt, args);
  va_end (args);
  if (ln >= (int) sizeof (smallbuf) - 1)
    {
      buf = calloc (((prime_above_BM (ln / 4) | 7) + 2) * 4, 1);
      if (!buf)
        FATAL_BM ("failed to calloc for %d bytes (%m)", ln);
      va_start (args, fmt);
      vsnprintf (buf, ln + 1, fmt, args);
      va_end (args);
    }
  log_puts_message_BM (buf);
  if (buf != smallbuf)
    free (buf);
}                               /* end log_printf_message_BM */





// queue some external process; its stdin is /dev/null; both stdout &
// stderr are merged & captured; final string is given to the closure.
// dirstrv is the string of the directory to run it in (if NULL, use
// cwd) cmdnodv is a node with all sons being strings, for the command
// to run endclosv is the closure getting the status
// stringoutput, could fail
void
queue_process_BM (const stringval_tyBM * dirstrarg,
                  const node_tyBM * cmdnodarg,
                  const closure_tyBM * endclosarg,
                  struct stackframe_stBM *stkf)
{
  objectval_tyBM *k_queue_process = BMK_8DQ4VQ1FTfe_5oijDYr52Pb;
  objectval_tyBM *k_sbuf_object = BMK_77xbaw1emfK_1nhE4tp0bF3;
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ k_queue_process, //
                 const stringval_tyBM * dirstrv;        //
                 const node_tyBM * cmdnodv;     //
                 const closure_tyBM * endclosv; //
                 value_tyBM curargv;    //
                 value_tyBM errorv;     //
                 value_tyBM causev;     //
                 objectval_tyBM * bufob;        //
                 value_tyBM nodv;       //
    );
  _.dirstrv = dirstrarg;
  _.cmdnodv = cmdnodarg;
  _.endclosv = endclosarg;
  bool lockedproc = false;
  int failin = -1;
#define FAILHERE(Cause) do { failin = __LINE__ ;  _.causev = (value_tyBM)(Cause); goto failure; } while(0)
  if (_.dirstrv && !isstring_BM ((value_tyBM) _.dirstrv))
    FAILHERE (makenode1_BM (BMP_string, (value_tyBM) _.dirstrv));
  if (_.dirstrv && isstring_BM ((value_tyBM) _.dirstrv))
    {
      struct stat dirstat;
      int olderrno = errno;
      errno = 0;
      memset (&dirstat, 0, sizeof (dirstat));
      if (!stat (bytstring_BM (_.dirstrv), &dirstat)
          && (dirstat.st_mode & S_IFMT) != S_IFDIR)
        errno = ENOTDIR;
      int newerrno = errno;
      errno = olderrno;
      if (newerrno)
        FAILHERE (makenode2_BM
                  (BMP_node, (value_tyBM) _.dirstrv,
                   taggedint_BM (newerrno)));
    }
  if (!isnode_BM ((value_tyBM) _.cmdnodv))
    FAILHERE (makenode1_BM (BMP_node, (value_tyBM) _.cmdnodv));
  if (!isclosure_BM ((value_tyBM) _.endclosv))
    FAILHERE (makenode1_BM (BMP_closure, (value_tyBM) _.cmdnodv));
  unsigned cmdlen = nodewidth_BM ((value_tyBM) _.cmdnodv);
  if (cmdlen == 0)
    FAILHERE (makenode1_BM (BMP_node, (value_tyBM) _.cmdnodv));
  for (unsigned aix = 0; aix < cmdlen; aix++)
    {
      _.curargv = nodenthson_BM ((value_tyBM) _.cmdnodv, aix);
      if (!isstring_BM (_.curargv))
        FAILHERE (makenode2_BM
                  (BMP_node, (value_tyBM) _.cmdnodv, taggedint_BM (aix)));
    }
  ASSERT_BM (nbworkjobs_BM >= MINNBWORKJOBS_BM
             && nbworkjobs_BM <= MAXNBWORKJOBS_BM);
  {
    lockonion_runpro_mtx_at_BM (__LINE__);
    lockedproc = true;
    int slotpos = -1;
    for (int ix = 0; ix < nbworkjobs_BM; ix++)
      {
        if (onionrunprocarr_BM[ix].rp_pid == 0)
          {
            slotpos = ix;
            break;
          };
      }
    if (slotpos >= 0 && !onionrunpro_list_BM)
      {
        fork_onion_process_at_slot_BM (slotpos, _.dirstrv, _.cmdnodv,
                                       _.endclosv, CURFRAME_BM);
      }
    else
      {                         // append to onionrunpro_list_BM
        if (!onionrunpro_list_BM)
          onionrunpro_list_BM = makelist_BM ();
        _.nodv = (value_tyBM)
          makenode3_BM (k_queue_process, (value_tyBM) _.dirstrv,
                        (value_tyBM) _.cmdnodv, (value_tyBM) _.endclosv);
        listappend_BM (onionrunpro_list_BM, _.nodv);
      }
    ASSERT_BM (lockedproc);
    unlockonion_runpro_mtx_at_BM (__LINE__), lockedproc = false;
  }
  LOCALJUSTRETURN_BM ();
failure:
#undef FAILHERE
  if (lockedproc)
    unlockonion_runpro_mtx_at_BM (__LINE__), lockedproc = false;
  DBGPRINTF_BM
    ("queue_process failure failin %d dirstr %s, cmdnod %s endclos %s, cause %s",
     failin,
     bytstring_BM (_.dirstrv), debug_outstr_value_BM ((value_tyBM) _.cmdnodv,
                                                      CURFRAME_BM, 0),
     debug_outstr_value_BM ((value_tyBM) _.endclosv, CURFRAME_BM, 0),
     debug_outstr_value_BM ((value_tyBM) _.causev, CURFRAME_BM, 0));
  _.errorv = (value_tyBM) makenode4_BM (k_queue_process, (value_tyBM) _.dirstrv, (value_tyBM) _.cmdnodv,        //
                                        (value_tyBM) _.endclosv,
                                        (value_tyBM) _.causev);
  FAILURE_BM (failin, _.errorv, CURFRAME_BM);
}                               /* end queue_process_BM */



////////////////
static void
fork_onion_process_at_slot_BM (int slotpos,
                               const stringval_tyBM * dirstrarg,
                               const node_tyBM * cmdnodarg,
                               const closure_tyBM * endclosarg,
                               struct stackframe_stBM *stkf)
{
  objectval_tyBM *k_queue_process = BMK_8DQ4VQ1FTfe_5oijDYr52Pb;
  objectval_tyBM *k_sbuf_object = BMK_77xbaw1emfK_1nhE4tp0bF3;
  LOCALFRAME_BM ( /*prev: */ stkf,
                 /*descr: */ NULL,
                 const stringval_tyBM * dirstrv;        //
                 const node_tyBM * cmdnodv;     //
                 const closure_tyBM * endclosv; //
                 value_tyBM curargv;    //
                 objectval_tyBM * bufob;        //
    );
  _.dirstrv = dirstrarg;
  _.cmdnodv = cmdnodarg;
  _.endclosv = endclosarg;
  int cmdlen = nodewidth_BM ((value_tyBM) _.cmdnodv);
  ASSERT_BM (cmdlen > 0);
  ASSERT_BM (slotpos >= 0 && slotpos < MAXNBWORKJOBS_BM);
  DBGPRINTF_BM ("fork_onion_process_at_slot_BM slotpos %d cmdnod %s",
                slotpos,
                debug_outstr_value_BM ((value_tyBM) _.cmdnodv, CURFRAME_BM,
                                       0));
  /// should fork the process
  int pipfd[2] = { -1, -1 };
  char **args = calloc (cmdlen + 1, sizeof (char *));
  if (!args)
    FATAL_BM ("calloc args %d failed - %m", cmdlen);
  for (int aix = 0; aix < cmdlen; aix++)
    args[aix] = bytstring_BM (nodenthson_BM (_.cmdnodv, aix));
  if (pipe (pipfd))
    FATAL_BM ("pipe failed - %m");
  ASSERT_BM (pipfd[0] > 0 && pipfd[1] > 0);
  fflush (NULL);
  pid_t pid = fork ();
  if (pid < 0)
    FATAL_BM ("failed to fork %s - %m", args[0]);
  if (pid == 0)
    {
      {
        // child process; 
        sigset_t sigset = { };
        sigemptyset (&sigset);
        // restore default SIGTERM & SIGQUIT behavior
        signal (SIGTERM, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        sigaddset (&sigset, SIGTERM);
        sigaddset (&sigset, SIGQUIT);
        sigaddset (&sigset, SIGCHLD);
        sigprocmask (SIG_UNBLOCK, &sigset, NULL);
      }
      //in principle, most file descriptors should be
      //close-on-exec, but just in case we close some of them...
      for (int ix = 3; ix < 64; ix++)
        if (ix != pipfd[1])
          (void) close (ix);
      if (isstring_BM (_.dirstrv))
        {
          if (chdir (bytstring_BM (_.dirstrv)))
            {
              perror (bytstring_BM (_.dirstrv));
              exit (126);
            }
        }
      int fd = open ("/dev/null", O_RDONLY);
      dup2 (fd, STDIN_FILENO);
      close (fd), fd = -1;
      dup2 (pipfd[1], STDOUT_FILENO);
      dup2 (pipfd[1], STDERR_FILENO);
      execv (args[0], args);
      perror (args[0]);
      _exit (127);
    }
  else
    {                           // parent process
      struct onionproc_stBM *onproc = onionrunprocarr_BM + slotpos;
      ASSERT_BM (onproc->rp_pid <= 0 && onproc->rp_outpipe <= 0);
      onproc->rp_pid = pid;
      onproc->rp_outpipe = pipfd[0];
      fcntl (pipfd[0], F_SETFL, O_NONBLOCK);
      onproc->rp_cmdnodv = _.cmdnodv;
      onproc->rp_closv = _.endclosv;
      onproc->rp_dirstrv = _.dirstrv;
      _.bufob = makeobj_BM ();
      onproc->rp_bufob = _.bufob;
    }
}                               /* end fork_onion_process_at_slot_BM */


////////////////////////////////////////////////////////////////
static onion_connection_status
custom_onion_handler_BM (void *clientdata,
                         onion_request * req, onion_response * resp);
static onion_connection_status
login_onion_handler_BM (void *clientdata,
                        onion_request * req, onion_response * resp);
static onion_connection_status
do_forgot_onion_BM (char *username,
                    onion_request * req, onion_response * resp);

static onion_connection_status
do_login_redirect_onion_BM (objectval_tyBM * contribobarg,
                            const char *location, onion_request * req,
                            onion_response * resp,
                            struct stackframe_stBM *stkf);


static onion_connection_status
do_dynamic_onion_BM (objectval_tyBM * sessionobarg, const char *reqpath,
                     bool postrequest,
                     onion_request * req,
                     onion_response * resp, struct stackframe_stBM *stkf);

////////////////
void
run_onionweb_BM (int nbjobs)    // declared and used only in
                                // main_BM.c, called from main
                                // function
{
  char *webhost = NULL;
  int webport = 0;
  int pos = -1;
  int colonpos = -1;
  if (nbjobs < MINNBWORKJOBS_BM)
    nbjobs = MINNBWORKJOBS_BM;
  else if (nbjobs > MAXNBWORKJOBS_BM)
    nbjobs = MAXNBWORKJOBS_BM;
  if (!onion_web_base_BM)
    onion_web_base_BM = "localhost:8086";
  if (sscanf
      (onion_web_base_BM, "%m[.a-zA-Z0-9+-]%n:%d%n", &webhost, &colonpos,
       &webport, &pos) < 2 || pos < 0 || onion_web_base_BM[pos])
    FATAL_BM ("bad web base %s -host %s port %d", onion_web_base_BM,
              webhost ? : "??", webport);
  /// clear and initialize the dictionnary of web sessions, whose keys are cookies
  objputdictpayl_BM (BMP_the_web_sessions);
  myonion_BM =
    onion_new (O_THREADED | O_NO_SIGTERM | O_SYSTEMD | O_DETACH_LISTEN);
  if (!myonion_BM)
    FATAL_BM ("failed to create onion");
  onion_set_max_threads (myonion_BM, nbjobs);
  DBGPRINTF_BM ("run_onionweb webhost '%s' webport %d", webhost ? : "??",
                webport);
  if (webhost && webhost[0])
    {
      onion_set_hostname (myonion_BM, webhost);
      char *lastcolon = strrchr (onion_web_base_BM, ':');
      if (lastcolon && isdigit (lastcolon[1]))
        onion_set_port (myonion_BM, lastcolon + 1);
      char *webrootpath = NULL;
      if (asprintf (&webrootpath, "%s/webroot/", bismon_directory) < 0
          || !webrootpath || !webrootpath[0]
          || access (webrootpath, R_OK | X_OK))
        FATAL_BM ("failed to get or access webroot/ path - %m");
      onion_handler *filehdl = onion_handler_export_local_new (webrootpath);
      onion_handler *customhdl =
        onion_handler_new (custom_onion_handler_BM, NULL, NULL);
      if (!filehdl)
        FATAL_BM ("failed to get onion webroot handler for %s", webrootpath);
      onion_set_root_handler (myonion_BM, customhdl);
      DBGPRINTF_BM ("run_onionweb after set root handler filehdl@%p",
                    filehdl);
      onion_handler_add (customhdl, filehdl);
      ///
      /// create the command pipe
      {
        int piparr[2] = { -1, -1 };
        if (pipe2 (piparr, O_CLOEXEC | O_NONBLOCK))
          FATAL_BM ("run_onionweb pipe failure for the command pipe - %m");
        cmdpipe_rd_BM = piparr[0];
        cmdpipe_wr_BM = piparr[1];
        DBGPRINTF_BM
          ("run_onionweb before onion_listen cmdpiprd#%d cmdpipwr#%d",
           cmdpipe_rd_BM, cmdpipe_wr_BM);
      }
      int err = onion_listen (myonion_BM);      // since detached, returns now
      DBGPRINTF_BM ("run_onionweb after onion_listen err=%d", err);
      if (err)
        FATAL_BM ("failed to do onion_listen (err#%d / %s)", err,
                  strerror (err));
      ///
      /// should add our event loop, at least related to queued processes
      /// (and their output pipes), to SIGCHLD and SIGTERM + SIGQUIT
      /// see https://groups.google.com/a/coralbits.com/d/msg/onion-dev/m-wH-BY2MA0/QJqLNcHvAAAJ
      /// and https://groups.google.com/a/coralbits.com/d/msg/onion-dev/ImjNf1EIp68/R37DW3mZAAAJ
      plain_event_loop_BM ();
    }
}                               /* end run_onionweb_BM */

////////////////////////////////////////////////////////////////

////////////////
// GC support for websessiondata
void
websessiondatagcmark_BM (struct garbcoll_stBM *gc,
                         struct websessiondata_stBM *ws,
                         objectval_tyBM * fromob, int depth)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) ws) == typayl_websession_BM);
  ASSERT_BM (!fromob || isobject_BM (fromob));
  ASSERT_BM (ws->websess_magic == BISMONION_WEBSESS_MAGIC);
  ASSERT_BM (depth >= 0);
  ASSERT_BM (!fromob || !ws->websess_ownobj || ws->websess_ownobj == fromob);
  uint8_t oldmark = ((typedhead_tyBM *) ws)->hgc;
  if (oldmark)
    return;
  ((typedhead_tyBM *) ws)->hgc = MARKGC_BM;
  if (ws->websess_ownobj)
    gcobjmark_BM (gc, ws->websess_ownobj);
  if (ws->websess_contribob)
    gcobjmark_BM (gc, ws->websess_contribob);
  if (ws->websess_datav)
    EXTENDEDGCPROC_BM (gc, ws->websess_datav, fromob, depth + 1);
  gc->gc_nbmarks++;
}                               /* end websessiondatagcmark_BM */


void
websessiondatagcdestroy_BM (struct garbcoll_stBM *gc,
                            struct websessiondata_stBM *ws)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) ws) == typayl_websession_BM);
  ASSERT_BM (ws->websess_magic == BISMONION_WEBSESS_MAGIC);
  if (ws->websess_ownobj)
    {
      objectval_tyBM *ownerob = ws->websess_ownobj;
      ws->websess_ownobj = NULL;
      objlock_BM (ownerob);
      if (objpayload_BM (ownerob) == (extendedval_tyBM) ws)
        objclearpayload_BM (ownerob);
      objunlock_BM (ownerob);
    };
  if (ws->websess_websocket)
    {
      onion_websocket *websock = ws->websess_websocket;
      ws->websess_websocket = NULL;
      onion_websocket_free (websock);
    };
  memset ((void *) ws, 0, sizeof (*ws));
  free (ws);
  gc->gc_freedbytes += sizeof (*ws);
}                               /* end  websessiondatagcdestroy_BM */


void
websessiondatagckeep_BM (struct garbcoll_stBM *gc,
                         struct websessiondata_stBM *ws)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) ws) == typayl_websession_BM);
  ASSERT_BM (ws->websess_magic == BISMONION_WEBSESS_MAGIC);
  gc->gc_keptbytes += sizeof (*ws);
}                               /* end websessiondatagckeep_BM */



////////////////
// GC support for webexchangedata

void
webexchangedatagcmark_BM (struct garbcoll_stBM *gc,
                          struct webexchangedata_stBM *wex,
                          objectval_tyBM * fromob, int depth)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) wex) == typayl_webexchange_BM);
  ASSERT_BM (wex->webx_magic == BISMONION_WEBX_MAGIC);
  ASSERT_BM (!fromob || isobject_BM (fromob));
  ASSERT_BM (depth >= 0);
  ASSERT_BM (!fromob || !wex->webx_ownobj || wex->webx_ownobj == fromob);
  if (wex->webx_ownobj)
    gcobjmark_BM (gc, wex->webx_ownobj);
  if (wex->webx_sessobj)
    gcobjmark_BM (gc, wex->webx_sessobj);
  if (wex->webx_datav)
    EXTENDEDGCPROC_BM (gc, wex->webx_datav, fromob, depth + 1);
}                               /* end webexchangedatagcmark_BM */


void
webexchangedatagcdestroy_BM (struct garbcoll_stBM *gc,
                             struct webexchangedata_stBM *wex)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) wex) == typayl_webexchange_BM);
  ASSERT_BM (wex->webx_magic == BISMONION_WEBX_MAGIC);
  if (wex->webx_ownobj)
    {
      objectval_tyBM *ownerob = wex->webx_ownobj;
      wex->webx_ownobj = NULL;
      objlock_BM (ownerob);
      if (objpayload_BM (ownerob) == (extendedval_tyBM) wex)
        objclearpayload_BM (ownerob);
      objunlock_BM (ownerob);
    };
  if (wex->webx_requ)
    {
      onion_request *oreq = wex->webx_requ;
      wex->webx_requ = NULL;
      onion_request_free (oreq);
    };
  if (wex->webx_resp)
    {
      onion_response *oresp = wex->webx_resp;
      wex->webx_resp = NULL;
      onion_response_free (oresp);
    }
  memset ((void *) wex, 0, sizeof (*wex));
  free (wex);
  gc->gc_freedbytes += sizeof (*wex);
}                               /* end webexchangedatagcdestroy_BM */


void
webexchangedatagckeep_BM (struct garbcoll_stBM *gc,
                          struct webexchangedata_stBM *wex)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  ASSERT_BM (valtype_BM ((value_tyBM) wex) == typayl_webexchange_BM);
  ASSERT_BM (wex->webx_magic == BISMONION_WEBX_MAGIC);
  gc->gc_keptbytes += sizeof (*wex);
}                               /* end webexchangedatagckeep_BM */


/* delete functions are called by deleteobjectpayload_BM for
   objclearpayload_BM & objputpayload_BM */
void
websessiondelete_BM (objectval_tyBM * ownobj, struct websessiondata_stBM *ws)
{
#warning unimplemented websessiondelete_BM
  FATAL_BM ("unimplemented websessiondelete_BM ownobj %s ws@%p",
            objectdbg_BM (ownobj), (void *) ws);
}                               /* end websessiondelete_BM */

void
webexchangedelete_BM (objectval_tyBM * ownobj,
                      struct webexchangedata_stBM *wex)
{
#warning unimplemented webexchangedelete_BM
  FATAL_BM ("unimplemented webexchangedelete_BM ownobj %s wex@%p",
            objectdbg_BM (ownobj), (void *) wex);
}                               /* end webexchangedelete_BM */




onion_connection_status
custom_onion_handler_BM (void *clientdata,
                         onion_request * req, onion_response * resp)
{
  onion_connection_status result = OCS_NOT_PROCESSED;
  objectval_tyBM *k_custom_onion_handler = BMK_5C5Dfd8eVkR_3306NWk09Bn;
  objectval_tyBM *k_websession_dict_object = BMK_2HGGdFqLH2E_8HktHZxdBd8;
  LOCALFRAME_BM ( /*prev: */ NULL, /*descr: */ k_custom_onion_handler,
                 objectval_tyBM * sessionob;);
  while (agenda_need_gc_BM ())
    {
      DBGPRINTF_BM ("custom_onion_handler need GC");
      agenda_wait_gc_BM ();
    }
  const char *reqpath = onion_request_get_path (req);
  unsigned reqflags = onion_request_get_flags (req);
  unsigned reqmeth = (reqflags & OR_METHODS);
  const char *bcookie = onion_request_get_cookie (req, "BISMONCOOKIE");
  char dbgmethbuf[16];
  DBGPRINTF_BM ("custom_onion_handler reqpath '%s' reqflags %#x:%s bcookie %s", //
                reqpath, reqflags,      //
                ((reqmeth == OR_GET) ? "GET"    //
                 : (reqmeth == OR_HEAD) ? "HEAD"        //
                 : (reqmeth == OR_POST) ? "POST"        //
                 : (reqmeth == OR_OPTIONS) ? "OPTIONS"  //
                 : (reqmeth == OR_PROPFIND) ? "PROPFIND"        //
                 : snprintf (dbgmethbuf, sizeof (dbgmethbuf),   ///
                             "meth#%d", reqmeth)),
                bcookie ? bcookie : "*none*");
  if (!strcmp (reqpath, "_login") || !strcmp (reqpath, "/_login"))
    {
      DBGPRINTF_BM ("custom_onion_handler login page reqpath %s", reqpath);
      return login_onion_handler_BM (clientdata, req, resp);
    }
  // for some reason, I can't make the webroot/ work reliably with this.
  // so I special-case when the request corresponds to an existing file path
  // but for safety I don't do that for any URL containing /. or ..
  if (strlen (reqpath) > 2
      && !strstr (reqpath, "/.") && !strstr (reqpath, ".."))
    {
      char *fipath = NULL;
      if (asprintf (&fipath, "%s/webroot/%s", bismon_directory, reqpath) > 0
          && fipath && !access (fipath, R_OK))
        {
          DBGPRINTF_BM ("custom_onion_handler found fipath %s", fipath);
          free (fipath), fipath = NULL;
          return OCS_NOT_PROCESSED;

        }
    }
  register_onion_thread_stack_BM (CURFRAME_BM);
  bool goodcookie = false;
  {
    unsigned cookrank = 0;
    uint32_t cookrand1 = 0, cookrand2 = 0;
    int cookposoid = -1;
    rawid_tyBM cookoid = { 0, 0 };
    char *endcookie = NULL;
    int blencookie = bcookie ? strlen (bcookie) : 0;
    char oidbuf[32];
    if (blencookie > BISMONION_WEBSESS_SUFLEN / 2
        && sscanf (bcookie, "n%uR%ut%uo%n", &cookrank, &cookrand1,
                   &cookrand2, &cookposoid) >= 3 && cookposoid > 4
        && bcookie[cookposoid] == '_' && isdigit (bcookie[cookposoid + 1])
        && ((cookoid = parse_rawid_BM (bcookie + cookposoid, &endcookie)),
            validid_BM (cookoid)) && endcookie && *endcookie == (char) 0)
      {
        memset (oidbuf, 0, sizeof (oidbuf));
        idtocbuf32_BM (cookoid, oidbuf);
        DBGPRINTF_BM
          ("custom_onion_handler reqpath '%s' good cookie %s oid %s", reqpath,
           bcookie, oidbuf);
        goodcookie = true;
      }
    else if (bcookie)
      DBGPRINTF_BM ("custom_onion_handler reqpath '%s' bad cookie %s",
                    reqpath, bcookie);
    else
      DBGPRINTF_BM ("custom_onion_handler reqpath '%s' no cookie", reqpath);

    objlock_BM (BMP_the_web_sessions);
    if (objclass_BM (BMP_the_web_sessions) != k_websession_dict_object)
      FATAL_BM
        ("the_web_sessions is broken, should be of websession_dict_object class but is of %s",
         objectdbg_BM (objclass_BM (BMP_the_web_sessions)));
    if (!objhasdictpayl_BM (BMP_the_web_sessions))
      FATAL_BM
        ("the_web_sessions is broken, it has no dictionnary payload - for web BISMONCOOKIE-s");
    DBGPRINTF_BM ("custom_onion_handler the_web_sessions keys are %s",
                  debug_outstr_value_BM (objdictnodeofkeyspayl_BM
                                         (BMP_the_web_sessions,
                                          BMP_the_web_sessions), CURFRAME_BM,
                                         0));
    if (goodcookie)
      {
        int nbsess = objdictsizepayl_BM (BMP_the_web_sessions);
        _.sessionob =
          objdictgetpayl_BM (BMP_the_web_sessions, makestring_BM (bcookie));
        DBGPRINTF_BM
          ("custom_onion_handler reqpath '%s' nbsess %d sessionob %s",
           reqpath, nbsess, objectdbg_BM (_.sessionob));
      }
    else
      _.sessionob = NULL;
    objunlock_BM (BMP_the_web_sessions);
    perhaps_suspend_for_gc_onion_thread_stack_BM (CURFRAME_BM);
    DBGPRINTF_BM ("custom_onion_handler reqpath '%s' sessionob %s",
                  reqpath, objectdbg_BM (_.sessionob));
    if (!_.sessionob)
      goodcookie = false;
    else if (!equalid_BM (cookoid, objid_BM (_.sessionob)))
      {
        char cookoidbuf[32];
        memset (cookoidbuf, 0, sizeof (cookoidbuf));
        idtocbuf32_BM (cookoid, cookoidbuf);
        DBGPRINTF_BM ("custom_onion_handler bad cookoid %s want sessionob %s",
                      cookoidbuf, objectdbg_BM (_.sessionob));
        goodcookie = false;
      }
    if (goodcookie)
      {
        DBGPRINTF_BM ("custom_onion_handle good cookie %s sessionob %s",
                      bcookie, objectdbg_BM (_.sessionob));
        objlock_BM (_.sessionob);
        if (valtype_BM (objpayload_BM (_.sessionob)) != typayl_websession_BM)
          {
            DBGPRINTF_BM
              ("custom_onion_handle bad sessionob %s of payload@%p of type#%d %s",
               objectdbg_BM (_.sessionob), objpayload_BM (_.sessionob),
               valtype_BM (objpayload_BM (_.sessionob)),
               typestring_BM (valtype_BM (objpayload_BM (_.sessionob))));
            goodcookie = false;
          }
        else
          {
            struct websessiondata_stBM *ws =
              (struct websessiondata_stBM *) objpayload_BM (_.sessionob);
            ASSERT_BM (ws != NULL
                       && ws->websess_magic == BISMONION_WEBSESS_MAGIC);
            if (ws->websess_ownobj != _.sessionob)
              goodcookie = false;
            else
              {
                if (ws->websess_rank != cookrank)
                  goodcookie = false;
                else if (ws->websess_rand1 != cookrand1)
                  goodcookie = false;
                else if (ws->websess_rand2 != cookrand2)
                  goodcookie = false;
              }
            objunlock_BM (_.sessionob);
          }
      }
    DBGPRINTF_BM ("custom_onion_handle goodcookie %s",
                  goodcookie ? "true" : "false");
    if (!goodcookie)
      _.sessionob = false;
  }
  DBGPRINTF_BM ("custom_onion_handle sessionob %s reqpath '%s'",
                objectdbg_BM (_.sessionob), reqpath);
  if (!_.sessionob)
    {
      if (reqmeth == OR_GET || reqmeth == OR_HEAD)
        {                       // no session for a GET or HEAD request
          char pidbuf[16];
          snprintf (pidbuf, sizeof (pidbuf), "%d", (int) getpid ());
          time_t nowt = 0;
          time (&nowt);
          struct tm nowtm;
          char nowbuf[64];
          memset (nowbuf, 0, sizeof (nowbuf));
          memset (&nowtm, 0, sizeof (nowtm));
          localtime_r (&nowt, &nowtm);
          strftime (nowbuf, sizeof (nowbuf), "%c %Z", &nowtm);
          // send a fresh login form, using login_ONIONBM_thtml(onion_dict *context, onion_response *res)
          onion_dict *ctxdic = onion_dict_new ();
          onion_dict_add (ctxdic, "origpath",
                          (reqpath
                           && reqpath[0]) ? reqpath : "/", OD_DUP_VALUE);
          onion_dict_add (ctxdic, "host", myhostname_BM, OD_DUP_VALUE);
          onion_dict_add (ctxdic, "pid", pidbuf, OD_DUP_VALUE);
          onion_dict_add (ctxdic, "extra", "initial login", OD_DUP_VALUE);
          onion_dict_add (ctxdic, "buildtime", bismon_timestamp,
                          OD_DUP_VALUE);
          onion_dict_add (ctxdic, "lastgitcommit", bismon_lastgitcommit,
                          OD_DUP_VALUE);
          onion_dict_add (ctxdic, "checksum", bismon_checksum, OD_DUP_VALUE);
          onion_dict_add (ctxdic, "gentime", nowbuf, OD_DUP_VALUE);
          login_ONIONBM_thtml (ctxdic, resp);
          onion_dict_free (ctxdic);
          result = OCS_PROCESSED;
          unregister_onion_thread_stack_BM (CURFRAME_BM);
          return result;
        }
      else                      // no session for a POST or other (non-GET, non-HEAD) request
        {
          // deny the request so return OCS_FORBIDDEN; we could use
          // onion_server_set_internal_error_handler to make the error
          // HTML page sexier
          DBGPRINTF_BM
            ("onion request to '%s' which is not GET or HEAD without valid cookie",
             reqpath);
          result = OCS_FORBIDDEN;
          unregister_onion_thread_stack_BM (CURFRAME_BM);
          return result;
        }
    }
  else
    {
      // got sessionob, should process the request by making some webexchange
      bool posthttp = false;
      if ((reqmeth == OR_GET) || (reqmeth == OR_HEAD))
        posthttp = false;
      else if (reqmeth == OR_POST)
        posthttp = true;
      else
        {
          WARNPRINTF_BM ("custom_onion_handler unexpected method "      //
                         " reqpath '%s' reqflags %#x:%s sessionob %s",  //
                         reqpath, reqflags,     //
                         ((reqmeth == OR_GET) ? "GET"   //
                          : (reqmeth == OR_HEAD) ? "HEAD"       //
                          : (reqmeth == OR_POST) ? "POST"       //
                          : (reqmeth == OR_OPTIONS) ? "OPTIONS" //
                          : (reqmeth == OR_PROPFIND) ? "PROPFIND"       //
                          : snprintf (dbgmethbuf, sizeof (dbgmethbuf),  ///
                                      "meth#%d", reqmeth)),
                         objectdbg_BM (_.sessionob));
          result = OCS_NOT_IMPLEMENTED;
          unregister_onion_thread_stack_BM (CURFRAME_BM);
          return result;
        }
      if (strstr (reqpath, "/.") || strstr (reqpath, ".."))
        {
          DBGPRINTF_BM ("onion request to invalid '%s'", reqpath);
          result = OCS_FORBIDDEN;
          unregister_onion_thread_stack_BM (CURFRAME_BM);
          return result;
        }
      result = do_dynamic_onion_BM (_.sessionob, reqpath, posthttp, req, resp,
                                    CURFRAME_BM);
      unregister_onion_thread_stack_BM (CURFRAME_BM);
      return result;
    }
  DBGPRINTF_BM ("end custom_onion_handler reqpath '%s' reqflags %#x:%s bcookie %s",     //
                reqpath, reqflags,      //
                ((reqmeth == OR_GET) ? "GET"    //
                 : (reqmeth == OR_HEAD) ? "HEAD"        //
                 : (reqmeth == OR_POST) ? "POST"        //
                 : (reqmeth == OR_OPTIONS) ? "OPTIONS"  //
                 : (reqmeth == OR_PROPFIND) ? "PROPFIND"        //
                 : snprintf (dbgmethbuf, sizeof (dbgmethbuf),   ///
                             "meth#%d", reqmeth)),
                bcookie ? bcookie : "*none*");
#warning unimplemented custom_onion_handler_BM
  /// probably should return OCS_PROCESSED if handled, or OCS_NOT_PROCESSED, OCS_FORBIDDEN, OCS_INTERNAL_ERROR, etc...
  result = OCS_NOT_PROCESSED;
  unregister_onion_thread_stack_BM (CURFRAME_BM);
  return result;
}                               /* end custom_onion_handler_BM */



onion_connection_status
login_onion_handler_BM (void *_clientdata __attribute__ ((unused)),
                        onion_request * req, onion_response * resp)
{
  objectval_tyBM *k_login_onion_handler = BMK_8qHowkDvzRL_03sltCgsDN2;
  LOCALFRAME_BM ( /*prev: */ NULL, /*descr: */ k_login_onion_handler,
                 objectval_tyBM * contribob;);
  const char *reqpath = onion_request_get_path (req);
  unsigned reqflags = onion_request_get_flags (req);
  unsigned reqmeth = (reqflags & OR_METHODS);
  const char *bcookie = onion_request_get_cookie (req, "BISMONCOOKIE");
  char dbgmethbuf[16];
  DBGPRINTF_BM ("login_onion_handler reqpath '%s' fullpath '%s' reqflags %#x:%s bcookie %s",    //
                reqpath, onion_request_get_fullpath (req), reqflags,    //
                ((reqmeth == OR_GET) ? "GET"    //
                 : (reqmeth == OR_HEAD) ? "HEAD"        //
                 : (reqmeth == OR_POST) ? "POST"        //
                 : (reqmeth == OR_OPTIONS) ? "OPTIONS"  //
                 : (reqmeth == OR_PROPFIND) ? "PROPFIND"        //
                 : snprintf (dbgmethbuf, sizeof (dbgmethbuf),   ///
                             "meth#%d", reqmeth)),
                bcookie ? bcookie : "*none*");
  if (reqmeth == OR_POST)
    {
      // see the login form in login_ONIONBM.thtml template
      const char *formorigpath = onion_request_get_post (req, "origpath");
      const char *formuser = onion_request_get_post (req, "user");
      const char *formpassword = onion_request_get_post (req, "password");
      const char *formdologin = onion_request_get_post (req, "dologin");
      const char *formdoforgot = onion_request_get_post (req, "doforgot");
      bool good = false;
      DBGPRINTF_BM ("login_onion_handler POST form origpath %s,"
                    " user %s, password %s, dologin %s, doforgot %s",
                    formorigpath ? : "*no-origpath*",
                    formuser ? : "*no-user*",
                    formpassword ? : "*no-password*",
                    formdologin ? : "*no-dologin*",
                    formdoforgot ? : "*no-doforgot*");
      if (formuser && formpassword && formdologin && formorigpath)
        {
          _.contribob = find_contributor_BM (formuser, CURFRAME_BM);
          DBGPRINTF_BM ("login_onion_handler POST contribob %s",
                        objectdbg_BM (_.contribob));
          if (_.contribob)
            {
              good =
                check_contributor_password_BM (_.contribob, formpassword,
                                               CURFRAME_BM);
              DBGPRINTF_BM ("login_onion_handler POST contribob %s "
                            "password '%s' is %s",
                            objectdbg_BM (_.contribob), formpassword,
                            good ? "good" : "bad");
            }
          else
            {
              DBGPRINTF_BM
                ("login_onion_handler POST no contributor '%s' found",
                 formuser);
              good = false;
            }
          if (good)
            {
              DBGPRINTF_BM ("login_onion_handler POST good");
              return
                do_login_redirect_onion_BM (_.contribob,
                                            formorigpath ? : "/",
                                            req, resp, CURFRAME_BM);
            }
          else
            {
              time_t nowt = 0;
              time (&nowt);
              struct tm nowtm;
              char nowbuf[64];
              char pidbuf[16];
              memset (nowbuf, 0, sizeof (nowbuf));
              memset (pidbuf, 0, sizeof (pidbuf));
              memset (&nowtm, 0, sizeof (nowtm));
              snprintf (pidbuf, sizeof (pidbuf), "%d", (int) getpid ());
              localtime_r (&nowt, &nowtm);
              strftime (nowbuf, sizeof (nowbuf), "%c %Z", &nowtm);
              onion_dict *ctxdic = onion_dict_new ();
              {
                char *origpath = (reqpath && reqpath[0]) ? reqpath : "/";
                if (reqpath && !strcmp (reqpath, "_login"))
                  {
                    if (formorigpath)
                      origpath = formorigpath;
                  }
                onion_dict_add (ctxdic, "origpath", origpath, OD_DUP_VALUE);
              }
              onion_dict_add (ctxdic, "host", myhostname_BM, OD_DUP_VALUE);
              onion_dict_add (ctxdic, "pid", pidbuf, OD_DUP_VALUE);
              onion_dict_add (ctxdic, "extra", "Invalid user or password.",
                              OD_DUP_VALUE);
              onion_dict_add (ctxdic, "buildtime", bismon_timestamp,
                              OD_DUP_VALUE);
              onion_dict_add (ctxdic, "lastgitcommit", bismon_lastgitcommit,
                              OD_DUP_VALUE);
              onion_dict_add (ctxdic, "gentime", nowbuf, OD_DUP_VALUE);
              onion_dict_add (ctxdic, "checksum", bismon_checksum,
                              OD_DUP_VALUE);
              onion_response_set_code (resp, HTTP_UNAUTHORIZED);
              DBGPRINTF_BM ("login_onion_handler POST unauthorized");
              login_ONIONBM_thtml (ctxdic, resp);
              onion_dict_free (ctxdic);
              return OCS_PROCESSED;
            }
        }
      else if (formuser && formdoforgot)
        {
          DBGPRINTF_BM
            ("login_ONIONBM_thtml doforgot %s user '%s' unimplemented",
             formdoforgot, formuser);
          return do_forgot_onion_BM (formuser, req, resp);
        }
    }
  /// temporary
  WARNPRINTF_BM ("login_onion_handler incomplete");
#warning login_onion_handler_BM incomplete
  return OCS_FORBIDDEN;
}                               /* end login_onion_handler_BM */


static onion_connection_status
do_forgot_onion_BM (char *formuser,
                    onion_request * req, onion_response * resp)
{
  WARNPRINTF_BM ("do_forgot_onion_BM %s unimplemented", formuser);

  char *respbuf = NULL;
  size_t respsiz = 0;
  FILE *fresp = open_memstream (&respbuf, &respsiz);
  if (!fresp)
    FATAL_BM ("login_onion_handler open_memstream failure %m");
  fprintf (fresp, "<!DOCTYPE html>\n");
  fprintf (fresp,
           "<html><head><title>Bismon forgot password unimplemented</title></head>\n");
  fprintf (fresp,
           "<body><h1>Bismon <i>forgot password</i> feature unimplemented</h1>\n");
  fprintf (fresp,
           "<p>The <i>forgot password</i> feature (for user <tt>%s</tt>)"
           " is <b>not implemented</b> yet.\n", formuser);
  fprintf (fresp,
           "Sorry about that. So temporarily, use the <tt>--add-passwords</tt>"
           " program option on the Bismon server to change your password.<br/>\n"
           "See our <a href='https://github.com/bstarynk/bismon/blob/master/README.md'>"
           "README.md</a> file for more.</p>\n");
  fprintf (fresp, "<hr/>\n");
  time_t nowt = 0;
  time (&nowt);
  struct tm nowtm;
  char nowbuf[64];
  memset (nowbuf, 0, sizeof (nowbuf));
  memset (&nowtm, 0, sizeof (nowtm));
  localtime_r (&nowt, &nowtm);
  strftime (nowbuf, sizeof (nowbuf), "%c %Z", &nowtm);
  fprintf (fresp, "<p><small>generated on <i>%s</i></small></p>\n", nowbuf);
  fprintf (fresp, "</body></html>\n");
  fflush (fresp);
  long ln = ftell (fresp);
  fclose (fresp), fresp = NULL;
  onion_response_set_length (resp, ln);
  onion_response_set_code (resp, HTTP_NOT_IMPLEMENTED);
  onion_response_write (resp, respbuf, ln);
  onion_response_flush (resp);
  free (respbuf), respbuf = NULL;
  return OCS_PROCESSED;
}                               /* end do_forgot_onion_BM */



onion_connection_status
do_login_redirect_onion_BM (objectval_tyBM * contribobarg,
                            const char *location, onion_request * req,
                            onion_response * resp,
                            struct stackframe_stBM * stkf)
{
  static long sessioncounter;
  objectval_tyBM *k_websession_object = BMK_56KY6TzyCU5_12De0mHE48M;
  objectval_tyBM *k_websession_dict_object = BMK_2HGGdFqLH2E_8HktHZxdBd8;
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * contribob;    //
                 objectval_tyBM * sessionob;    //
                 value_tyBM cookiestrv;
    );
  ASSERT_BM (isobject_BM (contribobarg));
  _.contribob = contribobarg;
  DBGPRINTF_BM ("do_login_redirect_onion_BM start contribob %s location '%s'",
                objectdbg_BM (_.contribob), location);
  /// create the session
  char sessidbuf[32];
  memset (sessidbuf, 0, sizeof (sessidbuf));
  _.sessionob = makeobj_BM ();
  idtocbuf32_BM (objid_BM (_.sessionob), sessidbuf);
  struct websessiondata_stBM *wsess =
    allocgcty_BM (typayl_websession_BM, sizeof (*wsess));
  wsess->websess_magic = 1;
  wsess->websess_rank = 0;
  wsess->websess_rand1 = 100 + (g_random_int () % (INT_MAX / 2));
  wsess->websess_rand2 = 105 + (g_random_int () % (INT_MAX / 2));
  wsess->websess_ownobj = _.sessionob;
  wsess->websess_contribob = _.contribob;
  wsess->websess_createtime = clocktime_BM (CLOCK_REALTIME);
  wsess->websess_expiretime =
    wsess->websess_createtime + WEBSESSION_EXPIRATION_DELAY;
  objputpayload_BM (_.sessionob, wsess);
  objputclass_BM (_.sessionob, k_websession_object);
  wsess->websess_magic = BISMONION_WEBSESS_MAGIC;
  objtouchnow_BM (_.sessionob);
  /// add the session to the_web_sessions; its lock also serializes
  /// access to our sessioncounter...
  {
    objlock_BM (BMP_the_web_sessions);
    // this should never happen, but it is better to check
    if (!objhasdictpayl_BM (BMP_the_web_sessions)
        || objclass_BM (BMP_the_web_sessions) != k_websession_dict_object)
      FATAL_BM ("corrupted `the_web_sessions` (of class %s)",
                objectdbg_BM (objclass_BM (BMP_the_web_sessions)));
    sessioncounter++;
    wsess->websess_rank = sessioncounter;
    _.cookiestrv =
      sprintfstring_BM ("n%06ldR%dt%do%s", wsess->websess_rank,
                        wsess->websess_rand1, wsess->websess_rand2,
                        sessidbuf);
    DBGPRINTF_BM ("do_login_redirect_onion_BM for contribob %s,\n"
                  "... adding sessionob %s of cookie '%s'",
                  objectdbg_BM (_.contribob), objectdbg1_BM (_.sessionob),
                  bytstring_BM (_.cookiestrv));
    objdictputpayl_BM (BMP_the_web_sessions, _.cookiestrv, _.sessionob);
    objunlock_BM (BMP_the_web_sessions);
  }
  bool addedcookie =            //
    onion_response_add_cookie (resp, "BISMONCOOKIE",
                               bytstring_BM (_.cookiestrv),
                               (time_t) wsess->websess_expiretime,
                               "/",
                               NULL,    /// domain
                               0);
  onion_response_set_code (resp, HTTP_REDIRECT);
  onion_response_set_header (resp, "Location", location);
  DBGPRINTF_BM ("do_login_redirect_onion_BM sessionob %s addedcookie %s"
                " location %s",
                objectdbg_BM (_.sessionob),
                addedcookie ? "true" : "false", location);
  char *respbuf = NULL;
  size_t respsiz = 0;
  FILE *fresp = open_memstream (&respbuf, &respsiz);
  if (!fresp)
    FATAL_BM ("do_login_redirect_onion open_memstream failure %m");
  fprintf (fresp, "<!DOCTYPE html>\n");
  fprintf (fresp, "<html><head><title>Bismon login redirect</title>\n"
           "<meta http-equiv='refresh' content='2; URL=http://%s/%s'/>\n",
           onion_web_base_BM, location);
  fprintf (fresp, "</head>\n<body>\n");
  fprintf (fresp,
           "<h1>Bismon login redirection to <a href='http://%s/%s'>%s</a></h1>\n",
           onion_web_base_BM, location, location);
  fprintf (fresp, "<hr/>\n");
  time_t nowt = 0;
  time (&nowt);
  struct tm nowtm;
  char nowbuf[64];
  memset (nowbuf, 0, sizeof (nowbuf));
  memset (&nowtm, 0, sizeof (nowtm));
  localtime_r (&nowt, &nowtm);
  strftime (nowbuf, sizeof (nowbuf), "%c %Z", &nowtm);
  fprintf (fresp, "<p><small>generated on <i>%s</i></small></p>\n", nowbuf);
  fprintf (fresp, "</body>\n</html>\n");
  fflush (fresp);
  long ln = ftell (fresp);
  fclose (fresp), fresp = NULL;
  onion_response_set_length (resp, ln);
  onion_response_write (resp, respbuf, ln);
  onion_response_flush (resp);
  DBGPRINTF_BM ("do_login_redirect_onion_BM sessionob %s"
                " redirection to %s:\n%s\n",
                objectdbg_BM (_.sessionob), location, respbuf);
  free (respbuf), respbuf = NULL;
  DBGPRINTF_BM ("do_login_redirect_onion  sessionob %s done",
                objectdbg_BM (_.sessionob));
  return OCS_PROCESSED;
}                               /* end do_login_redirect_onion_BM */






//////////////////////////////////////////////////////////////////////////
// lock for the web exchange count
static pthread_mutex_t webexonion_mtx_BM = PTHREAD_MUTEX_INITIALIZER;
static long webexonion_count_BM;
////////////////

static value_tyBM find_web_handler_BM (objectval_tyBM * sessionobarg,
                                       objectval_tyBM * dictobarg,
                                       onion_request * req,
                                       int depth,
                                       int *poffset,
                                       struct stackframe_stBM *stkf);


////////////////
#define WEBEXCHANGE_DELAY_BM   (debugmsg_BM?40.0:2.5)
onion_connection_status
do_dynamic_onion_BM (objectval_tyBM * sessionobarg, const char *reqpath,
                     bool postrequest,
                     onion_request * req,
                     onion_response * resp, struct stackframe_stBM *stkf)
{
  objectval_tyBM *k_no_value = BMK_7SYPOwPm7jc_2PeGiJ8uQiX;
  objectval_tyBM *k_webexchange_object = BMK_8keZiP7vbFw_1ovBXqd6a0d;
  objectval_tyBM *k_failure_bad_closure = BMK_373gFe8m21E_47xzvCGxpI9;
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * sessionob; objectval_tyBM * webexob;
                 value_tyBM failreasonv;        //
                 value_tyBM webhandlerv;        //
                 value_tyBM restpathv;  //
                 value_tyBM appresv;    //
    );
  unsigned reqflags = onion_request_get_flags (req);
  unsigned reqmeth = (reqflags & OR_METHODS);
  ASSERT_BM (isobject_BM (sessionobarg));
  _.sessionob = sessionobarg;
  DBGPRINTF_BM ("do_dynamic_onion start sessionob %s reqpath '%s' post %s",
                objectdbg_BM (_.sessionob), reqpath,
                postrequest ? "true" : "false");
  _.webexob = makeobj_BM ();
  objputclass_BM (_.webexob, k_webexchange_object);
  struct webexchangedata_stBM *wexda =
    allocgcty_BM (typayl_webexchange_BM, sizeof (*wexda));
  unsigned inisizew = 8192;
  char *dbuf = malloc (inisizew * sizeof (void *));
  if (!dbuf)
    FATAL_BM ("malloc %zu bytes failed (%m)", inisizew * sizeof (void *));
  memset (dbuf, 0, inisizew * sizeof (void *));
  wexda->webx_sbuf.sbuf_indent = 0;
  wexda->webx_sbuf.sbuf_dbuf = dbuf;
  wexda->webx_sbuf.sbuf_size = inisizew * sizeof (void *);
  wexda->webx_sbuf.sbuf_curp = dbuf;
  wexda->webx_sbuf.sbuf_lastnl = NULL;
  {
    pthread_mutex_lock (&webexonion_mtx_BM);
    webexonion_count_BM++;
    wexda->webx_num = webexonion_count_BM;
    pthread_mutex_unlock (&webexonion_mtx_BM);
  }
  register_onion_thread_stack_BM (CURFRAME_BM);
  wexda->webx_ownobj = _.webexob;
  wexda->webx_sessobj = _.sessionob;
  wexda->webx_datav = NULL;
  wexda->webx_time = clocktime_BM (CLOCK_REALTIME);
  wexda->webx_requ = req;
  wexda->webx_resp = resp;
  pthread_cond_init (&wexda->webx_cond_ready, NULL);
  objputpayload_BM (_.webexob, wexda);
  objtouchnow_BM (_.webexob);
  wexda->webx_magic = BISMONION_WEBX_MAGIC;
  WARNPRINTF_BM
    ("do_dynamic_onion unimplemented  sessionob %s reqpath '%s' post %s wexnum %ld webexob %s",
     objectdbg_BM (_.sessionob), reqpath, postrequest ? "true" : "false",
     wexda->webx_num, objectdbg1_BM (_.webexob));
  int failcod = 0;
  _.failreasonv = NULL;
  struct failurehandler_stBM *prevfailureh = curfailurehandle_BM;
  {
    struct failurelockset_stBM flockset = { };
    initialize_failurelockset_BM (&flockset, sizeof (flockset));
    LOCAL_FAILURE_HANDLE_BM (&flockset, lab_failureweb, failcod,
                             _.failreasonv);
    curfailurehandle_BM = prevfailureh;
    if (failcod)
    lab_failureweb:
      {
        WARNPRINTF_BM ("do_dynamic_onion failure reqpath '%s' failcod=%d failreasonv %s", reqpath, failcod,     //
                       debug_outstr_value_BM (_.failreasonv, CURFRAME_BM, 0));
        destroy_failurelockset_BM (&flockset);
        curfailurehandle_BM = NULL;
        char *respbuf = NULL;
        size_t respsiz = 0;
        FILE *fresp = open_memstream (&respbuf, &respsiz);
        if (!fresp)
          FATAL_BM
            ("do_dynamic_onion (failure case failcode %d) open_memstream failure %m",
             failcod);
        fprintf (fresp, "<!DOCTYPE html>\n");
        fprintf (fresp, "<html><head><title>Bismon failure</title>\n");
        fprintf (fresp, "</head>\n<body>\n");
        fprintf (fresp, "<h1>Bismon failure</h1>\n");
        fprintf (fresp, "<p>Request <i>%s</i> of path '<tt>%s</tt>' <b>failed</b>.<br/>\n",     //
                 ((reqmeth == OR_GET) ? "GET"   //
                  : (reqmeth == OR_HEAD) ? "HEAD"       //
                  : (reqmeth == OR_POST) ? "POST"       //
                  : "???"), reqpath);
        fprintf (fresp, "Failure code %d, reason %s</p>\n", failcod,
                 debug_outstr_value_BM (_.failreasonv, CURFRAME_BM, 0));
        time_t nowt = 0;
        time (&nowt);
        struct tm nowtm;
        char nowbuf[64];
        memset (nowbuf, 0, sizeof (nowbuf));
        memset (&nowtm, 0, sizeof (nowtm));
        localtime_r (&nowt, &nowtm);
        strftime (nowbuf, sizeof (nowbuf), "%c %Z", &nowtm);
        fprintf (fresp,
                 "<p><small>generated at <i>%s</i> on <tt>%s</tt> pid %d</small></p>\n",
                 nowbuf, myhostname_BM, (int) getpid ());
        fprintf (fresp, "</body>\n</html>\n");
        fflush (fresp);
        long ln = ftell (fresp);
        fclose (fresp), fresp = NULL;
        onion_response_set_code (resp, HTTP_INTERNAL_ERROR);
        onion_response_set_length (resp, ln);
        onion_response_write (resp, respbuf, ln);
        onion_response_flush (resp);
        return OCS_PROCESSED;
      }
    else
      {
        int off = 0;
        int reqlen = strlen (reqpath);
        // normal case, should find the web processing closure then apply it
        DBGPRINTF_BM
          ("do_dynamic_onion normal  sessionob %s reqpath '%s' post %s wexnum %ld webexob %s",
           objectdbg_BM (_.sessionob), reqpath,
           postrequest ? "true" : "false", wexda->webx_num,
           objectdbg1_BM (_.webexob));
        _.webhandlerv =
          find_web_handler_BM (_.sessionob, BMP_webdict_root, req, 0, &off,
                               CURFRAME_BM);
        if (!_.webhandlerv)
          FAILURE_BM (__LINE__, k_no_value, CURFRAME_BM);
        else if (!isclosure_BM (_.webhandlerv))
          FAILURE_BM (__LINE__,
                      makenode1_BM (k_failure_bad_closure, _.webhandlerv),
                      CURFRAME_BM);
        if (off < 0)
          off = 0;
        else if (off > reqlen)
          off = reqlen;
        _.restpathv = (value_tyBM) makestring_BM (reqpath + off);
        DBGPRINTF_BM ("do_dynamic_onion before apply webhandler %s restpath %s webexob %s",     //
                      debug_outstr_value_BM (_.webhandlerv, CURFRAME_BM, 0),    //
                      debug_outstr_value_BM (_.restpathv, CURFRAME_BM, 0),      //
                      objectdbg_BM (_.webexob));
        _.appresv =
          apply2_BM (_.webhandlerv, CURFRAME_BM, _.restpathv, _.webexob);
        DBGPRINTF_BM ("do_dynamic_onion after apply webhandler %s restpath %s webexob %s appres %s",    //
                      debug_outstr_value_BM (_.webhandlerv, CURFRAME_BM, 0),    //
                      debug_outstr_value_BM (_.restpathv, CURFRAME_BM, 0),      //
                      objectdbg_BM (_.webexob), //
                      debug_outstr_value_BM (_.appresv, CURFRAME_BM, 0));
        destroy_failurelockset_BM (&flockset);
        curfailurehandle_BM = NULL;
      }
  }
  perhaps_suspend_for_gc_onion_thread_stack_BM (CURFRAME_BM);
  // should wait for the wexda->webx_cond_ready
#warning missing code in do_dynamic_onion to wait
  return OCS_NOT_IMPLEMENTED;
}                               /* end do_dynamic_onion_BM */


#define MAX_WEB_HANDLER_DEPTH_BM 40
value_tyBM
find_web_handler_BM (objectval_tyBM * sessionobarg,
                     objectval_tyBM * dictobarg,
                     onion_request * req,
                     int depth, int *poffset, struct stackframe_stBM * stkf)
{
  objectval_tyBM *k_depth = BMK_17YdW6dWrBA_2mn4QmBjMNs;
  objectval_tyBM *k_web_empty_handler = BMK_8Rwc7G3hQ0G_230O74aoi1w;
  objectval_tyBM *k_webhandler_dict_object = BMK_23YbAS1S796_1ZeW8OZfp1J;
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * sessionob;    //
                 objectval_tyBM * dictob;       //
                 objectval_tyBM * valob;        //
                 objectval_tyBM * nextdictob;   //
                 value_tyBM wordv;      //
                 value_tyBM valv;       //
    );
  _.sessionob = sessionobarg;
  _.dictob = dictobarg;
  ASSERT_BM (isobject_BM (_.dictob));
  ASSERT_BM (isobject_BM (_.sessionob));
  const char *path = onion_request_get_path (req);
  int pathlen = strlen (path);
  int off = *poffset;
  if (off > pathlen)
    off = pathlen;
  DBGPRINTF_BM
    ("find_web_handler start sessionob %s dictob %s depth %d path '%s' off#%d",
     objectdbg_BM (_.sessionob), objectdbg1_BM (_.dictob), depth, path, off);
  if (depth > MAX_WEB_HANDLER_DEPTH_BM)
    {
      WARNPRINTF_BM ("find_web_handler path '%s' too deep %d", path, depth);
      FAILURE_BM (__LINE__, k_depth, CURFRAME_BM);
    }
  const char *subpath = path + off;
  const char *slash = strchr (subpath, '/');
  if (!slash)
    slash = subpath + strlen (subpath);
  int wordlen = slash - subpath;
  const char *word = NULL;
  char wordbuf[64];
  memset (wordbuf, 0, sizeof (wordbuf));
  if (wordlen < sizeof (wordbuf))
    {
      if (wordlen > 0)
        strncpy (wordbuf, subpath, wordlen - 1);
      word = wordbuf;
    }
  else
    {
      word = strndup (subpath, wordlen - 1);
      if (!word)
        FATAL_BM ("strndup failure (wordlen=%d)", wordlen);
    }
  DBGPRINTF_BM ("find_web_handler path='%s' dictob %s off=%d word='%s'",
                path, objectdbg_BM (_.dictob), off, word);
  {
    objlock_BM (_.dictob);
    if (word && word[0])
      {
        _.wordv = (value_tyBM) makestring_BM (word);
        _.valv = objdictgetpayl_BM (_.dictob, _.wordv);
        *poffset = off + wordlen;
      }
    else
      _.valv = objgetattr_BM (_.dictob, k_web_empty_handler);
    objunlock_BM (_.dictob);
  }
  DBGPRINTF_BM ("find_web_handler word '%s' path '%s' dictob %s valv %s",
                word, path, objectdbg_BM (_.dictob),
                debug_outstr_value_BM (_.valv, CURFRAME_BM, 0));
  if (isclosure_BM (_.valv))
    LOCALRETURN_BM (_.valv);
  else if (isobject_BM (_.valv))
    {
      _.nextdictob = NULL;
      _.valob = objectcast_BM (_.valv);
      objlock_BM (_.valob);
      if (objectisinstance_BM (_.valob, k_webhandler_dict_object))
        _.nextdictob = _.valob;
      objunlock_BM (_.valob);
      DBGPRINTF_BM ("find_web_handler word '%s' path '%s' nextdictob %s",
                    word, path, objectdbg_BM (_.nextdictob));
      _.valv = NULL;
      if (_.nextdictob)
        _.valv = find_web_handler_BM (_.sessionob, _.nextdictob,
                                      req, depth + 1, poffset, CURFRAME_BM);
      DBGPRINTF_BM
        ("find_web_handler word '%s' path '%s' nextdictob %s depth %d gives valv %s",
         word, path, objectdbg_BM (_.nextdictob), depth,
         debug_outstr_value_BM (_.valv, CURFRAME_BM, 0));

      LOCALRETURN_BM (_.valv);
    }
  DBGPRINTF_BM ("find_web_handler word '%s' path '%s' not found in dictob %s",
                word, path, objectdbg_BM (_.dictob));
  LOCALRETURN_BM (NULL);
}                               /* end find_web_handler_BM */

void
objwebexchangeputdatapayl_BM (const objectval_tyBM * obj, value_tyBM val)
{
  struct webexchangedata_stBM *wxda = objgetwebexchangepayl_BM (obj);
  if (!wxda)
    return NULL;
  ASSERT_BM (wxda->webx_magic == BISMONION_WEBX_MAGIC);
  wxda->webx_datav = val;
}                               /* end  objwebexchangeputdatapayl_BM */

void
objwebexchangecompletepayl_BM (const objectval_tyBM * obj, int httpstatus,
                               const char *mimetype)
{
  struct webexchangedata_stBM *wxda = objgetwebexchangepayl_BM (obj);
  if (!wxda)
    return NULL;
  ASSERT_BM (wxda->webx_magic == BISMONION_WEBX_MAGIC);
  if (mimetype)
    {
      memset (wxda->webx_mimetype, 0, sizeof (wxda->webx_mimetype));
      strncpy (wxda->webx_mimetype, mimetype,
               sizeof (wxda->webx_mimetype) - 1);
    }
  ASSERT_BM (wxda->webx_resp != NULL);
  onion_response_set_code (wxda->webx_resp, httpstatus);
  pthread_cond_broadcast (&wxda->webx_cond_ready);
}                               /* end objwebexchangecompletepayl_BM */

/******************************************************************/

void
stop_onion_event_loop_BM (void)
{
  atomic_store (&onionlooprunning_BM, false);
}                               /* end stop_onion_event_loop_BM */



void
webonion_suspend_before_gc_BM (void)
{
  FATAL_BM ("unimplemented webonion_suspend_before_gc_BM");
#warning unimplemented webonion_suspend_before_gc_BM
}                               /* end webonion_suspend_before_gc_BM */

void
webonion_continue_after_gc_BM (void)
{
  FATAL_BM ("unimplemented webonion_continue_after_gc_BM");
#warning unimplemented webonion_continue_after_gc_BM
}                               /* end webonion_continue_after_gc_BM */

/// remember that only plain_event_loop_BM is allowed to *remove*
/// things from onionrunprocarr_BM or onionrunpro_list_BM
void
plain_event_loop_BM (void)
{
  int termsigfd = -1;
  int quitsigfd = -1;
  int chldsigfd = -1;
  LOCALFRAME_BM ( /*prev: */ NULL, /*descr: */ NULL,
                 objectval_tyBM * bufob;
    );
  atomic_init (&onionlooprunning_BM, true);
  // see also https://ldpreload.com/blog/signalfd-is-useless
  {
    sigset_t termsigset = { };
    sigemptyset (&termsigset);
    sigaddset (&termsigset, SIGTERM);
    if (sigprocmask (SIG_BLOCK, &termsigset, NULL) == -1)
      FATAL_BM ("sigprocmask termsigset failure");
    termsigfd = signalfd (-1, &termsigset, SFD_NONBLOCK | SFD_CLOEXEC);
    if (termsigfd < 0)
      FATAL_BM ("signalfd failed for SIGTERM");
  }
  {
    sigset_t quitsigset = { };
    sigemptyset (&quitsigset);
    sigaddset (&quitsigset, SIGQUIT);
    if (sigprocmask (SIG_BLOCK, &quitsigset, NULL) == -1)
      FATAL_BM ("sigprocmask quitsigset failure");
    quitsigfd = signalfd (-1, &quitsigset, SFD_NONBLOCK | SFD_CLOEXEC);
    if (quitsigfd < 0)
      FATAL_BM ("signalfd failed for SIGQUIT");
  }
  {
    sigset_t chldsigset = { };
    sigemptyset (&chldsigset);
    sigaddset (&chldsigset, SIGCHLD);
    if (sigprocmask (SIG_BLOCK, &chldsigset, NULL) == -1)
      FATAL_BM ("sigprocmask chldsigset failure");
    chldsigfd = signalfd (-1, &chldsigset, SFD_NONBLOCK | SFD_CLOEXEC);
    if (chldsigfd < 0)
      FATAL_BM ("signalfd failed for SIGCHLD");
  }
  DBGPRINTF_BM ("plain_event_loop_BM before loop termsigfd=%d quitsigfd=%d chldsigfd=%d",       //
                termsigfd, quitsigfd, chldsigfd);
  long loopcnt = 0;
  while (atomic_load (&onionlooprunning_BM))
    {
      loopcnt++;
      struct pollfd pollarr[MAXNBWORKJOBS_BM + 8];
      pid_t endedprocarr[MAXNBWORKJOBS_BM];
      memset (pollarr, 0, sizeof (pollarr));
      memset (endedprocarr, 0, sizeof (endedprocarr));
      int nbpoll = 0;
      enum
      { pollix_term, pollix_quit, pollix_chld,
        pollix_cmdp, pollix__lastsig
      };
      pollarr[pollix_term].fd = termsigfd;
      pollarr[pollix_term].events = POLL_IN;
      pollarr[pollix_quit].fd = quitsigfd;
      pollarr[pollix_quit].events = POLL_IN;
      pollarr[pollix_chld].fd = chldsigfd;
      pollarr[pollix_chld].events = POLL_IN;
      pollarr[pollix_cmdp].fd = cmdpipe_rd_BM;
      pollarr[pollix_cmdp].events = POLL_IN;
      nbpoll = pollix__lastsig;
      {
        lockonion_runpro_mtx_at_BM (__LINE__);
        for (int j = 0; j < nbworkjobs_BM; j++)
          if (onionrunprocarr_BM[j].rp_pid > 0
              && onionrunprocarr_BM[j].rp_outpipe > 0)
            {
              pollarr[nbpoll].fd = onionrunprocarr_BM[j].rp_outpipe;
              pollarr[nbpoll].events = POLL_IN;
              nbpoll++;
            }
        unlockonion_runpro_mtx_at_BM (__LINE__);
      }
#define POLL_DELAY_MILLISECS_BM 750
      if (loopcnt % 4 == 0)
        DBGPRINTF_BM ("plain_event_loop_BM before poll nbpoll=%d loop#%ld",
                      nbpoll, loopcnt);
      int nbready = poll (&pollarr, nbpoll, POLL_DELAY_MILLISECS_BM);
      if (loopcnt % 4 == 0)
        DBGPRINTF_BM ("plain_event_loop_BM nbready %d loop#%ld", nbready,
                      loopcnt);
      if (nbready == 0)         // no file descriptor read, timed out
        continue;
      if (nbready < 0)
        {
          if (errno != EINTR)
            FATAL_BM ("plain_event_loop_BM poll failure");
          continue;
        }
      {
        char pipbuf[1024 + 4];
        lockonion_runpro_mtx_at_BM (__LINE__);
        int runix = 0;
        for (int ix = pollix__lastsig; ix < nbpoll; ix++)
          {
            if (pollarr[ix].revents & POLL_IN)
              {
                int curfd = pollarr[ix].fd;
                while (runix < nbworkjobs_BM)
                  {
                    if (onionrunprocarr_BM[runix].rp_outpipe == curfd)
                      {
                        struct onionproc_stBM *onproc =
                          onionrunprocarr_BM + runix;
                        runix++;
                        int bytcnt = 0;
                        do
                          {
                            memset (pipbuf, 0, sizeof (pipbuf));
                            // we might do ioctl(curfd, FIONBIO, &cnt)
                            // but it is not worth doing since the pipe
                            // is non-blocking. See https://stackoverflow.com/a/1151077/841108
                            bytcnt =
                              read (curfd, pipbuf, sizeof (pipbuf) - 4);
                            if (bytcnt < 0 && errno == EINTR)
                              continue;
                            if (bytcnt < 0 && errno == EWOULDBLOCK)
                              break;
                            if (bytcnt < 0)
                              {
                                // this probably should not happen
                                FATAL_BM
                                  ("unexpected error %m on output pipe fd#%d for pid %d",
                                   curfd, (int) onproc->rp_pid);
                              }
                            if (bytcnt == 0)
                              { // end of file on pipe
                                // the forked process might close its stdout
                                // and still be running, even if this is
                                // unfriendly...
                                close (curfd);
                                onproc->rp_outpipe = -1;
                                break;
                              }
                            ASSERT_BM (bytcnt > 0);
                            pipbuf[bytcnt] = (char) 0;
                            if (strlen (pipbuf) < bytcnt)
                              {
                                WARNPRINTF_BM
                                  ("unexpected null byte from process pid#%d command node %s in %s",
                                   (int) onionrunprocarr_BM[runix].rp_pid,
                                   debug_outstr_value_BM
                                   (onproc->rp_cmdnodv, CURFRAME_BM, 0),
                                   bytstring_BM (onproc->rp_dirstrv) ? :
                                   "./");
                                if (kill (onproc->rp_pid, SIGTERM) == 0)
                                  WARNPRINTF_BM ("sent SIGTERM to pid#%d",
                                                 onproc->rp_pid);
                                close (curfd);
                                onproc->rp_outpipe = -1;
                                usleep (1000);

                              }
                            _.bufob = onproc->rp_bufob;
                            ASSERT_BM (isobject_BM (_.bufob));
                            ASSERT_BM (objhasstrbufferpayl_BM (_.bufob));
                            objstrbufferappendcstrpayl_BM (_.bufob, pipbuf);
                            _.bufob = NULL;
                            pipbuf[0] = (char) 0;
                          }
                        while (bytcnt > 0);
                        break;
                      }
                  }
              }
            _.bufob = NULL;
          }
        unlockonion_runpro_mtx_at_BM (__LINE__);
      }
      if (pollarr[pollix_term].revents & POLL_IN)
        {
          read_sigterm_BM (termsigfd);
          atomic_store (&onionlooprunning_BM, false);
          break;
        }
      if (pollarr[pollix_quit].revents & POLL_IN)
        {
          read_sigquit_BM (quitsigfd);
          atomic_store (&onionlooprunning_BM, false);
          break;
        };
      if (pollarr[pollix_chld].revents & POLL_IN)
        read_sigchld_BM (chldsigfd);
      if (pollarr[pollix_cmdp].revents & POLL_IN)
        read_commandpipe_BM ();
    }                           /* end while onionlooprunning */
  DBGPRINTF_BM ("plain_event_loop_BM ended loopcnt=%ld", loopcnt);
}                               /* end plain_event_loop_BM */



static void
read_sigterm_BM (int sigfd)     // called from plain_event_loop_BM
{
  struct signalfd_siginfo sigterminf;
  memset (&sigterminf, 0, sizeof (sigterminf));
  DBGPRINTF_BM ("read_sigterm_BM start sigfd %d", sigfd);
  int nbr = read (sigfd, &sigterminf, sizeof (sigterminf));
  if (nbr != sizeof (sigterminf))       // very unlikely, probably impossible
    FATAL_BM ("read_sigterm_BM: read fail (%d bytes read, want %d) - %m",
              nbr, (int) sizeof (sigterminf));
  stop_agenda_work_threads_BM ();
  /// forcibly remove the payload of the_web_sessions.
  // In principle, even if it remains, it should not be dumped (because the class of the_web_sessions should be 
  objclearpayload_BM (BMP_the_web_sessions);
  char *rp = realpath (dump_dir_BM ? : ".", NULL);
  INFOPRINTF_BM
    ("before dumping state into %s (really %s) after SIGTERM to process %d",
     dump_dir_BM, rp, (int) getpid ());
  free (rp), rp = NULL;
  struct dumpinfo_stBM di = dump_BM (dump_dir_BM, NULL);
  INFOPRINTF_BM
    ("after dumping state into %s for SIGTERM: scanned %ld, emitted %ld objects\n"
     "did %ld todos, wrote %ld files\n"
     "in %.3f elapsed, %.4f cpu seconds.\n", dump_dir_BM,
     di.dumpinfo_scanedobjectcount, di.dumpinfo_emittedobjectcount,
     di.dumpinfo_todocount, di.dumpinfo_wrotefilecount,
     di.dumpinfo_elapsedtime, di.dumpinfo_cputime);
  DBGPRINTF_BM ("read_sigterm_BM ending");
}                               /* end read_sigterm_BM */


static void
read_sigquit_BM (int sigfd)     // called from plain_event_loop_BM
{
  struct signalfd_siginfo sigquitinf;
  memset (&sigquitinf, 0, sizeof (sigquitinf));
  DBGPRINTF_BM ("read_sigquit_BM start");
  int nbr = read (sigfd, &sigquitinf, sizeof (sigquitinf));
  if (nbr != sizeof (sigquitinf))       // very unlikely, probably impossible
    FATAL_BM ("read_sigquit_BM: read fail (%d bytes read, want %d) - %m",
              nbr, (int) sizeof (sigquitinf));
  stop_agenda_work_threads_BM ();
  INFOPRINTF_BM ("quitting BISMON process %d without dump thru SIGQUIT",
                 (int) getpid ());
}                               /* end read_sigquit_BM */


static void
read_sigchld_BM (int sigfd)
{
  objectval_tyBM *k_queue_process = BMK_8DQ4VQ1FTfe_5oijDYr52Pb;
  bool didfork = false;
  LOCALFRAME_BM ( /*prev: */ NULL, /*descr: */ NULL,
                 /// for the terminating child process
                 value_tyBM chdirstrv;  //
                 value_tyBM chcmdnodv;  //
                 value_tyBM chclosv;    //
                 objectval_tyBM * chbufob;      //
                 value_tyBM choutstrv;  //
                 // for queued commands
                 value_tyBM qnodv;      //
                 value_tyBM newdirstrv; //
                 value_tyBM newcmdnodv; //
                 value_tyBM newendclosv;        //
    );
  struct signalfd_siginfo sigchldinf;
  memset (&sigchldinf, 0, sizeof (sigchldinf));
  int nbr = read (sigfd, &sigchldinf, sizeof (sigchldinf));
  if (nbr != sizeof (sigchldinf))
    // very unlikely, probably impossible
    FATAL_BM
      ("read_sigchld_BM: read fail (%d.by read, want %d) - %m",
       nbr, (int) sizeof (sigchldinf));
  pid_t pid = sigchldinf.ssi_pid;
  int wstatus = 0;
  pid_t wpid = waitpid (pid, &wstatus, WNOHANG);
  if (wpid == pid)
    {
      DBGPRINTF_BM ("read_sigchld_BM pid %d", (int) pid);
      {
        int chix = -1;
        int nbruncmds = 0;
        lockonion_runpro_mtx_at_BM (__LINE__);
        for (int oix = 0; oix < MAXNBWORKJOBS_BM; oix++)
          {
            struct onionproc_stBM *onproc = onionrunprocarr_BM + oix;
            if (!onproc->rp_pid)
              continue;
            nbruncmds++;
            if (onproc->rp_pid == pid)
              {
                ASSERT_BM (chix < 0);
                chix = oix;
                _.chdirstrv = onproc->rp_dirstrv;
                _.chcmdnodv = onproc->rp_cmdnodv;
                _.chclosv = onproc->rp_closv;
                _.chbufob = onproc->rp_bufob;
                memset ((void *) onproc, 0, sizeof (struct onionproc_stBM));
              }
          }
        _.qnodv = nodecast_BM (listfirst_BM (onionrunpro_list_BM));
        if (_.qnodv)
          {
            ASSERT_BM (nodeconn_BM (_.qnodv) == k_queue_process);
            if (nbruncmds <= nbworkjobs_BM)
              {
                listpopfirst_BM (onionrunpro_list_BM);
                _.newdirstrv = stringcast_BM (nodenthson_BM (_.qnodv, 0));
                _.newcmdnodv = nodecast_BM (nodenthson_BM (_.qnodv, 1));
                _.newendclosv = nodecast_BM (nodenthson_BM (_.qnodv, 2));
                ASSERT_BM (isnode_BM (_.newcmdnodv));
                ASSERT_BM (isclosure_BM (_.newendclosv));
                DBGPRINTF_BM
                  ("read_sigchld_BM chix#%d newdirstrv %s newcmdnodv %s newendclosv %s beforefork",
                   chix, debug_outstr_value_BM (_.newdirstrv, CURFRAME_BM,
                                                0),
                   debug_outstr_value_BM (_.newcmdnodv, CURFRAME_BM, 0),
                   debug_outstr_value_BM (_.newendclosv, CURFRAME_BM, 0));
                fork_onion_process_at_slot_BM (chix, _.newdirstrv,
                                               _.newcmdnodv, _.newendclosv,
                                               CURFRAME_BM);
                didfork = true;
              }
          }
        unlockonion_runpro_mtx_at_BM (__LINE__);
        if (chix >= 0)
          {
            _.choutstrv =
              (value_tyBM)
              makestring_BM (objstrbufferbytespayl_BM (_.chbufob));
            DBGPRINTF_BM
              ("read_sigchld_BM defer-apply chclosv %s choutstrv %s wstatus %#x=%d",
               debug_outstr_value_BM (_.chclosv, CURFRAME_BM, 0),
               debug_outstr_value_BM (_.choutstrv, CURFRAME_BM, 0), wstatus,
               wstatus);
            do_main_defer_apply3_BM (_.chclosv, _.choutstrv,
                                     taggedint_BM (wstatus), NULL,
                                     CURFRAME_BM);
          }
      }
    }
  else
    FATAL_BM ("read_sigchld_BM waitpid failure pid#%d", pid);
  if (didfork)
    usleep (1000);              // sleep a little bit, to let the child process start
}                               /* end read_sigchld_BM */


static void
read_commandpipe_BM (void)
{
  char buf[4];
  memset (&buf, 0, sizeof (buf));
  int nbr = read (cmdpipe_rd_BM, buf, 1);
  if (nbr == 1)
    {
      DBGPRINTF_BM ("read_commandpipe_BM '%s' incomplete", buf);
      WARNPRINTF_BM ("read_commandpipe_BM unimplemented");
      // if buf[0] is 'X', execute a deferred command
      // if buf[0] is 'G', run the garbage collector. Not sure!
#warning read_commandpipe_BM incomplete
      // should handle the command
    }
  else
    DBGPRINTF_BM ("read_commandpipe_BM nbr %d - %s", nbr,
                  (nbr < 0) ? strerror (errno) : "--");
}                               /* end read_commandpipe_BM */

void
add_defer_command_onion_BM (void)
{
  char buf[4];
  memset (&buf, 0, sizeof (buf));
  buf[0] = 'X';
  int count = 0;
  while (count < 256)
    {                           /* this loop usually runs once */
      int nbw = write (cmdpipe_wr_BM, buf, 1);
      if (nbw < 0 && errno == EINTR)
        continue;
      if (nbw < 0 && errno == EWOULDBLOCK)
        {
          usleep (2000);
          continue;
        };
      if (nbw == 1)
        return;
      FATAL_BM ("add_defer_command_onion_BM nbw %d - %s", nbw,
                (nbw < 0) ? strerror (errno) : "--");
    }
  FATAL_BM ("add_defer_command_onion_BM failed");
}                               /* end add_defer_command_onion_BM */

////////////////////////////////////////////////////////////////

static pthread_mutex_t onionstack_mtx_bm = PTHREAD_MUTEX_INITIALIZER;
struct onionstackinfo_stBM
{
  pthread_t ost_thread;
  struct stackframe_stBM *ost_stkf;
};
static struct onionstackinfo_stBM onionstackinfo_bm[MAXNBWORKJOBS_BM + 1];
static thread_local struct onionstackinfo_stBM *curonionstackinfo_BM;





void
register_onion_thread_stack_BM (struct stackframe_stBM *stkf)
{
  ASSERT_BM (stkf != NULL);
  ASSERT_BM (stkf->stkfram_pA.htyp == typayl_StackFrame_BM);
  pthread_mutex_lock (&onionstack_mtx_bm);
  if (!curonionstackinfo_BM)
    {
      for (int ix = 0; ix < MAXNBWORKJOBS_BM; ix++)
        {
          if (onionstackinfo_bm[ix].ost_stkf == NULL
              && onionstackinfo_bm[ix].ost_thread == (pthread_t) 0)
            {
              curonionstackinfo_BM = onionstackinfo_bm + ix;
              onionstackinfo_bm[ix].ost_thread = pthread_self ();
              break;
            };
        }
    }
  ASSERT_BM (curonionstackinfo_BM
             && curonionstackinfo_BM->ost_thread == pthread_self ());
  curonionstackinfo_BM->ost_stkf = stkf;
  pthread_mutex_unlock (&onionstack_mtx_bm);
}                               /* end register_onion_thread_stack_BM */


void
unregister_onion_thread_stack_BM (struct stackframe_stBM *stkf)
{
  ASSERT_BM (stkf != NULL);
  ASSERT_BM (stkf->stkfram_pA.htyp == typayl_StackFrame_BM);
  pthread_mutex_lock (&onionstack_mtx_bm);
  ASSERT_BM (curonionstackinfo_BM != NULL);
  ASSERT_BM (curonionstackinfo_BM->ost_thread == pthread_self ());
  curonionstackinfo_BM->ost_thread = (pthread_t) 0;
  curonionstackinfo_BM->ost_stkf = NULL;
  curonionstackinfo_BM = NULL;
  pthread_mutex_unlock (&onionstack_mtx_bm);
}                               /* end unregister_onion_thread_stack_BM */


void
perhaps_suspend_for_gc_onion_thread_stack_BM (struct stackframe_stBM *stkf)
{
  ASSERT_BM (stkf != NULL);
  ASSERT_BM (stkf->stkfram_pA.htyp == typayl_StackFrame_BM);
  DBGPRINTF_BM
    ("perhaps_suspend_for_gc_onion_thread_stack_BM start tid#%ld  elapsed %.3f s",
     (long) gettid_BM (), elapsedtime_BM ());
  pthread_mutex_lock (&onionstack_mtx_bm);
  ASSERT_BM (curonionstackinfo_BM != NULL);
  ASSERT_BM (curonionstackinfo_BM->ost_thread == pthread_self ());
  curonionstackinfo_BM->ost_stkf = stkf;
  pthread_mutex_unlock (&onionstack_mtx_bm);
  for (;;)
    {
      if (!agenda_need_gc_BM ())
        return;
      DBGPRINTF_BM
        ("perhaps_suspend_for_gc_onion_thread_stack need GC tid#%ld  elapsed %.3f s",
         (long) gettid_BM (), elapsedtime_BM ());
      agenda_wait_gc_BM ();
    }
  DBGPRINTF_BM
    ("perhaps_suspend_for_gc_onion_thread_stack_BM end tid#%ld  elapsed %.3f s",
     (long) gettid_BM (), elapsedtime_BM ());
}                               /* end perhaps_suspend_for_gc_onion_thread_stack_BM */


// mark the stack of every webonion pthread; they all should be "inactive"
void
gcmarkwebonion_BM (struct garbcoll_stBM *gc)
{
  ASSERT_BM (gc && gc->gc_magic == GCMAGIC_BM);
  pthread_mutex_lock (&onionstack_mtx_bm);
  for (int ix = 0; ix < MAXNBWORKJOBS_BM; ix++)
    {
      if (onionstackinfo_bm[ix].ost_stkf != NULL
          && onionstackinfo_bm[ix].ost_thread != (pthread_t) 0)
        gcframemark_BM (gc, onionstackinfo_bm[ix].ost_stkf, 0);
    }
  pthread_mutex_unlock (&onionstack_mtx_bm);
}                               /* end gcmarkwebonion_BM */

//// end of file web_ONIONBM.c 
