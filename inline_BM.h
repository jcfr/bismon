// file inline_BM.h
#ifndef INLINE_BM_INCLUDED
#define INLINE_BM_INCLUDED

double
clocktime_BM (clockid_t clid)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (clid, &ts))
    return NAN;
  return (double) ts.tv_sec + 1.0e-9 * ts.tv_nsec;
}

double
cputime_BM (void)
{
  return clocktime_BM (CLOCK_PROCESS_CPUTIME_ID);
}                               /* end cputime_BM */

double
elapsedtime_BM (void)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (CLOCK_MONOTONIC, &ts))
    return NAN;
  return (double) (ts.tv_sec - startrealtimespec_BM.tv_sec)
    + 1.0e-9 * (ts.tv_nsec - startrealtimespec_BM.tv_nsec);
}                               /* end elapsedtime_BM */



bool
istaggedint_BM (value_tyBM v)
{
  return (uintptr_t) v & 1;
}                               /* end istaggedint_BM */

intptr_t
getint_BM (value_tyBM v)
{
  if (istaggedint_BM (v))
    return ((intptr_t) v) >> 1;
}                               /* end getint_BM */

value_tyBM
taggedint_BM (intptr_t i)
{
  return (value_tyBM) ((i << 1) | 1);
}                               /* end taggedint_BM */

int
valtype_BM (const value_tyBM v)
{
  if (!v)
    return tyNone_BM;
  if (istaggedint_BM (v))
    return tyInt_BM;
  if (((uintptr_t) v & 3) == 0)
    {
      typedhead_tyBM *ht = (typedhead_tyBM *) v;
      assert (ht->htyp != 0);
      return ht->htyp;
    }
  return tyNone_BM;
}                               /* end valtype_BM */

bool
validserial63_BM (serial63_tyBM s)
{
  return (s > MINSERIAL_BM && s < MAXSERIAL_BM);
}                               /* end validserial63_BM */


unsigned
bucknumserial63_BM (serial63_tyBM s)
{
  return s / (DELTASERIAL_BM / MAXBUCKETS_BM);
}

uint64_t
buckoffserial63_BM (serial63_tyBM s)
{
  return s % (DELTASERIAL_BM / MAXBUCKETS_BM);
}

bool
validid_BM (rawid_tyBM id)
{
  return (validserial63_BM (id.id_hi) && validserial63_BM (id.id_lo));
}                               /* end validid_BM */



hash_tyBM
hashid_BM (rawid_tyBM id)
{
  if (!validid_BM (id))
    return 0;
  hash_tyBM h = (id.id_hi % 1073741939) ^ (id.id_lo % 596789351);
  if (h == 0)
    h = (id.id_hi & 0xffffff) + (id.id_lo & 0x3ffffff) + 17;
  assert (h > 0);
  return h;
}                               /* end hashid_BM */



int
cmpid_BM (rawid_tyBM id1, rawid_tyBM id2)
{
  if (id1.id_hi == id2.id_hi)
    {
      if (id1.id_lo == id2.id_lo)
        return 0;
      else if (id1.id_lo < id2.id_lo)
        return -1;
      else
        return +1;
    }
  else if (id1.id_hi < id2.id_hi)
    return -1;
  else
    return +1;
}                               /* end cmpid_BM */


rawid_tyBM
objid_BM (const objectval_tyBM * obj)
{
  if ((valtype_BM ((const value_tyBM) obj) != tyObject_BM))
    return (rawid_tyBM)
    {
    0, 0};
  return obj->ob_id;
}                               /* end objid_BM */


void
objtouchmtime_BM (objectval_tyBM * obj, double mtime)
{
  if ((valtype_BM ((const value_tyBM) obj) != tyObject_BM))
    return;
  obj->ob_mtime = mtime;
}                               /* end objtouchmtime_BM */


void
objtouchnow_BM (objectval_tyBM * obj)
{
  if ((valtype_BM ((const value_tyBM) obj) != tyObject_BM))
    return;
  obj->ob_mtime = clocktime_BM (CLOCK_MONOTONIC);
}                               /* end objtouchnow_BM */

bool
equalid_BM (rawid_tyBM id1, rawid_tyBM id2)
{
  return id1.id_hi == id2.id_hi && id1.id_lo == id2.id_lo;
}                               /* end equalid_BM */



/// object support
hash_tyBM
objecthash_BM (const objectval_tyBM * pob)
{
  if (!pob || ((intptr_t) pob & 3))
    return 0;
  if (((typedhead_tyBM *) pob)->htyp != tyObject_BM)
    return 0;
  return ((typedhead_tyBM *) pob)->hash;
}                               /* end objecthash_BM */

int
objectcmp_BM (const objectval_tyBM * ob1, const objectval_tyBM * ob2)
{
  if (ob1 == ob2)
    return 0;
  if (ob1
      && (((intptr_t) ob1 & 3)
          || ((typedhead_tyBM *) ob1)->htyp != tyObject_BM))
    FATAL_BM ("bad ob1@%p for objectcmp_BM", ob1);
  if (ob2
      && (((intptr_t) ob1 & 3)
          || ((typedhead_tyBM *) ob2)->htyp != tyObject_BM))
    FATAL_BM ("bad ob2@%p for objectcmp_BM", ob2);
  if (!ob1)
    return -1;
  if (!ob2)
    return +1;
  return cmpid_BM (ob1->ob_id, ob2->ob_id);
}                               /* end objectcmp_BM */

bool
isassoc_BM (const value_tyBM v)
{
  int ty = valtype_BM (v);
  return ty == tydata_assocbucket_BM || ty == tydata_assocpairs_BM;
}                               /* end isassoc_BM */

unsigned
assoc_nbkeys_BM (const anyassoc_tyBM * assoc)
{
  int ty = valtype_BM ((value_tyBM) assoc);
  if (ty == tydata_assocpairs_BM)
    {
      struct assocpairs_stBM *apair = (struct assocpairs_stBM *) assoc;
      return ((typedsize_tyBM *) apair)->size;
    }
  else if (ty == tydata_assocbucket_BM)
    {
      struct assocbucket_stBM *abuck = (struct assocbucket_stBM *) assoc;
      return ((typedsize_tyBM *) abuck)->size;
    }
  return 0;
}                               /* end assoc_nbkeys_BM */


unsigned
datavectlen_BM (const struct datavectval_stBM *dvec)
{
  if (valtype_BM ((const value_tyBM) dvec) != tydata_vectval_BM)
    return 0;
  return ((typedsize_tyBM *) dvec)->size;
}                               /* end datavectlen_BM */

value_tyBM
datavectnth_BM (const struct datavectval_stBM * dvec, int rk)
{
  unsigned sz = datavectlen_BM (dvec);
  if (rk < 0)
    rk += (int) sz;
  if (rk >= 0 && rk < (int) sz)
    return dvec->vec_data[rk];
  return NULL;
}                               /* end datavectnth_BM */


#endif /*INLINE_BM_INCLUDED */