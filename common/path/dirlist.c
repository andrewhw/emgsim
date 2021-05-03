/** ------------------------------------------------------------
 ** create and destroy lists of files in directories in a
 ** portable way
 ** ------------------------------------------------------------
 ** $Id: dirlist.c 11 2008-04-24 22:13:19Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#ifdef OS_WINDOWS_NT
#include <io.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif
#endif

#include "error.h"
#include "pathtools.h"
#include "massert.h"
#include "stringtools.h"
#include "tclCkalloc.h"

typedef struct Mask
{
	char          **portion_;
	int             nPortions_;
	int             flags_;
}               Mask;
#define         MASK_MATCH_BEGIN                0x01
#define         MASK_MATCH_END                  0x02


static Mask     buildMask(const char *mask);
static void     addEntryToList(const char *name, DirList * list);

/**
 ** Open a file, creating the directory path it resides in
 ** if necessary.  Path names will be converted to the
 ** appropriate OS convention.
 **/
OS_EXPORT DirList *
dirListLoadEntries(const char *path, const char *filenameMask)
{
	DirList        *result;

	/** allocate the result to return */
	result = ckalloc(sizeof(DirList));
	result->directory_name = osIndependentPath(path);
	result->n_entries = 0;
	result->entry_name = NULL;

#ifdef          OS_WINDOWS_NT
	{
		char           *fullSearchPath;
		struct _finddata_t fileData;
		int             searchHandle;

		if (result->directory_name[0] != OS_PATH_DELIM
			&& result->directory_name[1] != ':')
		{
			char            qualifiedPath[1024];
			_fullpath(qualifiedPath, result->directory_name, 1024);
			ckfree((void *) result->directory_name);
			result->directory_name = ckstrdup(qualifiedPath);
		}
		fullSearchPath =
			strconcat(path,
					  OS_PATH_DELIM_STRING,
					  filenameMask,
					  NULL
			);

		searchHandle = _findfirst(fullSearchPath, &fileData);
		if (searchHandle < 0)
		{
			ckfree(fullSearchPath);
			goto FAIL;
		}
		/** we have at least one file matching */
		addEntryToList(fileData.name, result);

		while (_findnext(searchHandle, &fileData) >= 0)
		{
			addEntryToList(fileData.name, result);
		}

		ckfree(fullSearchPath);
	}
#else
	{
		DIR            *directory;
		struct dirent  *curEntry;
		Mask            mask;
		int             matchFound;
		char           *matchLocation, *searchLocation;
		int             i;


		directory = opendir(result->directory_name);
		if (directory == NULL)
		{
			goto FAIL;
		}
		mask = buildMask(filenameMask);

		/** scan the entire directory */
		while ((curEntry = readdir(directory)) != NULL)
		{

			matchFound = 1;
			searchLocation = curEntry->d_name;

			if ((mask.nPortions_ == 1) && (strcmp(filenameMask, "*") == 0))
			{
				matchFound = 1;

			} else
			{
				for (i = 0; i < mask.nPortions_; i++)
				{

					if (i == 0 &&
						((mask.flags_ & MASK_MATCH_BEGIN) != 0))
					{
						matchLocation = curEntry->d_name;
						if (strncmp(matchLocation,
									mask.portion_[0],
									strlen(mask.portion_[0])) != 0)
							matchLocation = NULL;


					} else if (i == mask.nPortions_ - 1 &&
							   ((mask.flags_ & MASK_MATCH_END) != 0))
					{
						matchLocation =
							&curEntry->d_name[
											  strlen(curEntry->d_name)
											  - strlen(mask.portion_[
														 mask.nPortions_ - 1
																	 ])
							];
						if (strcmp(matchLocation,
								   mask.portion_[
												 mask.nPortions_ - 1
												 ]) != 0)
							matchLocation = NULL;

					} else
					{

						matchLocation = strstr(searchLocation,
											   mask.portion_[i]);
					}

					if (matchLocation == NULL)
					{
						matchFound = 0;
						break;
					}
					searchLocation = searchLocation
						+ strlen(mask.portion_[i]);
				}
			}
			if (matchFound)
			{
				addEntryToList(curEntry->d_name, result);
			}
		}

		for (i = 0; i < mask.nPortions_; i++)
		{
			if (mask.portion_[i] != NULL)
				ckfree(mask.portion_[i]);
		}
		ckfree(mask.portion_);
		closedir(directory);
	}
#endif


	return result;


FAIL:
	dirListDelete(result);
	return NULL;
}

/**
 ** Clean up a DirList structure
 **/
OS_EXPORT void 
dirListDelete(DirList * list)
{
	int             i;

	if (list != NULL)
	{
		if (list->entry_name != NULL)
		{
			for (i = 0; i < list->n_entries; i++)
			{
				if (list->entry_name[i] != NULL)
				{
					ckfree((void *) list->entry_name[i]);
				}
			}
			ckfree((void *) list->entry_name);
		}
		if (list->directory_name != NULL)
		{
			ckfree((void *) list->directory_name);
		}
		ckfree(list);
	}
}

static void 
addEntryToList(const char *name, DirList * list)
{
	const char    **oldlist;
	int             i, nEntries;

	oldlist = list->entry_name;
	nEntries = list->n_entries + 1;
	list->entry_name = ckalloc(sizeof(char *) * nEntries);
	if (list->n_entries > 0)
	{
		for (i = 0; i < list->n_entries; i++)
		{
			list->entry_name[i] = oldlist[i];
		}
		ckfree((void *) oldlist);
	}
	list->entry_name[list->n_entries] = ckstrdup(name);
	list->n_entries++;
}

static Mask 
buildMask(const char *mask)
{
	Mask            result;
	int             countPortions = 0;
	const char     *loadPtr, *starPtr;

	result.flags_ = 0;

	/** count up chunks in the mask */
	loadPtr = mask;
	while ((starPtr = strchr(loadPtr, '*')) != NULL)
	{

		if (starPtr == loadPtr)
		{
			loadPtr++;
		} else
		{
			if (*(starPtr + 1) != 0)
				countPortions++;
			loadPtr = (starPtr + 1);
		}
	}
	result.portion_ = ckalloc(sizeof(char *) * (countPortions + 1));
	memset(result.portion_, 0, sizeof(char *) * (countPortions + 1));

	/** now load the mask up into the portions */
	loadPtr = mask;
	result.nPortions_ = countPortions + 1;
	countPortions = 0;
	while ((starPtr = strchr(loadPtr, '*')) != NULL)
	{

		if (countPortions == 0 && starPtr != loadPtr)
			result.flags_ |= MASK_MATCH_BEGIN;

		if (starPtr == loadPtr)
		{
			loadPtr++;
		} else
		{
			result.portion_[countPortions++] =
				ckstrndup(loadPtr, starPtr - loadPtr);
			loadPtr = (starPtr + 1);
		}
	}

	if (*loadPtr != 0)
	{
		result.portion_[countPortions] = ckstrdup(loadPtr);
		result.flags_ |= MASK_MATCH_END;
	}
	return result;
}

OS_EXPORT int 
dirToolsGetNextId(const char *path, const char *mask)
{
	DirList        *dirList;
	char           *wildcardPos, *eptr;
	int             i, headLen, curNum, maxNum = (-1);

	wildcardPos = strchr(mask, '*');
	MSG_ASSERT(wildcardPos != NULL, "No wildcard given!");

	headLen = wildcardPos - mask;

	dirList = dirListLoadEntries(path, mask);
	if (dirList == NULL)
		return 0;

	for (i = 0; i < dirList->n_entries; i++)
	{
		curNum = strtol(&dirList->entry_name[i][headLen], &eptr, 10);

		/** check that we got a number */
		if (eptr == &dirList->entry_name[i][headLen])
		{
			/** if true, then we failed completely */
			return (-1);
		}
		if (curNum > maxNum)
			maxNum = curNum;
	}

	dirListDelete(dirList);

	return (maxNum + 1);
}

