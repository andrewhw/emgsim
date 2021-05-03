/** ------------------------------------------------------------
 ** Log file and debugging control tools
 ** ------------------------------------------------------------
 ** $Id: log.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef         LOG_HEADER__
#define         LOG_HEADER__

#ifndef MAKEDEPEND
#include        <stdio.h>
#endif

#include        "os_defs.h"

/**
 ** MACRO DEFINITIONS
 **/

/*
#define         LOG_SCREEN		0x001
#define         LOG_FILE		4x002
#define         LOG_SCREEN_FD		1
#define         LOG_ERROR_FD		2
*/
#define         LOGDEST_STDERR		0x01
#define         LOGDEST_FILE		0x02
#define         LOGDEST_LOCALFILE	0x04
#define         LOGDEST_TIMESTAMP	0x08
#define         LOGDEST_FUNCTION	0x10
#define         LOGDEST_NO_ID		0x20
#define         LOGDEST_UNINIT		0x20

#define         LOG_ID_LEN		8

#define         LOG_DIR_ENV		"RWLOGDIR"
#define         LOG_ERROR_EXT		".log"


#define         LOG_EMERG               1
#define         LOG_ALERT               2
#define         LOG_CRIT                3
#define         LOG_ERR                 4
#define         LOG_WARNING             5
#define         LOG_NOTICE              6
#define         LOG_INFO                7
#define         LOG_DEBUG               8


/**
 ** ------------------------------------------------------------
 ** YAP*() debugging macros -- the whole idea is to have a
 ** printf() like function that will disappear at the time
 ** when DEBUG is undefined
 **     
 ** ------------------------------------------------------------
 **
 ** this turns on conditional YAP(id, flag, variable, ("message"));
 ** type commands.
 ** the '(0)' replacement instead of the null replacement
 ** is due to a semi-mythical bug reported by Rodney in gcc,
 ** where having null lines caused incorrect compilation
 **/


#ifdef          DEBUG
# define        YAP(_yap_id, _yap_flags, _yap_variable, _yap_string)\
                    do { \
                        if ( LogIsOpened_() ) { \
                            if ( (_yap_flags) & (_yap_variable) ) { \
                                LogId_  ( _yap_id ); \
                                LogMessage_      _yap_string; \
                            } \
                        } \
                    } while ( 0 )

# define        YAP_AT(_yap_id, _yap_flags, _yap_variable, _yap_string)\
                    do { \
                        if ( LogIsOpened_() ) { \
                            if ( (_yap_flags) & (_yap_variable) ) { \
                                LogId_  ( _yap_id ); \
                                LogMessage_     ("%s (%d):\t", \
                                        __FILE__, __LINE__); \
                                LogMessage_     _yap_string; \
                            } \
                        } \
                    } while ( 0 )

# define        YAP_FUNC(_yap_id, _yap_flags, _yap_variable, _yap_func)\
                    do { \
                        if ( LogIsOpened_() ) { \
                            if ( (_yap_flags) & (_yap_variable) ) { \
                                _yap_func; \
                            } \
                        } \
                    } while ( 0 )

#else

# ifdef OS_BARE_SEMI_OK
#  define        YAP(i, f, v, s)
#  define        YAP_AT(i, f, v, s)
#  define        YAP_FUNC(i, f, v, ff)
# else
#  define        YAP(i, f, v, s)         ( 0 )
#  define        YAP_AT(i, f, v, s)      ( 0 )
#  define        YAP_FUNC(i, f, v, ff)   ( 0 )
# endif

#endif

#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

#define LogError        LogErr
#define LogWarning      LogWarn

    /** log.h **/
void            LogId_(const char *idString);
int             LogMessage_(const char *fmt, ...);

OS_EXPORT int LogEmerg(const char *fmt, ...);
OS_EXPORT int LogAlert(const char *fmt, ...);
OS_EXPORT int LogCrit(const char *fmt, ...);
OS_EXPORT int LogErr(const char *fmt, ...);
OS_EXPORT int LogWarn(const char *fmt, ...);
OS_EXPORT int LogNotice(const char *fmt, ...);
OS_EXPORT int LogInfo(const char *fmt, ...);
OS_EXPORT int LogDebug(
			const char *file,
			int line,
			const char *fmt,
			...
		    );

OS_EXPORT int LogOpen(
			const char *progname,
			int destflags,
			const char *filename
		    );
OS_EXPORT int LogSetProgname(const char *progname);
OS_EXPORT int LogClose(void);
OS_EXPORT int LogFromFD(const char *id, int fd, int size);

OS_EXPORT int LogFunction(
			void (*logFunction)
			(void *userdata, int loglevel,
				char *buffer, int size),
			void *userData
                );

OS_EXPORT void (*LogGetLogFunction()) (void *, int, char *, int);
OS_EXPORT void *LogGetLogFunctionUserData();

OS_EXPORT int LogIsOpened_(void);
OS_EXPORT void LogFlush(void);
OS_EXPORT char *LogPromptUser(const char *message);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif
#endif

#endif  /* LOG_HEADER__ */


