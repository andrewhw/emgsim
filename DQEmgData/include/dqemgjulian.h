/**
 ** This header describes the interface to the Julian/Gregorian
 ** calendar convertr from the Communications of the ACM
 ** Volume 6, No. 8, (Aug. 1963).
 **
 ** $Id: dqemgjulian.h 117 2010-04-24 15:27:52Z andrew $
 **/
#ifndef __ACM_JULIAN_GREGORIAN_CONVERTER_HEADER__
#define __ACM_JULIAN_GREGORIAN_CONVERTER_HEADER__

	////////////////////////////////
	// Is the given year a leap year?
	int dqIsLeapYear(int year);

	////////////////////////////////
	// Is the given day/month/year combo
	// valid for the given (possibly leap)
	// year?
	int dqIsDayInMonth(int year, int month, int day);

	////////////////////////////////
	// Return the year portion of a Julian date
	long dqGetYearFromJulianDate(long julianDate);

	////////////////////////////////
	// Return the month portion of a Julian date
	unsigned long dqSGetMonthFromJulianDate(long julianDate);

	////////////////////////////////
	// Return the day portion of a Julian date
	unsigned long dqGetDayFromJulianDate(long julianDate);

	////////////////////////////////
	// Is the Gregorian date a valid date?
	int dqIsGregorianDateValid(
				long year,
				unsigned long month,
				unsigned long day
			);


	////////////////////////////////
	// Convert Gregorian calendar date to the corresponding Julian day
	// number j.
	//
	// From 199 from Communications of the ACM, Volume 6, No. 8,
	// (Aug. 1963), p. 444.
	//
	// Gregorian calendar started on Sep. 14, 1752; this function
	// not valid before that.
	// 
	int dqGetJulianDate(long *julianDate, int year, int month, int day);

	////////////////////////////////
	// Convert a Julian day number to its corresponding Gregorian
	// calendar date.
	//
	// Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
	// (Aug. 1963), p. 444.
	//
	// Gregorian calendar started on Sep. 14, 1752; this function not
	// valid for dates before that.  
	//
	int dqGetGregorianDate(
				unsigned long* year,
				unsigned long* month,
				unsigned long* day,
				long julianDate
			);

#endif
