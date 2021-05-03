/** ------------------------------------------------------------
 ** log subsystem
 ** ------------------------------------------------------------
 ** $Id: log.c 107 2013-06-22 20:22:05Z andrew $
 **/

#include "os_defs.h"


#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#ifndef                OS_WINDOWS_NT
#include <sys/fcntl.h>
#include <unistd.h>
#else
#include <fcntl.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#endif

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


#include "tclCkalloc.h"
#include "stringtools.h"
#include "massert.h"
#include "pathtools.h"
#include "dir_defaults.h"
#include "log.h"
#include "listalloc.h"
#include "msgir.h"



/** local defines               **/
#define                 LOG_BUFSIZE                     8192
#define                 LOG_LOAD_BUFSIZE                4096

/** local variables             **/
static int      log_fd = (-1),
                log_dest = LOGDEST_UNINIT,
                log_time = 0,
                log_isInit = 0,
                log_print_id_option = 1,
                log_last_loglevel = 0,
                log_prevOutputOffset = 0;
static char     log_obuf[LOG_BUFSIZE], log_writebuf[LOG_BUFSIZE], log_id_string[LOG_ID_LEN + 2], *log_filename = NULL, *log_progname = NULL;

/** local functions             **/
static int 
log_output_(
		int logleve,
		const char *string,
		int useTime
	);
static int      log_output_id_(char *writebuffer);
static int 
log_flush_line_(
		int loglevel,
		char *msgline,
		int withcr
	);
static int      log_print_time_(int loglevel);
static int      log_save_file_info_(const char *filename);
static int      log_open_log_file_(void);
static int      log_close_log_file_(void);

static void     (*log_user_function) (void *userData, int loglevel,
                                             char *buffer, int size) = NULL;
static void    *log_user_data = NULL;


/*
 * actually print out the message
 */
static int
log_output_(loglevel, message, useTime)
    int             loglevel;
    const char     *message;
    int             useTime;
{
	static int      log_print_id_flag = 1;

	log_last_loglevel = loglevel;


	while (*message)
	{

		/* check if we are buffering a carriage return flush */
		if (log_print_id_flag)
		{
			if (log_print_id_option)
			{
				if (!log_output_id_(log_writebuf))
					return (0);
				log_prevOutputOffset = strlen(log_writebuf);
			} else
			{
				log_prevOutputOffset = 0;
			}
			log_print_id_flag = 0;
		}
		log_writebuf[log_prevOutputOffset++] = *message;

		if (*message == '\n')
		{
			log_print_id_flag = 1;

			/** truncate the string */
			log_writebuf[log_prevOutputOffset] = 0;

			if (useTime && log_time)
				log_print_time_(loglevel);

			log_flush_line_(loglevel, log_writebuf, 0);

			log_prevOutputOffset = 0;
		}
		message++;
	}

	return (1);
}

static int
log_flush_line_(loglevel, msgline, withcr)
    int             loglevel;
    char           *msgline;
    int             withcr;
{
	int             lineLen;

	/** if we haven't set anything up; default to stderr */
	if (log_dest == LOGDEST_UNINIT)
	{
		log_dest = LOGDEST_STDERR;
	}

	lineLen = strlen(msgline);
	if (log_dest & (LOGDEST_FUNCTION))
	{
		if (log_user_function != NULL)
			(void) (*log_user_function) (
										 log_user_data,
										 loglevel,
										 msgline,
										 strlen(msgline));
	}
	/** if we have any non-functon output, do it */
	if ((log_dest & (~LOGDEST_FUNCTION)) != 0)
	{

		if (log_dest & (LOGDEST_FILE | LOGDEST_LOCALFILE))
			logWrite(log_fd, msgline, lineLen);


		if (log_dest == 0 || log_dest & LOGDEST_STDERR)
		{
			fputs(msgline, stderr);
		}
	}
	return (1);
}


static int
log_print_time_(loglevel)
    int             loglevel;
{
	static char     outBuf[256], *loadBuf = NULL;
	time_t          curTime;
	int             len;


	if (log_progname == NULL)
	{
		strlcpy(outBuf, "???", 256);
	} else
	{
		strlcpy(outBuf, log_progname, 256);
	}
	len = strlen(outBuf);
	outBuf[len] = ' ';
	loadBuf = &outBuf[len + 1];

	/** add in id               **/
	log_output_id_(loadBuf);

	curTime = time(NULL);
	slnprintf(&loadBuf[LOG_ID_LEN + 1], LOG_BUFSIZE - (LOG_ID_LEN + 1),
					"Time -- %s", ctime(&curTime));
	len = strlen(outBuf);

	if (log_dest & (LOGDEST_FUNCTION))
	{
		if (log_user_function != NULL)
			(void) (*log_user_function) (log_user_data,
										 loglevel, outBuf, len);
	}
	if (log_dest & (LOGDEST_FILE | LOGDEST_LOCALFILE))
		logWrite(log_fd, outBuf, len);

	if (log_dest == 0 || log_dest & LOGDEST_STDERR)
		fputs(outBuf, stderr);

	return (1);
}




/*
 * print out the ID string
 */
static int
log_output_id_(outbuf)
    char           *outbuf;
{
	memcpy(outbuf, log_id_string, LOG_ID_LEN + 1);
	outbuf[LOG_ID_LEN + 1] = 0;
	return (1);
}
/*
#endif          / * OS_WINDOWS_NT * /
*/




/*
 * ---------------------------------------------
 * register the actual string for the ID, pad
 * with blanks
 * ---------------------------------------------
 */
void
LogId_(s)
    const char     *s;
{
	int             len = strlen(s);

	if (len > LOG_ID_LEN)
		len = LOG_ID_LEN;

	(void) memcpy(log_id_string, s, len);

	if (len < LOG_ID_LEN)
		(void) memset(&log_id_string[len], ' ', LOG_ID_LEN - len);

	log_id_string[LOG_ID_LEN] = ':';
}




/*
 * ---------------------------------------------
 * Log a message at debug level, using current
 * id (used from the YAP system)
 * ---------------------------------------------
 */
int
LogMessage_(const char *fmt,...)
{
	va_list         vargs;

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_DEBUG, log_obuf, 0));
}




/*
 * ---------------------------------------------
 * Log an EMERGENCY message
 * ---------------------------------------------
 */
OS_EXPORT int
LogEmerg(const char *fmt,...)
{
	va_list         vargs;

	LogId_("Emergency");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_EMERG, log_obuf, 1));
}




/*
 * ---------------------------------------------
 * Log an ALERT message
 * ---------------------------------------------
 */
OS_EXPORT int
LogAlert(const char *fmt,...)
{
	va_list         vargs;

	LogId_("Alert");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_ALERT, log_obuf, 1));
}




/*
 * ---------------------------------------------
 * Log a CRITICAL message
 * ---------------------------------------------
 */
OS_EXPORT int
LogCrit(const char *fmt,...)
{
	va_list         vargs;

	LogId_("Critical");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_CRIT, log_obuf, 1));
}




/*
 * ---------------------------------------------
 * Log an ERROR message
 * ---------------------------------------------
 */
OS_EXPORT int
LogErr(const char *fmt,...)
{
	va_list vargs;
	int status;

	LogId_("Error");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	status = log_output_(LOG_ERR, log_obuf, 0);
	return status;
}





/*
 * ---------------------------------------------
 * Log a WARNING message
 * ---------------------------------------------
 */
OS_EXPORT int
LogWarn(const char *fmt,...)
{
	va_list         vargs;

	LogId_("Warning");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_WARNING, log_obuf, 1));
}




/*
 * ---------------------------------------------
 * Log a NOTICE level message
 * ---------------------------------------------
 */
OS_EXPORT int
LogNotice(const char *fmt,...)
{
	va_list         vargs;

	LogId_("Notice");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_NOTICE, log_obuf, 1));
}





/*
 * ---------------------------------------------
 * Log an INFORMATION level message
 * ---------------------------------------------
 */
OS_EXPORT int
LogInfo(const char *fmt,...)
{
	va_list         vargs;

	/* LogId_("  Info"); */
	LogId_("");

	va_start(vargs, fmt);
#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(log_obuf, LOG_BUFSIZE, fmt, vargs);
#else
	(void) vsprintf(log_obuf, fmt, vargs);
#endif
	va_end(vargs);

	return (log_output_(LOG_INFO, log_obuf, 0));
}




/*
 * ---------------------------------------------
 * Log a debugging message (see also LogMessage_)
 * ---------------------------------------------
 */
OS_EXPORT int
LogDebug(const char *file, int line, const char *fmt,...)
{
	va_list vargs;
	int bufUsed, bufRemain;
	LogId_("Debug");

	va_start(vargs, fmt);
	(void) slnprintf(log_obuf, LOG_BUFSIZE, "%s (%d) : ", file, line);

	bufUsed = strlen(log_obuf);
	bufRemain = LOG_BUFSIZE - bufUsed;

#ifdef		OS_HAS_SNPRINTF
	(void) vsnprintf(&log_obuf[bufUsed], bufRemain, fmt, vargs);
#else
	(void) vsprintf(&log_obuf[bufUsed], fmt, vargs);
#endif

	va_end(vargs);

	return (log_output_(LOG_DEBUG, log_obuf, 0));
}




/*
 * ---------------------------------------------
 * Open the log system
 * ---------------------------------------------
 */
OS_EXPORT int
LogOpen(progname, destflags, filename)
    const char     *progname;
    int             destflags;
    const char     *filename;
{
	char           *delimpos;

	if (destflags == 0)
		destflags = LOGDEST_STDERR;

	if (destflags & LOGDEST_TIMESTAMP)
		log_time = 1;

	if (destflags & LOGDEST_NO_ID)
		log_print_id_option = 0;

	log_dest = destflags;
	if (log_progname != NULL)
	{
		ckfree(log_progname);
	}
	if (progname == NULL)
	{
		log_progname = NULL;
	} else
	{
		if ((delimpos = strrchr(progname, OS_PATH_DELIM)) != NULL)
		{
			log_progname = ckstrdup(delimpos + 1);
		} else
		{
			log_progname = ckstrdup(progname);
		}
		MSG_ASSERT(log_progname != NULL, "ckalloc failed");
	}


	if (log_dest & LOGDEST_FILE)
	{
#ifndef  OS_WINDOWS_NT
		/**
		 ** set up for logging to the file
		 **/
		char           *errLogDir, *tmp_filename;

		errLogDir = getBaseDirectory();

		if (confirmDirPath(errLogDir, 0777) < 0)
		{
			fprintf(stderr, "Cannot create path '%s' for log dir\n",
					errLogDir);
			return (0);
		}
		if (errLogDir[strlen(errLogDir) - 1] == OS_PATH_DELIM)
		{
			if (log_progname == NULL)
				tmp_filename = strconcat(errLogDir, "log",
										 LOG_ERROR_EXT, NULL);
			else
				tmp_filename = strconcat(errLogDir, log_progname,
										 LOG_ERROR_EXT, NULL);
		} else
		{
			if (log_progname == NULL)
				tmp_filename = strconcat(errLogDir,
										 OS_PATH_DELIM_STRING,
										 "log",
										 LOG_ERROR_EXT, NULL);
			else
				tmp_filename = strconcat(errLogDir,
										 OS_PATH_DELIM_STRING,
										 log_progname,
										 LOG_ERROR_EXT, NULL);
		}

		log_save_file_info_(tmp_filename);
		ckfree(tmp_filename);
#else
		log_dest |= LOGDEST_LOCALFILE;
#endif                          /* OS_WINDOWS_NT */
	}
	if (log_dest & LOGDEST_LOCALFILE)
	{
		log_save_file_info_(filename);
	}
	(void) log_open_log_file_();

	log_isInit = 1;

	return (1);
}





/*
 * ---------------------------------------------
 * Open the log system
 * ---------------------------------------------
 */
OS_EXPORT int
LogClose()
{
	log_time = 0;
	log_dest = LOGDEST_UNINIT;
	log_isInit = 0;

	if (log_progname != NULL)
	{
		ckfree(log_progname);
	}
	log_progname = NULL;
	if (log_filename != NULL)
	{
		ckfree(log_filename);
	}
	log_filename = NULL;

	(void) log_close_log_file_();

	return (1);
}




/*
 * ---------------------------------------------
 * (re)set the name we log under (lines start
 * with this, we keep the same log file)
 * ---------------------------------------------
 */
OS_EXPORT int
LogSetProgname(progname)
    const char     *progname;
{
	char           *delimpos;

	if (log_progname != NULL)
	{
		ckfree(log_progname);
	}
	if ((delimpos = strrchr(progname, OS_PATH_DELIM)) != NULL)
	{
		log_progname = ckstrdup(delimpos + 1);
	} else
	{
		log_progname = ckstrdup(progname);
	}
	MSG_ASSERT(log_progname != NULL, "ckalloc failed");

	return (1);
}




/*
 * ---------------------------------------------
 * util to open a file
 * ---------------------------------------------
 */
static int
log_open_log_file_(void)
{
	(void) log_close_log_file_();

	if (log_filename != NULL)
	{
		log_fd = openPath(log_filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	}
	return (log_fd > 0 ? 1 : 0);
}


/*
 * ---------------------------------------------
 * yeah?
 * ---------------------------------------------
 */
static int
log_close_log_file_(void)
{
	if (log_fd >= 0)
	{
		(void) irClose(log_fd);
		log_fd = (-1);
	}
	return (1);
}


/*
 * ---------------------------------------------
 * set the current file name
 * ---------------------------------------------
 */
static int
log_save_file_info_(filename)
    const char     *filename;
{
	log_filename = ckstrdup(filename);
	MSG_ASSERT(log_filename != NULL, "ckalloc failed");

	return (1);
}




/*
 * ---------------------------------------------
 * log a file descriptor into the system (like
 * stderr from the remote process)
 * ---------------------------------------------
 */
OS_EXPORT int
LogFromFD(id, fd, fileSize)
    const char     *id;
    int             fd, fileSize;
{
	char            loadbuf[LOG_LOAD_BUFSIZE + 1];
	int             expectedRead, bytesRemain, bytesRead = 0, totalRead = 0;

	LogId_(id);
	bytesRemain = fileSize;
	while (bytesRemain > 0)
	{
		expectedRead = LOG_LOAD_BUFSIZE < fileSize ?
			LOG_LOAD_BUFSIZE : fileSize;

		bytesRead = irRead(fd, loadbuf, expectedRead);

		if (bytesRead <= 0)
		{
			LogErr("Filelog truncated after %d bytes of expected %d\n",
				   totalRead, fileSize);
			return (0);
		}
		if (bytesRead > 0)
		{

			loadbuf[bytesRead] = 0;
			log_output_(LOG_NOTICE, loadbuf, 0);

			bytesRemain -= bytesRead;
			totalRead += bytesRead;
		}
	}

	/** flush output if trailing character is not a CR **/
	if ((totalRead > 0) && (loadbuf[bytesRead - 1] != '\n'))
		log_output_(LOG_NOTICE, "\n", 0);

	return (1);
}




/*
 * ---------------------------------------------
 * Are we set up?
 * ---------------------------------------------
 */
OS_EXPORT int
LogIsOpened_()
{
	return (log_isInit);
}


/*
 * ---------------------------------------------
 * Register a function to log with
 * ---------------------------------------------
 */
OS_EXPORT int
LogFunction(
            void (*userFunction) (void *userdata, int loglevel,
                                  char *buffer, int size),
            void *userData)
{
	log_dest |= LOGDEST_FUNCTION;
	log_user_data = userData;
	log_user_function = userFunction;

	return 1;
}

OS_EXPORT void (* LogGetLogFunction()) (void *, int, char *, int)
{
	return log_user_function;
}

OS_EXPORT void *
LogGetLogFunctionUserData()
{
	return log_user_data;
}

/*
 * ---------------------------------------------
 * Register a function to log with
 * ---------------------------------------------
 */
OS_EXPORT void
LogFlush()
{
	log_writebuf[log_prevOutputOffset] = 0;
	log_flush_line_(log_last_loglevel, log_writebuf, 1);
	log_prevOutputOffset = 0;
}

/**
 * let the user type in a line, return the first BUFSIZ characters
 */

#define MAX_LINE_LEN    128
OS_EXPORT char *
LogPromptUser(const char *message)
{
	int             c, k = 0;
	int             sawFirst = 0;
	static char     buffer[MAX_LINE_LEN];

	LogInfo(message);
	LogFlush();

	/**
	 * read what the user types, storing everything we see after the
	 * first non-whitespace character
	 */
	do
	{
		c = getc(stdin);
		if (sawFirst)
		{
			buffer[k++] = c;
		} else if (!isspace(c))
		{
			buffer[k++] = c;
			sawFirst = 1;
		}
	} while (c != '\n' && c != '\r' && k < (MAX_LINE_LEN - 1));
	buffer[k] = 0;

	/** eat the rest of the line the user may have typed */
	while (c != '\n' && c != '\r')
	{
		c = getc(stdin);
	}

	/** trim any whitespace which may have appeared at the end of the line */
	while (isspace(buffer[--k]))
		buffer[k] = 0;

	/** return what we got */
	return buffer;
}

