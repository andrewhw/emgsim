/** ------------------------------------------------------------
 ** Read/write attribute value files
 ** ------------------------------------------------------------
 ** $Id: attvalfile.h 10 2008-04-24 18:37:51Z andrew $
 **/

#ifndef CONFIGURATION_FILE_HEADER__
#define CONFIGURATION_FILE_HEADER__

#include "os_defs.h"
#include "attvalfile.h"

typedef struct ConfigurationSettings {
	attValList *list;
} ConfigurationSettings;

#ifndef         lint
/**
 ** PROTOTYPES
 **/

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

	/**
	 * Create an empty configuration settings storage, which
	 * can then be used with the set interface in
	 * cfgSetString() or cfgSetInt(), below.
	 */
OS_EXPORT ConfigurationSettings *cfgCreateEmptyConfiguration();

	/**
	 * Create a configuration settings storage populated with
	 * values from the indicated file
	 */
OS_EXPORT ConfigurationSettings *cfgLoadConfiguration(const char *filename);

	/**
	 * Save a populated configuration settings storage into
	 * the indicated file in the format desired by the load function
	 */
OS_EXPORT int cfgSaveConfiguration(
				const char *filename,
				ConfigurationSettings *cfg
			);

	/**
	 * Delete a populated configuration settings storage safely and completely
	 */
OS_EXPORT void cfgDeleteConfiguration(ConfigurationSettings *cfg);


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
			);
	/**
	 * Get an integer value, or the default value if not present
	 */
OS_EXPORT int cfgGetInt(
				ConfigurationSettings *cfg,
				const char *key,
				int defaultValue
			);


	/**
	 * Set a string value, storing a copy of the provided value;
	 * if this value already exists, it is overwritten, otherwise
	 * it is added to the storage
	 */
OS_EXPORT void cfgSetString(
				ConfigurationSettings *cfg,
				const char *key,
				const char *value
			);
	/**
	 * Set an integer value; if this value already exists, it is
	 * overwritten, otherwise it is added to the storage
	 */
OS_EXPORT void cfgSetInt(
				ConfigurationSettings *cfg,
				const char *key,
				int value
			);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif

#endif  /* CONFIGURATION_FILE_HEADER__ */
