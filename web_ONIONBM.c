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
extern void run_onionweb_BM (int nbjobs);
static onion *myonion_BM;



//////////////// for process queue
/// running processes; similar to gtkrunprocarr_BM in newgui_GTKBM.c
struct
{
  pid_t rp_pid;
  int rp_outpipe;
  const stringval_tyBM *rp_dirstrv;
  const node_tyBM *rp_cmdnodv;
  const closure_tyBM *rp_closv;
  objectval_tyBM *rp_bufob;
} onionrunprocarr_BM[MAXNBWORKJOBS_BM];

/// queued process commands, of nodes (dir, cmd, clos)
struct listtop_stBM *onionrunpro_list_BM;
pthread_mutex_t onionrunpro_mtx_BM = PTHREAD_MUTEX_INITIALIZER;

static void lock_runpro_mtx_at_BM (int lineno);
static void unlock_runpro_mtx_at_BM (int lineno);

void
lock_runpro_mtx_at_BM (int lineno)
{
  DBGPRINTFAT_BM (__FILE__, lineno, "lock_gtkrunpro_mtx_BM thrid=%ld",
                  (long) gettid_BM ());
  pthread_mutex_lock (&onionrunpro_mtx_BM);
}                               /* end lock_runpro_mtx_at_BM */


void
unlock_runpro_mtx_at_BM (int lineno)
{
  DBGPRINTFAT_BM (__FILE__, lineno, "unlock_gtkrunpro_mtx_BM thrid=%ld",
                  (long) gettid_BM ());
  pthread_mutex_unlock (&onionrunpro_mtx_BM);
}                               /* end lock_runpro_mtx_at_BM */

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
  strftime (nowtibuf, sizeof (nowtibuf), "%T", &nowtm);
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
      buf = calloc ((prime_above_BM (ln + 2) | 7) + 1, 1);
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
  if (_.dirstrv && !isstring_BM (_.dirstrv))
    FAILHERE (makenode1_BM (BMP_string, (value_tyBM) _.dirstrv));
  if (!isnode_BM (_.cmdnodv))
    FAILHERE (makenode1_BM (BMP_node, (value_tyBM) _.cmdnodv));
  if (!isclosure_BM (_.endclosv))
    FAILHERE (makenode1_BM (BMP_closure, (value_tyBM) _.cmdnodv));
  unsigned cmdlen = nodewidth_BM (_.cmdnodv);
  if (cmdlen == 0)
    FAILHERE (makenode1_BM (BMP_node, (value_tyBM) _.cmdnodv));
  for (unsigned aix = 0; aix < cmdlen; aix++)
    {
      _.curargv = nodenthson_BM (_.cmdnodv, aix);
      if (!isstring_BM (_.curargv))
        FAILHERE (makenode2_BM
                  (BMP_node, (value_tyBM) _.cmdnodv, taggedint_BM (aix)));
    }
  ASSERT_BM (nbworkjobs_BM >= MINNBWORKJOBS_BM
             && nbworkjobs_BM <= MAXNBWORKJOBS_BM);
#warning should nearly reproduce queue_process_BM from newgui_GTKBM.c
  FATAL_BM ("queue_process_BM unimplemented in web_ONIONBM");
failure:
#undef FAILHERE
  if (lockedproc)
    unlock_runpro_mtx_at_BM (__LINE__), lockedproc = false;
  DBGPRINTF_BM
    ("queue_process failure failin %d dirstr %s, cmdnod %s endclos %s, cause %s",
     failin,
     bytstring_BM (_.dirstrv), debug_outstr_value_BM ((value_tyBM) _.cmdnodv,
                                                      CURFRAME_BM, 0),
     debug_outstr_value_BM (_.endclosv, CURFRAME_BM, 0),
     debug_outstr_value_BM (_.causev, CURFRAME_BM, 0));
  _.errorv =
    makenode4_BM (k_queue_process, _.dirstrv, _.cmdnodv, _.endclosv,
                  _.causev);
  FAILURE_BM (failin, _.errorv, CURFRAME_BM);
#warning queue_process_BM unimplemented in web_ONIONBM
}                               /* end queue_process_BM */

////////////////////////////////////////////////////////////////
static onion_connection_status
custom_onion_handler_BM (void *clientdata,
                         onion_request * req, onion_response * resp);
void
run_onionweb_BM (int nbjobs)    // declared and used only in main_BM.c
{
  char *webhost = NULL;
  int webport = 0;
  int pos = -1;
  if (nbjobs < MINNBWORKJOBS_BM)
    nbjobs = MINNBWORKJOBS_BM;
  else if (nbjobs > MAXNBWORKJOBS_BM)
    nbjobs = MAXNBWORKJOBS_BM;
  if (!onion_web_base_BM)
    onion_web_base_BM = "localhost:8086";
  if (sscanf
      (onion_web_base_BM, "%m[.a-zA-Z0-9+-]:%d%n", &webhost, &webport,
       &pos) < 3 || pos < 0 || onion_web_base_BM[pos])
    FATAL_BM ("bad web base %s", onion_web_base_BM);
  myonion_BM = onion_new (O_THREADED | O_NO_SIGTERM | O_SYSTEMD | O_DETACHED);
  if (!myonion_BM)
    FATAL_BM ("failed to create onion");
  onion_set_max_threads (myonion_BM, nbjobs);
  DBGPRINTF_BM ("run_onionweb webhost '%s' webport %d", webhost, webport);
  if (webhost && webhost[0])
    onion_set_hostname (myonion_BM, webhost);
  char *lastcolon = strrchr (onion_web_base_BM, ':');
  if (lastcolon && isdigit (lastcolon[1]))
    onion_set_port (myonion_BM, lastcolon + 1);
  onion_handler *roothdl = onion_get_root_handler (myonion_BM);
  if (!roothdl)
    FATAL_BM ("failed to get onion root handler");
  char *webrootpath = NULL;
  if (asprintf (&webrootpath, "%s/webroot/", bismon_directory) < 0
      || !webrootpath || !webrootpath[0] || access (webrootpath, R_OK | X_OK))
    FATAL_BM ("failed to get or access webroot/ path - %m");
  onion_handler *filehdl = onion_handler_export_local_new (webrootpath);
  if (!filehdl)
    FATAL_BM ("failed to get onion webroot handler for %s", webrootpath);
  onion_handler_add (roothdl, filehdl);
  onion_url_add_handler (onion_root_url (myonion_BM),
                         "onion_status", onion_internal_status ());
  onion_handler *customhdl =
    onion_handler_new (custom_onion_handler_BM, NULL, NULL);
  onion_handler_add (roothdl, customhdl);
  ///
  /// should add our internal handlers
  ///
  int err = onion_listen (myonion_BM);  // since detached, returns now
  if (err)
    FATAL_BM ("failed to do onion_listen (err#%d / %s)", err, strerror (err));
  ///
  /// should add our event loop, at least related to queued processes
  /// (and their output pipes), to SIGCHLD and SIGTERM + SIGQUIT
  /// see https://groups.google.com/a/coralbits.com/d/msg/onion-dev/m-wH-BY2MA0/QJqLNcHvAAAJ
  /// and https://groups.google.com/a/coralbits.com/d/msg/onion-dev/ImjNf1EIp68/R37DW3mZAAAJ
#warning run_onionweb_BM unimplemented
  FATAL_BM ("run_onionweb_BM unimplemented, nbjobs %d webrootpath %s", nbjobs,
            webrootpath);
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
  if (ws->websess_userob)
    gcobjmark_BM (gc, ws->websess_userob);
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
custom_onion_handler_BM (void *_clientdata __attribute__ ((unused)),
                         onion_request * req, onion_response * resp)
{
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
#warning unimplemented custom_onion_handler_BM
  /// probably should return OCS_PROCESSED if handled, or OCS_NOT_PROCESSED, OCS_FORBIDDEN, OCS_INTERNAL_ERROR, etc...
  return OCS_NOT_PROCESSED;
}                               /* end custom_onion_handler_BM */

////////////////////////////////////////////////////////////////
