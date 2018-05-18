// file guicode_BM.c

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
#include "bismon.h"
#include "guicode_BM.const.h"


/// method to browse_in_object for object-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_23ViGouPnAg_15P5mpG9x3d);
value_tyBM
ROUTINEOBJNAME_BM (_23ViGouPnAg_15P5mpG9x3d)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the depth
 const value_tyBM arg3 __attribute__ ((unused)),        //
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  ASSERT_BM (isobject_BM (arg1));
  ASSERT_BM (istaggedint_BM (arg2));
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const objectval_tyBM * objbrows;
                 const setval_tyBM * setattrs; const objectval_tyBM * curattr;
                 value_tyBM curval;);
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  char objbroid[32];
  memset (objbroid, 0, sizeof (objbroid));
  idtocbuf32_BM (objid_BM (_.objbrows), objbroid);
  const char *objbroname = findobjectname_BM (_.objbrows);
  gtk_text_buffer_insert_with_tags      //
    (brobuf, &browserit_BM, "|\342\227\274 ",   /* U+25FC BLACK MEDIUM SQUARE ◼ */
     -1, miscomm_brotag_BM, NULL);
  if (objbroname)
    {
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,
                                        objbroname, -1, objname_brotag_BM,
                                        miscomm_brotag_BM, NULL);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, " = ", -1,
                                        miscomm_brotag_BM, NULL);
    }
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,
                                    objbroid, -1, objid_brotag_BM,
                                    objrefcomm_brotag_BM, NULL);
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, "\n ", -1,
                                    miscomm_brotag_BM, NULL);
  ///
  //// show hi&lo id and hash
  Dl_info di = { };
  if (_.objbrows->ob_rout
      && dladdr (_.objbrows->ob_rout, &di) && di.dli_fname && di.dli_sname)
    {
      char *commbuf = NULL;
      asprintf (&commbuf,
                "id:%lld,%lld; h:%d\n ... µ%s;%s @%p|",
                (long long) _.objbrows->ob_id.id_hi,
                (long long) _.objbrows->ob_id.id_lo,
                objecthash_BM (_.objbrows),
                di.dli_fname, di.dli_sname, (void *) _.objbrows->ob_rout);
      if (!commbuf)
        FATAL_BM ("asprintf failure in browse_in_object]object");

      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,
                                        commbuf, -1, miscomm_brotag_BM, NULL);
      free (commbuf), commbuf = NULL;
    }
  else
    {
      char idcomm[128];
      memset (idcomm, 0, sizeof (idcomm));
      snprintf (idcomm, sizeof (idcomm), "id:%lld,%lld; h:%d|",
                (long long) _.objbrows->ob_id.id_hi,
                (long long) _.objbrows->ob_id.id_lo,
                objecthash_BM (_.objbrows));
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,
                                        idcomm, -1, miscomm_brotag_BM, NULL);
    }
  gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  ///
  //// show mtime & space
  {
    char mbuf[64];
    double mtime = _.objbrows->ob_mtime;
    snprintf (mbuf, sizeof (mbuf), "!@ %.2f ", mtime);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, mbuf, -1, NULL, NULL);
    double now = clocktime_BM (CLOCK_REALTIME);
    time_t mtimt = (time_t) mtime;
    struct tm mtimtm = {
    };
    localtime_r (&mtimt, &mtimtm);
    char commbuf[96];
    memset (commbuf, 0, sizeof (commbuf));
    char spabuf[16];
    memset (spabuf, 0, sizeof (spabuf));
    switch (_.objbrows->ob_space)
      {
      case TransientSp_BM:
        strcpy (spabuf, "transient");
        break;
      case PredefSp_BM:
        strcpy (spabuf, "predefined");
        break;
      case GlobalSp_BM:
        strcpy (spabuf, "global");
        break;
      default:
        snprintf (spabuf, sizeof (spabuf), "user#%d", _.objbrows->ob_space);
        break;
      };
    if (mtime < now && mtime + 1.0e6 > now)     // a million second is about 11 days
      strftime (mbuf, sizeof (mbuf), "%a %d, %T %Z", &mtimtm);
    else if (mtime < now && mtime + 25e6 > now) // 25 million seconds is less than 10 months
      strftime (mbuf, sizeof (mbuf), "%a %d %b, %T %Z", &mtimtm);
    else
      strftime (mbuf, sizeof (mbuf), "%c", &mtimtm);
    snprintf (commbuf, sizeof (commbuf), "|mtim:%s space:%s|", mbuf, spabuf);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, commbuf, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    if (_.objbrows->ob_class)
      {
        gtk_text_buffer_insert (brobuf, &browserit_BM, "!$ ", -1);
        browse_value_BM ((const value_tyBM) _.objbrows->ob_class,
                         CURFRAME_BM, 2, 0);
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      };
  }
  ///
  //// show attributes
  {
    char commbuf[64];
    memset (commbuf, 0, sizeof (commbuf));
    _.setattrs = objsetattrs_BM (_.objbrows);
    unsigned nbattrs = setcardinal_BM (_.setattrs);
    snprintf (commbuf, sizeof (commbuf), "|%d attributes:|", nbattrs);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, commbuf, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    objectval_tyBM *tinyarr[TINYSIZE_BM] = {
    };
    objectval_tyBM **arr = (nbattrs < TINYSIZE_BM) ? tinyarr    //
      : calloc (prime_above_BM (nbattrs),
                sizeof (const objectval_tyBM *));
    if (!arr)
      FATAL_BM ("failed to calloc arr for %d attrs", nbattrs);
    for (unsigned ix = 0; ix < nbattrs; ix++)
      arr[ix] = setelemnth_BM (_.setattrs, ix);
    sortnamedobjarr_BM (arr, nbattrs);
    for (unsigned aix = 0; aix < nbattrs; aix++)
      {
        _.curattr = arr[aix];
        _.curval = objgetattr_BM (_.objbrows, _.curattr);
        gtk_text_buffer_insert (brobuf, &browserit_BM, "!: ", -1);
        browse_value_BM ((const value_tyBM) _.curattr,
                         CURFRAME_BM, maxdepth, 2);
        browsespacefordepth_BM (1);
        browse_value_BM ((const value_tyBM) _.curval,
                         CURFRAME_BM, maxdepth, 1);
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      };
    if (arr != tinyarr)
      free (arr), arr = NULL;
  }
  ///
  //// show the components
  {
    char commbuf[64];
    memset (commbuf, 0, sizeof (commbuf));
    unsigned nbcomps = objnbcomps_BM (_.objbrows);
    snprintf (commbuf, sizeof (commbuf), "|%d components:|", nbcomps);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, commbuf, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    for (unsigned cix = 0; cix < nbcomps; cix++)
      {
        gtk_text_buffer_insert (brobuf, &browserit_BM, "!& ", -1);
        snprintf (commbuf, sizeof (commbuf), "|#%d:|", cix);
        gtk_text_buffer_insert_with_tags (brobuf,
                                          &browserit_BM, commbuf, -1,
                                          miscomm_brotag_BM, NULL);
        _.curval = objgetcomp_BM (_.objbrows, cix);
        browsespacefordepth_BM (1);
        browse_value_BM ((const value_tyBM) _.curval,
                         CURFRAME_BM, maxdepth, 1);
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      }
  }
  ///
  // extra data
  extendedval_tyBM payl = objpayload_BM (_.objbrows);
  if (payl)
    {
      send1_BM ((const value_tyBM) _.objbrows, BMP_browse_data,
                CURFRAME_BM, taggedint_BM (maxdepth));
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  /// return itself
  LOCALRETURN_BM (_.objbrows);
}                               /* end  ROUTINEOBJNAME_BM (_23ViGouPnAg_15P5mpG9x3d) */





/// method to browse_value for object-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_0BAnB0xjs23_0WEOCOi5Nbe);
value_tyBM
ROUTINEOBJNAME_BM (_0BAnB0xjs23_0WEOCOi5Nbe)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_value for object-s _0BAnB0xjs23_0WEOCOi5Nbe");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const objectval_tyBM * objbrows;);
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  ASSERT_BM (maxdepth > 0);
  int curdepth = getint_BM (arg3);
  //  ASSERT_BM (curdepth <= maxdepth);
  const char *objnam = findobjectname_BM (_.objbrows);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  char idbuf[32];
  memset (idbuf, 0, sizeof (idbuf));
  idtocbuf32_BM (objid_BM (_.objbrows), idbuf);
  if (objnam)
    {
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        objnam, -1, objname_brotag_BM, NULL);
      if (curdepth < 2)
        {
          gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                            " |=", -1, objrefcomm_brotag_BM,
                                            NULL);
          gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                            idbuf,
                                            -1, objrefcomm_brotag_BM, NULL);
          gtk_text_buffer_insert_with_tags (brobuf,
                                            &browserit_BM, "|", -1,
                                            objrefcomm_brotag_BM, NULL);
        }
    }
  else
    {                           // anonymous
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        idbuf, -1, objid_brotag_BM, NULL);
    }
  LOCALRETURN_BM (_.objbrows);
}                               /* end  ROUTINEOBJNAME_BM (_0BAnB0xjs23_0WEOCOi5Nbe) */


/// method to browse_data for class-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_09DxyieS5Wz_7pkad4F88FA);
value_tyBM
ROUTINEOBJNAME_BM (_09DxyieS5Wz_7pkad4F88FA)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3 __attribute__ ((unused)),        //
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const objectval_tyBM * objbrows; const setval_tyBM * setsel;   //
                 const objectval_tyBM * cursel;
                 const objectval_tyBM * supercla;
                 value_tyBM curval;);
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for class-s _09DxyieS5Wz_7pkad4F88FA");
  _.objbrows = (const objectval_tyBM *) arg1;
  int depth = getint_BM (arg2);
  ASSERT_BM (isobject_BM ((const value_tyBM) _.objbrows));
  ASSERT_BM (objhasclassinfo_BM (_.objbrows));
  _.setsel = objgetclassinfosetofselectors_BM (_.objbrows);
  _.supercla = objgetclassinfosuperclass_BM (_.objbrows);
  unsigned nbmeth = setcardinal_BM (_.setsel);
  objectval_tyBM *tinyarr[TINYSIZE_BM] = {
  };
  objectval_tyBM **arr = (nbmeth < TINYSIZE_BM) ? tinyarr       //
    : calloc (prime_above_BM (nbmeth),
              sizeof (const objectval_tyBM *));
  if (!arr)
    FATAL_BM ("failed to calloc arr for %d methods", nbmeth);
  for (unsigned ix = 0; ix < nbmeth; ix++)
    arr[ix] = setelemnth_BM (_.setsel, ix);
  sortnamedobjarr_BM (arr, nbmeth);
  gtk_text_buffer_insert_with_tags (brobuf,
                                    &browserit_BM,
                                    "!~ class (~ ", -1, NULL, NULL);
  if (_.supercla)
    {
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM,
                                        "|superclass=|", -1,
                                        miscomm_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.supercla, CURFRAME_BM, 2, 0);
    }
  else
    {
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM,
                                        "|nosuperclass|", -1,
                                        miscomm_brotag_BM, NULL);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, " __\n", -1,
                                        NULL, NULL);
    }
  for (unsigned ix = 0; ix < nbmeth; ix++)
    {
      _.cursel = arr[ix];
      _.curval = (value_tyBM) objgetclassinfomethod_BM (_.objbrows, _.cursel);
      browsenewlinefordepth_BM (1);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, "~: ", -1, NULL, NULL);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM,
                                        "|selector:|", -1,
                                        miscomm_brotag_BM, NULL);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, " ", -1, NULL, NULL);
      browse_value_BM ((const value_tyBM) _.cursel, CURFRAME_BM, 2, 1);
      browsespacefordepth_BM (1);
      browse_value_BM ((const value_tyBM) _.curval, CURFRAME_BM, depth, 1);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, "\n", -1, NULL, NULL);
    }
  if (arr != tinyarr)
    free (arr), arr = NULL;
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, "~)\n", -1,
                                    NULL, NULL);
  LOCALRETURN_BM (_.objbrows);
}                               /* end ROUTINEOBJNAME_BM (_09DxyieS5Wz_7pkad4F88FA) */


/// method to browse_value for tuple-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_0B1PYH9bN34_3RZdP24AVyt);
value_tyBM
ROUTINEOBJNAME_BM (_0B1PYH9bN34_3RZdP24AVyt)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4_ __attribute__ ((unused)),
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  if (!istuple_BM (arg1))
    FATAL_BM
      ("non-tuple for  method to browse_value for tuple-s _0BAnB0xjs23_0WEOCOi5Nb");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const tupleval_tyBM * tupbrows;
                 const objectval_tyBM * objbrows;);
  _.tupbrows = (const tupleval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  int curdepth = getint_BM (arg3);
  int oboff = browse_show_start_offset_BM ();
  ASSERT_BM (curdepth <= maxdepth);
  int openoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "[", -1, nest_brotag_BM, NULL);
  unsigned tupsiz = tuplesize_BM (_.tupbrows);
  if (curdepth < maxdepth)
    {
      for (unsigned tix = 0; tix < tupsiz; tix++)
        {
          _.objbrows = tuplecompnth_BM (_.tupbrows, tix);
          if (tix > 0)
            browsespacefordepth_BM (curdepth + 1);
          browse_value_BM ((const value_tyBM) _.objbrows,
                           CURFRAME_BM, (maxdepth), (curdepth + 1));
        }
    }
  else
    {
      char msgbuf[64];
      memset (msgbuf, 0, sizeof (msgbuf));
      snprintf (msgbuf, sizeof (msgbuf), "|\342\200\246"        /*U+2026 HORIZONTAL ELLIPSIS … */
                " %d objects in tuple|", tupsiz);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        msgbuf, -1, toodeep_brotag_BM, NULL);
    }
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "]", -1, nest_brotag_BM, NULL);
  int closeoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  browse_add_parens_BM (openoff, closeoff, -1, 1, 1, 0, curdepth,
                        CURFRAME_BM);
  LOCALRETURN_BM (_.objbrows);
}                               /* end ROUTINEOBJNAME_BM (_0BAnB0xjs23_0WEOCOi5Nb)  */





/// method to browse_value for set-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_3rne4qbpnV9_0pywzeJp3Qr);
value_tyBM
ROUTINEOBJNAME_BM (_3rne4qbpnV9_0pywzeJp3Qr)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),        // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!isset_BM (arg1))
    FATAL_BM
      ("non-set for  method to browse_value for set-s _3rne4qbpnV9_0pywzeJp3Qr");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const setval_tyBM * setbrows;
                 const objectval_tyBM * objbrows;);
  _.setbrows = (const setval_tyBM *) arg1;
  unsigned setcard = setcardinal_BM (_.setbrows);
  int maxdepth = getint_BM (arg2);
  int curdepth = getint_BM (arg3);
  ASSERT_BM (curdepth <= maxdepth);
  int oboff = browse_show_start_offset_BM ();
  int openoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "{", -1, nest_brotag_BM, NULL);
  if (curdepth < maxdepth)
    {
      objectval_tyBM *tinyarr[TINYSIZE_BM] = {
      };
      objectval_tyBM **arr =    //
        (setcard < TINYSIZE_BM) ? tinyarr
        : calloc (setcard + 1, sizeof (const objectval_tyBM *));
      if (!arr)
        FATAL_BM ("calloc failed for %d elements", setcard);
      for (unsigned eix = 0; eix < setcard; eix++)
        arr[eix] = setelemnth_BM (_.setbrows, eix);
      sortnamedobjarr_BM (arr, setcard);
      for (unsigned eix = 0; eix < setcard; eix++)
        {
          _.objbrows = arr[eix];
          if (eix > 0)
            browsespacefordepth_BM (curdepth + 1);
          browse_value_BM ((const value_tyBM) _.objbrows,
                           CURFRAME_BM, (maxdepth), (curdepth + 1));
        }
      if (arr != tinyarr)
        free (arr), arr = NULL;
    }
  else
    {
      char msgbuf[64];
      memset (msgbuf, 0, sizeof (msgbuf));
      snprintf (msgbuf, sizeof (msgbuf), "|\342\200\246"        /*U+2026 HORIZONTAL ELLIPSIS … */
                " %d objects in set|", setcard);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        msgbuf, -1, toodeep_brotag_BM, NULL);
    }
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "}", -1, nest_brotag_BM, NULL);
  int closeoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  browse_add_parens_BM (openoff, closeoff, -1, 1, 1, 0, curdepth,
                        CURFRAME_BM);
  LOCALRETURN_BM (_.objbrows);
}                               /* end ROUTINEOBJNAME_BM (_3rne4qbpnV9_0pywzeJp3Qr)  */





/// method to browse_value for int-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_0HBMCM5CeLn_7L5YEV2jO7Y);
value_tyBM
ROUTINEOBJNAME_BM (_0HBMCM5CeLn_7L5YEV2jO7Y)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!istaggedint_BM (arg1))
    FATAL_BM
      ("non-int for method to browse_value for int-s _0HBMCM5CeLn_7L5YEV2jO7Y");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
    );
  intptr_t i = getint_BM (arg1);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  char ibuf[32];
  memset (ibuf, 0, sizeof (ibuf));
  snprintf (ibuf, sizeof (ibuf), "%lld", (long long) i);
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    ibuf, -1, num_brotag_BM, NULL);
  LOCALRETURN_BM (arg1);
}                               /* end ROUTINEOBJNAME_BM (_0HBMCM5CeLn_7L5YEV2jO7Y)  */



/// method to browse_value for string-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_63ZPkXUI2Uv_6Cp3qmh6Uud);
#define WANTEDLINEWIDTH_BM 64
value_tyBM
ROUTINEOBJNAME_BM (_63ZPkXUI2Uv_6Cp3qmh6Uud)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!isstring_BM (arg1))
    FATAL_BM
      ("non-string for method to browse_value for string-s _63ZPkXUI2Uv_6Cp3qmh6Uud");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const stringval_tyBM * strbrows;);
  _.strbrows = arg1;
  int maxdepth = getint_BM (arg2);
  int curdepth = getint_BM (arg3);
  ASSERT_BM (curdepth <= maxdepth);
  unsigned l = lenstring_BM (_.strbrows);
  const char *str = bytstring_BM (_.strbrows);
  if (l >= WANTEDLINEWIDTH_BM / 2)
    browsespacefordepth_BM (curdepth);
  int oboff = browse_show_start_offset_BM ();
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  int openoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "\"", -1, stresc_brotag_BM, NULL);
  int ccnt = 0;
  int linecnt = 0;
#define WANTEDLINEWIDTH_BM 64
  for (const char *pc = str; *pc; pc = g_utf8_next_char (pc))
    {
      gunichar uc = g_utf8_get_char (pc);
      ccnt++;
      linecnt++;
#define ADDESCAPESTR_BM(S)						\
    gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,	\
				      (S), -1, stresc_brotag_BM, NULL)
      switch (uc)
        {
        case '\"':
          ADDESCAPESTR_BM ("\\\"");
          break;
        case '\'':
          ADDESCAPESTR_BM ("\\\'");
          break;
        case '\a':
          ADDESCAPESTR_BM ("\\a");
          break;
        case '\b':
          ADDESCAPESTR_BM ("\\b");
          break;
        case '\f':
          ADDESCAPESTR_BM ("\\f");
          break;
        case '\n':
          if (linecnt > 3 * WANTEDLINEWIDTH_BM / 4 && pc[1] && pc[2])
            {
              ADDESCAPESTR_BM ("\"+");
              browsenewlinefordepth_BM (curdepth);
              ADDESCAPESTR_BM ("\"");
              linecnt = 0;
            }
          else
            ADDESCAPESTR_BM ("\\n");
          break;
        case '\r':
          ADDESCAPESTR_BM ("\\r");
          break;
        case '\t':
          ADDESCAPESTR_BM ("\\t");
          break;
        case '\v':
          ADDESCAPESTR_BM ("\\v");
          break;
        case '\033' /*ESCAPE*/:
          ADDESCAPESTR_BM ("\\e");
          break;
        default:
          if (linecnt > 3 * WANTEDLINEWIDTH_BM / 4 && pc[1] && pc[2]
              && g_unichar_isspace (uc))
            {
              ADDESCAPESTR_BM ("\"&");
              browsenewlinefordepth_BM (curdepth);
              ADDESCAPESTR_BM ("\"");
              linecnt = 0;
            }
          else if (linecnt > WANTEDLINEWIDTH_BM && pc[1])
            {
              ADDESCAPESTR_BM ("\"&");
              browsenewlinefordepth_BM (curdepth);
              ADDESCAPESTR_BM ("\"");
              linecnt = 0;
            }
          if (g_unichar_isprint (uc))
            {
              char ubuf[8];
              memset (ubuf, 0, sizeof (ubuf));
              g_unichar_to_utf8 (uc, ubuf);
              gtk_text_buffer_insert_with_tags (brobuf,
                                                &browserit_BM, ubuf, -1,
                                                str_brotag_BM, NULL);
            }
          else if (uc < 0xffff)
            {
              char ubuf[24];
              memset (ubuf, 0, sizeof (ubuf));
              snprintf (ubuf, sizeof (ubuf), "\\u%04x", uc);
              ADDESCAPESTR_BM (ubuf);
            }
          else
            {
              char ubuf[24];
              memset (ubuf, 0, sizeof (ubuf));
              snprintf (ubuf, sizeof (ubuf), "\\U%08x", uc);
              ADDESCAPESTR_BM (ubuf);
            }
          break;
        }
    }                           /* end for pc */
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "\"", -1, stresc_brotag_BM, NULL);
  int closeoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  browse_add_parens_BM (openoff, closeoff, -1, 1, 1, 0, curdepth,
                        CURFRAME_BM);
  if (ccnt >= WANTEDLINEWIDTH_BM / 2)
    browsespacefordepth_BM (curdepth);
  LOCALRETURN_BM (arg1);
}                               /* end ROUTINEOBJNAME_BM (_63ZPkXUI2Uv_6Cp3qmh6Uud) */



/// method to browse_value for node-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_7fJKfG4SN0U_1QTu5J832xg);
value_tyBM
ROUTINEOBJNAME_BM (_7fJKfG4SN0U_1QTu5J832xg)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),        // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!isnode_BM (arg1))
    FATAL_BM
      ("non-node for method to browse_value for node-s _7fJKfG4SN0U_1QTu5J832xg");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const node_tyBM * nodbrows;
                 const objectval_tyBM * connob; value_tyBM curson;);
  _.nodbrows = arg1;
  int maxdepth = getint_BM (arg2);
  int curdepth = getint_BM (arg3);
  ASSERT_BM (curdepth <= maxdepth);
  unsigned nw = nodewidth_BM ((const value_tyBM) _.nodbrows);
  _.connob = nodeconn_BM ((const value_tyBM) _.nodbrows);
  ASSERT_BM (isobject_BM ((const value_tyBM) _.connob));
  int oboff = browse_show_start_offset_BM ();
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  int xtraoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "*", -1, nest_brotag_BM, NULL);
  gtk_text_buffer_insert (brobuf, &browserit_BM, " ", 1);
  browse_value_BM ((const value_tyBM) _.connob, CURFRAME_BM,
                   (maxdepth), (curdepth + 1));
  browsespacefordepth_BM (curdepth + 1);
  int openoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "(", -1, nest_brotag_BM, NULL);
  if (curdepth < maxdepth)
    {
      for (unsigned six = 0; six < nw; six++)
        {
          _.curson = nodenthson_BM ((const value_tyBM) _.nodbrows, six);
          if (six > 0)
            browsespacefordepth_BM (curdepth + 1);
          browse_value_BM ((const value_tyBM) _.curson,
                           CURFRAME_BM, (maxdepth), (curdepth + 1));
        }
    }
  else
    {
      char msgbuf[64];
      memset (msgbuf, 0, sizeof (msgbuf));
      snprintf (msgbuf, sizeof (msgbuf), "|\342\200\246"        /*U+2026 HORIZONTAL ELLIPSIS … */
                " %d sons in node|", nw);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        msgbuf, -1, toodeep_brotag_BM, NULL);
    }
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    ")", -1, nest_brotag_BM, NULL);
  int closeoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  browse_add_parens_BM (openoff, closeoff, xtraoff, 1, 1, 1, curdepth,
                        CURFRAME_BM);
  LOCALRETURN_BM (arg1);
}                               /* end ROUTINEOBJNAME_BM (_7fJKfG4SN0U_1QTu5J832xg) */





/// method to browse_value for closure-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_7CohjJ9tkfZ_4UMAIZCgwac);
value_tyBM
ROUTINEOBJNAME_BM (_7CohjJ9tkfZ_4UMAIZCgwac)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3,         // the current depth
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  if (!isclosure_BM (arg1))
    FATAL_BM
      ("non-closure for method to browse_value for closure-s _7CohjJ9tkfZ_4UMAIZCgwac");
  ASSERT_BM (istaggedint_BM (arg2));
  ASSERT_BM (istaggedint_BM (arg3));
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const closure_tyBM * clobrows;
                 const objectval_tyBM * connob; value_tyBM curson;);
  _.clobrows = arg1;
  int maxdepth = getint_BM (arg2);
  int curdepth = getint_BM (arg3);
  ASSERT_BM (curdepth <= maxdepth);
  unsigned cw = closurewidth_BM ((const value_tyBM) _.clobrows);
  _.connob = closureconn_BM ((const value_tyBM) _.clobrows);
  ASSERT_BM (isobject_BM ((const value_tyBM) _.connob));
  int oboff = browse_show_start_offset_BM ();
  int xtraoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "%", -1, nest_brotag_BM, NULL);
  gtk_text_buffer_insert (brobuf, &browserit_BM, " ", 1);
  browse_value_BM ((const value_tyBM) _.connob, CURFRAME_BM,
                   (maxdepth), (curdepth + 1));
  browsespacefordepth_BM (curdepth + 1);
  int openoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    "(", -1, nest_brotag_BM, NULL);
  if (curdepth < maxdepth)
    {
      for (unsigned cix = 0; cix < cw; cix++)
        {
          _.curson = closurenthson_BM ((const value_tyBM) _.clobrows, cix);
          if (cix > 0)
            browsespacefordepth_BM (curdepth + 1);
          browse_value_BM ((const value_tyBM) _.curson,
                           CURFRAME_BM, (maxdepth), (curdepth + 1));
        }
    }
  else
    {
      char msgbuf[64];
      memset (msgbuf, 0, sizeof (msgbuf));
      snprintf (msgbuf, sizeof (msgbuf), "|\342\200\246"        /*U+2026 HORIZONTAL ELLIPSIS … */
                " %d sons in closure|", cw);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        msgbuf, -1, toodeep_brotag_BM, NULL);
    }
  gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,      //
                                    ")", -1, nest_brotag_BM, NULL);
  int closeoff = gtk_text_iter_get_offset (&browserit_BM) - oboff;
  browse_add_parens_BM (openoff, closeoff, xtraoff, 1, 1, 1, curdepth,
                        CURFRAME_BM);
  LOCALRETURN_BM (arg1);
}                               /* end ROUTINEOBJNAME_BM ( _7CohjJ9tkfZ_4UMAIZCgwac) */



/// method to browse_data for basiclo_function-s
extern objrout_sigBM ROUTINEOBJNAME_BM (_9zpvXnTuDeB_2B7ZiBtN8fA);
value_tyBM
ROUTINEOBJNAME_BM (_9zpvXnTuDeB_2B7ZiBtN8fA)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3 __attribute__ ((unused)), const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const objectval_tyBM * objbrows;
                 value_tyBM curval;);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for basiclo_function-s _9zpvXnTuDeB_2B7ZiBtN8fA");
  _.objbrows = (const objectval_tyBM *) arg1;
  int depth = getint_BM (arg2);
  ASSERT_BM (isobject_BM ((const value_tyBM) _.objbrows));
  _.curval = objpayload_BM (_.objbrows);
  int tyval = valtype_BM (_.curval);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  if (tyval == tyInt_BM
      || (tyval >= type_FIRST_BM && tyval <= type_LASTREAL_BM))
    {
      char vcommbuf[40];
      memset (vcommbuf, 0, sizeof (vcommbuf));
      snprintf (vcommbuf, sizeof (vcommbuf), "|valdata h=%u:| ",
                valhash_BM (_.curval));
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, vcommbuf, -1,
                                        miscomm_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.curval, CURFRAME_BM, depth, 0);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    };
  LOCALRETURN_BM (_.objbrows);
}                               /* end ROUTINE _9zpvXnTuDeB_2B7ZiBtN8fA */



/// method to browse_data°hset_object
extern objrout_sigBM ROUTINEOBJNAME_BM (_4IshjBIv6ol_5korHKUIjeK);
value_tyBM
ROUTINEOBJNAME_BM (_4IshjBIv6ol_5korHKUIjeK)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3 __attribute__ ((unused)),        //
 const value_tyBM arg4 __attribute__ ((unused)),        //
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 const objectval_tyBM * objbrows; value_tyBM curval;
                 const objectval_tyBM * curkeyob; value_tyBM curkeyval;
                 const setval_tyBM * setk;
    );
  if (!isobject_BM (arg1))
    FATAL_BM ("non-object for method to browse_data for hset_object");
  _.objbrows = (const objectval_tyBM *) arg1;
  int depth = getint_BM (arg2);
  ASSERT_BM (isobject_BM ((const value_tyBM) _.objbrows));
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  _.curval = objpayload_BM (_.objbrows);
  if (!_.curval)
    {
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, "|no hset|", -1,
                                        miscomm_brotag_BM, NULL);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      LOCALRETURN_BM (_.objbrows);
    }
  int tyval = valtype_BM (_.curval);
  WEAKASSERT_BM (tyval == typayl_hashsetobj_BM);
  _.setk = hashsetobj_to_set_BM (_.curval);
  char commbuf[32];
  memset (commbuf, 0, sizeof (commbuf));
  snprintf (commbuf, sizeof (commbuf), "|hset %d:|", setcardinal_BM (_.setk));
  gtk_text_buffer_insert_with_tags (brobuf,
                                    &browserit_BM, commbuf, -1,
                                    miscomm_brotag_BM, NULL);
  gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  browse_value_BM ((const value_tyBM) _.setk, CURFRAME_BM, depth, 0);
  gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°hset_object _4IshjBIv6ol_5korHKUIjeK */




////////////////////////////////////////////////////////////////

/// command handler for put
extern objrout_sigBM ROUTINEOBJNAME_BM (_1Xc5XJ7S5r7_3nYIzlf2XAw);
value_tyBM
ROUTINEOBJNAME_BM (_1Xc5XJ7S5r7_3nYIzlf2XAw)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the attr or index
 const value_tyBM arg3,         // the comp
 const value_tyBM arg4_ __attribute__ ((unused)),       // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * recv; value_tyBM attv;
                 value_tyBM compv;
                 value_tyBM reasonv;
    );
  objectval_tyBM *k_failure_non_object = BMK_6yWldsq3Rmk_01WqTfwSIDV;
  objectval_tyBM *k_failure_bad_attribute = BMK_4GumRf8w4jT_6lfDDJ5Y3TH;
  if (!isobject_BM (arg1))
    {
      FAILURE_BM (__LINE__, k_failure_non_object, CURFRAME_BM);
    }
  _.recv = arg1;
  _.attv = arg2;
  _.compv = arg3;
  NONPRINTF_BM ("commandhandler°put recv=%s attv=%s compv=%s", objectdbg_BM (_.recv), debug_outstr_value_BM    //
                (_.attv, CURFRAME_BM, 0), debug_outstr_value_BM //
                (_.compv, CURFRAME_BM, 0));
  if (isobject_BM (_.attv))
    {
      objlock_BM (_.recv);
      if (_.compv)
        objputattr_BM (_.recv, _.attv, _.compv);
      else
        objremoveattr_BM (_.recv, _.attv);
      objtouchnow_BM (_.recv);
      objunlock_BM (_.recv);
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("In object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_puts_message_BM (" put attribute ");
          log_object_message_BM (objectcast_BM (_.attv));
          log_puts_message_BM (".");
          log_end_message_BM ();
        }
      LOCALRETURN_BM (_.recv);
    }
  else if (istaggedint_BM (_.attv))
    {
      int attix = getint_BM (_.attv);
      objlock_BM (_.recv);
      objputcomp_BM (_.recv, attix, _.compv);
      objtouchnow_BM (_.recv);
      objunlock_BM (_.recv);
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("In object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_printf_message_BM (" put component#%d.", attix);
          log_end_message_BM ();
        }
      LOCALRETURN_BM (_.recv);
    }
  else
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("In object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_printf_message_BM
            (" bad attribute %s to put.",
             debug_outstr_value_BM (_.attv, CURFRAME_BM, 0));
          log_end_message_BM ();
        }
      FAILURE_BM (__LINE__, k_failure_bad_attribute, CURFRAME_BM);
    }
  LOCALRETURN_BM (NULL);
}                               /* end  command handler for put _1Xc5XJ7S5r7_3nYIzlf2XAw */


/// command handler for add
extern objrout_sigBM ROUTINEOBJNAME_BM (_5v5ChlG1IYh_1Pv87MZJFPl);
value_tyBM
ROUTINEOBJNAME_BM (_5v5ChlG1IYh_1Pv87MZJFPl)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the comp
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       // 
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * recv;
                 value_tyBM compv;
    );
  ASSERT_BM (pthread_self () == mainthreadid_BM);
  objectval_tyBM *k_failure_non_object = BMK_6yWldsq3Rmk_01WqTfwSIDV;
  if (!isobject_BM (arg1))
    {
      FAILURE_BM (__LINE__, k_failure_non_object, CURFRAME_BM);
    }
  _.recv = arg1;
  _.compv = arg2;
  NONPRINTF_BM ("commandhandler°add recv=%s compv=%s",
                objectdbg_BM (_.recv),
                debug_outstr_value_BM (_.compv, CURFRAME_BM, 0));
  int oblen = 0;
  objlock_BM (_.recv);
  objappendcomp_BM (_.recv, _.compv);
  objtouchnow_BM (_.recv);
  oblen = objnbcomps_BM (_.recv);
  objunlock_BM (_.recv);
  log_begin_message_BM ();
  log_puts_message_BM ("In object ");
  log_object_message_BM (objectcast_BM (_.recv));
  log_printf_message_BM (" added component#%d.", oblen);
  log_end_message_BM ();
  LOCALRETURN_BM (_.recv);
}                               /* end commandhandler°add _5v5ChlG1IYh_1Pv87MZJFPl */

/// command handler for remove
extern objrout_sigBM ROUTINEOBJNAME_BM (_0zf6nSKwSlU_6Cv3LMh1MmV);
value_tyBM
ROUTINEOBJNAME_BM (_0zf6nSKwSlU_6Cv3LMh1MmV)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the attr or index
 const value_tyBM arg3_ __attribute__ ((unused)),       // 
 const value_tyBM arg4_ __attribute__ ((unused)),       // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * recv; value_tyBM attv;
    );
  objectval_tyBM *k_failure_non_object = BMK_6yWldsq3Rmk_01WqTfwSIDV;
  objectval_tyBM *k_failure_bad_attribute = BMK_4GumRf8w4jT_6lfDDJ5Y3TH;
  if (!isobject_BM (arg1))
    {
      FAILURE_BM (__LINE__, k_failure_non_object, CURFRAME_BM);
    }
  _.recv = arg1;
  _.attv = arg2;
  NONPRINTF_BM ("commandhandler°remove recv=%s attv=%s",
                objectdbg_BM (_.recv),
                debug_outstr_value_BM (_.attv, CURFRAME_BM, 0));
  if (isobject_BM (_.attv))
    {
      objlock_BM (_.recv);
      objremoveattr_BM (_.recv, _.attv);
      objtouchnow_BM (_.recv);
      objunlock_BM (_.recv);
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("In object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_puts_message_BM (" remove attribute ");
          log_object_message_BM (objectcast_BM (_.attv));
          log_puts_message_BM (".");
          log_end_message_BM ();
        }
      LOCALRETURN_BM (_.recv);
    }
  else if (istaggedint_BM (_.attv))
    {
      int attix = getint_BM (_.attv);
      objlock_BM (_.recv);
      objremoveonecomp_BM (_.recv, attix);
      objunlock_BM (_.recv);
      log_begin_message_BM ();
      log_puts_message_BM ("In object ");
      log_object_message_BM (objectcast_BM (_.recv));
      log_printf_message_BM (" remove component#%d.", attix);
      log_end_message_BM ();
      LOCALRETURN_BM (_.recv);
    }
  else
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("In object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_printf_message_BM
            (" bad attribute %s to remove.",
             debug_outstr_value_BM (_.attv, CURFRAME_BM, 0));
          log_end_message_BM ();
        }
      FAILURE_BM (__LINE__, k_failure_bad_attribute, CURFRAME_BM);
    }
  LOCALRETURN_BM (NULL);
}                               /* end  command handler for remove _0zf6nSKwSlU_6Cv3LMh1MmV  */


/// command handler for init_class
extern objrout_sigBM ROUTINEOBJNAME_BM (_797zacMjvvt_3I2uxNJRfdq);
value_tyBM
ROUTINEOBJNAME_BM (_797zacMjvvt_3I2uxNJRfdq)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the superclass
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * recv; objectval_tyBM * superob;
    );
  objectval_tyBM *k_failure_non_object = BMK_6yWldsq3Rmk_01WqTfwSIDV;
  objectval_tyBM *k_failure_bad_attribute = BMK_4GumRf8w4jT_6lfDDJ5Y3TH;
  objectval_tyBM *k_failure_bad_class = BMK_797zacMjvvt_3I2uxNJRfdq;
  ASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    {
      FAILURE_BM (__LINE__, k_failure_non_object, CURFRAME_BM);
    }
  if (!isobject_BM (arg2))
    {
      FAILURE_BM (__LINE__, k_failure_bad_class, CURFRAME_BM);
    };
  _.recv = (objectval_tyBM *) arg1;
  _.superob = (objectval_tyBM *) arg2;
  bool badsuper = false;
  {
    objlock_BM (_.superob);
    if (objclass_BM (_.superob) != BMP_class)
      badsuper = true;
    objunlock_BM (_.superob);
  }
  if (badsuper)
    {
      FAILURE_BM (__LINE__, k_failure_bad_class, CURFRAME_BM);
    };
  objlock_BM (_.recv);
  objputclass_BM (_.recv, BMP_class);
  objputclassinfo_BM (_.recv, _.superob);
  objtouchnow_BM (_.recv);
  objunlock_BM (_.recv);
  if (pthread_self () == mainthreadid_BM)
    {
      log_begin_message_BM ();
      log_puts_message_BM ("Object ");
      log_object_message_BM (objectcast_BM (_.recv));
      log_puts_message_BM (" made class of super ");
      log_object_message_BM (_.superob);
      log_puts_message_BM (".");
      log_end_message_BM ();
    }
  LOCALRETURN_BM (_.recv);
}                               /* end commandhandler°init_class _797zacMjvvt_3I2uxNJRfdq  */


/// command handler for put_method
extern objrout_sigBM ROUTINEOBJNAME_BM (_9QuovXgtk9K_17pMbAD0XmX);
value_tyBM
ROUTINEOBJNAME_BM (_9QuovXgtk9K_17pMbAD0XmX)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the selector
 const value_tyBM arg3,         // the closure
 const value_tyBM arg4_ __attribute__ ((unused)),       // 
 const quasinode_tyBM * restargs __attribute__ ((unused)))
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * recv; objectval_tyBM * obselv;
                 value_tyBM closv;
    );
  objectval_tyBM *k_failure_non_object = BMK_6yWldsq3Rmk_01WqTfwSIDV;
  objectval_tyBM *k_failure_bad_class = BMK_797zacMjvvt_3I2uxNJRfdq;
  objectval_tyBM *k_failure_bad_closure = BMK_373gFe8m21E_47xzvCGxpI9;
  objectval_tyBM *k_failure_bad_selector = BMK_9KxVIxneLrM_8xLQNri9PmJ;
  ASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    {
      FAILURE_BM (__LINE__, k_failure_non_object, CURFRAME_BM);
    }
  _.recv = (objectval_tyBM *) arg1;
  if (!isobject_BM (arg2))
    {
      FAILURE_BM (__LINE__, k_failure_bad_selector, CURFRAME_BM);
    }
  _.obselv = arg2;
  if (arg3 && !isclosure_BM (arg3))
    {
      FAILURE_BM (__LINE__, k_failure_bad_closure, CURFRAME_BM);
    }
  _.closv = arg3;
  bool badclass = false;
  char *actword = NULL;
  objlock_BM (_.recv);
  if (objhasclassinfo_BM (_.recv))
    {
      if (_.closv)
        {
          objclassinfoputmethod_BM (_.recv, _.obselv, _.closv);
          actword = "put";
        }
      else
        {
          objclassinforemovemethod_BM (_.recv, _.obselv);
          actword = "remove";
        }
    }
  else
    badclass = true;
  objunlock_BM (_.recv);
  if (badclass)
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("Object ");
          log_object_message_BM (objectcast_BM (_.recv));
          log_puts_message_BM (" is not a class.");
          log_end_message_BM ();
        }
      FAILURE_BM (__LINE__, k_failure_bad_class, CURFRAME_BM);
    }
  if (pthread_self () == mainthreadid_BM)
    {
      log_begin_message_BM ();
      log_puts_message_BM ("In object ");
      log_object_message_BM (objectcast_BM (_.recv));
      log_printf_message_BM (" did %s method for ", actword);
      log_object_message_BM (objectcast_BM (_.obselv));
      log_puts_message_BM (".");
      log_end_message_BM ();
    }
  LOCALRETURN_BM (_.recv);
}                               /* end commandhandler°put_method _9QuovXgtk9K_17pMbAD0XmX */



// method browse_data°list_object _2EtVNhr2mHz_8CsOQJdYeCE
extern objrout_sigBM ROUTINEOBJNAME_BM (_2EtVNhr2mHz_8CsOQJdYeCE);

value_tyBM
ROUTINEOBJNAME_BM (_2EtVNhr2mHz_8CsOQJdYeCE)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_2EtVNhr2mHz_8CsOQJdYeCE,
                 value_tyBM resultv;    //
                 objectval_tyBM * objbrows;     //
                 value_tyBM nodv;       //
                 value_tyBM cursonv;    //
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for list_object-s _2EtVNhr2mHz_8CsOQJdYeCE");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhaslistpayl_BM (_.objbrows));
  _.nodv = objlisttonodepayl_BM (_.objbrows, BMP_node);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  int nodlen = nodewidth_BM (_.nodv);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|list %d:|", nodlen);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  for (int ix = 0; ix < nodlen; ix++)
    {
      if (ix > 0 && ix % 10 == 0)
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      _.cursonv = nodenthson_BM (_.nodv, ix);
      char bufmsg[48];
      memset (bufmsg, 0, sizeof (bufmsg));
      snprintf (bufmsg, sizeof (bufmsg), "\342\255\231 %d: ",   //U+2B59 HEAVY CIRCLED SALTIRE ⭙
                ix);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, bufmsg, -1,
                                        nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.cursonv,
                       CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|end list of %d|", nodlen);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°list_object _2EtVNhr2mHz_8CsOQJdYeCE */



//browse_data°assoc_object _0f1S28bCE26_3X6Vpz9lG4A
extern objrout_sigBM ROUTINEOBJNAME_BM (_0f1S28bCE26_3X6Vpz9lG4A);

value_tyBM
ROUTINEOBJNAME_BM (_0f1S28bCE26_3X6Vpz9lG4A)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         //
 const value_tyBM arg2,         //
 const value_tyBM arg3,         //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_0f1S28bCE26_3X6Vpz9lG4A,
                 value_tyBM resultv; const objectval_tyBM * objbrows;
                 value_tyBM setv; value_tyBM tupv;
                 const objectval_tyBM * curobj; value_tyBM curvalv;
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for assoc_object-s _0f1S28bCE26_3X6Vpz9lG4A");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhasassocpayl_BM (_.objbrows));
  _.setv = objassocsetattrspayl_BM (_.objbrows);
  _.tupv = settonamedsortedtuple_BM (_.setv);
  int nbelem = tuplesize_BM (_.tupv);
  ASSERT_BM (nbelem == setcardinal_BM (_.setv));
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|assoc %d:|", nbelem);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  for (int ix = 0; ix < nbelem; ix++)
    {
      if (ix > 0 && ix % 10 == 0)
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      _.curobj = tuplecompnth_BM (_.tupv, ix);
      _.curvalv = objassocgetattrpayl_BM (_.objbrows, _.curobj);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, "\342\210\230 ", //U+2218 RING OPERATOR ∘
                                        -1, nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.curobj, CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, " \342\206\222 ",        //U+2192 RIGHTWARDS ARROW →
                                        -1, nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.curvalv,
                       CURFRAME_BM, maxdepth, 2);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|end assoc %d|", nbelem);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°assoc_object  _0f1S28bCE26_3X6Vpz9lG4A */



/////
// browse_data°dict_object _6xPQJolJkUw_2jCiJ3IOKXE
extern objrout_sigBM ROUTINEOBJNAME_BM (_6xPQJolJkUw_2jCiJ3IOKXE);

value_tyBM
ROUTINEOBJNAME_BM (_6xPQJolJkUw_2jCiJ3IOKXE)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_6xPQJolJkUw_2jCiJ3IOKXE,
                 value_tyBM resultv; const objectval_tyBM * objbrows;
                 value_tyBM nodv; value_tyBM cursonv; value_tyBM curvalv;
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for dict_object-s _6xPQJolJkUw_2jCiJ3IOKXE");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhasdictpayl_BM (_.objbrows));
  _.nodv = objdictnodeofkeyspayl_BM (_.objbrows, BMP_node);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  int nbnames = nodewidth_BM (_.nodv);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|dict %d:|", nbnames);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  for (int ix = 0; ix < nbnames; ix++)
    {
      if (ix > 0 && ix % 10 == 0)
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      _.cursonv = nodenthson_BM (_.nodv, ix);
      WEAKASSERT_BM (isstring_BM (_.cursonv));
      _.curvalv = objdictgetpayl_BM (_.objbrows, _.cursonv);
      WEAKASSERT_BM (_.curvalv != NULL);
      char bufmsg[48];
      memset (bufmsg, 0, sizeof (bufmsg));
      snprintf (bufmsg, sizeof (bufmsg), "\342\257\216 %d: ",   //U+2BCE WHITE FOUR POINTED CUSP ⯎
                ix);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, bufmsg, -1,
                                        nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.cursonv,
                       CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        " \342\206\246 ", -1,   // U+21A6 RIGHTWARDS ARROW FROM BAR ↦
                                        nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.curvalv,
                       CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|end dict of %d|", nbnames);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°dict_object _6xPQJolJkUw_2jCiJ3IOKXE */



//////
// browse_data°sbuf_object _8L6mTIICJRt_4RccFDlgACX

extern objrout_sigBM ROUTINEOBJNAME_BM (_8L6mTIICJRt_4RccFDlgACX);

value_tyBM
ROUTINEOBJNAME_BM (_8L6mTIICJRt_4RccFDlgACX)    // browse_data°sbuf_object 
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // the reciever
 const value_tyBM arg2,         // the browse maxdepth
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_8L6mTIICJRt_4RccFDlgACX,
                 value_tyBM resultv;
                 const objectval_tyBM * objbrows;
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for sbuf_object-s _8L6mTIICJRt_4RccFDlgACX");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhasstrbufferpayl_BM (_.objbrows));
  const char *strsb = objstrbufferbytespayl_BM (_.objbrows);
  int bylen = objstrbufferlengthpayl_BM (_.objbrows);
  int unilen = g_utf8_strlen (strsb, bylen);
  int nblines = 0;
  for (char *pc = strsb, *eol = NULL; pc != NULL && *pc != 0;
       pc = eol ? eol + 1 : NULL)
    {
      eol = strchr (pc, '\n');
      nblines++;
    }
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg),
              "|sbuf %d unicodes, %d bytes, %d lines:|", unilen, bylen,
              nblines);
    gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, strsb, bylen,
                                      str_brotag_BM, NULL);
    if (bylen > 0 && strsb[bylen - 1] == '\n')
      {
        memset (bufmsg, 0, sizeof (bufmsg));
        snprintf (bufmsg, sizeof (bufmsg), "|:endsbuf %d|", unilen);
        gtk_text_buffer_insert_with_tags (brobuf,
                                          &browserit_BM, bufmsg, -1,
                                          miscomm_brotag_BM, NULL);
      }
    else
      {
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
        snprintf (bufmsg, sizeof (bufmsg), "|!endsbuf %d|", unilen);
        gtk_text_buffer_insert_with_tags (brobuf,
                                          &browserit_BM, bufmsg, -1,
                                          miscomm_brotag_BM, NULL);
      }
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°sbuf_object  _8L6mTIICJRt_4RccFDlgACX */



/////
// browse_data°hashsetval_object _9dKLCRPRn9Z_1tczFz5weBe
extern objrout_sigBM ROUTINEOBJNAME_BM (_9dKLCRPRn9Z_1tczFz5weBe);

value_tyBM
ROUTINEOBJNAME_BM (_9dKLCRPRn9Z_1tczFz5weBe)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         //
 const value_tyBM arg2,         //
 const value_tyBM arg3,         //
 const value_tyBM arg4,         //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_9dKLCRPRn9Z_1tczFz5weBe,
                 value_tyBM resultv; const objectval_tyBM * objbrows;
                 value_tyBM nodv; value_tyBM cursonv;
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for hashsetval_object-s _9dKLCRPRn9Z_1tczFz5weBe");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhashashsetvalpayl_BM (_.objbrows));
  _.nodv = objhashsetvalmakenodepayl_BM (_.objbrows, BMP_node);
  WEAKASSERT_BM (isnode_BM (_.nodv));
  int nbvals = nodewidth_BM (_.nodv);
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|hashsetval %d:|", nbvals);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  for (int ix = 0; ix < nbvals; ix++)
    {
      if (ix > 0 && ix % 10 == 0)
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      _.cursonv = nodenthson_BM (_.nodv, ix);
      WEAKASSERT_BM (_.cursonv != NULL);
      char bufmsg[48];
      memset (bufmsg, 0, sizeof (bufmsg));
      snprintf (bufmsg, sizeof (bufmsg), "\342\230\222 %d: ",   //U+2612 BALLOT BOX WITH X ☒
                ix);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, bufmsg, -1,
                                        nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.cursonv,
                       CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|end hashsetval %d|", nbvals);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°hashsetval_object  _9dKLCRPRn9Z_1tczFz5weBe */


// browse_data°hashmapval_object _0M2jApBzFAy_8H8cpCjGpTi
extern objrout_sigBM ROUTINEOBJNAME_BM (_0M2jApBzFAy_8H8cpCjGpTi);

value_tyBM
ROUTINEOBJNAME_BM (_0M2jApBzFAy_8H8cpCjGpTi)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // object
 const value_tyBM arg2,         // depth
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_0M2jApBzFAy_8H8cpCjGpTi,
                 value_tyBM resultv; const objectval_tyBM * objbrows;
                 value_tyBM nodv; value_tyBM cursonv; value_tyBM curvalv;
    );
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  if (!isobject_BM (arg1))
    FATAL_BM
      ("non-object for method to browse_data for hashmapval_object-s _0M2jApBzFAy_8H8cpCjGpTi");
  _.objbrows = (const objectval_tyBM *) arg1;
  int maxdepth = getint_BM (arg2);
  WEAKASSERT_BM (objhashashmapvalpayl_BM (_.objbrows));
  _.nodv = objhashmapvalmakenodeofkeyspayl_BM (_.objbrows, BMP_node);
  DBGPRINTF_BM ("browse_data°hashmapval_object start objbrows=%s nodv=%s\n",
                objectdbg_BM (_.objbrows),
                debug_outstr_value_BM (_.nodv, CURFRAME_BM, 0));
  WEAKASSERT_BM (isnode_BM (_.nodv));
  GtkTextBuffer *brobuf = gtk_text_iter_get_buffer (&browserit_BM);
  int nbkeys = nodewidth_BM (_.nodv);
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|hashmapval %d:|", nbkeys);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  for (int ix = 0; ix < nbkeys; ix++)
    {
      if (ix > 0 && ix % 10 == 0)
        gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      _.cursonv = nodenthson_BM (_.nodv, ix);
      WEAKASSERT_BM (_.cursonv != NULL);
      _.curvalv = objhashmapvalgetpayl_BM (_.objbrows, _.cursonv);
      WEAKASSERT_BM (_.curvalv != NULL);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM, "\342\232\255 ", // U+26AD MARRIAGE SYMBOL ⚭);
                                        -1, nest_brotag_BM, NULL);
      char bufmsg[48];
      memset (bufmsg, 0, sizeof (bufmsg));
      snprintf (bufmsg, sizeof (bufmsg),        //
                "|#%d| ", ix);
      gtk_text_buffer_insert_with_tags (brobuf,
                                        &browserit_BM, bufmsg,
                                        -1,
                                        nest_brotag_BM, miscomm_brotag_BM,
                                        NULL);
      browse_value_BM ((const value_tyBM) _.cursonv, CURFRAME_BM, maxdepth,
                       1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
      gtk_text_buffer_insert_with_tags (brobuf, &browserit_BM,  //
                                        " \342\206\246 ", -1,   // U+21A6 RIGHTWARDS ARROW FROM BAR ↦
                                        nest_brotag_BM, NULL);
      browse_value_BM ((const value_tyBM) _.curvalv,
                       CURFRAME_BM, maxdepth, 1);
      gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
    }
  {
    char bufmsg[48];
    memset (bufmsg, 0, sizeof (bufmsg));
    snprintf (bufmsg, sizeof (bufmsg), "|end hashmapval of %d|", nbkeys);
    gtk_text_buffer_insert_with_tags (brobuf,
                                      &browserit_BM, bufmsg, -1,
                                      miscomm_brotag_BM, NULL);
    gtk_text_buffer_insert (brobuf, &browserit_BM, "\n", -1);
  }
  LOCALRETURN_BM (_.objbrows);
}                               /* end browse_data°hashmapval_object _0M2jApBzFAy_8H8cpCjGpTi */



////////////////////////////////////////////////////////////////
// test_agenda command_handler _1eQ1a8KHixZ_1XDNH5iTQ0I
extern objrout_sigBM ROUTINEOBJNAME_BM (_1eQ1a8KHixZ_1XDNH5iTQ0I);
value_tyBM
ROUTINEOBJNAME_BM (_1eQ1a8KHixZ_1XDNH5iTQ0I)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // nbtasklets
 const value_tyBM arg2,         //
 const value_tyBM arg3,         //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_1eQ1a8KHixZ_1XDNH5iTQ0I,
                 objectval_tyBM * taskob; value_tyBM arg1v;
                 value_tyBM arg2v;
                 value_tyBM closv;
    );
  objectval_tyBM *k_tiny_tasklet = BMK_8dM7Xmdup4k_4jwzwOtmC1a;
  objectval_tyBM *k_rank = BMK_8zRh2medTlP_0ImnPyO8NKH;
  objectval_tyBM *k_todo = BMK_7ljWW4bj70g_9PL9dZkgBxZ;
  objectval_tyBM *kk_testagendafun = BMK_7XDuHagbhi8_3V9zhBpbrrV;
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  _.arg1v = arg1;
  _.arg2v = arg2;
  int nbtasklets = getint_BM (arg1);
  DBGPRINTF_BM ("test_agenda command_handler start nbtasklets=%d arg1=%s",      //
                nbtasklets,     //
                debug_outstr_value_BM (_.arg1v, //
                                       CURFRAME_BM, 0));
  if (nbtasklets <= 0 || nbtasklets > 25 * MILLION_BM)
    {
      log_begin_message_BM ();
      log_printf_message_BM (",test_agenda command bad nbtasklets %d",
                             nbtasklets);
      log_end_message_BM ();
      LOCALRETURN_BM (NULL);
    }
  int nbhi = 0;
  log_begin_message_BM ();
  log_printf_message_BM (",test_agenda will add %d tasklets.", nbtasklets);
  log_end_message_BM ();
  double starteltime = elapsedtime_BM ();
  double preveltime = 0.0;
  //
  for (int ix = 0; ix < nbtasklets; ix++)
    {
      _.taskob = makeobj_BM ();
      objputclass_BM (_.taskob, k_tiny_tasklet);
      objputattr_BM (_.taskob, k_rank, taggedint_BM (ix));
      _.closv = makeclosurevar_BM (kk_testagendafun, _.arg2v, NULL);
      objputattr_BM (_.taskob, k_todo, _.closv);
      objtouchnow_BM (_.taskob);
      bool hi = false;
      if (ix % 8 == 0)
        {
          agenda_add_high_priority_tasklet_BM (_.taskob);
          hi = true;
          nbhi++;
        }
      else
        agenda_add_low_priority_tasklet_BM (_.taskob);
      NONPRINTF_BM ("test_agenda ix=%d taskob %s / %s", ix,
                    objectdbg_BM (_.taskob), hi ? "hi" : "lo");
      if (ix % 100 == 0 && preveltime + 0.6 < elapsedtime_BM ())
        {
          preveltime = elapsedtime_BM ();
          DBGPRINTF_BM ("test_agenda **ix#%d taskob %s / %s elapsed %.3f s",
                        ix, objectdbg_BM (_.taskob), hi ? "hi" : "lo",
                        preveltime);
          log_begin_message_BM ();
          log_printf_message_BM (",test_agenda adding tasklet#%d ", ix);
          log_object_message_BM (_.taskob);
          log_printf_message_BM (" (%d of high priority), elapsed %.3f s.",
                                 nbhi, preveltime);
          log_end_message_BM ();
          usleep (1000);
        }
    }
  double realduration = elapsedtime_BM () - starteltime;
  DBGPRINTF_BM ("test_agenda ending realduration %.3f %d tasklets",
                realduration, nbtasklets);
  log_begin_message_BM ();
  log_printf_message_BM
    (",test_agenda added %d tasklets (with %d hi) in %.3f real seconds (%.2f µs/ob).",
     nbtasklets, nbhi, realduration, (1.0e6 * realduration) / nbtasklets);
  log_end_message_BM ();
  LOCALRETURN_BM (BMK_1eQ1a8KHixZ_1XDNH5iTQ0I);
}                               /* end routine _1eQ1a8KHixZ_1XDNH5iTQ0I */


//  test_agenda gtk _7rRjqfXs3QI_4Nwk2Lfm569
extern objrout_sigBM ROUTINEOBJNAME_BM (_7rRjqfXs3QI_4Nwk2Lfm569);

value_tyBM
ROUTINEOBJNAME_BM (_7rRjqfXs3QI_4Nwk2Lfm569)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // taskob
 const value_tyBM arg2,         // rank
 const value_tyBM arg3,         //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_7rRjqfXs3QI_4Nwk2Lfm569,
                 objectval_tyBM * taskob; value_tyBM rankv;
                 value_tyBM resultv;
    );
  _.taskob = objectcast_BM (arg1);
  _.rankv = arg2;
  WEAKASSERT_BM (isobject_BM (_.taskob));
  WEAKASSERT_BM (istaggedint_BM (_.rankv));
  WEAKASSERT_BM (pthread_self () == mainthreadid_BM);
  int rk = getint_BM (_.rankv);
  NONPRINTF_BM ("test_agenda gtk taskob %s rk %d", objectdbg_BM (_.taskob),
                rk);
  log_begin_message_BM ();
  log_printf_message_BM ("testagenda elapsed %.3f s tasklet ",
                         elapsedtime_BM ());
  log_object_message_BM (_.taskob);
  log_printf_message_BM (" rank %d.", rk);
  log_end_message_BM ();
  LOCALRETURN_BM (_.taskob);
}                               /* end routine _7rRjqfXs3QI_4Nwk2Lfm569 */



// dump command_handler _5oSaKNehPiO_3GSlpH5boCW
extern objrout_sigBM ROUTINEOBJNAME_BM (_5oSaKNehPiO_3GSlpH5boCW);
value_tyBM
ROUTINEOBJNAME_BM (_5oSaKNehPiO_3GSlpH5boCW)    //
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // optional dumpstring
 const value_tyBM arg2_,        //
 const value_tyBM arg3_,        //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_5oSaKNehPiO_3GSlpH5boCW,
                 value_tyBM argv;
    );
  extern char *dump_dir_bm;
  char *dumpstr = NULL;
  char *realstr = NULL;
  _.argv = arg1;
  if (isstring_BM (_.argv))
    {
      dumpstr = strdup (bytstring_BM (_.argv));
      if (!dumpstr)
        FATAL_BM ("failed to strdup dump string %s", bytstring_BM (_.argv));
    }
  else
    {
      dumpstr = strdup (dump_dir_bm ? dump_dir_bm : ".");
      if (!dumpstr)
        FATAL_BM ("failed to strdup default dump string %s",
                  dump_dir_bm ? dump_dir_bm : ".");
      _.argv = makestring_BM (dumpstr);
    }
  realstr = realpath (dumpstr, NULL);
  log_begin_message_BM ();
  log_printf_message_BM (",dump into  directory %s (%s)", dumpstr,
                         realstr ? realstr : "??");
  log_puts_message_BM (".\n");
  struct dumpinfo_stBM di = dump_BM (dump_dir_bm, NULL);
  log_printf_message_BM ("dump: scanned %ld, emitted %ld objects\n",
                         di.dumpinfo_scanedobjectcount,
                         di.dumpinfo_emittedobjectcount);
  log_printf_message_BM ("did %ld todos, wrote %ld files\n",
                         di.dumpinfo_todocount, di.dumpinfo_wrotefilecount);
  log_printf_message_BM ("in %.3f elapsed, %.4f cpu seconds.\n",
                         di.dumpinfo_elapsedtime, di.dumpinfo_cputime);
  log_end_message_BM ();
  if (gui_command_log_file_BM)
    {
      time_t nowtim = time (NULL);
      struct tm nowtm = { };
      localtime_r (&nowtim, &nowtm);
      char nowbuf[64];
      memset (nowbuf, 0, sizeof (nowbuf));
      strftime (nowbuf, sizeof (nowbuf), "%c", &nowtm);
      fprintf (gui_command_log_file_BM,
               "\n//// dumped %ld objects, %ld files to %s i.e. %s at %s\n\n",
               di.dumpinfo_emittedobjectcount, di.dumpinfo_wrotefilecount,
               dumpstr, realstr ? realstr : dumpstr, nowbuf);
      fflush (gui_command_log_file_BM);
    }
  free (dumpstr), dumpstr = NULL;
  free (realstr), realstr = NULL;
  LOCALRETURN_BM (_.argv);
}                               /* end dump command_handler _5oSaKNehPiO_3GSlpH5boCW */

// emit_module command_handler  _2bzzB0nZuUO_2xfj3rDb3DN

extern objrout_sigBM ROUTINEOBJNAME_BM (_2bzzB0nZuUO_2xfj3rDb3DN);

value_tyBM
ROUTINEOBJNAME_BM (_2bzzB0nZuUO_2xfj3rDb3DN)    // emit_module#command_handler
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // modulob
 const value_tyBM arg2,         //
 const value_tyBM arg3,         //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_2bzzB0nZuUO_2xfj3rDb3DN,
                 objectval_tyBM * modulob; value_tyBM resultv;
                 value_tyBM failres;
    );
  _.modulob = objectcast_BM (arg1);
  DBGPRINTF_BM ("emit_module command start modulob=%s",
                objectdbg_BM (_.modulob));
  if (!_.modulob)
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM
            ("missing argument to ,emit_module (module object expected).");
          log_end_message_BM ();
        };
      LOCALRETURN_BM (NULL);
    }
  if (pthread_self () == mainthreadid_BM)
    {
      log_begin_message_BM ();
      log_puts_message_BM ("should emit module ");
      log_object_message_BM (_.modulob);
      log_end_message_BM ();
    };
  /////
  _.failres = NULL;
  int failcod = 0;
  struct failurelockset_stBM flockset = { };
  struct failurehandler_stBM *prevfailurehandle = curfailurehandle_BM;
  initialize_failurelockset_BM (&flockset, sizeof (flockset));
  LOCAL_FAILURE_HANDLE_BM (&flockset, lab_failureemit, failcod, _.failres);
  if (failcod > 0)
  lab_failureemit:{
      destroy_failurelockset_BM (&flockset);
      curfailurehandle_BM = prevfailurehandle;
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("failed to emit module ");
          log_object_message_BM (_.modulob);
          log_printf_message_BM (" with failcode#%d\n.. failres %s\n",
                                 failcod,
                                 debug_outstr_value_BM (_.failres,
                                                        CURFRAME_BM, 0));
          log_end_message_BM ();
          LOCALRETURN_BM (NULL);
        };
    };
  objlock_BM (_.modulob);
  _.resultv = send0_BM (_.modulob, BMP_emit_module, CURFRAME_BM);
  objunlock_BM (_.modulob);
  destroy_failurelockset_BM (&flockset);
  curfailurehandle_BM = prevfailurehandle;

  DBGPRINTF_BM ("emit_module command end modulob=%s result %s", objectdbg_BM (_.modulob),       //
                debug_outstr_value_BM (_.resultv, CURFRAME_BM, 0));
  if (_.resultv != NULL)
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("did emit module ");
          log_object_message_BM (_.modulob);
          log_puts_message_BM (" into ");
          log_object_message_BM (objectcast_BM (_.resultv));
          log_puts_message_BM (" .");
          log_end_message_BM ();
          show_answer_value_newgui_BM (_.resultv, CURFRAME_BM);
          LOCALRETURN_BM (_.resultv);
        };
    }
  else
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("bad emit module ");
          log_object_message_BM (_.modulob);
          log_end_message_BM ();
        };
    }
  LOCALRETURN_BM (NULL);
}                               /* end emit_module command_handler _2bzzB0nZuUO_2xfj3rDb3DN */



//  command_handler#transient _2WuloCaOgGk_9oQOcW9c5Je

extern objrout_sigBM ROUTINEOBJNAME_BM (_2WuloCaOgGk_9oQOcW9c5Je);

value_tyBM
ROUTINEOBJNAME_BM (_2WuloCaOgGk_9oQOcW9c5Je)    //command_handler#transient 
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // transientv
 const value_tyBM arg2_ __attribute__ ((unused)),       //
 const value_tyBM arg3_ __attribute__ ((unused)),       //
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  LOCALFRAME_BM (stkf, /*descr: */ BMK_2WuloCaOgGk_9oQOcW9c5Je,
                 value_tyBM transientv; objectval_tyBM * transob;
                 value_tyBM resultv;
    );
  _.transientv = arg1;
again:
  if (isobject_BM (_.transientv))
    {
      _.transob = (objectval_tyBM *) _.transientv;
      objlock_BM (_.transob);
      objputspacenum_BM (_.transob, TransientSp_BM);
      objtouchnow_BM (_.transob);
      objunlock_BM (_.transob);
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_puts_message_BM ("made transient ");
          log_object_message_BM (_.transob);
          log_puts_message_BM (".");
          log_end_message_BM ();
        };
    }
  else if (issequence_BM (_.transientv))
    {
      unsigned siz = sequencesize_BM (_.transientv);
      for (int ix = 0; ix < (int) siz; ix++)
        {
          _.transob = sequencenthcomp_BM (_.transientv, ix);
          objlock_BM (_.transob);
          objputspacenum_BM (_.transob, TransientSp_BM);
          objtouchnow_BM (_.transob);
          objunlock_BM (_.transob);
        }
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_printf_message_BM ("made transient %d objects:", siz);
          for (int ix = 0; ix < (int) siz; ix++)
            {
              _.transob = sequencenthcomp_BM (_.transientv, ix);
              if (ix % 4 == 0)
                log_puts_message_BM ("\n");
              else
                log_puts_message_BM (" ");
              log_object_message_BM (_.transob);
            };
          log_puts_message_BM (".");
          log_end_message_BM ();
        }

    }
  else if (isclosure_BM (_.transientv))
    {
      _.resultv = apply0_BM (_.transientv, CURFRAME_BM);
      if (isobject_BM (_.resultv) || issequence_BM (_.resultv))
        {
          _.transientv = _.resultv;
          goto again;
        }
      else
        {
          if (pthread_self () == mainthreadid_BM)
            {
              log_begin_message_BM ();
              log_printf_message_BM ("bad ,transient closure %s gave %s.",
                                     debug_outstr_value_BM (_.transientv,
                                                            CURFRAME_BM, 0),
                                     debug_outstr_value_BM (_.resultv,
                                                            CURFRAME_BM, 0));
              log_end_message_BM ();
            }
        }
    }
  else
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_printf_message_BM ("invalid argument to ,transient %s.",
                                 debug_outstr_value_BM (_.transientv,
                                                        CURFRAME_BM, 0));
          log_end_message_BM ();
        }
    }
  LOCALRETURN_BM (_.transientv);
}                               /* end command_handler#transient _2WuloCaOgGk_9oQOcW9c5Je */




////////////////////////////////////////////////////////////////

// command_handler#find_object _0FdMKAvShgD_7itPSCL8D6P

/// ,find_object (criteria moreobjects predicateskip)
extern objrout_sigBM ROUTINEOBJNAME_BM (_0FdMKAvShgD_7itPSCL8D6P);

value_tyBM
ROUTINEOBJNAME_BM (_0FdMKAvShgD_7itPSCL8D6P)    // command_handler#find_object
(struct stackframe_stBM * stkf, //
 const value_tyBM arg1,         // criterv
 const value_tyBM arg2,         // moreobjv
 const value_tyBM arg3,         // predskipv
 const value_tyBM arg4_ __attribute__ ((unused)),       //
 const quasinode_tyBM * restargs_ __attribute__ ((unused)))
{
  /// see module first_misc_module for the *_minifunc listed here
  objectval_tyBM *k_element_of_closed_minifunc = BMK_0YOXUe7rEJC_3jFMjOAQRs9;
  objectval_tyBM *k_same_as_closed_minifunc = BMK_3yQlckX4DRh_4b9l9FBSSSL;
  objectval_tyBM *k_equal_to_closed_minifunc = BMK_4iEFTEcHxeb_6lH464uFkTC;
  objectval_tyBM *k_findrun_object = BMK_64UbCFBD19G_43TeBXhcYMy;
  objectval_tyBM *k_scan_queue = BMK_6eWBdICnzoa_2FKvuyZ7Ivq;
  objectval_tyBM *k_visited_hashset = BMK_8w8gLezc1gm_4plK3EfhuGk;
  objectval_tyBM *k_criteria = BMK_0EKpB4Re4KE_6v0jMEEJgPe;
  objectval_tyBM *k_skip = BMK_1IZ2mh67gTz_0bHC4LiI29H;
  objectval_tyBM *k_in = BMK_0eMGYofuNVh_8ZP2mXdhtHO;
  LOCALFRAME_BM (stkf, /*descr: */ BMK_0FdMKAvShgD_7itPSCL8D6P,
                 value_tyBM resultv;    //
                 value_tyBM criterv;    //
                 value_tyBM moreobjv;   //
                 value_tyBM predskipv;  //
                 value_tyBM criterclosv;        //
                 value_tyBM moresetv;   //
                 value_tyBM qexpv;      //
                 value_tyBM valv;       //
                 value_tyBM skipclosv;  //
                 objectval_tyBM * findrunob;    //
                 objectval_tyBM * scanquob;     //
                 objectval_tyBM * vihsetob;     //
    );
  _.criterv = arg1;
  _.moreobjv = arg2;
  _.predskipv = arg3;
  DBGPRINTF_BM ("command_handler#find_object start criterv=%s moreobjv=%s predskipv=%s",        //
                debug_outstr_value_BM (_.criterv, CURFRAME_BM, 0),      //
                debug_outstr_value_BM (_.moreobjv, CURFRAME_BM, 0),     //
                debug_outstr_value_BM (_.predskipv, CURFRAME_BM, 0));
  ///
  /// build the criteria closure
  if (isobject_BM (_.criterv))
    _.criterclosv = makeclosure1_BM (k_same_as_closed_minifunc, _.criterv);
  else if (isset_BM (_.criterv))
    _.criterclosv = makeclosure1_BM (k_element_of_closed_minifunc, _.criterv);
  else if (isclosure_BM (_.criterv))
    _.criterclosv = _.criterv;
  else if (isnode_BM (_.criterv) && nodewidth_BM (_.criterv) == 1
           && nodeconn_BM (_.criterv) == BMP_exclam)
    {
      _.qexpv = nodenthson_BM (_.criterv, 0);
      _.criterclosv = makeclosure1_BM (k_equal_to_closed_minifunc, _.qexpv);
    }
  else
    _.criterclosv = makeclosure1_BM (k_equal_to_closed_minifunc, _.criterv);
  DBGPRINTF_BM ("command_handler#find_object criterclos=%s",
                debug_outstr_value_BM (_.criterclosv, CURFRAME_BM, 0));
  if (!isclosure_BM (_.criterclosv))
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_printf_message_BM
            ("bad  criteria %s\n... to ,find_obj",
             debug_outstr_value_BM (_.criterv, CURFRAME_BM, 0));
          log_end_message_BM ();
        };
      LOCALRETURN_BM (NULL);
    }
  ///
  /// construct the moreset of additional objects to scan
  if (isobject_BM (_.moreobjv))
    {
      _.moresetv = makesizedset_BM (1, _.moreobjv);
    }
  else if (isset_BM (_.moreobjv))
    {
      _.moresetv = _.moreobjv;
    }
  else if (_.moreobjv != NULL)
    {
      if (pthread_self () == mainthreadid_BM)
        {
          log_begin_message_BM ();
          log_printf_message_BM
            ("bad  more objects %s\n... to ,find_obj",
             debug_outstr_value_BM (_.moreobjv, CURFRAME_BM, 0));
          log_end_message_BM ();
        };
      LOCALRETURN_BM (NULL);
    }
  else
    _.moresetv = NULL;
  DBGPRINTF_BM ("command_handler#find_object moreset=%s",
                debug_outstr_value_BM (_.moresetv, CURFRAME_BM, 0));
  ///
  /// construct the skipclosv closure, which returns non-nil to skip a value (perhaps an object)
  if (isobject_BM (_.predskipv))
    _.skipclosv = makeclosure1_BM (k_same_as_closed_minifunc, _.predskipv);
  else if (isset_BM (_.predskipv))
    _.skipclosv = makeclosure1_BM (k_element_of_closed_minifunc, _.predskipv);
  else if (isclosure_BM (_.predskipv))
    _.skipclosv = _.predskipv;
  else if (isnode_BM (_.predskipv) && nodewidth_BM (_.predskipv) == 1
           && nodeconn_BM (_.predskipv) == BMP_exclam)
    {
      _.qexpv = nodenthson_BM (_.predskipv, 0);
      _.skipclosv = makeclosure1_BM (k_equal_to_closed_minifunc, _.qexpv);
    }
  else if (_.predskipv)
    _.skipclosv = makeclosure1_BM (k_equal_to_closed_minifunc, _.predskipv);
  else
    _.skipclosv = NULL;
  DBGPRINTF_BM ("command_handler#find_object skipclos=%s",
                debug_outstr_value_BM (_.skipclosv, CURFRAME_BM, 0));
  ///
  _.findrunob = makeobj_BM ();
  objputclass_BM (_.findrunob, k_findrun_object);
  objputattr_BM (_.findrunob, k_criteria, _.criterclosv);
  if (_.skipclosv)
    objputattr_BM (_.findrunob, k_skip, _.skipclosv);
  _.scanquob = makeobj_BM ();
  objputattr_BM (_.scanquob, k_in, _.findrunob);
  objputlistpayl_BM (_.scanquob);


  DBGPRINTF_BM ("command_handler#find_object findrunob=%s",
                objectdbg_BM (_.findrunob));
#warning unimplemented command_handler#find_object _0FdMKAvShgD_7itPSCL8D6P routine
  WEAKASSERT_BM (false
                 &&
                 "unimplemented command_handler#find_object _0FdMKAvShgD_7itPSCL8D6P routine");
  LOCALRETURN_BM (_.resultv);
}                               /* end command_handler#find_object _0FdMKAvShgD_7itPSCL8D6P */
