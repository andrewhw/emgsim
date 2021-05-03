/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: testCommandlineParsing.c 12 2008-04-24 22:22:26Z andrew $
 **/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "tclCkalloc.h"
#include "commandpipe.h"
#include "massert.h"


static int
testParse(char *testdata, int nResults, ...)
{
    va_list args;
    char **argv = NULL;
    char *resultArg;
    int argc = 0;
    int status = 1;
    int i;

    printf("<TEST> Testing commandline with string [%s] . . .\n", testdata);

    if ( ! commandLineToArgv(&argc, &argv, testdata)) {
	return 0;
    }

    if (argc != nResults) {
	printf("<FAIL> expected %d results, got %d\n", nResults, argc);
	status = 0;
    }

    va_start(args, nResults);
    for (i = 0; i < argc; i++) {
	if (i < nResults) {
	    resultArg = (char *) va_arg(args, char *);
	    if (strcmp(resultArg, argv[i]) == 0) {
		printf("<PASS> %s and %s match\n", resultArg, argv[i]);
	    } else {
		printf("<FAIL> expected %s got %s\n", resultArg, argv[i]);
	    }
	}
	ckfree(argv[i]);
    }
    ckfree(argv);
    va_end(args);

    return status;
}

int
testCommandlineParsing(int argc, char **argv)
{
    int status = 1;

    if ( ! testParse("echo \"t s  t\" '\"2\"'  \"3\"'4'", 4,
    		"echo", "t s  t", "\"2\"", "34", NULL) ) {
	printf("<FAIL> Test failed\n");
	status = 0;
    }

    if ( ! testParse("test", 1, "test", NULL) ) {
	printf("<FAIL> Test failed\n");
	status = 0;
    }

    return status;
}

