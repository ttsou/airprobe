#ifndef _RXBURST_H
#define _RXBURST_H

#include "GSMCommon.h"
#include "BitVector.h"

namespace GSM {

/**@name Positions of stealing bits within a normal burst, GSM 05.03 3.1.4. */
//@{
static const unsigned gHlIndex = 60;		///< index of first stealing bit, GSM 05.03 3.1.4
static const unsigned gHuIndex = 87;		///< index of second stealing bit, GSM 05.03 3.1.4
//@}

static const unsigned gSlotLen = 148;	///< number of symbols per slot, not counting guard periods


/**
	Class to represent one timeslot of channel bits with soft encoding.
*/
class RxBurst : public SoftVector {

	private:

	Time mTime;				///< timeslot and frame on which this was received
//	float mTimingError;		///< Timing error in symbol steps, <0 means early.
//	float mRSSI;			///< RSSI estimate associated with the slot, dB wrt full scale.

	public:

	/** Wrap an RxBurst around an existing float array. */
	RxBurst(float* wData, const Time &wTime)
		:SoftVector(wData,gSlotLen),mTime(wTime)
//		mTimingError(wTimingError),mRSSI(wRSSI)
	{ }


	Time time() const { return mTime; }

	void time(const Time& wTime) { mTime = wTime; }
	
//	float RSSI() const { return mRSSI; }

//	float timingError() const { return mTimingError; }

	/** Return a SoftVector alias to the first data field. */
	const SoftVector data1() const { return segment(3, 57); }

	/** Return a SoftVector alias to the second data field. */
	const SoftVector data2() const { return segment(88, 57); }

	/** Return upper stealing bit. */
	bool Hu() const { return bit(gHuIndex); }

	/** Return lower stealing bit. */
	bool Hl() const { return bit(gHlIndex); }

// 	friend std::ostream& operator<<(std::ostream& os, const RxBurst& ts);
};

// std::ostream& operator<<(std::ostream& os, const RxBurst& ts){
//   os << "time=" << ts.time();
//   os << " data=(" << (const SoftVector&)ts << ")" ;
//   return os;
// }


}
#endif
