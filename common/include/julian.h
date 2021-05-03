/**
 ** This header describes the interface to the Julian/Gregorian
 ** calendar convertr from the Communications of the ACM
 ** Volume 6, No. 8, (Aug. 1963).
 **/
#ifndef	__ACM_JULIAN_GREGORIAN_CONVERTER_HEADER__
#define	__ACM_JULIAN_GREGORIAN_CONVERTER_HEADER__

/**
 ** The implementation of these routines is in C; we therefore
 ** protect them for C++ inclusion and use
 **/
# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

    int jgIsLeapYear(int year);
    int jgIsDayInMonth(int year, int month, int day);
    int jgGetYearFromJulianDate(long julianDate);
    unsigned long jgGetMonthFromJulianDate(long julianDate);
    unsigned long jgGetDayFromJulianDate(long julianDate);
    int jgIsGregorianDateValid(
    		long year,
		unsigned long month,
		unsigned long day
	    );


    /**
     * Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
     * (Aug. 1963), p. 444.
     *
     * Months are specified in the range 1 upwards with 1==Jan, 12==Dec.
     * Days are the direct integer value for the day (ie; beginning at
     * 1 also)
     *
     * Note that the Gregorian calendar started on Sep. 14, 1752; this
     * function is not valid for dates prior to this point, but will
     * return date values as though the Gregorian calendar was in force.
     * Therefore Julian-sequence days are returned from the date of
     * March 1, year 0 of the Gregorian calendar sequence (Julian 1721120),
     * and can be used to extrapolate date ranges using this method.
     */

    /**
     * Convert Gregorian calendar date to the corresponding Julian day
     * number j.
     */
    int jgGetJulianDate(long *julianDate, int year, int month, int day);

    /**
     * Convert a Julian day number to its corresponding Gregorian
     * calendar date.
     */
    int jgGetGregorianDate(
    		unsigned long* year,
		unsigned long* month,
		unsigned long* day,
		long julianDate
	    );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

