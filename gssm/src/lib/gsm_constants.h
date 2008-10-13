// $Id: gsm_constants.h,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#pragma once

static const double	QN_TIME		= 12.0l / 13.0l / 1000000.0l;
static const double	BN_TIME		= 4.0l * QN_TIME;
static const double	TN_TIME		= 156.25l * BN_TIME;
static const double	FN_TIME		= 8.0l * TN_TIME;

static const double	GSM_RATE	= 812500.0l / 3.0l;
static const int	MAX_FN		= 26 * 51 * 2048;

static const int	BURST_LENGTH	= 156;


/*
 * frequency correction:
 * 	tn = 0	fnm51 = 0
 *
 * sync:
 * 	tn = 0	fnm51 = 1
 * 	tn = 0	fnm51 = 11
 * 	tn = 0	fnm51 = 21
 * 	tn = 0	fnm51 = 31
 * 	tn = 0	fnm51 = 41
 *
 * bcch:
 * 	tn = 0	fnm51 = 2, 3, 4, 5
 * 	tn = 2	fnm51 = 2, 3, 4, 5
 * 	tn = 4	fnm51 = 2, 3, 4, 5
 * 	tn = 6	fnm51 = 2, 3, 4, 5
 *
 * bcch_ext:
 * 	tn = 0	fnm51 = 6, 7, 8, 9
 * 	tn = 2	fnm51 = 6, 7, 8, 9
 * 	tn = 4	fnm51 = 6, 7, 8, 9
 * 	tn = 6	fnm51 = 6, 7, 8, 9
 *
 * pch and agch:
 * 	tn = 0	fnm51 = 6, 7, 8, 9
 * 	tn = 0	fnm51 = 12, 13, 14, 15
 * 	tn = 0	fnm51 = 16, 17, 18, 19
 * 	tn = 0	fnm51 = 22, 23, 24, 25
 * 	tn = 0	fnm51 = 26, 27, 28, 29
 * 	tn = 0	fnm51 = 32, 33, 34, 35
 * 	tn = 0	fnm51 = 36, 37, 38, 39
 * 	tn = 0	fnm51 = 42, 43, 44, 45
 * 	tn = 0	fnm51 = 46, 47, 48, 49
 * 	tn = 2	fnm51 = 6, 7, 8, 9
 * 	tn = 2	fnm51 = 12, 13, 14, 15
 * 	tn = 2	fnm51 = 16, 17, 18, 19
 * 	tn = 2	fnm51 = 22, 23, 24, 25
 * 	tn = 2	fnm51 = 26, 27, 28, 29
 * 	tn = 2	fnm51 = 32, 33, 34, 35
 * 	tn = 2	fnm51 = 36, 37, 38, 39
 * 	tn = 2	fnm51 = 42, 43, 44, 45
 * 	tn = 2	fnm51 = 46, 47, 48, 49
 * 	tn = 4	fnm51 = 6, 7, 8, 9
 * 	tn = 4	fnm51 = 12, 13, 14, 15
 * 	tn = 4	fnm51 = 16, 17, 18, 19
 * 	tn = 4	fnm51 = 22, 23, 24, 25
 * 	tn = 4	fnm51 = 26, 27, 28, 29
 * 	tn = 4	fnm51 = 32, 33, 34, 35
 * 	tn = 4	fnm51 = 36, 37, 38, 39
 * 	tn = 4	fnm51 = 42, 43, 44, 45
 * 	tn = 4	fnm51 = 46, 47, 48, 49
 * 	tn = 6	fnm51 = 6, 7, 8, 9
 * 	tn = 6	fnm51 = 12, 13, 14, 15
 * 	tn = 6	fnm51 = 16, 17, 18, 19
 * 	tn = 6	fnm51 = 22, 23, 24, 25
 * 	tn = 6	fnm51 = 26, 27, 28, 29
 * 	tn = 6	fnm51 = 32, 33, 34, 35
 * 	tn = 6	fnm51 = 36, 37, 38, 39
 * 	tn = 6	fnm51 = 42, 43, 44, 45
 * 	tn = 6	fnm51 = 46, 47, 48, 49
 */
