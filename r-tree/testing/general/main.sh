#!/bin/sh

##
## Generate main line from test routine files
##


FILETARGET=`echo $0 | sed -e 's/.sh$/.c/'`

cat > ${FILETARGET} << __EOF__
/**
 * This file is generated automatically from the make functionality,
 * built using filename matching from the list of tests in this
 * directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tclCkalloc.h>
#include <filetools.h>


/** prototypes */
__EOF__

for file in test*.c
do
    funcname=`echo $file | sed -e 's/.c$//'`
    echo "int ${funcname}();" >> ${FILETARGET};
done



cat >> ${FILETARGET} << __EOF__

/**
 * Print out simple help
 */
void printHelp()
{
    printf("Test cases in testsuite scaffold\n");
    printf("\n");
    printf("Available tests are:\n");
__EOF__

for file in test*.c
do
    funcname=`echo $file | sed -e 's/.c$//'`
cat >> ${FILETARGET} << __EOF__
    printf("  ${funcname}\n");
__EOF__
done

cat >> ${FILETARGET} << __EOF__

}

/**
 * mainline
 */
int
main(int argc, char **argv)
{
    int status = 1;
    int runAll = 0;
    int ranATest = 0;
    int runThis;
    int s, i;


#ifndef OS_WINDOWS_NT
    system("rm -f ckalloc.log");
    system("rm -rf plots");
#endif

    if (argc == 1) {
	runAll = 1;
    }

    for (i=1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    printHelp();
	    exit(0);
	}
    }
__EOF__

for file in test*.c
do
    funcname=`echo $file | sed -e 's/.c$//'`
cat >> ${FILETARGET} << __EOF__

    runThis = 0;
    for (i=1; i < argc; i++) {
	if (strcmp(argv[i],
		"${funcname}") == 0) {
	    runThis = 1;
	}
	if (strcmp(argv[i],
		"${funcname}.c") == 0) {
	    runThis = 1;
	}
    }
    if (runThis || runAll) {
	ranATest = 1;
	printf("<TESTCASE> ${funcname}()\n");
	s = ${funcname}();
	status = s && status;
    }
__EOF__
done


cat >> ${FILETARGET} << __EOF__

#ifndef OS_WINDOWS_NT
    copyFileIfPresent(1, "ckalloc.log");
#endif

    if (ranATest == 0) {
	printf("<FAILURE> -- no tests specified!\n");
	return 1;
    }


    if (status) {
	printf("<SUCCESS>\n");
	return 0;
    }

    return 1;
}
__EOF__


##
## $Id: main.sh 637 2004-09-23 14:23:03Z andrew $
##
