// $Id: gsm_constants.h,v 1.3 2007/03/14 05:44:53 jl Exp $

#pragma once

static const double	QN_TIME		= 12.0l / 13.0l / 1000000.0l;
static const double	BN_TIME		= 4.0l * QN_TIME;
static const double	TN_TIME		= 156.25l * BN_TIME;
static const double	FN_TIME		= 8.0l * TN_TIME;

static const double	GSM_RATE	= 812500.0l / 3.0l;
static const int	MAX_FN		= 26 * 51 * 2048;
static const int	BURST_LENGTH	= 156;
