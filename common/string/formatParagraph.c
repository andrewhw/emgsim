/**
 * ------------------------------------------------------------
 * Format a paragraph so that it appears within a given width
 * ------------------------------------------------------------
 * $Id: formatParagraph.c 17 2008-07-03 17:24:49Z andrew $
 */

#ifndef MAKEDEPEND
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#endif

#include <stringtools.h>
#include <tclCkalloc.h>
#include <listalloc.h>
#include <massert.h>

#include <os_defs.h>


/**
 * create the structure we need to track data allocated in formatting
 */
OS_EXPORT struct strFormatData *
createFormatter(const char *indentString, int maxLineLen)
{
	struct strFormatData *result;

	result = ckalloc(sizeof(struct strFormatData));
	memset(result, 0, sizeof(struct strFormatData));

	result->maxLineLen_ = maxLineLen;

	result->indentStr_ = ckstrdup(indentString);
	result->indentLen_ = strlen(indentString);

	return result;
}

OS_EXPORT void
deleteFormatter(struct strFormatData * config)
{
	if (config != NULL)
	{
		if (config->data_ != NULL)
		{
			ckfree(config->data_);
			config->data_ = NULL;
			config->nBlocks_ = 0;
		}
		if (config->indentStr_ != NULL)
		{
			ckfree(config->indentStr_);
			config->indentLen_ = 0;
		}
		ckfree(config);
	}
}

static void
addAToken(struct strFormatData * config,
		const char *token,
		int tokenLen
	)
{
	int status;

	if (token[0] == '\n')
	{

		status = listCheckSize(
							   config->loadOffset_ + config->indentLen_ + 2,
								 (void **) &config->data_,
								 &config->nBlocks_, BUFSIZ / 2,
								 1);
		MSG_ASSERT(status, "memory allocation failure");

		config->data_[config->loadOffset_++] = '\n';
		memcpy(&config->data_[config->loadOffset_],
			   config->indentStr_, config->indentLen_);
		config->loadOffset_ += config->indentLen_;
		config->curLineLen_ = config->indentLen_;
		config->data_[config->loadOffset_] = 0;
		config->continueLine_ = 0;

		addAToken(config, &token[1], tokenLen - 1);

	} else if ((config->curLineLen_ + tokenLen + 1) >= config->maxLineLen_)
	{

		status = listCheckSize(
								 config->loadOffset_ + config->indentLen_
								 + tokenLen + 2,
								 (void **) &config->data_,
								 &config->nBlocks_, BUFSIZ / 2,
								 1);
		MSG_ASSERT(status, "memory allocation failure");

		config->data_[config->loadOffset_++] = '\n';
		memcpy(&config->data_[config->loadOffset_],
			   config->indentStr_, config->indentLen_);
		config->loadOffset_ += config->indentLen_;
		memcpy(&config->data_[config->loadOffset_], token, tokenLen);
		config->loadOffset_ += tokenLen;
		config->curLineLen_ = config->indentLen_ + tokenLen;
		config->data_[config->loadOffset_] = 0;
		config->continueLine_ = 1;

	} else
	{

		status = listCheckSize(
								 config->loadOffset_ + tokenLen + 2,
								 (void **) &config->data_,
								 &config->nBlocks_, BUFSIZ / 2, 1);
		MSG_ASSERT(status, "memory allocation failure");
		if (config->continueLine_)
		{
			config->data_[config->loadOffset_++] = ' ';
			config->curLineLen_++;
		} else
		{
			config->continueLine_ = 1;
		}
		memcpy(&config->data_[config->loadOffset_], token, tokenLen);
		config->loadOffset_ += tokenLen;
		config->curLineLen_ += tokenLen;
		config->data_[config->loadOffset_] = 0;
	}
}

static void
setupFormatter(struct strFormatData * config, const char *firstIndent)
{
	int status;

	config->curLineLen_ = 0;
	config->continueLine_ = 0;
	config->loadOffset_ = 0;

	MSG_ASSERT(strlen(firstIndent) <= config->indentLen_, "indent bad");

	status = listCheckSize(
							 config->loadOffset_ + config->indentLen_ + 2,
							 (void **) &config->data_,
							 &config->nBlocks_, BUFSIZ / 2,
							 1);
	MSG_ASSERT(status, "memory allocation failure");

	slnprintf(config->data_,
					config->nBlocks_ * (BUFSIZ / 2),
					"%*s",
				(int) config->indentLen_,
				firstIndent);
	config->loadOffset_ += config->indentLen_;
	config->data_[config->loadOffset_] = 0;
	config->curLineLen_ = config->indentLen_;
}



static void
finishParagraph(struct strFormatData * config)
{
	int status;

	if (config->continueLine_)
	{
		status = listCheckSize(
								 config->loadOffset_ + 2,
								 (void **) &config->data_,
								 &config->nBlocks_, BUFSIZ / 2, 1);
		MSG_ASSERT(status, "memory allocation failure");
		config->data_[config->loadOffset_++] = '\n';
		config->data_[config->loadOffset_++] = 0;
	}
}


/*
 * -------------------------------------------------
 * Trim any extra zeros at the end of a double value
 * -------------------------------------------------
 */
OS_EXPORT char *
formatParagraph(
		struct strFormatData * config,
		const char *firstIndent,
		const char *data
	)
{
	const char     *beginToken;
	const char     *endToken;
	int             tokenLen;

	beginToken = data;

	setupFormatter(config, firstIndent);

	while ((endToken = strchr(beginToken, ' ')) != NULL)
	{
		tokenLen = (endToken - beginToken);

		addAToken(config, beginToken, tokenLen);
		beginToken = endToken + 1;
	}

	tokenLen = strlen(beginToken);
	addAToken(config, beginToken, tokenLen);

	finishParagraph(config);

	return config->data_;
}

