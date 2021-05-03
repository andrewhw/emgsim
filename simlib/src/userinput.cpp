/**
 **        Give the user a chance to override the various values
 **        in the globals structure
 **
 **        $Id: userinput.cpp 28 2020-09-25 20:00:58Z andrew $
 **/

#include "os_defs.h"

#ifndef    MAKEDEPEND
# include    <stdlib.h>
# include    <ctype.h>
# include    <stdio.h>
# include    <string.h>
#endif

#include "SimulatorControl.h"
#include "MUP.h"

#include "error.h"
#include "userinput.h"
#include "pathtools.h"
#include "stringtools.h"
#include "tclCkalloc.h"

#include "attvalfile.h"
#include "tokens.h"

#include "DQEmgData.h"

#ifndef OS_WINDOWS_NT
//#define USE_CURSES

# ifdef USE_CURSES
# include <ncurses.h>
# endif
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



#define NAME_SIZE				16
#define HELP_SIZE				38
#define CENTRAL_SIZE				(NAME_SIZE + HELP_SIZE)



#define TYPE_INT				0
#define TYPE_SHORT				1
#define TYPE_FLOAT				2
#define TYPE_STRING				3
#define TYPE_ENUMERATION		4
#define TYPE_FUNCTION				5

typedef struct {
	int			value_;
	const char	*tag_;
	const char	*shortTag_;
} enumeratedValue;

enumeratedValue    needleTypes[] = {
			{    (-1),    "invalid",            "XX"    },
			{    1,        "single-fibre",      "SF"    },
			{    2,        "concentric",        "CN"    },
			{    3,        "monopolar",         "MN"    },
			{    4,        "bipolar",           "BN"    },
			{    0,        NULL,                NULL    }
		};

enumeratedValue    booleanTypes[] = {
			{    0,     "false",        "false"  },
			{    1,     "true",         "true"   },
			{    (-1),  NULL,           NULL     }
		};

enumeratedValue    muLayoutTypes[] = {
			{    RANDOM_MU_LAYOUT,
			            "random",       "random"        },
			{    GRID_MU_LAYOUT,
			            "grid",         "grid"          },
			{    (-1),    NULL,         NULL            }
		};

enumeratedValue    lateralTypes[] = {
			{    DQEmgData::NOT_RECORDED, "unknown", "unknown"    },
			{    DQEmgData::LEFT,         "left",    "left"    },
			{    DQEmgData::RIGHT,        "right",   "right"    },
			{    (-1),    NULL,        NULL    }
		};

enumeratedValue    needleReferenceTypes[] = {
			{    (-1),
			                    "invalid",
			                    "invalid"    },
			{    MUP::TIP_ONLY,
			                    "tip only",
			                    "tip"    },
			{    MUP::CANNULA_ONLY,
			                    "cannula only",
			                    "cannula"    },
			{    MUP::TIP_VERSUS_CANNULA,
			                    "tip versus cannula",
			                    "difference"    },
			{    (-1),    NULL,        NULL    }
		};

//enumeratedValue    layoutFunctionTypes[] = {
//            {    (-1),
//                                "invalid",
//                                "invalid"    },
//            {    LAYOUT_PROBABILITY,
//                                "probability",
//                                "probability"    },
//            {    LAYOUT_WEIGHTING,
//                                "weighting",
//                                "weighting"    },
//            {    (-1),    NULL,        NULL    }
//        };


typedef struct
{
	union {
		int *iptr_;
		short *shptr_;
		float *fptr_;
		char *strptr_;
	} data_;
	int type_;
	int hasTrailingSpace_;
	unsigned int maxSize_;
	const char *help_;
	const char *name_;
	enumeratedValue *enumeration_;
	int (*displayFunction_)(int displayIndex, void *userData);
	int (*updateFunction_)(int displayIndex, void *userData);
	void *userData_;
} validation;

int changeValueByType(int i);

static validation fields_[256];
static int nFields_ = 0;
static int currentDisplay_ = 0;

static void charValueN(
	char *data,
	const char *name,
	int maxSize,
	const char *help)
{
	attVal *attribute;

	if ((attribute = getAttVal(g->list_, name)) == NULL)
	{
		addAttVal(g->list_, createStringAttribute(name, data));
	} else
	{
		if (attribute->type_ == TT_STRING)
		{
			strncpy(data, attribute->data_.strptr_, maxSize);
		} else
		{
			updateAttVal(g->list_, createStringAttribute(name, data));
		}
	}

	fields_[nFields_].type_ = TYPE_STRING;
	fields_[nFields_].data_.strptr_ = data;
	fields_[nFields_].maxSize_ = maxSize;
	fields_[nFields_].help_ = help;
	fields_[nFields_].name_ = name;
	fields_[nFields_].hasTrailingSpace_ = 0;
	fields_[nFields_].enumeration_ = NULL;
	fields_[nFields_].displayFunction_ = NULL;
	fields_[nFields_].updateFunction_ = NULL;
	fields_[nFields_].userData_ = NULL;
	nFields_++;
}

static void charValue(char *data, const char *name, const char *help)
{
	charValueN(data, name, FILENAME_MAX, help);
}

static void intValue(int *data, const char *name, const char *help)
{
	attVal *attribute;

	if ((attribute = getAttVal(g->list_, name)) == NULL)
	{
		addAttVal(g->list_, createIntegerAttribute(name, *data));
	} else
	{
		if (attribute->type_ == TT_INTEGER)
		{
			*data = attribute->data_.ival_;
		} else
		{
			updateAttVal(g->list_, createIntegerAttribute(name, *data));
		}
	}

	fields_[nFields_].type_ = TYPE_INT;
	fields_[nFields_].data_.iptr_ = data;
	fields_[nFields_].help_ = help;
	fields_[nFields_].name_ = name;
	fields_[nFields_].hasTrailingSpace_ = 0;
	fields_[nFields_].enumeration_ = NULL;
	fields_[nFields_].displayFunction_ = NULL;
	fields_[nFields_].updateFunction_ = NULL;
	fields_[nFields_].userData_ = NULL;
	nFields_++;
}

static void shortValue(short *data, const char *name, const char *help)
{
	attVal *attribute;

	if ((attribute = getAttVal(g->list_, name)) == NULL)
	{
		addAttVal(g->list_, createIntegerAttribute(name, (int) *data));
	} else
	{
		if (attribute->type_ == TT_INTEGER)
		{
			*data = attribute->data_.ival_;
		} else
		{
			updateAttVal(g->list_, createIntegerAttribute(name, *data));
		}
	}

	fields_[nFields_].type_ = TYPE_SHORT;
	fields_[nFields_].data_.shptr_ = data;
	fields_[nFields_].help_ = help;
	fields_[nFields_].name_ = name;
	fields_[nFields_].hasTrailingSpace_ = 0;
	fields_[nFields_].enumeration_ = NULL;
	fields_[nFields_].displayFunction_ = NULL;
	fields_[nFields_].updateFunction_ = NULL;
	fields_[nFields_].userData_ = NULL;
	nFields_++;
}

static void floatValue(float *data, const char *name, const char *help)
{
	attVal *attribute;

	if ((attribute = getAttVal(g->list_, name)) == NULL)
	{
		addAttVal(g->list_, createRealAttribute(name, (double) *data));
	} else
	{
		if (attribute->type_ == TT_REAL)
		{
			*data = (float) attribute->data_.dval_;
		} else if (attribute->type_ == TT_INTEGER)
		{
			*data = (float) attribute->data_.ival_;
			updateAttVal(g->list_, createRealAttribute(name, *data));
		} else
		{
			updateAttVal(g->list_, createRealAttribute(name, *data));
		}
	}

	fields_[nFields_].type_ = TYPE_FLOAT;
	fields_[nFields_].data_.fptr_ = data;
	fields_[nFields_].help_ = help;
	fields_[nFields_].name_ = name;
	fields_[nFields_].hasTrailingSpace_ = 0;
	fields_[nFields_].enumeration_ = NULL;
	fields_[nFields_].displayFunction_ = NULL;
	fields_[nFields_].updateFunction_ = NULL;
	fields_[nFields_].userData_ = NULL;
	nFields_++;
}

static void enumValue(int *data, const char *name,
		const char *help, enumeratedValue *enumeration)
{
	attVal *attribute;

	if ((attribute = getAttVal(g->list_, name)) == NULL)
	{
		addAttVal(g->list_, createIntegerAttribute(name, (int) *data));
	} else
	{
		if (attribute->type_ == TT_INTEGER)
		{
			*data = attribute->data_.ival_;
		} else
		{
			updateAttVal(g->list_, createIntegerAttribute(name, *data));
		}
	}

	fields_[nFields_].type_ = TYPE_ENUMERATION;
	fields_[nFields_].data_.iptr_ = data;
	fields_[nFields_].help_ = help;
	fields_[nFields_].name_ = name;
	fields_[nFields_].hasTrailingSpace_ = 0;
	fields_[nFields_].enumeration_ = enumeration;
	fields_[nFields_].displayFunction_ = NULL;
	fields_[nFields_].updateFunction_ = NULL;
	fields_[nFields_].userData_ = NULL;
	nFields_++;
}

//static void functionValue(
//		const char *name,
//        const char *help,
//		int (*displayFunction)(int, void *),
//		int (*updateFunction)(int, void *),
//		void *userData)
//{
//    fields_[nFields_].type_ = TYPE_FUNCTION;
//    fields_[nFields_].help_ = help;
//    fields_[nFields_].name_ = name;
//    fields_[nFields_].updateFunction_ = updateFunction;
//    fields_[nFields_].displayFunction_ = displayFunction;
//    fields_[nFields_].userData_ = userData;
//    nFields_++;
//}

static void space()
{
	fields_[nFields_-1].hasTrailingSpace_ = 1;
}

void dumpGlobals()
{
	FILE *fp;
	int i;
	fp = fopen("dumpglobals.txt", "w");
	if (fp != NULL)
	{
		for (i = 0; i < nFields_ ; i++)
		{
			if (fields_[i].type_ == TYPE_STRING)
			{
			    fprintf(fp, "\t\"%s\",\t\t/* %s - %s*/\n",
			            fields_[i].data_.strptr_,
			            fields_[i].name_, fields_[i].help_);

			} else if (fields_[i].type_ == TYPE_INT
			        || fields_[i].type_ == TYPE_ENUMERATION)
			{
			    fprintf(fp, "\t%d,\t\t\t/* %s - %s*/\n",
			            *fields_[i].data_.iptr_,
			            fields_[i].name_, fields_[i].help_);

			} else if (fields_[i].type_ == TYPE_SHORT)
			{
			    fprintf(fp, "\t%d,\t\t\t/* %s - %s*/\n",
			            (int) (*fields_[i].data_.shptr_),
			            fields_[i].name_, fields_[i].help_);

			} else if (fields_[i].type_ == TYPE_FLOAT)
			{
			    fprintf(fp, "\t%f,\t\t\t/* %s - %s*/\n",
			            *fields_[i].data_.fptr_,
			            fields_[i].name_, fields_[i].help_);

			}
		}
		fclose(fp);
	}
}

void loadValues(struct globals *g)
{

	if (g->list_ == NULL)
	{
		g->list_ = (attValList *) ckalloc(sizeof(attValList));
		memset(g->list_, 0, sizeof(attValList));
	}

	nFields_ = 0;

	/**
	 * these values are moved to the top as
	 *    they are probably more useful than the rest
	 */
	floatValue(&g->firing_.contractionLevelAsPercentMVC,
					"contractionLevelAsPercentMVC",
				"contraction level");
	intValue(&g->muscle_->numMotorUnits, "nmu_in_mscl",
			    "number of motor units in muscle");
	enumValue(&g->electrode_type, "electrode_type",
			    "electrode_type - between 1 and 4",
			    needleTypes);

	enumValue(&g->generateMFPsWithoutInitiation,
				"generateMFPsWithoutInitiation",
			    "Generate MFPs without extinction events?", booleanTypes);

	space();

	floatValue(&g->pathology.neuropathicMULossFraction,
					"pathology_neuropathy_MU_loss_fraction",
			    "Neuropathic MU Loss Fraction");

	intValue(&g->pathology.neuropathicMaxAdoptionDistanceInUM,
					"pathology_neuropathy_dist",
			    "Max Adoption Distance In um");


	floatValue(&g->pathology.neuropathicEnlargementFraction,
					"pathology_neuropathy_enlargement_fraction",
			    "Neuropathic MU Enlargement Fraction");

	space();



	floatValue(&g->pathology.myopathicFractionOfFibresAffected,
					"pathology_myopathy_fibre_affected_fraction",
			    "Myopathic Fibre Affected Fraction");

	intValue(&g->pathology.myopathicCycleNewInvolvementPercentage,
				"pathology_myopathy_percentage_new_involvement",
				"Myopathic New Involvement Percentage In Each Cycle");

	intValue(&g->pathology.myopathicFibreGraduallyDying,
				"pathology_myopathic_fibre_gradually_dying",
			    "Myopathic fibre gradually dying?");

	intValue(&g->pathology.myopathicDependentProcedure,
				"pathology_myopathic_dependent_procedure",
				"Dying and Splitting Depending on Affection Procedure?");

	floatValue(&g->pathology.myopathicFibreDeathDiameter,
					"pathology_myopathy_death_threshold",
			    "Myopathic Threshold of Fibre Death");

	floatValue(&g->pathology.myopathicPercentageOfAffectedFibersDying,
				"pathology_myopathy_percentage_affected_dying",
				"Percentage Of Affected Fibers Dying");

	floatValue(&g->pathology.myopathicHypertrophicFibreFraction,
					"pathology_myopathy_hypertrophy_fraction",
			    "Myopathic Fraction of Fibres Becoming Hypertrophic");

	floatValue(&g->pathology.myopathicHypertrophySplitThreshold,
					"pathology_myopathy_hypertrophy_allowed_fraction",
			    "Factor of original area at which hypertrophic fibres split");

	floatValue(&g->pathology.myopathicPercentageOfHypertrophicFibersSplit,
				"pathology_myopathy_percentage_hypertrophy_split",
				"Percentage Of Hypertrophic Fibers Splitting");

	floatValue(&g->pathology.myopathicAtrophyRatePerCycle,
					"pathology_myopathicAtrophyRate",
			    "Myopathic Rate of Atrophy");

	floatValue(&g->pathology.myopathicHypertrophyRatePerCycle,
					"pathology_myopathicHypertrophyRate",
			    "Myopathic Rate of Hypertrophy");

	/**
	 * this is now calculated by the muscle radius, and is not
	 * able to be overridden by the users
	 */
	space();

	intValue(&g->tipUptakeDistance, "tipUptakeDistance",
			    "tip uptake distance");
	intValue(&g->canUptakeDistance, "canUptakeDistance",
			    "cannula uptake distance");
	intValue(&g->canPhysicalRadius, "canPhysicalRadius",
			    "radius of cannula shaft");

	floatValue(&g->cannula_length, "cannula_length",
			    "Cannula Length (in mm)");

	floatValue(&g->needle_x_position, "needle_x_position",
			    "Needle X Postion (in mm)");
	floatValue(&g->needle_y_position, "needle_y_position",
			    "Needle Y Postion (in mm)");
	floatValue(&g->needle_z_position, "needle_z_position",
			    "Needle Z Postion from NMJ in mm");

	enumValue(&g->needleReferenceSetup, "needleReferenceSetup",
			    "tip/cannula reference setup", needleReferenceTypes);

	space();

	enumValue(&g->doJitter, "doJitter", "Enable Jitter?", booleanTypes);
	intValue(&g->jitter, "jitter", "Jitter (variance) in us");

	floatValue(&g->jitterAccelThreshold, "jitterAccThresh",
			    "MFP threshold for jitter OR MU GST Inclusion threshold (kV/s^2)");

	enumValue(&g->seekNeedle,
					"seekNeedle",
			    "Seek needle to nearby fibres?", booleanTypes);

	floatValue(&g->minimumMuscleMetricThreshold,
					"minimumMuscleMetricThreshold",
			    "Minimum metric to seek needle to");

	space();

	enumValue(&g->filter_raw_signal, "filter_raw_signal",
			    "Bandpass filter raw signal?", booleanTypes);

	enumValue(&g->use_noise, "use_noise",
			    "Generate noise?", booleanTypes);

	floatValue(&g->signalToNoiseRatio, "signalToNoiseRatio",
			    "S/N ratio?");

	space();

	charValue(g->fileDescription.operator_name, "operator_name",
			    "Recorded Operator Name");
	charValue(g->fileDescription.patient_name, "patient_name",
			    "Recorded Patient Name");
	charValue(g->fileDescription.muscle_description, "muscle_description",
			    "Recorded Muscle Name");

	intValue(&g->fileDescription.patient_id, "patient_id", "Patient ID");

	enumValue(&g->fileDescription.muscle_side, "muscle_side",
			    "Laterality", lateralTypes);

	enumValue(&g->fileDescription.new_operator, "new_operator",
			    "New Operator?", booleanTypes);
	enumValue(&g->fileDescription.new_patient, "new_patient",
			    "New Patient?", booleanTypes);
	enumValue(&g->fileDescription.new_muscle, "new_muscle",
			    "New Muscle?", booleanTypes);

	{
		static char tmpbuf[FILENAME_MAX];
		attVal *attribute = getAttVal(g->list_, "LAST_OUTPUT");
		if (attribute == NULL || attribute->data_.strptr_ == NULL)
			tmpbuf[0] = 0;
		else
			strncpy(tmpbuf, attribute->data_.strptr_, FILENAME_MAX);

		charValue(tmpbuf, "LAST_OUTPUT",
			    "Last Directory Name");
		enumValue(&g->use_last_muscle, "useLastMuscle",
			    "Use Last Muscle?", booleanTypes);
		enumValue(&g->use_old_firing_times, "useOldFiringTimes",
			    "Re-use the old firing times?", booleanTypes);

	}
	space();

	charValue(g->patient_name, "patient_dir_name",
			    "patient directory name");
	floatValue(&g->firing_.maximumFiringThreshold,
					"firing_maximumFiringThreshold",
			    "maximum recruitment thrshld");

	intValue(&g->emg_elapsed_time, "emg_elapsed_time",
			    "total time for EMG generation");

	space();
	shortValue(&g->maxShortVoltage, "maxShortVoltage",
			    "max (scaled) value in 16-bit output");

	intValue(&g->jitterInterpolationExpansion,
			    "jitterInterpExp",
			    "internal interp. factor for jitter");

	space();

	floatValue(&g->muscle_->fibreDensity,
					"mscl_fib_dens", "muscle fibre density");
	floatValue(&g->muscle_->areaPerFibre,
				 "mscl_area_per_fib", "area of 1 muscl fibre");
	floatValue(&g->muscle_->minMUDiam,
					"min_mu_diam", "min motor unit diameter");
	floatValue(&g->muscle_->maxMUDiam,
				"max_mu_diam", "max motor unit diameter");
	floatValue(&g->firing_.recruitmentSlope,
					"firing_recruitmentSlope",
				"IPI firing slope");
	floatValue(&g->firing_.minimumFiringRate,
					"firing_minimumFiringRate",
				"min firing rate");
	floatValue(&g->firing_.maximumFiringRate,
					"firing_maximumFiringRate",
				"max firing rate");
	floatValue(&g->firing_.coefficientOfVarianceInFiringTimes,
					"coefficientOfVarianceInFiringTimes",
				"coeff of variance");

	enumValue(&g->mu_layout_type, "mu_layout_type",
			    "MU Layout Type", muLayoutTypes);

	enumValue(&g->generate_second_channel,
				"generate_second_channel",
				"Generate Second Channel?",
				booleanTypes);

	enumValue(&g->recordMFPPeakToPeak, "recordMFPPeakToPeak",
			    "Dump MFP Peak-to-Peak Values?", booleanTypes);
}



char *getUserInput(const char *message)
{
	int c, k = 0;
	int sawFirst = 0;
	static char buffer[128];

	LogInfo(message);
	LogFlush();

	do
	{
		c = getc(stdin);
		if (sawFirst)
		{
			buffer[k++] = c;
		} else if ( ! isspace(c))
		{
			buffer[k++] = c;
			sawFirst = 1;
		}
	} while (c != '\n' && c != '\r' && k < 127);
	buffer[k] = 0;
	while (isspace(buffer[--k]))
		buffer[k] = 0;

	return buffer;
}

static char *centralArea(const char *name, const char *help, int index)
{
	static char buffer[FILENAME_MAX];
	int helpLen;
	int i, startPos;

	if (index % 3 == 0)
	{
		memset(buffer, ' ', 128);
		helpLen = strlen(help);
		startPos = HELP_SIZE - helpLen;
		for (i = 0; i < startPos; i++)
		{
			buffer[i] = (i % 3 == 1) ? '-' : ' ';
		}
		strncpy(&buffer[i], help, FILENAME_MAX - i);
	} else
	{
		slnprintf(buffer, FILENAME_MAX, "%*s", HELP_SIZE, help);
	}

	return buffer;
}

int displayChoice(int i)
{
	int nLines = 1;

	if (i < 0 || i > nFields_)
	{
		LogError("i (%d) out of bounds 0 - %d\n", i, nFields_);
		return 0;
	}

	if (fields_[i].type_ == TYPE_STRING)
	{
		LogInfo(" %2d - %s [%s]\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1),
			    fields_[i].data_.strptr_);

	} else if (fields_[i].type_ == TYPE_INT)
	{
		LogInfo(" %2d - %s [%d]\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1),
			    *fields_[i].data_.iptr_);

	} else if (fields_[i].type_ == TYPE_SHORT)
	{
		LogInfo(" %2d - %s [%d]\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1),
			    (int) (*fields_[i].data_.shptr_));

	} else if (fields_[i].type_ == TYPE_FLOAT)
	{
		LogInfo(" %2d - %s [%s]\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1),
			    niceDouble(*fields_[i].data_.fptr_));

	} else if (fields_[i].type_ == TYPE_ENUMERATION)
	{
		LogInfo(" %2d - %s [%d - %s]\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1),
			    *fields_[i].data_.iptr_,
			     fields_[i].enumeration_[
			            *fields_[i].data_.iptr_
			        ].tag_);

	} else if (fields_[i].type_ == TYPE_FUNCTION)
	{
		LogInfo(" %2d - %s\n", i + 1,
			    centralArea(fields_[i].name_, fields_[i].help_, i+1));
		nLines += (*fields_[i].displayFunction_)(i, fields_[i].userData_);

	} else
	{
		LogWarn(" %2d - bad type\n", i + 1);
	}

	if (fields_[i].hasTrailingSpace_)
	{
		LogInfo("\n");
	}

	return nLines;
}

char *displayChoices(int nLinesOnScreen, int *nLinesDisplayed)
{
	int i;
	int maxDisplay = 0;
	int nDisplayed = 0;
	int lineCount = 0;
	int nLinesFromChoice;


	lineCount = 0;
	nDisplayed = 0;
	for (i = 0; (lineCount < nLinesOnScreen) ; i++)
	{
		nDisplayed++;
		lineCount++;
		if (fields_[i].hasTrailingSpace_)
		{
			lineCount++;
		}
	}

	maxDisplay = nDisplayed + currentDisplay_;
	if (maxDisplay > nFields_) maxDisplay = nFields_;

	LogInfo("\n\n");
	LogInfo("Total %d values\n\n", nFields_);

	lineCount = 0;
	for (i = 0; (lineCount < nLinesOnScreen)
			&& (i + currentDisplay_ < nFields_); i++)
	{
		nLinesFromChoice = displayChoice(i + currentDisplay_);
		if ( nLinesFromChoice <= 0)
			return NULL;

		lineCount += nLinesFromChoice;

		if (fields_[i].hasTrailingSpace_)
		{
			lineCount++;
		}
	}
	*nLinesDisplayed = i;

	LogInfo("\n");
	LogInfo("%s, %s, %s, %s, %s\n\n",
			"[N]ext",
			"[P]rev",
			"[Q]uit",
			"[R]un",
			"<Number> - Change Value");
	return getUserInput("Command :");
}

int changeValue(char *entry, int nLinesOnScreen)
{
	int lowerentry = tolower(entry[0]);
	int value;
	char *endptr;

	if (lowerentry == 'n' || lowerentry == 'p')
	{
		if (lowerentry == 'n')
			currentDisplay_ += nLinesOnScreen - 1;

		if (lowerentry == 'p')
			currentDisplay_ -= nLinesOnScreen - 1;

		if (currentDisplay_ < 0)
			currentDisplay_ = 0;

		if (currentDisplay_ + nLinesOnScreen > nFields_)
			currentDisplay_ = nFields_ - (nLinesOnScreen - 1);
		return 1;
	}

	value = strtol(entry, &endptr, 10);
	if (value == 0 && endptr == entry)
	{
		LogInfo("Invalid entry : [%s]\n", entry);
		return 0;
	}

	return changeValueByType(value);
}

int changeStringValue(int i)
{
	char *userEntry;
	attVal *attribute;
	int    done = 0;

	while (! done)
	{
		displayChoice(i);
		LogInfo("Max field length : %d\n", (fields_[i].maxSize_ - 1));

		userEntry = getUserInput("New Value :  ");

		if (userEntry[0] == 0)
		{
			break;
		}
		if (strlen(userEntry) >= fields_[i].maxSize_)
		{
			LogInfo("Value [%s] is too long -- max %d chars\n",
			    userEntry, (fields_[i].maxSize_ - 1));
			continue;
		}

		/** update the value */
		strncpy(fields_[i].data_.strptr_, userEntry, FILENAME_MAX);

		attribute = getAttVal(g->list_, fields_[i].name_);
		if (attribute != NULL)
		{
			if (attribute->data_.strptr_ != NULL)
			    ckfree((void *) attribute->data_.strptr_);
			attribute->data_.strptr_ = ckstrdup(userEntry);
		}

		done = 1;
	}

	displayChoice(i);
	return 1;
}


int changeIntValue(int i)
{
	char *userEntry;
	char prompt[BUFSIZ];
	attVal *attribute;
	int    newValue, done = 0;

	while (! done)
	{
		displayChoice(i);

		slnprintf(prompt, BUFSIZ,
				"New Integer Value [%d] :", *fields_[i].data_.iptr_);
		userEntry = getUserInput(prompt);

		if (userEntry[0] == 0)
		{
			break;
		}

		if (sscanf(userEntry, "%d", &newValue) <= 0)
		{
			LogInfo("Cannot get int in garbled input data [%s]\n", userEntry);
			break;
		}


		/** update the value */
		*fields_[i].data_.iptr_ = newValue;

		attribute = getAttVal(g->list_, fields_[i].name_);
		if (attribute != NULL)
		{
			attribute->data_.ival_ = newValue;
		}

		done = 1;
	}

	displayChoice(i);
	return 1;
}


int changeShortValue(int i)
{
	char *userEntry;
	char prompt[BUFSIZ];
	attVal *attribute;
	int    newValue, done = 0;

	while (! done)
	{
		displayChoice(i);

		slnprintf(prompt, BUFSIZ,
				"New Short Value [%d] : ", *fields_[i].data_.iptr_);
		userEntry = getUserInput(prompt);

		if (userEntry[0] == 0)
		{
			break;
		}

		if (sscanf(userEntry, "%d", &newValue) <= 0)
		{
			LogInfo("Cannot get int in garbled input data [%s]\n", userEntry);
			break;
		}

		if (newValue > SHRT_MAX || newValue < SHRT_MIN)
		{
			LogInfo("Value [%s] out of range\n", userEntry);
			break;
		}


		/** update the value */
		*fields_[i].data_.shptr_ = (short) newValue;

		attribute = getAttVal(g->list_, fields_[i].name_);
		if (attribute != NULL)
		{
			attribute->data_.ival_ = newValue;
		}

		done = 1;
	}

	displayChoice(i);
	return 1;
}


int changeEnumValue(int i)
{
	char *userEntry;
	char prompt[BUFSIZ];
	attVal *attribute;
	int    newValue, done = 0, j;

	while (! done)
	{
		displayChoice(i);

		LogInfo("    Possible Values Are:\n");
		for (j = 0; fields_[i].enumeration_[j].tag_ != NULL; j++)
		{
			if (fields_[i].enumeration_[j].value_ >= 0)
			    LogInfo("        %d = '%s'\n",
			            fields_[i].enumeration_[j].value_,
			            fields_[i].enumeration_[j].tag_);
		}

		slnprintf(prompt, BUFSIZ,
				"New (Integer) Value [%d] : ", *fields_[i].data_.iptr_);
		userEntry = getUserInput(prompt);

		if (userEntry[0] == 0)
		{
			break;
		}

		if (sscanf(userEntry, "%d", &newValue) <= 0)
		{
			LogInfo("Cannot get int in garbled input data [%s]\n", userEntry);
			continue;
		}

		if (newValue < 0)
		{
			LogInfo("Cannot use negative enumerated values\n");
			continue;
		}


		/** update the value */
		for (j = 0; fields_[i].enumeration_[j].tag_ != NULL; j++)
		{
			if (newValue == fields_[i].enumeration_[j].value_)
			{
			    *fields_[i].data_.iptr_ = newValue;

			    attribute = getAttVal(g->list_, fields_[i].name_);
			    if (attribute != NULL)
				{
			        attribute->data_.ival_ = newValue;
			    }

			    done = 1;
			}
		}

		if (! done)
		{
			LogInfo("Value '%d' not in enumerated list\n", newValue);
		}
	}

	displayChoice(i);
	return 1;
}

int changeFunctionValue(int i)
{
	int status;

	status = (*fields_[i].updateFunction_)(i, fields_[i].userData_);

	displayChoice(i);
	return status;
}

int changeFloatValue(int i)
{
	char *userEntry;
	attVal *attribute;
	float newValue;
	int    done = 0;

	while (! done)
	{
		displayChoice(i);

		userEntry = getUserInput("New Float Value : ");

		if (userEntry[0] == 0)
		{
			break;
		}

		if (sscanf(userEntry, "%f", &newValue) <= 0)
		{
			LogInfo("Cannot get float in garbled input data [%s]\n", userEntry);
		}


		/** update the value */
		*fields_[i].data_.fptr_ = newValue;

		attribute = getAttVal(g->list_, fields_[i].name_);
		if (attribute != NULL)
		{
			attribute->data_.dval_ = (double) newValue;
		}

		done = 1;
	}

	displayChoice(i);
	return 1;
}


int changeValueByType(int value)
{
	int index = value - 1;
	LogInfo("Changing value %d -- press [ENTER] to leave the same\n", value);

	if (index < 0 || index > nFields_)
	{
		LogInfo("ERROR : index (%d) out of bounds 0 - %d\n", index, nFields_);
		return 0;
	}

	if (fields_[index].type_ == TYPE_STRING)
	{
		return changeStringValue(index);

	} else if (fields_[index].type_ == TYPE_INT)
	{
		return changeIntValue(index);

	} else if (fields_[index].type_ == TYPE_SHORT)
	{
		return changeShortValue(index);

	} else if (fields_[index].type_ == TYPE_FLOAT)
	{
		return changeFloatValue(index);

	} else if (fields_[index].type_ == TYPE_ENUMERATION)
	{
		return changeEnumValue(index);

	} else if (fields_[index].type_ == TYPE_FUNCTION)
	{
		return changeFunctionValue(index);

	}
	LogInfo("BAD TYPE\n");
	return 0;
}


#ifndef    OS_WINDOWS_NT
static void fixPath(char *path)
{
	int pathlen;
	char *delimpos;

	pathlen = strlen(path);

	/** get rid of drive on UNIX if there */
	if (pathlen > 2 && path[1] == ':' && path[2] == '\\')
	{
		/** move the rest of the path, including the null character, up two */
		memmove(&path[0], &path[3], pathlen - 1);
	}

	/** convert between "\" and "/" in case of OS mixup */
	while ((delimpos = strchr(path, OS_PATH_INVALID_DELIM)) != NULL)
	{
		*delimpos = OS_PATH_DELIM;
	}
}
#endif


/**
 **    update the path names for the correct OS
 **/
void
fixPathnames(struct globals *g)
{
#ifndef    OS_WINDOWS_NT
	fixPath(g->muscle_dir_sub);
	fixPath(g->firings_dir_sub);
	fixPath(g->MUPs_dir_sub);
	fixPath(g->output_dir_sub);
#endif    /* OS_WINDOWS_NT */

}


/**
 **    Load and validate all the approriate global values
 **/
int
validateGlobals(struct globals *g, int *isQuitting)
{
	char *entry, lowerfirst;
	int nLinesOnScreen = 16;
	int nLinesDisplayed = 0;

	*isQuitting = 0;


#ifdef USE_CURSES
	/* if we are not on Windows, get this info from curses */
	initscr();
	nLinesOnScreen = LINES - 8;
	nLinesOnScreen = ((nLinesOnScreen < 25) ? nLinesOnScreen : 25);
	nLinesOnScreen = ((nLinesOnScreen > 5) ? nLinesOnScreen : 5);
	endwin();
#endif


	entry = displayChoices(nLinesOnScreen, &nLinesDisplayed);
	if (entry == NULL)
		return 0;

	lowerfirst = tolower(entry[0]);
	while (lowerfirst != 'r' && lowerfirst != 'q')
	{

		(void) changeValue(entry, nLinesDisplayed);

#ifdef USE_CURSES
		initscr();
		clear();
		refresh();
		endwin();
#endif

		entry = displayChoices(nLinesOnScreen, &nLinesDisplayed);
		if (entry == NULL)
			return 0;

		lowerfirst = tolower(entry[0]);
	}


	if (lowerfirst == 'q')
	{
		*isQuitting = 1;
		return 1;
	}

	return 1;
}


/**
 **    Print out all the current settings
 **/
void
dumpGlobalSettings(struct globals *g)
{
	int i;

	LogInfo("\nCurrent Settings:\n\n");

	for (i = 0; i < nFields_; i++)
	{
		(void) displayChoice(i);
	}
}


/**
 **    Set up the stuff in the globals structure with info from
 **    the various directory paths
 **/
int
setupGlobalDirectoryInfoForOpen(
		struct globals *g,
		const char *path
	)
{
	char tmpBuffer[BUFSIZ];
	const char *lastdelim;

	lastdelim = &path[strlen(path) - 2];
	lastdelim = strrchr(path, OS_PATH_DELIM);
	if (lastdelim != NULL)
	{
		strncpy(g->output_stem, path, (int) (lastdelim - path));
		g->output_stem[(int) (lastdelim - path)] = 0;
	}

	if (path[strlen(path) - 1] == OS_PATH_DELIM)
	{
		slnprintf(tmpBuffer, BUFSIZ, "%s%s", path, g->patient_name);
	} else
	{
		slnprintf(tmpBuffer, BUFSIZ, "%s\\%s", path, g->patient_name);
	}
	g->muscle_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->firings_dir_sub);
	g->firings_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->MUPs_dir_sub);
	g->MUPs_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->output_dir_sub);
	g->output_dir = osIndependentPath(tmpBuffer);


	return 1;
}


/**
 **    Set up the stuff in the globals structure with info from
 **    the various directory paths
 **/
int
saveGlobalDirectoryInfo(struct globals *g, const char *userPathBase)
{
	DirList *dirList;
	char tmpBuffer[BUFSIZ];
	int id = 0;
	const char *pathBase;

	if (userPathBase == NULL)
	{
		pathBase = "run";
	} else
	{
		pathBase = userPathBase;
	}

	slnprintf(tmpBuffer, BUFSIZ, "%s*", pathBase);
	dirList = dirListLoadEntries(g->output_stem, tmpBuffer);
	if (dirList == NULL)
	{
		id = 0;
	} else
	{
		id = dirList->n_entries;
		dirListDelete(dirList);
	}




#ifdef          EXTENDED_DIRECTORY_NAMES
    {
        const char *percentMvcStr;
        const char *needleType;

        needleType = needleTypes[g->electrode_type].shortTag_;
        percentMvcStr =
                niceDouble(g->firing_.contractionLevelAsPercentMVC);

        slnprintf(tmpBuffer, BUFSIZ,
			    "%s\\%s%03d-%s-%smvc-%03djitter\\%s",
			    g->output_stem,
			    pathBase,
			    id,
			    needleType,
			    percentMvcStr,
			    g->jitter,
			    g->patient_name);
    }
#else
	slnprintf(tmpBuffer, BUFSIZ,
			    "%s\\%s%03d\\%s",
			    g->output_stem,
			    pathBase,
			    id,
			    g->patient_name);
#endif

	g->muscle_dir = osIndependentPath(tmpBuffer);
	updateAttVal(g->list_,
			    createStringAttribute("LAST_OUTPUT",
			    g->muscle_dir));


	slnprintf(tmpBuffer, BUFSIZ,
				"%s\\%s", g->muscle_dir, g->firings_dir_sub);
	g->firings_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
				"%s\\%s", g->muscle_dir, g->MUPs_dir_sub);
	g->MUPs_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
				"%s\\%s", g->muscle_dir, g->output_dir_sub);
	g->output_dir = osIndependentPath(tmpBuffer);


	return 1;
}

/**
 **    Save the config info to a file in the output directory
 **/
int
saveOutputDirConfigFile(struct globals *g, int fileId)
{
	char    tmpbuf[FILENAME_MAX];
	char *saveConfigName;
	FILE *fp;

	/*
	 *    Save the globals to a file in the output dir
	 */
	slnprintf(tmpbuf, FILENAME_MAX, "%d", fileId);
	saveConfigName = strconcat(
			    g->output_dir, OS_PATH_DELIM_STRING,
			    "simulator", tmpbuf, ".cfg", NULL);


	if ((fp = fopenpath(saveConfigName, "wb")) == NULL)
	{
		Error("Error : Failed to open \"%s\" for writing.",
			        saveConfigName);
		return 0;
	}

	/* save globals structure */
	slnprintf(tmpbuf, FILENAME_MAX,
			"version 2.1 simulator config file for macro%d.dat",
			fileId);

	writeAttValList(fp, g->list_, tmpbuf);
	fclose(fp);
	ckfree(saveConfigName);
	return 1;
}


/**
 **    Set up the stuff in the globals structure with info from
 **    the various directory paths
 **/
int
reuseGlobalDirectoryInfo(struct globals *g)
{
	char    tmpBuffer[BUFSIZ];
	attVal    *item;

	item = getAttVal(g->list_, "LAST_OUTPUT");
	if (item == NULL)
	{
		LogError("No directory named saved from last study\n");
		return 0;
	} else
	{
		g->muscle_dir = ckstrdup(item->data_.strptr_);
	}


	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->firings_dir_sub);
	g->firings_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->MUPs_dir_sub);
	g->MUPs_dir = osIndependentPath(tmpBuffer);

	slnprintf(tmpBuffer, BUFSIZ,
			"%s\\%s", g->muscle_dir, g->output_dir_sub);
	g->output_dir = osIndependentPath(tmpBuffer);

	LogInfo("Set output_dir to '%s'\n", g->output_dir);

	return 1;
}


