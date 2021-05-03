/**
 ** Convert the output from the emg.dat file into the 16 bit
 ** format required by the DQEMG program.
 **
 ** $Id: io_utils.c 33 2009-04-26 13:48:18Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
#include       <stdio.h>
#include       <string.h>
#include       <errno.h>
#include       <sys/types.h>
#include       <sys/stat.h>
#ifndef        OS_WINDOWS_NT
#include     <unistd.h>
#else
#include     <io.h>
#endif
#include       <fcntl.h>
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

#include "pathtools.h"
#include "stringtools.h"
#include "error.h"
#include "tclCkalloc.h"
#include "massert.h"

#include "io_utils.h"


OS_EXPORT FP *
openFP(const char *name, const char *mode)
{
	FP *result;

	result = ckalloc(sizeof(FP));
	result->name = osIndependentPath(name);;
	MSG_ASSERT(result->name != NULL, "malloc failed");

	result->fp = fopenpath(name, mode);
	if (result->fp == NULL)
	{
		Error("Failure opening file '%s' : %s\n",
			  name, strerror(errno));
		closeFP(result);
		return NULL;
	}
	return result;
}

OS_EXPORT void 
closeFP(FP *fp)
{
	if (fp == NULL)
		return;

	if (fp->fp != NULL)
		fclose(fp->fp);
	if (fp->name != NULL)
		ckfree(fp->name);

	ckfree(fp);
}

OS_EXPORT int
fileExists(const char *name)
{
	char *osIndepName;
	int status = 1;
	FILE *fp;

	osIndepName = osIndependentPath(name);;

	fp = fopenpath(osIndepName, "rb");
	if (fp == NULL)
	{
		status = 0;
	}
	fclose(fp);
	ckfree(osIndepName);

	return status;
}

OS_EXPORT int
getFileLength(const char *name)
{
	struct stat sb;

	if (stat(name, &sb) < 0)
	{
		return (-1);
	}
	return sb.st_size;
}

OS_EXPORT int
getFDFileLength(int fd)
{
	struct stat sb;

	if (fstat(fd, &sb) < 0)
	{
		return (-1);
	}
	return sb.st_size;
}

OS_EXPORT int
r2byteInt(FP *fp, osInt16 *value)
{
	int status;
	int sz;

	/** ensure that we know what size types we are talking about */
	sz = sizeof(short);
	MSG_ASSERT(sz == 2, "Size mismatch: sizeof(short) != 2");

	status = fread(value, 1, sizeof(short), fp->fp);
	if (status != sizeof(short))
	{
		Error("Failure reading short into file '%s'\n", fp->name);
		return 0;
	}
#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			short s;
			char c[1];
		} pre, post;
		pre.s = *value;
		post.c[0] = pre.c[1];
		post.c[1] = pre.c[0];
		*value = post.s;
	}
#endif

	return 1;
}

OS_EXPORT int
r4byteInt(FP *fp, osInt32 *value)
{
	int status;
	int sz;

	/** ensure that we know what size types we are talking about */
	sz = sizeof(int);
	MSG_ASSERT(sz == 4, "Size mismatch: sizeof(int) != 4");

	status = fread(value, 1, sizeof(int), fp->fp);
	if (status != sizeof(int))
	{
		Error("Failure reading 4byte int into file '%s'\n", fp->name);
		return 0;
	}
#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			int l;
			char c[1];
		} pre, post;
		pre.l = *value;
		post.c[0] = pre.c[3];
		post.c[1] = pre.c[2];
		post.c[2] = pre.c[1];
		post.c[3] = pre.c[0];
		*value = post.l;
	}
#endif

	return 1;
}

OS_EXPORT int
rFloat(FP *fp, float *value)
{
	int status;
	int sz;

	/** ensure that we know what size types we are talking about */
	sz = sizeof(float);
	MSG_ASSERT(sz == 4, "Size mismatch: sizeof(float) != 4");

	status = fread(value, 1, sizeof(float), fp->fp);
	if (status != sizeof(float))
	{
		Error("Failure reading float from file '%s'\n", fp->name);
		return 0;
	}
#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			float f;
			char c[1];
		} pre, post;
		pre.f = *value;
		post.c[0] = pre.c[3];
		post.c[1] = pre.c[2];
		post.c[2] = pre.c[1];
		post.c[3] = pre.c[0];
		*value = post.f;
	}
#endif

	return 1;
}

OS_EXPORT int
rGeneric(FP *fp, void *value, int len)
{
	int status;

	status = fread(value, 1, len, fp->fp);
	if (status != len)
	{
		Error("Failure reading data of length %d into file '%s'\n",
			  len, fp->name);
		return 0;
	}
	return 1;
}

OS_EXPORT int
w2byteInt(FP *fp, osInt16 value)
{
	int status;

#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			short s;
			char c[1];
		} pre, post;
		pre.s = value;
		post.c[0] = pre.c[1];
		post.c[1] = pre.c[0];
		value = post.s;
	}
#endif

	status = fwrite(&value, 1, sizeof(short), fp->fp);
	if (status != sizeof(short))
	{
		Error("Failure writing short into file '%s'\n", fp->name);
		return 0;
	}
	return 1;
}

OS_EXPORT int
w4byteInt(FP *fp, osInt32 value)
{
	int status;

#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			int l;
			char c[1];
		} pre, post;
		pre.l = value;
		post.c[0] = pre.c[3];
		post.c[1] = pre.c[2];
		post.c[2] = pre.c[1];
		post.c[3] = pre.c[0];
		value = post.l;
	}
#endif

	status = fwrite(&value, 1, sizeof(int), fp->fp);
	if (status != sizeof(int))
	{
		Error("Failure writing long into file '%s'\n", fp->name);
		return 0;
	}
	return 1;
}

OS_EXPORT int
wFloat(FP *fp, float value)
{
	int status;

#if defined(OS_BIG_ENDIAN)
	{
		union
		{
			float f;
			char c[1];
		} pre, post;
		pre.f = value;
		post.c[0] = pre.c[3];
		post.c[1] = pre.c[2];
		post.c[2] = pre.c[1];
		post.c[3] = pre.c[0];
		value = post.f;
	}
#endif

	status = fwrite(&value, 1, sizeof(float), fp->fp);
	if (status != sizeof(float))
	{
		Error("Failure writing float into file '%s'\n", fp->name);
		return 0;
	}
	return 1;
}

OS_EXPORT int
wGeneric(FP * fp, void *value, int len)
{
	int status;

	status = fwrite(value, 1, len, fp->fp);
	if (status != len)
	{
		Error("Failure writing data of length %d into file '%s'\n",
			  len, fp->name);
		return 0;
	}
	return 1;
}

