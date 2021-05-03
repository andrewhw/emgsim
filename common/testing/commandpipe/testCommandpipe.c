/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: testCommandpipe.c 12 2008-04-24 22:22:26Z andrew $
 **/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tclCkalloc.h"
#include "commandpipe.h"
#include "massert.h"


static int
testSublist(char *command, char *data, char *match)
{
    CommandPipe cp;
    char buf[BUFSIZ];
    int size, exitStatus, i, wasCR = 1;

    printf("<TEST> Testing pipe to sub-process [%s] . . .\n", command);

    if (execCommandPipe(&cp, command) < 0) {
	printf("<FAIL> creating pipe\n");
	return 0;
    }

    if (write(cp.ifd_, data, strlen(data)) < 0) {
	printf("<FAIL> write to pipe failed\n");
	return 0;
    }

    close(cp.ifd_);

    size = read(cp.ofd_, buf, BUFSIZ);
    if (size < 0) {
	printf("<FAIL> read from pipe failed\n");
	return 0;
    }

    if (waitCommandPipe(&exitStatus, &cp) < 0) {
	printf("<FAIL> wait failed\n");
	return 0;
    }

    close(cp.ofd_);

    if (strncmp(match, buf, size) == 0) {
	printf("<PASS> output ok!:\n");
	for (i = 0; i < size; i++) {
	    if (wasCR) {
		printf("<PASS> output:");
		wasCR = 0;
	    }
	    fputc(buf[i], stdout);
	    if (buf[i] == '\n') {
		wasCR = 1;
	    }
	}
	if (wasCR != 1) {
	    fputc('\n', stdout);
	}
	return 1;
    }

    buf[size] = 0;
    printf("<FAIL> String mismatch!\n");
    printf("<FAIL> expected [%s]\n", match);
    printf("<FAIL>      got [%s]\n", buf);
    return 0;
}


int
testCommandpipe(int argc, char **argv)
{
    char *data = "fred\nbill\namy\nzeb\nbill\nxerxes\n";
    int status = 1;

    {
	char *match = "amy\nbill\nfred\nxerxes\nzeb\n";
	if ( ! testSublist("sort -u", data, match) ) {
	    printf("<FAIL> Test sublist failed\n");
	    status = 0;
	}
    }

    {
	char *match = "XXXfred\nXXXbill\nXXXamy\nXXXzeb\nXXXbill\nXXXxerxes\n";
	if ( ! testSublist("sed -e 's/^/XXX/'", data, match) ) {
	    printf("<FAIL> Test sublist failed\n");
	    status = 0;
	}
    }

    {
	char *match = data;
	if ( ! testSublist("tee file.output", data, match) ) {
	    printf("<FAIL> Test sublist failed\n");
	    status = 0;
	}
    }

    return status;
}

