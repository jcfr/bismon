// file user_BM.c; management of European GDPR related personal data; see userlogin.md

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

----
    Contact me (Basile Starynkevitch) by email
    basile@starynkevitch.net and/or basile.starynkevitch@cea.fr
***/
#include "bismon.h"
#include "user_BM.const.h"

static objectval_tyBM *add_contributor_name_email_alias_BM
  (const char *name,
   const char *email,
   const char *alias, char **perrmsg, struct stackframe_stBM *stkf);

bool
valid_email_BM (const char *email, char **perrmsg)
{
  if (perrmsg)
    *perrmsg = NULL;
  if (!email)
    {
      if (perrmsg)
        asprintf (perrmsg, "no email");
      return false;
    }
  const char *end = NULL;
  if (!g_utf8_validate (email, -1, &end) && end && *end)
    {
      if (perrmsg)
        asprintf (perrmsg, "invalid utf8 email %s", email);
      return false;
    }
  if (!isalpha (email[0]))
    {
      if (perrmsg)
        asprintf (perrmsg, "mail address %s don't start with letter", email);
      return false;
    }
  const char *at = strchr (email, '@');
  if (!at || !at[1])
    {
      if (perrmsg)
        asprintf (perrmsg, "mail address %s don't have at-sign", email);
      return false;
    }
  if (strchr (at + 1, '@'))
    {
      if (perrmsg)
        asprintf (perrmsg, "mail address %s has more than one at-sign",
                  email);
      return false;
    }
  for (const char *pc = email; pc < at; pc++)
    {
      if (isalnum (*pc))
        continue;
      if (*pc == '.' || *pc == '+' || *pc == '-' || *pc == '_')
        {
          if (!isalnum (pc[-1]) || !isalnum (pc[1]))
            {
              if (perrmsg)
                asprintf (perrmsg,
                          "mail address %s with bad . + - or _ occurrences before at-sign",
                          email);
              return false;
            }
          else
            continue;
        }
      if (perrmsg)
        asprintf (perrmsg,
                  "mail address %s with unexpected characters before at-sign\n",
                  email);
      return false;
    }
  for (const char *pc = at + 1; pc < at; pc++)
    {
      if (isalnum (*pc))
        continue;
      if (*pc == '.' || *pc == '+' || *pc == '-' || *pc == '_')
        {
          if (!isalnum (pc[-1]) || !isalnum (pc[1]))
            {
              if (perrmsg)
                asprintf (perrmsg,
                          "mail address %s with bad . + - or _ occurrences after at-sign",
                          email);
              return false;
            }
          else
            continue;
        }
      if (perrmsg)
        asprintf (perrmsg,
                  "mail address %s with unexpected characters after at-sign",
                  email);
      return false;
    }
  struct addrinfo *res = NULL;
  int err = getaddrinfo (at + 1, "mail", NULL, &res);
  if (err)
    {
      if (perrmsg)
        asprintf (perrmsg,
                  "mail address %s domain %s failed on getaddrinfo: %s",
                  email, at + 1, gai_strerror (err));
      return false;
    }
  if (res)
    freeaddrinfo (res);
  else
    {
      if (perrmsg)
        asprintf (perrmsg,
                  "mail address %s domain %s invalid\n", email, at + 1);
      return false;
    }
  return true;
}                               /* end valid_email_BM */

bool
valid_contributor_name_BM (const char *name, char **perrmsg)
{
  if (perrmsg)
    *perrmsg = NULL;
  if (!name)
    {
      if (perrmsg)
        asprintf (perrmsg, "no contributor name");
      return false;
    }
  const char *end = NULL;
  if (!g_utf8_validate (name, -1, &end) && end && *end)
    {
      if (perrmsg)
        asprintf (perrmsg, "invalid utf8 contributor name %s", name);
      return false;
    }
  // validate the name:
  gunichar uc = 0;
  gunichar prevuc = 0;
  for (const char *p = name;
       *p && (uc = g_utf8_get_char (p)) != 0;
       (p = g_utf8_next_char (p)), (prevuc = uc))
    {
      if (g_unichar_isalpha (uc))
        continue;
      else if (g_unichar_isalnum (uc) && p > name)
        continue;
      else
        if ((uc == '_' || uc == '-' || uc == '+' || uc == ' ')
            && g_unichar_isalnum (prevuc) && *g_utf8_next_char (p))
        continue;
      if (perrmsg)
        asprintf (perrmsg, "invalid contributor name '%s'", name);
      return false;
    }
  return true;
}                               /* end valid_contributor_name_BM */


objectval_tyBM *add_contributor_name_email_alias_BM
  (const char *name, const char *email, const char *alias,
   char **perrmsg, struct stackframe_stBM * stkf)
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * userob;       //
    );
  if (!alias)
    alias = "";
  if (perrmsg)
    *perrmsg = NULL;
  DBGPRINTF_BM
    ("add_contributor_name_email_alias start name='%s' email='%s' alias='%s' perrmsg@%p",
     name, email, alias, (void *) perrmsg);
  ASSERT_BM (name && name[0]);
  ASSERT_BM (email && email[0]);
  const char *end = NULL;
  if (!valid_contributor_name_BM (name, perrmsg))
    LOCALRETURN_BM (NULL);
  if (!valid_email_BM (email, perrmsg))
    LOCALRETURN_BM (NULL);
  if (alias && alias[0] && !valid_email_BM (alias, perrmsg))
    LOCALRETURN_BM (NULL);
  FILE *fil = fopen (CONTRIBUTORS_FILE_BM, "r+");
  if (!fil)
    {
      if (perrmsg)
        asprintf (perrmsg, "fail to open %s : %m", CONTRIBUTORS_FILE_BM);
      LOCALRETURN_BM (NULL);
    }
  int fd = fileno (fil);
  if (flock (fd, LOCK_EX))
    FATAL_BM ("failed to flock fd#%d for %s", fd, CONTRIBUTORS_FILE_BM);
#warning add_contributor_name_email_alias unimplemented
  FATAL_BM
    ("add_contributor_name_email_alias unimplemented user name '%s' email '%s' alias '%s'",
     name, email, alias);
}                               /* end add_contributor_name_email_alias_BM */



objectval_tyBM *
add_contributor_user_BM (const char *str,
                         char **perrmsg, struct stackframe_stBM *stkf)
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * userob;       //
    );
  const char *namestr = NULL;
  const char *emailstr = NULL;
  const char *aliasstr = NULL;
  if (!str)
    return NULL;
  if (perrmsg)
    *perrmsg = NULL;
  DBGPRINTF_BM ("add_contributor_user_BM str='%s'", str);
  if (!str[0])
    {
      if (perrmsg)
        asprintf (perrmsg, "empty user string");
      return NULL;
    }
  const char *end = NULL;
  if (!g_utf8_validate (str, -1, &end) && end && *end)
    {
      if (perrmsg)
        asprintf (perrmsg, "invalid utf8 string %s", str);
      return false;
    }
  if (isspace (str[0]))
    {
      if (perrmsg)
        asprintf (perrmsg, "user string '%s' cannot start with a space", str);
      return NULL;
    }
  if (isdigit (str[0]) || str[0] == '_')
    {
      if (perrmsg)
        asprintf (perrmsg,
                  "user string '%s' cannot start with a digit or underscore\n",
                  str);
      return NULL;
    }
  for (const char *pc = str; *pc; pc++)
    if (*pc == '\n' || *pc == '\r' || *pc == '\t'
        || *pc == '\r' || *pc == '\v' || *pc == '\f'
        || (*pc != ' ' && (isspace (*pc) || iscntrl (*pc))))
      {
        if (perrmsg)
          asprintf (perrmsg,
                    "user string '%s' cannot contain control or tab, return or weird space characters",
                    str);
        return NULL;
      }
  const char *endstr = str + strlen (str);
  const char *lt = strchr (str, '<');
  const char *at = lt ? strchr (lt, '@') : NULL;
  const char *gt = at ? strchr (at, '>') : NULL;
  if (lt && at && gt && lt > str && gt == endstr - 1)
    {
      // str could be like 'First Lastname <email@example.com>'
      char *namend = lt - 1;
      while (namend > str && *namend == ' ')
        namend--;
      namestr = strndup (str, namend - str + 1);
      if (!namestr)
        FATAL_BM ("strndup failed, when extracting name from %s", str);
      emailstr = strndup (lt + 1, gt - lt - 1);
      if (!emailstr)
        FATAL_BM ("strndup failed, when extracting email from %s", str);
      DBGPRINTF_BM
        ("add_contributor_user_BM namestr='%s' emailstr='%s'",
         namestr, emailstr);
      _.userob =
        add_contributor_name_email_alias_BM (namestr,
                                             emailstr, NULL,
                                             perrmsg, CURFRAME_BM);
      DBGPRINTF_BM
        ("add_contributor_user_BM userob=%s for namestr='%s' emailstr='%s'",
         objectdbg_BM (_.userob), namestr, emailstr);
      free ((void *) namestr), namestr = NULL;
      free ((void *) emailstr), emailstr = NULL;
      LOCALRETURN_BM (_.userob);
    }
  // or like: 'First Lastname;email@example.com;aliasmail@example.org'
  // or just: 'First Lastname;email@example.com'
  const char *semcol1 = strchr (str, ';');
  const char *semcol2 = semcol1 ? strchr (semcol1 + 1, ';') : NULL;
  if (semcol1)
    {
      char *namend = semcol1 - 1;
      while (namend > str && *namend == ' ')
        namend--;
      namestr = strndup (str, namend - str + 1);
      if (!namestr)
        FATAL_BM ("strndup failed, when extracting name from %s", str);
      if (semcol2)
        {
          emailstr = strndup (semcol1 + 1, semcol2 - semcol1 - 1);
          if (!emailstr)
            FATAL_BM ("strndup failed, when extracting email from %s", str);
          aliasstr = strdup (semcol2 + 1);
          if (!aliasstr)
            FATAL_BM ("strndup failed, when extracting alias from %s", str);
          DBGPRINTF_BM
            ("add_contributor_user_BM namestr='%s' emailstr='%s' aliasstr='%s'",
             namestr, emailstr, aliasstr);
        }
      else
        {
          emailstr = strdup (semcol1 + 1);
          DBGPRINTF_BM
            ("add_contributor_user_BM namestr='%s' emailstr='%s'",
             namestr, emailstr);
        }
      _.userob =
        add_contributor_name_email_alias_BM (namestr,
                                             emailstr,
                                             aliasstr, perrmsg, CURFRAME_BM);
      if (aliasstr)
        DBGPRINTF_BM
          ("add_contributor_user_BM userob=%s for namestr='%s' emailstr='%s' aliasstr='%s'",
           objectdbg_BM (_.userob), namestr, emailstr, aliasstr);
      else
        DBGPRINTF_BM
          ("add_contributor_user_BM userob=%s for namestr='%s' emailstr='%s'",
           objectdbg_BM (_.userob), namestr, emailstr);
      free ((void *) namestr), namestr = NULL;
      free ((void *) emailstr), emailstr = NULL;
      free ((void *) aliasstr), aliasstr = NULL;
      LOCALRETURN_BM (_.userob);
    }
  if (perrmsg)
    asprintf (perrmsg,
              "invalid user string '%s',\n"
              "... expecting 'First Lastname <email@example.com>'\n"
              "... or 'First Lastname;email@example.com;aliasmail@example.org'\n",
              str);
  LOCALRETURN_BM (NULL);
}                               /* end add_contributor_user_BM */



objectval_tyBM *
remove_contributor_user_by_string_BM (const char *str,
                                      char **perrmsg,
                                      struct stackframe_stBM *stkf)
{
  LOCALFRAME_BM ( /*prev: */ stkf, /*descr: */ NULL,
                 objectval_tyBM * userob;       //
    );
  FATAL_BM ("unimplemented remove_contributor_user_by_string_BM str %s", str);
#warning unimplemented remove_contributor_user_by_string_BM
}                               /* end remove_contributor_user_by_string_BM */
