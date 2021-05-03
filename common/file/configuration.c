/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: attvalue.c 77 2010-03-25 19:45:31Z andrew $
 **/

#ifndef MAKEDEPEND
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#endif

#include "os_defs.h"
#include "tclCkalloc.h"
#include "configurationtools.h"
#include "stringtools.h"
#include "tokens.h"

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


/**
 * Create an empty configuration settings storage, which
 * can then be used with the set interface in
 * cfgSetString() or cfgSetInt(), below.
 */
OS_EXPORT ConfigurationSettings *cfgCreateEmptyConfiguration()
{
	ConfigurationSettings *cfg;

	cfg = ckalloc(sizeof(ConfigurationSettings));
	if (cfg == NULL) return NULL;
	memset(cfg, 0, sizeof(ConfigurationSettings));

	cfg->list = (attValList *) ckalloc(sizeof(attValList));
	if (cfg->list == NULL) goto FAIL;
		memset(cfg->list, 0, sizeof(attValList));

	return cfg;

FAIL:
	ckfree(cfg);
	return NULL;
}

/**
 * Create a configuration settings storage populated with
 * values from the indicated file
 */
OS_EXPORT ConfigurationSettings *cfgLoadConfiguration(const char *filename)
{
	ConfigurationSettings *cfg;

	cfg = ckalloc(sizeof(ConfigurationSettings));
	if (cfg == NULL) return NULL;
	memset(cfg, 0, sizeof(ConfigurationSettings));

	cfg->list = loadAttValFile(filename);
	if (cfg->list == NULL) goto FAIL;

	return cfg;

FAIL:
	cfgDeleteConfiguration(cfg);
	return NULL;
}

/**
 * Save a populated configuration settings storage into
 * the indicated file in the format desired by the load function
 */
OS_EXPORT int cfgSaveConfiguration(
		const char *filename,
		ConfigurationSettings *cfg
	)
{
	char tag[BUFSIZ];
	time_t t;
	FILE *fp;

	t = time(NULL);
	slnprintf(tag, BUFSIZ, "Configuration Settings saved: %s", ctime(&t));
	tag[strlen(tag)-1] = 0;

	if ((fp = fopen(filename, "w")) == NULL)
		return (-1);

	if (writeAttValList(fp, cfg->list, tag) <= 0)
		return (-1);

	fclose(fp);

	return 1;
}

/**
 * Delete a populated configuration settings storage safely and completely
 */
OS_EXPORT void cfgDeleteConfiguration(ConfigurationSettings *cfg)
{
	if (cfg->list != NULL) deleteAttValList(cfg->list);
	ckfree(cfg);
}


/**
 * Get a string value, or the default value if not present
 * if a non-NULL value is returned, the value returned is
 * a reference, either to the default value provided, or to
 * internal memory: in either case it should not be modified
 */
OS_EXPORT const char *cfgGetString(
		ConfigurationSettings *cfg,
		const char *key,
		const char *defaultValue
	)
{
	attVal *value;
	value = getAttVal(cfg->list, key);
	if (value == NULL || value->type_ != TT_STRING)
		return defaultValue;

	return value->data_.strptr_;
}

/**
 * Get an integer value, or the default value if not present
 */
OS_EXPORT int cfgGetInt(
		ConfigurationSettings *cfg,
		const char *key,
		int defaultValue
	)
{
	attVal *value;
	value = getAttVal(cfg->list, key);
	if (value == NULL || value->type_ != TT_INTEGER)
		return defaultValue;

	return value->data_.ival_;
}


/**
 * Set a string value, storing a copy of the provided value;
 * if this value already exists, it is overwritten, otherwise
 * it is added to the storage
 */
OS_EXPORT void cfgSetString(
		ConfigurationSettings *cfg,
		const char *key,
		const char *value
	)
{
	attVal *newValue;
	newValue = createStringAttribute(key, value);
	updateAttVal(cfg->list, newValue);
}


/**
 * Set an integer value; if this value already exists, it is
 * overwritten, otherwise it is added to the storage
 */
OS_EXPORT void cfgSetInt(
		ConfigurationSettings *cfg,
		const char *key,
		int value
	)
{
	attVal *newValue;
	newValue = createIntegerAttribute(key, value);
	updateAttVal(cfg->list, newValue);
}


