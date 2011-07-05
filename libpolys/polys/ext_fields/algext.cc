/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id$ */
/*
* ABSTRACT: numbers in an algebraic extension field K[a] / < f(a) >
*           Assuming that we have a coeffs object cf, then these numbers
*           are polynomials in the polynomial ring K[a] represented by
*           cf->extRing.
*           IMPORTANT ASSUMPTIONS:
*           1.) So far we assume that cf->extRing is a valid polynomial
*               ring in exactly one variable, i.e., K[a], where K is allowed
*               to be any field (representable in SINGULAR and which may
*               itself be some extension field, thus allowing for extension
*               towers).
*           2.) Moreover, this implementation assumes that
*               cf->extRing->minideal is not NULL but an ideal with at
*               least one non-zero generator which may be accessed by
*               cf->extRing->minideal->m[0] and which represents the minimal
*               polynomial f(a) of the extension variable 'a' in K[a].
*           3.) As soon as an std method for polynomial rings becomes
*               availabe, all reduction steps modulo f(a) should be replaced
*               by a call to std. Moreover, in this situation one can finally
*               move from K[a] / < f(a) > to
*                  K[a_1, ..., a_s] / I, with I some zero-dimensional ideal
*                                        in K[a_1, ..., a_s] given by a lex
*                                        Gröbner basis.
*               The code in algext.h and algext.cc is then capable of
*               computing in K[a_1, ..., a_s] / I.
*/

#include "config.h"
#include <misc/auxiliary.h>

#include <omalloc/omalloc.h>

#include <reporter/reporter.h>

#include <coeffs/coeffs.h>
#include <coeffs/numbers.h>
#include <coeffs/longrat.h>

#include <polys/monomials/ring.h>
#include <polys/monomials/p_polys.h>
#include <polys/simpleideals.h>

#include <polys/ext_fields/algext.h>

/// our type has been defined as a macro in algext.h
/// and is accessible by 'naID'

/// forward declarations
BOOLEAN  naGreaterZero(number a, const coeffs cf); 
BOOLEAN  naGreater(number a, number b, const coeffs cf);
BOOLEAN  naEqual(number a, number b, const coeffs cf);
BOOLEAN  naIsOne(number a, const coeffs cf);
BOOLEAN  naIsMOne(number a, const coeffs cf);
BOOLEAN  naIsZero(number a, const coeffs cf);
number   naInit(int i, const coeffs cf);
int      naInt(number &a, const coeffs cf);
number   naNeg(number a, const coeffs cf);
number   naInvers(number a, const coeffs cf);
number   naPar(int i, const coeffs cf);
number   naAdd(number a, number b, const coeffs cf);
number   naSub(number a, number b, const coeffs cf);
number   naMult(number a, number b, const coeffs cf);
number   naDiv(number a, number b, const coeffs cf);
void     naPower(number a, int exp, number *b, const coeffs cf);
number   naCopy(number a, const coeffs cf);
void     naWrite(number &a, const coeffs cf);
number   naRePart(number a, const coeffs cf);
number   naImPart(number a, const coeffs cf);
number   naGetDenom(number &a, const coeffs cf);
number   naGetNumerator(number &a, const coeffs cf);
number   naGcd(number a, number b, const coeffs cf);
number   naLcm(number a, number b, const coeffs cf);
int      naSize(number a, const coeffs cf);
void     naDelete(number *a, const coeffs cf);
void     naCoeffWrite(const coeffs cf);
number   naIntDiv(number a, number b, const coeffs cf);
const char * naRead(const char *s, number *a, const coeffs cf);
static BOOLEAN naCoeffIsEqual(const coeffs cf, n_coeffType n, void * param);

#ifdef LDEBUG
BOOLEAN naDBTest(number a, const char *f, const int l, const coeffs cf)
{
  assume(getCoeffType(cf) == naID);
  if (a == NULL) return TRUE;
  p_Test((poly)a, naRing);
  assume(p_Totaldegree((poly)a, naRing) <= p_Totaldegree(naMinpoly, naRing));
  return TRUE;
}
#endif

void heuristicReduce(poly &p, poly reducer, const coeffs cf);
void definiteReduce(poly &p, poly reducer, const coeffs cf);

/* returns the bottom field in this field extension tower; if the tower
   is flat, i.e., if there is no extension, then r itself is returned;
   as a side-effect, the counter 'height' is filled with the height of
   the extension tower (in case the tower is flat, 'height' is zero) */
static coeffs nCoeff_bottom(const coeffs r, int &height)
{
  assume(r != NULL);
  coeffs cf = r;
  height = 0;
  while (nCoeff_is_Extension(cf))
  {
    assume(cf->extRing != NULL); assume(cf->extRing->cf != NULL);
    cf = cf->extRing->cf;
    height++;
  }
  return cf;
}

BOOLEAN naIsZero(number a, const coeffs cf)
{
  naTest(a);
  return (a == NULL);
}

void naDelete(number * a, const coeffs cf)
{
  if (*a == NULL) return;
  poly aAsPoly = (poly)(*a);
  p_Delete(&aAsPoly, naRing);
  *a = NULL;
}

BOOLEAN naEqual(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  
  /// simple tests
  if (a == b) return TRUE;
  if ((a == NULL) && (b != NULL)) return FALSE;
  if ((b == NULL) && (a != NULL)) return FALSE;

  /// deg test
  int aDeg = 0;
  if (a != NULL) aDeg = p_Totaldegree((poly)a, naRing);
  int bDeg = 0;
  if (b != NULL) bDeg = p_Totaldegree((poly)b, naRing);
  if (aDeg != bDeg) return FALSE;
  
  /// subtraction test
  number c = naSub(a, b, cf);
  BOOLEAN result = naIsZero(c, cf);
  naDelete(&c, naCoeffs);
  return result;
}

number naCopy(number a, const coeffs cf)
{
  naTest(a);
  if (a == NULL) return NULL;
  return (number)p_Copy((poly)a, naRing);
}

number naGetNumerator(number &a, const coeffs cf)
{
  return naCopy(a, cf);
}

number naGetDenom(number &a, const coeffs cf)
{
  naTest(a);
  return naInit(1, cf);
}

BOOLEAN naIsOne(number a, const coeffs cf)
{
  naTest(a);
  poly aAsPoly = (poly)a;
  if (!p_IsConstant(aAsPoly, naRing)) return FALSE;
  return n_IsOne(p_GetCoeff(aAsPoly, naRing), naCoeffs);
}

BOOLEAN naIsMOne(number a, const coeffs cf)
{
  naTest(a);
  poly aAsPoly = (poly)a;
  if (!p_IsConstant(aAsPoly, naRing)) return FALSE;
  return n_IsMOne(p_GetCoeff(aAsPoly, naRing), naCoeffs);
}

/// this is in-place, modifies a
number naNeg(number a, const coeffs cf)
{
  naTest(a);
  if (a != NULL) a = (number)p_Neg((poly)a, naRing);
  return a;
}

number naImPart(number a, const coeffs cf)
{
  naTest(a);
  return NULL;
}

number naInit(int i, const coeffs cf)
{
  if (i == 0) return NULL;
  else        return (number)p_ISet(i, naRing);
}

int naInt(number &a, const coeffs cf)
{
  naTest(a);
  poly aAsPoly = (poly)a;
  if (!p_IsConstant(aAsPoly, naRing)) return 0;
  return n_Int(p_GetCoeff(aAsPoly, naRing), naCoeffs);
}

/* TRUE iff (a != 0 and (b == 0 or deg(a) > deg(b))) */
BOOLEAN naGreater(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (naIsZero(a, cf)) return FALSE;
  if (naIsZero(b, cf)) return TRUE;
  int aDeg = 0;
  if (a != NULL) aDeg = p_Totaldegree((poly)a, naRing);
  int bDeg = 0;
  if (b != NULL) bDeg = p_Totaldegree((poly)b, naRing);
  return (aDeg > bDeg);
}

/* TRUE iff a != 0 and (LC(a) > 0 or deg(a) > 0) */
BOOLEAN naGreaterZero(number a, const coeffs cf)
{
  naTest(a);
  if (a == NULL)                                            return FALSE;
  if (n_GreaterZero(p_GetCoeff((poly)a, naRing), naCoeffs)) return TRUE;
  if (p_Totaldegree((poly)a, naRing) > 0)                   return TRUE;
  return FALSE;
}

void naCoeffWrite(const coeffs cf)
{
  char *x = rRingVar(0, naRing);
  Print("//   Coefficients live in the extension field K[%s]/<f(%s)>\n", x, x);
  Print("//   with the minimal polynomial f(%s) = %s\n", x,
        p_String(naMinpoly, naRing));
  PrintS("//   and K: "); n_CoeffWrite(cf->extRing->cf);
}

number naPar(int i, const coeffs cf)
{
  assume(i == 1);   // there is only one parameter in this extension field
  poly p = p_ISet(1, naRing);   // p = 1
  p_SetExp(p, 1, 1, naRing);    // p = the sole extension variable
  p_Setm(p, naRing);
  return (number)p;
}

number naAdd(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (a == NULL) return naCopy(b, cf);
  if (b == NULL) return naCopy(a, cf);
  poly aPlusB = p_Add_q(p_Copy((poly)a, naRing),
                        p_Copy((poly)b, naRing), naRing);
  definiteReduce(aPlusB, naMinpoly, cf);
  return (number)aPlusB;
}

number naSub(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (b == NULL) return naCopy(a, cf);
  poly minusB = p_Neg(p_Copy((poly)b, naRing), naRing);
  if (a == NULL) return (number)minusB;
  poly aMinusB = p_Add_q(p_Copy((poly)a, naRing), minusB, naRing);
  definiteReduce(aMinusB, naMinpoly, cf);
  return (number)aMinusB;
}

number naMult(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (a == NULL) return NULL;
  if (b == NULL) return NULL;
  poly aTimesB = p_Mult_q(p_Copy((poly)a, naRing),
                          p_Copy((poly)b, naRing), naRing);
  definiteReduce(aTimesB, naMinpoly, cf);
  return (number)aTimesB;
}

number naDiv(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (b == NULL) WerrorS(nDivBy0);
  if (a == NULL) return NULL;
  poly bInverse = (poly)naInvers(b, cf);
  poly aDivB = p_Mult_q(p_Copy((poly)a, naRing), bInverse, naRing);
  definiteReduce(aDivB, naMinpoly, cf);
  return (number)aDivB;
}

/* 0^0 = 0;
   for |exp| <= 7 compute power by a simple multiplication loop;
   for |exp| >= 8 compute power along binary presentation of |exp|, e.g.
      p^13 = p^1 * p^4 * p^8, where we utilise that
      p^(2^(k+1)) = p^(2^k) * p^(2^k);
   intermediate reduction modulo the minimal polynomial is controlled by
   the in-place method heuristicReduce(poly, poly, coeffs); see there.
*/
void naPower(number a, int exp, number *b, const coeffs cf)
{
  naTest(a);
  
  /* special cases first */
  if (a == NULL)
  {
    if (exp >= 0) *b = NULL;
    else          WerrorS(nDivBy0);
  }
  else if (exp ==  0) *b = naInit(1, cf);
  else if (exp ==  1) *b = naCopy(a, cf);
  else if (exp == -1) *b = naInvers(a, cf);
  
  int expAbs = exp; if (expAbs < 0) expAbs = -expAbs;
  
  /* now compute a^expAbs */
  poly pow; poly aAsPoly = (poly)a;
  if (expAbs <= 7)
  {
    pow = p_Copy(aAsPoly, naRing);
    for (int i = 2; i <= expAbs; i++)
    {
      pow = p_Mult_q(pow, p_Copy(aAsPoly, naRing), naRing);
      heuristicReduce(pow, naMinpoly, cf);
    }
    definiteReduce(pow, naMinpoly, cf);
  }
  else
  {
    pow = p_ISet(1, naRing);
    poly factor = p_Copy(aAsPoly, naRing);
    while (expAbs != 0)
    {
      if (expAbs & 1)
      {
        pow = p_Mult_q(pow, p_Copy(factor, naRing), naRing);
        heuristicReduce(pow, naMinpoly, cf);
      }
      expAbs = expAbs / 2;
      if (expAbs != 0)
      {
        factor = p_Mult_q(factor, factor, naRing);
        heuristicReduce(factor, naMinpoly, cf);
      }
    }
    p_Delete(&factor, naRing);
    definiteReduce(pow, naMinpoly, cf);
  }
  
  /* invert if original exponent was negative */
  number n = (number)pow;
  if (exp < 0)
  {
    number m = naInvers(n, cf);
    naDelete(&n, cf);
    n = m;
  }
  *b = n;
}

/* may reduce p modulo the reducer by calling definiteReduce;
   the decision is made based on the following heuristic
   (which should also only be changed here in this method):
      if (deg(p) > 10*deg(reducer) then perform reduction;
   modifies p */
void heuristicReduce(poly &p, poly reducer, const coeffs cf)
{
  #ifdef LDEBUG
  p_Test((poly)p, naRing);
  p_Test((poly)reducer, naRing);
  #endif
  if (p_Totaldegree(p, naRing) > 10 * p_Totaldegree(reducer, naRing))
    definiteReduce(p, reducer, cf);
}

void naWrite(number &a, const coeffs cf)
{
  naTest(a);
  if (a == NULL)
    StringAppendS("0");
  else
  {
    poly aAsPoly = (poly)a;
    /* basically, just write aAsPoly using p_Write,
       but use brackets around the output, if a is not
       a constant living in naCoeffs = cf->extRing->cf */
    BOOLEAN useBrackets = !(p_IsConstant(aAsPoly, naRing));
    if (useBrackets) StringAppendS("(");
    p_String0(aAsPoly, naRing, naRing);
    if (useBrackets) StringAppendS(")");
  }
}

const char * naRead(const char *s, number *a, const coeffs cf)
{
  poly aAsPoly;
  const char * result = p_Read(s, aAsPoly, naRing);
  *a = (number)aAsPoly;
  return result;
}

/* implemented by the rule lcm(a, b) = a * b / gcd(a, b) */
number naLcm(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if (a == NULL) return NULL;
  if (b == NULL) return NULL;
  number theProduct = (number)p_Mult_q(p_Copy((poly)a, naRing),
                                       p_Copy((poly)b, naRing), naRing);
  /* note that theProduct needs not be reduced w.r.t. naMinpoly;
     but the final division will take care of the necessary reduction */
  number theGcd = naGcd(a, b, cf);
  return naDiv(theProduct, theGcd, cf);
}

/* expects *param to be castable to AlgExtInfo */
static BOOLEAN naCoeffIsEqual(const coeffs cf, n_coeffType n, void * param)
{
  if (naID != n) return FALSE;
  AlgExtInfo *e = (AlgExtInfo *)param;
  /* for extension coefficient fields we expect the underlying
     polynomial rings to be IDENTICAL, i.e. the SAME OBJECT;
     this expectation is based on the assumption that we have properly
     registered cf and perform reference counting rather than creating
     multiple copies of the same coefficient field/domain/ring */
  return (naRing == e->r);
  /* (Note that then also the minimal ideals will necessarily be
     the same, as they are attached to the ring.) */
}

int naSize(number a, const coeffs cf)
{
  if (a == NULL) return -1;
  /* this has been taken from the old implementation of field extensions,
     where we computed the sum of the degree and the number of terms in
     (poly)a; so we leave it at that, for the time being;
     maybe, the number of terms alone is a better measure? */
  poly aAsPoly = (poly)a;
  int theDegree = 0; int noOfTerms = 0;
  while (aAsPoly != NULL)
  {
    noOfTerms++;
    int d = p_GetExp(aAsPoly, 1, naRing);
    if (d > theDegree) theDegree = d;
    pIter(aAsPoly);
  }
  return theDegree + noOfTerms;
}

/* performs polynomial division and overrides p by the remainder
   of division of p by the reducer;
   modifies p */
void definiteReduce(poly &p, poly reducer, const coeffs cf)
{
  #ifdef LDEBUG
  p_Test((poly)p, naRing);
  p_Test((poly)reducer, naRing);
  #endif
  p_PolyDiv(p, reducer, FALSE, naRing);
}

/* IMPORTANT NOTE: Since an algebraic field extension is again a field,
                   the gcd of two elements is not very interesting. (It
                   is actually any unit in the field, i.e., any non-
                   zero element.) Note that the below method does not operate
                   in this strong sense but rather computes the gcd of
                   two given elements in the underlying polynomial ring. */
number naGcd(number a, number b, const coeffs cf)
{
  naTest(a); naTest(b);
  if ((a == NULL) && (b == NULL)) WerrorS(nDivBy0);
  return (number)p_Gcd((poly)a, (poly)b, naRing);
}

number naInvers(number a, const coeffs cf)
{
  naTest(a);
  if (a == NULL) WerrorS(nDivBy0);
  poly aFactor = NULL; poly mFactor = NULL;
  poly theGcd = p_ExtGcd((poly)a, aFactor, naMinpoly, mFactor, naRing);
  naTest((number)theGcd); naTest((number)aFactor); naTest((number)mFactor);
  /* the gcd must be 1 since naMinpoly is irreducible and a != NULL: */
  assume(naIsOne((number)theGcd, cf));      
  p_Delete(&theGcd, naRing);
  p_Delete(&mFactor, naRing);
  return (number)(aFactor);
}

/* assumes that src = Q, dst = Q(a) */
number naMap00(number a, const coeffs src, const coeffs dst)
{
  if (n_IsZero(a, src)) return NULL;
  assume(src == dst->extRing->cf);
  poly result = p_One(dst->extRing);
  p_SetCoeff(result, naCopy(a, src), dst->extRing);
  return (number)result;
}

/* assumes that src = Z/p, dst = Q(a) */
number naMapP0(number a, const coeffs src, const coeffs dst)
{
  if (n_IsZero(a, src)) return NULL;
  /* mapping via intermediate int: */
  int n = n_Int(a, src);
  number q = n_Init(n, dst->extRing->cf);
  poly result = p_One(dst->extRing);
  p_SetCoeff(result, q, dst->extRing);
  return (number)result;
}

/* assumes that either src = Q(a), dst = Q(a), or
                       src = Z/p(a), dst = Z/p(a)     */
number naCopyMap(number a, const coeffs src, const coeffs dst)
{
  return naCopy(a, dst);
}

/* assumes that src = Q, dst = Z/p(a) */
number naMap0P(number a, const coeffs src, const coeffs dst)
{
  if (n_IsZero(a, src)) return NULL;
  int p = rChar(dst->extRing);
  int n = nlModP(a, p, src);
  number q = n_Init(n, dst->extRing->cf);
  poly result = p_One(dst->extRing);
  p_SetCoeff(result, q, dst->extRing);
  return (number)result;
}

/* assumes that src = Z/p, dst = Z/p(a) */
number naMapPP(number a, const coeffs src, const coeffs dst)
{
  if (n_IsZero(a, src)) return NULL;
  assume(src == dst->extRing->cf);
  poly result = p_One(dst->extRing);
  p_SetCoeff(result, naCopy(a, src), dst->extRing);
  return (number)result;
}

/* assumes that src = Z/u, dst = Z/p(a), where u != p */
number naMapUP(number a, const coeffs src, const coeffs dst)
{
  if (n_IsZero(a, src)) return NULL;
  /* mapping via intermediate int: */
  int n = n_Int(a, src);
  number q = n_Init(n, dst->extRing->cf);
  poly result = p_One(dst->extRing);
  p_SetCoeff(result, q, dst->extRing);
  return (number)result;
}

nMapFunc naSetMap(const coeffs src, const coeffs dst)
{
  /* dst is expected to be an algebraic field extension */
  assume(getCoeffType(dst) == naID);
  
  int h = 0; /* the height of the extension tower given by dst */
  coeffs bDst = nCoeff_bottom(dst, h); /* the bottom field in the tower dst */
  
  /* for the time being, we only provide maps if h = 1 and if b is Q or
     some field Z/pZ: */
  if (h != 1) return NULL;
  if ((!nCoeff_is_Zp(bDst)) && (!nCoeff_is_Q(bDst))) return NULL;
  
  if (nCoeff_is_Q(src) && nCoeff_is_Q(bDst))
    return naMap00;                                      /// Q     -->  Q(a)
  
  if (nCoeff_is_Zp(src) && nCoeff_is_Q(bDst))
    return naMapP0;                                      /// Z/p   -->  Q(a)
  
  if (nCoeff_is_Q(src) && nCoeff_is_Zp(bDst))
    return naMap0P;                                      /// Q      --> Z/p(a)
  
  if (nCoeff_is_Zp(src) && nCoeff_is_Zp(bDst))
  {
    if (src->ch == dst->ch) return naMapPP;              /// Z/p    --> Z/p(a)
    else return naMapUP;                                 /// Z/u    --> Z/p(a)
  }
  
  coeffs bSrc = nCoeff_bottom(src, h); /* the bottom field in the tower src */
  if (h != 1) return NULL;
  if ((!nCoeff_is_Zp(bSrc)) && (!nCoeff_is_Q(bSrc))) return NULL;
  
  if (nCoeff_is_Q(bSrc) && nCoeff_is_Q(bDst))
  {
    if (strcmp(rRingVar(0, src->extRing),
               rRingVar(0, dst->extRing)) == 0)
      return naCopyMap;                                  /// Q(a)   --> Q(a)
    else
      return NULL;                                       /// Q(b)   --> Q(a)
  }
  
  if (nCoeff_is_Zp(bSrc) && nCoeff_is_Zp(bDst))
  {
    if (strcmp(rParameter(src->extRing)[0],
               rParameter(dst->extRing)[0]) == 0)
      return naCopyMap;                                  /// Z/p(a) --> Z/p(a)
    else
      return NULL;                                       /// Z/p(b) --> Z/p(a)
  }
  
  return NULL;                                           /// default
}

BOOLEAN naInitChar(coeffs cf, void * infoStruct)
{  
  AlgExtInfo *e = (AlgExtInfo *)infoStruct;
  /// first check whether cf->extRing != NULL and delete old ring???
  cf->extRing           = e->r;
  cf->extRing->minideal = e->i;

  assume(cf->extRing                     != NULL);      // extRing;
  assume((cf->extRing->minideal          != NULL) &&    // minideal has one
         (IDELEMS(cf->extRing->minideal) != 0)    &&    // non-zero generator
         (cf->extRing->minideal->m[0]    != NULL)    ); // at m[0];
  assume(cf->extRing->cf                 != NULL);      // extRing->cf;
  assume(getCoeffType(cf) == naID);                     // coeff type;
  
  /* propagate characteristic up so that it becomes
     directly accessible in cf: */
  cf->ch = cf->extRing->cf->ch;
  
  #ifdef LDEBUG
  p_Test((poly)naMinpoly, naRing);
  #endif
  
  cf->cfGreaterZero  = naGreaterZero;
  cf->cfGreater      = naGreater;
  cf->cfEqual        = naEqual;
  cf->cfIsZero       = naIsZero;
  cf->cfIsOne        = naIsOne;
  cf->cfIsMOne       = naIsMOne;
  cf->cfInit         = naInit;
  cf->cfInt          = naInt;
  cf->cfNeg          = naNeg;
  cf->cfPar          = naPar;
  cf->cfAdd          = naAdd;
  cf->cfSub          = naSub;
  cf->cfMult         = naMult;
  cf->cfDiv          = naDiv;
  cf->cfExactDiv     = naDiv;
  cf->cfPower        = naPower;
  cf->cfCopy         = naCopy;
  cf->cfWrite        = naWrite;
  cf->cfRead         = naRead;
  cf->cfDelete       = naDelete;
  cf->cfSetMap       = naSetMap;
  cf->cfGetDenom     = naGetDenom;
  cf->cfGetNumerator = naGetNumerator;
  cf->cfRePart       = naCopy;
  cf->cfImPart       = naImPart;
  cf->cfCoeffWrite   = naCoeffWrite;
  cf->cfDBTest       = naDBTest;
  cf->cfGcd          = naGcd;
  cf->cfLcm          = naLcm;
  cf->cfSize         = naSize;
  cf->nCoeffIsEqual  = naCoeffIsEqual;
  cf->cfInvers       = naInvers;
  cf->cfIntDiv       = naDiv;
  
  return FALSE;
}