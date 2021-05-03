/** ------------------------------------------------------------
 ** Random number generation for various platforms, using
 ** various distributions.
 ** ------------------------------------------------------------
 ** $Id: random.c 75 2010-03-24 18:44:17Z andrew $
 **/

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include       <stdio.h>
#include       <stdlib.h>
#include       <math.h>
#include       <time.h>
#include       <limits.h>
#include       <sys/types.h>
#ifndef OS_WINDOWS_NT
#include        <unistd.h>
#else
#include        <process.h>
#endif
#endif

#include        "random.h"


const double    EPS = 1.2e-7;


static long     nrRandomSeed = 0;
static long     nrRandomSeedIsInitialized = 0;

int             gDumpRandom = 1;

OS_EXPORT int 
localRandom()
{
    int             result;

    if (nrRandomSeedIsInitialized == 0)
    {
        seedLocalRandom((int) time(NULL) *
#ifdef	OS_WINDOWS_NT
			_getpid()
#else
			getpid()
#endif
			);
    }
    result = (int) (nr_ran2(&nrRandomSeed) * OS_RAND_MAX);
    return result;
}

/* a uniformly distributed random value between 0 and 1 */
OS_EXPORT double
localRandomDouble()
{
    double          result;

    if (nrRandomSeedIsInitialized == 0)
    {
        seedLocalRandom((int) time(NULL) *
#ifdef	OS_WINDOWS_NT
			_getpid()
#else
			getpid()
#endif
			);
    }
    result = nr_ran2(&nrRandomSeed);
    return result;
}

/**
 ** This function creates an integer, positive random number
 ** from a uniform distribution between 0 and range.
 **/
OS_EXPORT int 
intRangeRandom(int range)
{
    int             result;
    result = (int) (localRandomDouble() * range);
    return result;
}

/**
 ** Return a float value in the range 0 - 1.0
 **/
OS_EXPORT float 
floatNormalizedRandom()
{
    return (float) localRandomDouble();
}

/**
 ** Return a float value in the range -range to +range
 **/
OS_EXPORT float 
floatSignedRangeRandom(float range)
{
    float           fran;
    int             sign;

    fran = (float) (localRandomDouble() * range);
    sign = localRandom() & 0x01;
    if (sign == 0)
        fran = -fran;
    return (fran);
}

/**
 ** ----------------------------------------------------------------
 ** functions defined for Windows where these are missing
 **/
#ifdef  OS_WINDOWS_NT

/**
 ** ----------------------------------------------------------------
 ** Returns the natural log of the Gamma function of xx:
 ** ln[ Tau(xx) ], for xx > 0
 **/
OS_EXPORT double 
lgamma(double xx)
{
    static double   stp = 2.5066282746310005;
    static double   cof[6] = {
        76.18009172947146,
        -86.50532032941677,
        24.01409824083091,
        -1.231739572450155,
        0.128650973866179e-2
        - 0.5395239384953e-5
    };

    double          ser, tmp, x, y;
    int             j;

    x = xx;
    y = x;
    tmp = x + 5.5;
    tmp = (x + 0.5) * log(tmp) - tmp;
    ser = 1.000000000190015;
    for (j = 0; j < 6; j++)
    {
        y = y + 1.0;
        ser = ser + cof[j] / y;
    }

    return tmp + log(stp * ser / x);
}

#endif

/**
 ** Random value function found in Numerical Recipes as "ran2"
 **
 ** Long period (> 2 x 10^18) random number generator of L'Ecuyer
 ** with Bays-Durham shuffle and added safeguards.  Returns a
 ** uniform random deviate bewteen 0.0 and 1.0 (exclusive of the
 ** endpoint values).  Call with <idum> a negative integer to
 ** initialize; thereafter, do not alter <idum> between sucessive
 ** deviates in a sequence.  RNMX should approximate the largest
 ** floating value that is less than 1.
 **/
#define         RAN2_NTAB               32
OS_EXPORT double
nr_ran2(long *idum)
{
    const long      IM1 = 2147483563;
    const long      IM2 = 2147483399;
    const double    AM = 1.0 / IM1;
    const long      IMM1 = IM1 - 1;
    const long      IA1 = 40014;
    const long      IA2 = 40692;
    const long      IQ1 = 53668;
    const long      IQ2 = 52774;
    const long      IR1 = 12211;
    const long      IR2 = 3791;
    const long      NDIV = 1 + IMM1 / RAN2_NTAB;
    const double    RNMX = 1.0 - EPS;

    static long     idum2 = 123456789;
    static long     iy = 0;
    static long     iv[RAN2_NTAB] = {0};
    long            j, k;
    double          result;

    /** initialize */
    if ((*idum) <= 0)
    {

        /** be sure to prevent idum == 0 */
        (*idum) = (-(*idum) > 1) ? (-(*idum)) : 1;

        idum2 = (*idum);

        /** load the shuffle table (after 8 warm-ups) */
        for (j = RAN2_NTAB + 7; j >= 0; j--)
        {
            k = (*idum) / IQ1;
            (*idum) = IA1 * ((*idum) - k * IQ1) - k * IR1;
            if ((*idum) < 0)
                (*idum) = (*idum) + IM1;
            if (j < RAN2_NTAB)
                iv[j] = (*idum);
        }
        iy = iv[0];
    }
    /* start here when not initializing */
    k = (*idum) / IQ1;
    /*
     * Compute idum = mod(IA1 * idum, IM1) without overflows
     * by Schrage's method
     */
    (*idum) = IA1 * ((*idum) - k * IQ1) - k * IR1;
    if ((*idum) < 0)
        (*idum) = (*idum) + IM1;
    k = idum2 / IQ2;

    /** Compute idum2 = mod(IA2 * idum2, IM2) likewise */
    idum2 = IA2 * (idum2 - k * IQ2) - k * IR2;
    if (idum2 < 0)
        idum2 = idum2 + IM2;


    /** will be in the range 0:RAN2_NTAB-1 */
    j = iy / NDIV;

    /*
     * here idum is shuffled, idum and idum 2 are
     * combined to generate output
     */
    iy = iv[j] - idum2;
    iv[j] = (*idum);

    if (iy < 1)
        iy = iy + IMM1;

    result = (AM * iy) < RNMX ? (AM * iy) : RNMX;

    return result;
}


/**
 ** ----------------------------------------------------------------
 ** pgauss:     returns cumulative probability & density for a
 ** standard normal deviate
 **/
OS_EXPORT double 
pgauss(double zdev, double *prob, double *dens)
{
    double          T;

    T = 1.0 / (1.0 + 0.2316419 * fabs(zdev));
    *dens = 0.398942280 * exp(-zdev * zdev / 2.0);
    *prob = 1.0 - *dens * T * ((((1.330274 * T - 1.821256) * T
                                 + 1.781478) * T - 0.3565638) * T +
                               0.3193815);
    if (zdev < 0)
        *prob = 1 - *prob;

    return (*prob);
}

/**
 ** ----------------------------------------------------------------
 ** zgauss:     returns a standard normal [0,1] deviate corresponding to
 ** a given cumulative probabilty value
 **/
OS_EXPORT double 
zgauss(double prob)
{
    double          T, T2;
    double          zdev;

    if (prob <= 0)
    {
        printf("Probability passed in is zero -- returning -Infinity\n");
        return (log(0));
    }
    if ((1.0 - prob) <= 0)
    {
        printf("Probability passed in is one -- returning +Infinity\n");
        return (-log(0));
    }
    if (prob > 0.5)
        T = 1.0 - prob;
    else
        T = prob;

    T2 = log(1.0 / (T * T));
    T = sqrt(T2);

    zdev = T - (2.515517 + 0.802853 * T + 0.010328 * T2) /
        (1.0 + 1.432788 * T + 0.189269 * T2 + 0.001308 * T * T2);

    if (prob < 0.5)
        zdev = -zdev;

    return (zdev);
}

/**
 ** ----------------------------------------------------------------
 ** This function returns a normally distributed deviate with zero
 ** mean and unit variance, using localRandomDouble() as the
 ** source of the uniform deviate
 **/
OS_EXPORT double 
gauss01()
{
    static int      iset = 0;
    static double   gset;
    double          fac, rsq, v1, v2;

    if (iset == 0)
    {
        /* we don't have an extra deviate handy, so . . . */
        do
        {
            /*
             * Pick two uniform numbers in the square extending
             * from -1 to +1 in each direction
             */
            v1 = 2.0 * localRandomDouble() - 1.0;
            v2 = 2.0 * localRandomDouble() - 1.0;

            /* check if they are in the unit circle */
            rsq = v1 * v1 + v2 * v2;
        } while (rsq >= 1.0 || rsq == 0.0);

        fac = sqrt(-2.0 * log(rsq) / rsq);

        /*
         * Now make the Box-Muller transformation to et two
         * normal deviates.  Return one and save the other
         * for next time.
         */
        gset = v1 * fac;

        /* set flag */
        iset = 1;
        return v2 * fac;
    }
    /* unset the flag */
    iset = 0;
    return gset;
}


/**
 ** ----------------------------------------------------------------
 ** Returns as a floating-point number an integer value that is a
 ** random deviate drawn from a Poisson distribution of the given
 ** mean, using localRandomDouble() as a source of uniform random
 ** deviates.
 **
 ** uses localRandomDouble(), gammaln()
 **/
OS_EXPORT double 
poisson(double mean)
{
    static double   alxm = 0.0, g = 0.0, sq = 0.0;
    /** flag for whether mean has changed since last call */
    static double   oldmean = (-1.0);
    double          em, t, y;

    if (mean < 12)
    {
        /** use direct method */
        if (mean != oldmean)
        {
            oldmean = mean;
            /** if mean is new, compute the exponential */
            g = exp(-mean);
        }
        em = (-1);
        t = 1;
        do
        {
            em = em + 1;

            /*
             * Instead of adding exponential deviates it is
             * equivalent to muliply uniform deviates.  We
             * never actually have to take the log, merely
             * compare the pre-computed exponential
             */
            t = t * localRandomDouble();
        } while (t > g);

    } else
    {
        /** use rejection method */
        /*
         * if mean has changed since our last call,
         * then precompute some functions that
         * occur below
         */
        if (mean != oldmean)
        {
            oldmean = mean;
            sq = sqrt(2.0 * mean);
            alxm = log(mean);
            /*
             * the function gammaln is the natural log of the
             * gamma function
             */
            g = mean * alxm - lgamma(mean + 1.0);
        }
        do
        {
            do
            {
                /*
                 * y is a deviate from a Lorentzian comparison
                 * function
                 */
                y = tan(M_PI * localRandomDouble());
                /** em is y, shifted and scaled */
                em = sq * y + mean;
                /** reject if in regime of zero probability */
            } while (em < 0);

            /** the trick for integer-valued distributions */
            em = (double) ((long) em);

            /*
             * The ratio of the desired distribution to the
             * comparison function; we accept or reject by
             * comparing it to another uniform deviate.
             * The factor 0.9 is chosen so that t never
             * exceeds 1.0
             */
            t = 0.9 * (1.0 + y * y)
                * exp(em * alxm - lgamma(em + 1.0) - g);
        } while (localRandomDouble() > t);
    }

    return em;
}

/**
 ** Seed the random number generator and re-set the counts
 **/
OS_EXPORT void 
seedLocalRandom(int seed)
{
    if (seed > 0)
    {
        nrRandomSeed = -seed;
        srandom(seed);
    } else
    {
        nrRandomSeed = seed;
        srandom(-seed);
    }

    nrRandomSeedIsInitialized = 1;
}

