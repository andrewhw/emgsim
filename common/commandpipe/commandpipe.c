/**
 * ------------------------------------------------------------
 * Run a command with a pipe to and from it
 * ------------------------------------------------------------
 * $Id: commandpipe.c 17 2008-07-03 17:24:49Z andrew $
 */

#include        "os_defs.h"

#ifndef MAKEDEPEND
#include        <stdio.h>
#include        <errno.h>
#include        <stdarg.h>
#include        <string.h>
#include        <ctype.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#ifndef OS_WINDOWS_NT
#include        <sys/wait.h>
#include        <unistd.h>
#include        <stdlib.h>
#else
#include        <io.h>
#endif
#include        <stdio.h>
#include        <time.h>
#include        <errno.h>

#endif

#include        "tclCkalloc.h"
#include        "massert.h"
#include        "listalloc.h"
#include        "commandpipe.h"


#define ENSUREBUFFER(b, s, cur) \
		do { \
			int x__; \
			x__ = listCheckSize((s), (void **) &(buf), &(cur), 1, 1); \
            MSG_ASSERT(x__, "malloc failed"); \
		} while (0)

/**
 * write out the gnuplot standard header to the given file pointer
 */
OS_EXPORT int
commandLineToArgv(
		int *nArgs,
		char ***arglist,
		char *commandLine
	)
{
	int nArgBlocks = 0;
	int beginQuote = 0;
	int inArgument = 0;
	int bufSize = 0;
	int bufAllocatedSize = 0;
	int status;
	char *buf = NULL;
	char *p;

	p = commandLine;

	/** ensure we start at some data */
	while (isspace(*p))
		p++;

	/** run through the rest of the command line */
	while (*p != 0)
	{
		if (isspace(*p))
		{
			if (beginQuote != 0)
			{
				ENSUREBUFFER(buf, bufSize + 2, bufAllocatedSize);
				buf[bufSize++] = *p;
				buf[bufSize] = 0;
			} else if (inArgument)
			{
				status = listCheckSize((*nArgs) + 2,
										 (void **) arglist,
										 &nArgBlocks, 2, sizeof(char **));
				MSG_ASSERT(status, "malloc failed");
				(*arglist)[(*nArgs)++] = buf;
				(*arglist)[(*nArgs)] = NULL;
				buf = NULL;
				bufAllocatedSize = bufSize = 0;
				inArgument = 0;
			}
		} else
		{

			switch (*p)
			{
			case '"':
			case '\'':
				inArgument = 1;
				if (beginQuote == 0)
				{
					beginQuote = *p;
				} else if (beginQuote == *p)
				{
					beginQuote = 0;
				} else
				{
					ENSUREBUFFER(buf, bufSize + 2, bufAllocatedSize);
					buf[bufSize++] = *p;
					buf[bufSize] = 0;
				}
				break;

			case '\\':
				ENSUREBUFFER(buf, bufSize + 2, bufAllocatedSize);
				inArgument = 1;
				switch (*(++p))
				{
				case 'n':
					buf[bufSize++] = '\n';
					break;

				case 't':
					buf[bufSize++] = '\t';
					break;

				case 'b':
					buf[bufSize++] = '\b';
					break;

				case 'v':
					buf[bufSize++] = '\v';
					break;

				case 'a':
					buf[bufSize++] = '\a';
					break;

				case 'r':
					buf[bufSize++] = '\r';
					break;

				default:
					buf[bufSize++] = *p;
				}
				buf[bufSize] = 0;
				break;

			default:
				ENSUREBUFFER(buf, bufSize + 2, bufAllocatedSize);
				buf[bufSize++] = *p;
				buf[bufSize] = 0;
				inArgument = 1;
			}
		}

		p++;
	}


	if (beginQuote != 0)
	{
		return 0;
	}
	if (inArgument)
	{
		status = listCheckSize((*nArgs) + 2,
								 (void **) arglist,
								 &nArgBlocks, 2, sizeof(char **));
		MSG_ASSERT(status, "malloc failed");
		(*arglist)[(*nArgs)++] = buf;
		(*arglist)[(*nArgs)] = NULL;
		buf = NULL;
		bufAllocatedSize = bufSize = 0;
		inArgument = 0;
	}
	return 1;
}

OS_EXPORT int
execCommandPipe(CommandPipe * cPipe, char *commandLine)
{
	char          **argv = NULL;
	int             pipe_p2c[2], pipe_c2p[2];
	int             argc = 0;
	int             status = (-1);
	int             i;

	if (!commandLineToArgv(&argc, &argv, commandLine))
	{
		return (-1);
	}
#ifdef  UNUSED
	{
		char           *delim = "";
		printf("Command Line:");
		for (i = 0; i < argc; i++)
		{
			printf("%s\"%s\"", delim, argv[i]);
			delim = " ";
		}
		printf("\n");
	}
#endif

	/*
	 * Create a pipe before the fork which we can use to communicate
	 * with the child process
	 */
	if (pipe(pipe_p2c) < 0)
	{
		goto CLEANUP;
	}
	if (pipe(pipe_c2p) < 0)
	{
		goto CLEANUP;
	}
	if ((cPipe->pid_ = fork()) < 0)
	{
		goto CLEANUP;
	}
	/** run the child */
	if (cPipe->pid_ == 0)
	{
		/*
		 * make read side of pipe stdin -- this is the magic to get the
		 * new file descriptor to appear where you want.  This works
		 * because you will get the lowest numbered available file
		 * descriptor when you open a new one.
		 */
		close(0);
		dup(pipe_p2c[0]);
		if (close(pipe_p2c[0]) < 0)
		{
			fprintf(stderr, "Failed closing %d\n", pipe_p2c[0]);
		}
		if (close(pipe_p2c[1]) < 0)
		{
			fprintf(stderr, "Failed closing %d\n", pipe_p2c[1]);
		}
		close(1);
		dup(pipe_c2p[1]);
		if (close(pipe_c2p[0]) < 0)
		{
			fprintf(stderr, "Failed closing %d\n", pipe_c2p[0]);
		}
		if (close(pipe_c2p[1]) < 0)
		{
			fprintf(stderr, "Failed closing %d\n", pipe_c2p[1]);
		}
		execvp(argv[0], argv);

		/** if we get here, exec() failed, so bail out */
		fprintf(stderr, "Exec failed : %s\n", strerror(errno));
		{
			char           *delim = "";

			fprintf(stderr, "exec[%s]:[", argv[0]);
			for (i = 0; i < argc; i++)
			{
				if ((index(argv[i], ' ') != NULL) ||
					(index(argv[i], '\t') != NULL))
				{
					fprintf(stderr, "%s\"%s\"", delim, argv[i]);
				} else
				{
					fprintf(stderr, "%s%s", delim, argv[i]);
				}

				delim = " ";
			}
			fprintf(stderr, "]\n");
		}
		exit(1);
	}
	/**
	 * Now copy the fds used in the pipe into our structure
	 */
	cPipe->ifd_ = dup(pipe_p2c[1]);
	close(pipe_p2c[0]);
	close(pipe_p2c[1]);

	cPipe->ofd_ = dup(pipe_c2p[0]);
	close(pipe_c2p[0]);
	close(pipe_c2p[1]);

	status = 0;

CLEANUP:
	if (argv != NULL)
	{
		for (i = 0; i < argc; i++)
		{
			ckfree(argv[i]);
		}
		ckfree(argv);
	}
	return status;
}

OS_EXPORT int
waitCommandPipe(int *exitStatus, CommandPipe * cPipe)
{
	int             status;

	waitpid(cPipe->pid_, &status, 0);

	if (WIFEXITED(status))
	{
		if (exitStatus != NULL)
		{
			(*exitStatus) = WEXITSTATUS(status);
			return 0;
		}
	}
	return (-1);
}

