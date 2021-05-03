/**
 ** This code is derived from a publication in the Communications
 ** of the ACM Volume 6, No. 8, (Aug. 1963).
 **
 ** The ACM has placed this code in the public domain.
 **
 ** $Id: dqemgjulian.cpp 108 2009-12-24 20:11:56Z andrew $
 **/

static int sDQNumDaysInMonth_[12]
                = { 31, 28, 31, 30, 31, 30,
                    31, 31, 30, 31, 30, 31 };

#include "dqemgjulian.h"

int
dqIsLeapYear(int year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		return 1;

	return 0;
}


int
dqIsDayInMonth(int year, int month, int day)
{
	int d;

	if ((day <= 0) || (month <=0) || (month > 12))
		return 1;

	d = sDQNumDaysInMonth_[month-1];

	if (dqIsLeapYear(year) && month == 2)
		d++;
	return day <= d;
}


long
dqGetYearFromJulianDate(long julianDate)
{
	unsigned long m;
	unsigned long d;
	unsigned long y;

	dqGetGregorianDate(&y, &m, &d, julianDate);

	return y;
}

unsigned long 
dqSGetMonthFromJulianDate(long julianDate)
{
	unsigned long m;
	unsigned long d;
	unsigned long y;

	dqGetGregorianDate(&y, &m, &d, julianDate);

	return m;
}

unsigned long 
dqGetDayFromJulianDate(long julianDate)
{
	unsigned long m;
	unsigned long d;
	unsigned long y;

	dqGetGregorianDate(&y, &m, &d, julianDate);

	return d;
}

int
dqIsGregorianDateValid(long year, unsigned long month, unsigned long day)
{
	if ((month <= 0) || (month > 12))
	{
		return 0;
	}

	if ( ! dqIsDayInMonth(year, month, day) )
	{
		return 0;
	}

	return 1;
}


/**
 ** Convert Gregorian calendar date to the corresponding Julian day
 ** number j.
 **
 ** From 199 from Communications of the ACM, Volume 6, No. 8,
 ** (Aug. 1963), p. 444.
 **
 ** Gregorian calendar started on Sep. 14, 1752; this function
 ** not valid before that.
 **/
int
dqGetJulianDate(
        long *julianDate,
        int year,
        int month,
        int day
    )
{
	int c, ya, j;
	int yh;

	yh = year;
	if( ! dqIsGregorianDateValid(year, month, day) )
	{
		return 0;
	}


	if (month > 2)
	{
		month -= 3;
	}
	else
	{
		/* wash out the leap day */
		month += 9;
		year--;
	} 
	c = year / 100;
	ya = year - 100*c;

	j = ( ( 146097 * c ) >> 2 ) +
		( ( 1461 * ya ) >> 2 ) +
		(153 * month + 2 ) / 5 + day + 1721119;

	if (dqGetYearFromJulianDate(j) != yh)
		return 0;

	(*julianDate) = j;
	return 1;
} 


/**
 ** Convert a Julian day number to its corresponding Gregorian
 ** calendar date.
 **
 ** Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
 ** (Aug. 1963), p. 444.
 **
 ** Gregorian calendar started on Sep. 14, 1752; this function not
 ** valid for dates before that.  
 **/
int
dqGetGregorianDate(
        unsigned long* year,
        unsigned long* month,
        unsigned long* day,
        long julianDate
    )
{
	long d;
	long j = julianDate - 1721119;

	(*year) = (unsigned long) ( ( ( j << 2 ) - 1 ) / 146097 );
	j = ( j << 2 ) - 1 - 146097 * (*year);
	d = ( j >> 2);
	j = ( ( d << 2 ) + 3 ) / 1461;
	d = ( d << 2 ) + 3 - 1461 * j;
	d = ( d + 4 ) >> 2;
	(*month) = (unsigned long) ( 5 * d - 3 ) / 153;
	d = 5 * d - 3 - 153 * (*month);
	(*day) = (unsigned long) ( ( d + 5 ) / 5 );
	(*year) = (unsigned long) ( 100 * (*year) + j );

	if ((*month) < 10)
		(*month) += 3;
	else
	{
		(*month) -= 9;
		(*year)++;
	} 

	return 1;
} 

