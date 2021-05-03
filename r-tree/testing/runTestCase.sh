#!/bin/sh

VERBOSE=0
FILELIST=""

# Run all the tests in this directory, counting the lines of failure

for opt in "$@"
do
    case "${opt}" in
    -v*)
    	VERBOSE=`expr ${VERBOSE} + 1`
	;;
    -*)
    	echo "Unknown option ${opt}"
	exit 1
    	;;
    *)
    	FILELIST="${FILELIST} ${opt}"
    esac
done

./testcase ${FILELIST} | awk '
BEGIN	{
		nTotal = 0;
		nFail = 0;
		nCases = 0;
		matchLine = 0;
		sawSuccess = 0;
	}
/SUBTESTCASE/	{
		printf("        : - ");
		for (i = 2; i <= NF; i++) {
		    printf(" %s", $i);
		}
		printf("\n");
		matchLine = 1;
	}
/TESTCASE/	{
		if ( ! matchLine) {
		    nCases++;
		    printf("Testing :");
		    for (i = 2; i <= NF; i++) {
			printf(" %s", $i);
		    }
		    printf("\n");
		    matchLine = 1;
		}
	}
/TEST/	{
		if ( ! matchLine) {
		    if ('${VERBOSE}' > 1) {
			printf("   Test :");
			for (i = 2; i <= NF; i++) {
			    printf(" %s", $i);
			}
			printf("\n");
		    }
		}
		matchLine = 1;
	}
/DEBUG/	{
		if ('${VERBOSE}' > 0) {
		    print;
		}
		matchLine = 1;
	}
/PASS/	{
		nTotal++;
		matchLine = 1;
	}
/FAIL/	{
		nFail++;
		nTotal++;
		print;
		matchLine = 1;
	}
/SUCCESS/	{
		sawSuccess = 1;
		matchLine = 1;
	}
	{
		if (matchLine == 0)
		    printf("Extra Line : %s\n", $0);
		matchLine = 0;
	}
END	{
		if (sawSuccess) {
		    if (nFail == 0) {
			printf("SUCCESS -- No Failures in %d tests\n", nTotal);
		    } else {
			printf("FAILED %d Failures of %d Total (%f%% fail)\n", nFail, nTotal, (nFail * 100.0) / nTotal);
		    }
		} else {
		    printf("FAILURE -- test cases did not complete\n");
		    if (nFail > 0) {
			printf("   %d Failures of %d Total (%f%% fail)\n", nFail, nTotal, (nFail * 100.0) / nTotal);
		    }
		}
		printf("  [%3d Cases Tested]\n", nCases);
	}
'
