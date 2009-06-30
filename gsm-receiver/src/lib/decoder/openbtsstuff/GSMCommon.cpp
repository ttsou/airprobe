/*
* Copyright 2008 Free Software Foundation, Inc.
*
* This software is distributed under the terms of the GNU Public License.
* See the COPYING file in the main directory for details.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "GSMCommon.h"

using namespace GSM;
using namespace std;


ostream& GSM::operator<<(ostream& os, L3PD val)
{
	switch (val) {
		case L3CallControlPD: os << "Call Control"; break;
		case L3MobilityManagementPD: os << "Mobility Management"; break;
		case L3RadioResourcePD: os << "Radio Resource"; break;
		default: os << hex << "0x" << (int)val << dec;
	}
	return os;
}


const BitVector GSM::gTrainingSequence[] = {
    BitVector("00100101110000100010010111"),
    BitVector("00101101110111100010110111"),
    BitVector("01000011101110100100001110"),
    BitVector("01000111101101000100011110"),
    BitVector("00011010111001000001101011"),
    BitVector("01001110101100000100111010"),
    BitVector("10100111110110001010011111"),
    BitVector("11101111000100101110111100"),
};

const BitVector GSM::gDummyBurst("0001111101101110110000010100100111000001001000100000001111100011100010111000101110001010111010010100011001100111001111010011111000100101111101010000");

const BitVector GSM::gRACHSynchSequence("01001011011111111001100110101010001111000");



char encodeGSMChar(char ascii)
{
	// Given an ASCII char, return the corresponding GSM char.
	static char reverseTable[256]={0};
	static bool init = false;
	if (!init) {
		for (size_t i=0; i<sizeof(gGSMAlphabet); i++) {
			reverseTable[(unsigned)gGSMAlphabet[i]]=i;
		}
		init=true;
	}
	return reverseTable[(unsigned)ascii];
}


char encodeBCDChar(char ascii)
{
	// Given an ASCII char, return the corresponding BCD.
	if ((ascii>='0') && (ascii<='9')) return ascii-'0';
	switch (ascii) {
		case '.': return 11;
		case '*': return 11;
		case '#': return 12;
		case 'a': return 13;
		case 'b': return 14;
		case 'c': return 15;
		default: return 15;
	}
}




unsigned GSM::uplinkFreqKHz(GSMBand band, unsigned ARFCN)
{
	switch (band) {
		case GSM850:
			assert((ARFCN<252)&&(ARFCN>129));
			return 824200+200*(ARFCN-128);
		case EGSM900:
			if (ARFCN<=124) return 890000+200*ARFCN;
			assert((ARFCN>974)&&(ARFCN<1024));
			return 890000+200*(ARFCN-1024);
		case DCS1800:
			assert((ARFCN>511)&&(ARFCN<886));
			return 1710200+200*(ARFCN-512);
		case PCS1900:
			assert((ARFCN>511)&&(ARFCN<811));
			return 1850200+200*(ARFCN-512);
		default:
			abort();
	}
}


unsigned GSM::downlinkFreqKHz(GSMBand band, unsigned ARFCN)
{
	static unsigned uplinkOffset[] = {
		45000,	// 850
		45000,	// 900
		95000,	// 1800
		80000	// 1900
	};
	return uplinkFreqKHz(band,ARFCN) + uplinkOffset[band];
}




int32_t GSM::FNDelta(int32_t v1, int32_t v2)
{
	static const int64_t halfModulus = gHyperframe/2;
	int32_t delta = v1-v2;
	if (delta>halfModulus) delta -= gHyperframe;
	else if (delta<-halfModulus) delta += gHyperframe;
	return (int32_t) delta;
}

int GSM::FNCompare(int32_t v1, int32_t v2)
{
	int32_t delta = FNDelta(v1,v2);
	if (delta==0) return 0;
	else if (delta>0) return 1;
	else return -1;
}




ostream& GSM::operator<<(ostream& os, const Time& t)
{
	os << t.TN() << ":" << t.FN();
	return os;
}




void Clock::set(const Time& when)
{
	mLock.lock();
	mBaseTime = Timeval(0);
	mBaseFN = when.FN();
	mLock.unlock();
}


int32_t Clock::FN() const
{
	mLock.lock();
	Timeval now;
	int deltaSec = now.sec() - mBaseTime.sec();
	int deltaUSec = now.usec() - mBaseTime.usec();
	int elapsedUSec = 1000000*deltaSec + deltaUSec;
	int elapsedFrames = elapsedUSec / gFrameMicroseconds;
	int32_t currentFN = (mBaseFN + elapsedFrames) % gHyperframe;
	mLock.unlock();
	return currentFN;
}


void Clock::wait(const Time& when) const
{
	int32_t now = FN();
	int32_t target = when.FN();
	int32_t delta = FNDelta(target,now);
	if (delta<1) return;
	const int32_t maxSleep = 51*26;
	if (delta>maxSleep) delta=maxSleep;
	sleepFrames(delta);
}







ostream& GSM::operator<<(ostream& os, TypeOfNumber type)
{
	switch (type) {
		case UnknownTypeOfNumber: os << "unknown"; break;
		case InternationalNumber: os << "international"; break;
		case NationalNumber: os << "national"; break;
		case NetworkSpecificNumber: os << "network-specific"; break;
		case ShortCodeNumber: os << "short code"; break;
		default: os << "?" << type << "?";
	}
	return os;
}


ostream& GSM::operator<<(ostream& os, NumberingPlan plan)
{
	switch (plan) {
		case UnknownPlan: os << "unknown"; break;
		case E164Plan: os << "E.164/ISDN"; break;
		case X121Plan: os << "X.121/data"; break;
		case F69Plan: os << "F.69/Telex"; break;
		case NationalPlan: os << "national"; break;
		case PrivatePlan: os << "private"; break;
		default: os << "?" << (int)plan << "?";
	}
	return os;
}

ostream& GSM::operator<<(ostream& os, MobileIDType wID)
{
	switch (wID) {
		case NoIDType: os << "None"; break;
		case IMSIType: os << "IMSI"; break;
		case IMEIType: os << "IMEI"; break;
		case TMSIType: os << "TMSI"; break;
		case IMEISVType: os << "IMEISV"; break;
		default: os << "?" << (int)wID << "?";
	}
	return os;
}


ostream& GSM::operator<<(ostream& os, TypeAndOffset tao)
{
	switch (tao) {
		case TDMA_MISC: os << "(misc)"; break;
		case TCHF_0: os << "TCH/F"; break;
		case TCHH_0: os << "TCH/H-0"; break;
		case TCHH_1: os << "TCH/H-1"; break;
		case SDCCH_4_0: os << "SDCCH/4-0"; break;
		case SDCCH_4_1: os << "SDCCH/4-1"; break;
		case SDCCH_4_2: os << "SDCCH/4-2"; break;
		case SDCCH_4_3: os << "SDCCH/4-3"; break;
		case SDCCH_8_0: os << "SDCCH/8-0"; break;
		case SDCCH_8_1: os << "SDCCH/8-1"; break;
		case SDCCH_8_2: os << "SDCCH/8-2"; break;
		case SDCCH_8_3: os << "SDCCH/8-3"; break;
		case SDCCH_8_4: os << "SDCCH/8-4"; break;
		case SDCCH_8_5: os << "SDCCH/8-5"; break;
		case SDCCH_8_6: os << "SDCCH/8-6"; break;
		case SDCCH_8_7: os << "SDCCH/8-7"; break;
		case TDMA_BEACON: os << "(beacon)"; break;
		default: os << "?" << (int)tao << "?";
	}
	return os;
}

ostream& GSM::operator<<(ostream& os, ChannelType val)
{
	switch (val) {
		case UndefinedCHType: os << "undefined"; return os;
		case SCHType: os << "SCH"; break;
		case FCCHType: os << "FCCH"; break;
		case BCCHType: os << "BCCH"; break;
		case RACHType: os << "RACH"; break;
		case SDCCHType: os << "SDCCH"; break;
		case FACCHType: os << "FACCH"; break;
		case CCCHType: os << "CCCH"; break;
		case SACCHType: os << "SACCH"; break;
		case TCHFType: os << "TCH/F"; break;
		case TCHHType: os << "TCH/H"; break;
		case AnyTCHType: os << "any TCH"; break;
		case LoopbackFullType: os << "Loopback Full"; break;
		case LoopbackHalfType: os << "Loopback Half"; break;
		case AnyDCCHType: os << "any DCCH"; break;
		default: os << "?" << (int)val << "?";
	}
	return os;
}




bool Z100Timer::expired() const
{
	// A non-active timer does not expire.
	if (!mActive) return false;
	return mEndTime.passed();
}

void Z100Timer::set()
{
	mEndTime = Timeval(mLimitTime);
	mActive=true;
} 

long Z100Timer::remaining() const
{
	if (!mActive) return 0;
	long rem = mEndTime.remaining();
	if (rem<0) rem=0;
	return rem;
}

void Z100Timer::wait() const
{
	while (!expired()) msleep(remaining());
}

// vim: ts=4 sw=4
