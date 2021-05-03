/**
 ** Simulator Interface File
 ** Contains the definition of the simulator object, and the
 ** results it deals in
 **
 ** $Id: SimulationResult.cpp 13 2011-03-23 11:22:44Z andrew $
 **/

#include "os_defs.h"

#include <string.h>
#include <time.h>
#ifndef OS_WINDOWS_NT
# include <unistd.h>
#else
# include <io.h>
#endif

#include <Simulator.h>

#include "tclCkalloc.h"
#include "log.h"

#include "SimulatorControl.h"
#include "globalHandler.h"
#include "make16bit.h"
#include "userinput.h"
#include "MUP.h"
#include "MuscleData.h"

#include "dco.h"
#include "DQEmgData.h"

static const char *sErrorList[] = {
		        "No Error",
		        "General Error",
		        NULL
		};

extern struct globals *g;

const int SimulationResult::PRINT_EMG_DATA      = 0x01;
const int SimulationResult::PRINT_MUP_DATA     = 0x02;
const int SimulationResult::PRINT_MUSCLE_DATA   = 0x04;

const int SimulationResult::PRINT_ALL_DATA      =
		SimulationResult::PRINT_EMG_DATA      |
		SimulationResult::PRINT_MUP_DATA     |
		SimulationResult::PRINT_MUSCLE_DATA;

SimulationResult::SimulationResult()
{
	memset(this, 0, sizeof(*this));
}

SimulationResult::~SimulationResult()
{
	int i;

	if (muscleData_ != NULL)
	{
		muscleData_->validate();
	}

	if (nMUPs_ > 0 && MUPList_ != NULL)
	{
		for (i = 0; i < nMUPs_; i++)
		{
		    if (MUPList_[i] != NULL)
			{
		        delete MUPList_[i];
		    }
		}
		ckfree(MUPList_);
	}
	if (MUPIdList_ != NULL)
	{
		ckfree(MUPIdList_);
	}

	if (muscleData_ != NULL)
	{
		muscleData_->validate();
		delete muscleData_;
	}
	if (emgData_ != NULL)
	{
		delete emgData_;
	}
	if (dcoData_ != NULL)
	{
		deleteDcoData(dcoData_);
	}
	if (outputDirectory_ != NULL)
	{
		ckfree(outputDirectory_);
	}
	if (baseDirectory_ != NULL)
	{
		ckfree(baseDirectory_);
	}
	if (MUPPath_ != NULL)
	{
		ckfree(MUPPath_);
	}
}

/**
 * load a single MUP into the list and return it
 */
MUP *SimulationResult::getMUP(int index)
{
	if (index < 0 || index > nMUPs_)
		return NULL;

	if (MUPList_ == NULL)
	{
		MUPList_ = (MUP **) ckalloc(sizeof(MUP **) * nMUPs_);
		memset(MUPList_, 0, sizeof(MUP **) * nMUPs_);
	}

	if (MUPList_[index] == NULL)
	{
		if (MUPIdList_[index] >= 0)
		{
		    MUPList_[index] = new MUP(MUPPath_, MUPIdList_[index]);
		    MUPList_[index]->load();
		}
	}
	return MUPList_[index];
}

/**
 * Load all MUPs into memory and return as a list
 *
 * Note that this may be HUGE
 */
MUP **SimulationResult::getMUPList()
{
	int i;

	if (MUPList_ == NULL)
	{
		MUPList_ = (MUP **) ckalloc(sizeof(MUP **) * nMUPs_);
		memset(MUPList_, 0, sizeof(MUP **) * nMUPs_);

		for (i = 0; i < nMUPs_; i++)
		{
		    if (MUPList_[i] == NULL)
			{
		        if (MUPIdList_[i] >= 0)
				{
		            MUPList_[i] = new MUP(MUPPath_, MUPIdList_[i]);
		            MUPList_[i]->load();
		        }
		    }
		}
	}
	return MUPList_;
}

void SimulationResult::freeMUP(int index)
{
	if (MUPList_ == NULL)
	{
		return;
	}

	if (MUPList_[index] != NULL)
	{
		delete MUPList_[index];
		MUPList_[index] = NULL;
	}
}

void SimulationResult::freeMUPList()
{
	int i;

	if (MUPList_ == NULL)
	{
		return;
	}

	for (i = 0; i < nMUPs_; i++)
	{
		if (MUPList_[i] != NULL)
		{
		    delete MUPList_[i];
		}
	}
	ckfree(MUPList_);
}

int SimulationResult::getErrorState() const
{
	return errorState_;
}

const char *SimulationResult::getErrorMessage() const
{
	return errorMessage_;
}

int SimulationResult::getFileId() const
{
	return fileId_;
}

void SimulationResult::setFileId(int id)
{
	fileId_ = id;
}

void SimulationResult::setState(int error)
{
	errorState_ = error;
	errorMessage_ = sErrorList[ -error ];
}

const char *SimulationResult::getOutputDirectory()
{
	char *delim;
	const char *filename;

	if (outputDirectory_ == NULL)
	{
		if (emgData_ == NULL)
		    return NULL;

		filename = emgData_->getFileName();
		if (filename == NULL)
		    return NULL;

		outputDirectory_ = ckstrdup(filename);
		delim = strrchr(outputDirectory_, OS_PATH_DELIM);
		if (delim != NULL)
		{
		    *delim = 0;
		} else
		{
		    ckfree(outputDirectory_);
		    outputDirectory_ = ckstrdup(".");
		}
	}
	return outputDirectory_;

}

const char *SimulationResult::getBaseDirectory()
{
	if (baseDirectory_ == NULL)
	{
		char *delim;

		if (getOutputDirectory() == NULL)
		{
			return NULL;
		}
		baseDirectory_ = ckstrdup(getOutputDirectory());

		for (int i=1; i<=2; i++) // move up two directories from the output directory
		{
			delim = strrchr(baseDirectory_, OS_PATH_DELIM);
			if (delim != NULL)
			{
				*delim = 0;
			} else
			{
				ckfree(baseDirectory_);
				baseDirectory_ = ckstrdup(".");
			}
		}
	}
	return baseDirectory_;
}

void SimulationResult::dump(FILE *fp, int flags)
{
	int i;
	fprintf(fp, "Simulation Result Contents:\n");
	fprintf(fp, "File Id : %d\n", fileId_);
	fprintf(fp, "\n");
	/*
	if ((flags & PRINT_EMG_DATA) != 0 && emgData_ != NULL)
	{
		fprintf(fp, "Emg Data:\n");
		emgData_->dump(fp);
		fprintf(fp, "\n");
	}
	*/
	if ((flags & PRINT_MUP_DATA) != 0)
	{
		fprintf(fp, "MUP List\n");
		for (i = 0; i < nMUPs_; i++)
		{
		    fprintf(fp, "    id %d (%d MFPS)\n",
		                getMUP(i)->getId(),
		                getMUP(i)->getNMFPs());
		    freeMUP(i);
		}
		fprintf(fp, "\n");
	}
	if ((flags & PRINT_MUSCLE_DATA) != 0)
	{
		fprintf(fp, "Muscle Data:\n");
		// muscleData_->dump(fp, MuscleData::PRINT_ALL_DATA);
		muscleData_->dump(fp);
	}
	fprintf(fp, "\n");

	if (dcoData_ == NULL)
	{
		fprintf(fp, "DCO data is NULL\n");
	} else
	{
		dumpDco(fp, 4, dcoData_);
	}
}


