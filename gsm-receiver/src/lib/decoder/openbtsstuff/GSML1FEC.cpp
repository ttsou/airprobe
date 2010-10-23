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


#define NDEBUG


#include "GSML1FEC.h"
#include "GSMCommon.h"
#include "RxBurst.h"
//#include "GSMSAPMux.h"
//#include "GSMConfig.h"
#include "GSMTDMA.h"
#include "GSM610Tables.h"
#include "GSM660Tables.h"
#include "GSM690Tables.h"
#include "Assert.h"


using namespace std;
using namespace GSM;

/*
 Compilation flags:
 NOCONTROL Compile without referencing control layer functions.
*/


/*

 Notes on reading the GSM specifications.

 Every FEC section in GSM 05.03 uses standard names for the bits at
 different stages of the encoding/decoding process.

 This is all described formally in GSM 05.03 2.2.

 "d" -- data bits.  The actual payloads from L2 and the vocoders.
 "p" -- parity bits.  These are calculated from d.
 "u" -- uncoded bits.  A concatenation of d, p and inner tail bits.
 "c" -- coded bits.  These are the convolutionally encoded from u.
 "i" -- interleaved bits.  These are the output of the interleaver.
 "e" -- "encrypted" bits.  These are the channel bits in the radio bursts.

 The "e" bits are call "encrypted" even when encryption is not used.

 The encoding process is:

 L2 -> d -> -> calc p -> u -> c -> i -> e -> radio bursts

 The decoding process is:

 radio bursts -> e -> i -> c -> u -> check p -> d -> L2

 Bit ordering in d is LSB-first in each octet.
 Bit ordering everywhere else in the OpenBTS code is MSB-first
 in every field to give contiguous fields across byte boundaries.
 We use the BitVector::LSB8MSB() method to translate.

*/

TCHFACCHL1Decoder::TCHFACCHL1Decoder(const TDMAMapping& wMapping)
    : mTCHU(189), mTCHD(260), mC(456),
    mClass1_c(mC.head(378)), mClass1A_d(mTCHD.head(50)), mClass2_c(mC.segment(378, 78)),
    mTCHParity(0x0b, 3, 50), mMapping(wMapping), mMode(MODE_SPEECH_FR)
{
  for (int i = 0; i < 8; i++) {
    mI[i] = SoftVector(114);
  }
}


void TCHFACCHL1Decoder::writeLowSide(const RxBurst& inBurst)
{
  OBJDCOUT("TCHFACCHL1Decoder::writeLowSide " << inBurst);
  // If the channel is closed, ignore the burst.
// if (!active()) {
//  OBJDCOUT("TCHFACCHL1Decoder::writeLowSide not active, ignoring input");
//  return;
// }
  processBurst(inBurst);
}

bool TCHFACCHL1Decoder::processBurst( const RxBurst& inBurst)
{
  // Accept the burst into the deinterleaving buffer.
  // Return true if we are ready to interleave.

  // TODO -- One quick test of burst validity is to look at the tail bits.
  // We could do that as a double-check against putting garbage into
  // the interleaver or accepting bad parameters.

  // Get the physical parameters of the burst.
  // RSSI is dB wrt full scale.
// mRSSI = inBurst.RSSI();
  // Timing error is a float in symbol intervals.
// mTimingError = inBurst.timingError();
  // This flag is used as a half-ass semaphore.
  // It is cleared when the new value is read.
// mPhyNew = true;

  // The reverse index runs 0..3 as the bursts arrive.
  // It is the "B" index of GSM 05.03 3.1.3 and 3.1.4.
  int B = mMapping.reverseMapping(inBurst.time().FN()) % 8;
  // A negative value means that the demux is misconfigured.
  assert(B >= 0);
  OBJDCOUT("TCHFACCHL1Decoder::processBurst B=" << B << " " << inBurst);

  // Pull the data fields (e-bits) out of the burst and put them into i[B][].
  // GSM 05.03 3.1.4
  inBurst.data1().copyToSegment(mI[B], 0);
  inBurst.data2().copyToSegment(mI[B], 57);

  // Every 4th frame is the start of a new block.
  // So if this isn't a "4th" frame, return now.
  if (B % 4 != 3) return false;

  // Deinterleave according to the diagonal "phase" of B.
  // See GSM 05.03 3.1.3.
  // Deinterleaves i[] to c[]
  if (B == 3) deinterleave(4);
  else deinterleave(0);

  // See if this was the end of a stolen frame, GSM 05.03 4.2.5.
  bool stolen = inBurst.Hl();
  OBJDCOUT("TCHFACCHL!Decoder::processBurst Hl=" << inBurst.Hl() << " Hu=" << inBurst.Hu());
  /* if (stolen) {
    if (decode()) {
     OBJDCOUT("TCHFACCHL1Decoder::processBurst good FACCH frame");
     countGoodFrame();
     handleGoodFrame();
    } else {
     OBJDCOUT("TCHFACCHL1Decoder::processBurst bad FACCH frame");
     countBadFrame();
    }
   }*/

  // Always feed the traffic channel, even on a stolen frame.
  // decodeTCH will handle the GSM 06.11 bad frmae processing.
  bool traffic = decodeTCH(stolen);
//  if (traffic) {
  OBJDCOUT("TCHFACCHL1Decoder::processBurst good TCH frame");
//  countGoodFrame();
  // Don't let the channel timeout.
  // mLock.lock();
  // mT3109.set();
  // mLock.unlock();
//  }
// else countBadFrame();

  return traffic;
}

void TCHFACCHL1Decoder::deinterleave(int blockOffset )
{
  OBJDCOUT("TCHFACCHL1Decoder::deinterleave blockOffset=" << blockOffset);
  for (int k = 0; k < 456; k++) {
    int B = ( k + blockOffset ) % 8;
    int j = 2 * ((49 * k) % 57) + ((k % 8) / 4);
    mC[k] = mI[B][j];
    mI[B][j] = 0.5F;
    //OBJDCOUT("deinterleave k="<<k<<" B="<<B<<" j="<<j);
  }
}

bool TCHFACCHL1Decoder::decodeTCH(bool stolen)
{
  // GSM 05.02 3.1.2, but backwards

  // If the frame wasn't stolen, we'll update this with parity later.
  bool good = !stolen;

  if (!stolen) {

    // 3.1.2.2
    // decode from c[] to u[]
    mClass1_c.decode(mVCoder, mTCHU);
    //mC.head(378).decode(mVCoder,mTCHU);

    // 3.1.2.2
    // copy class 2 bits c[] to d[]
    mClass2_c.sliced().copyToSegment(mTCHD, 182);
    //mC.segment(378,78).sliced().copyToSegment(mTCHD,182);

    // 3.1.2.1
    // copy class 1 bits u[] to d[]
    for (unsigned k = 0; k <= 90; k++) {
      mTCHD[2*k] = mTCHU[k];
      mTCHD[2*k+1] = mTCHU[184-k];
    }

    // 3.1.2.1
    // check parity of class 1A
    unsigned sentParity = (~mTCHU.peekField(91, 3)) & 0x07;
    //unsigned calcParity = mTCHD.head(50).parity(mTCHParity) & 0x07;
    unsigned calcParity = mClass1A_d.parity(mTCHParity) & 0x07;

    // 3.1.2.2
    // Check the tail bits, too.
    unsigned tail = mTCHU.peekField(185, 4);

    OBJDCOUT("TCHFACCHL1Decoder::decodeTCH c[]=" << mC);
    //OBJDCOUT("TCHFACCHL1Decoder::decodeTCH u[]=" << mTCHU);
    OBJDCOUT("TCHFACCHL1Decoder::decodeTCH d[]=" << mTCHD);
    OBJDCOUT("TCHFACCHL1Decoder::decodeTCH sentParity=" << sentParity
             << " calcParity=" << calcParity << " tail=" << tail);
    good = (sentParity == calcParity) && (tail == 0);
    if (good) {
      if (mMode == MODE_SPEECH_FR) {
        // Undo Um's importance-sorted bit ordering.
        // See GSM 05.03 3.1 and Tablee 2.
        BitVector payload = mVFrame.payload();
        mTCHD.unmap(g610BitOrder, 260, payload);
        mVFrame.pack(mPrevGoodFrame);
        mPrevGoodFrameLength = 33;
      } else if (mMode == MODE_SPEECH_EFR) {
        BitVector payload = mVFrameAMR.payload();
        BitVector TCHW(260), EFRBits(244);

        // Undo Um's EFR bit ordering.
        mTCHD.unmap(g660BitOrder, 260, TCHW);

        // Remove repeating bits and CRC to get raw EFR frame (244 bits)
        for (unsigned k=0; k<71; k++)
          EFRBits[k] = TCHW[k] & 1;

        for (unsigned k=73; k<123; k++)
          EFRBits[k-2] = TCHW[k] & 1;

        for (unsigned k=125; k<178; k++)
          EFRBits[k-4] = TCHW[k] & 1;

        for (unsigned k=180; k<230; k++)
          EFRBits[k-6] = TCHW[k] & 1;

        for (unsigned k=232; k<252; k++)
          EFRBits[k-8] = TCHW[k] & 1;

        // Map bits as AMR 12.2k
        EFRBits.map(g690_12_2_BitOrder, 244, payload);

        // Put the whole frame (hdr + payload)
        mVFrameAMR.pack(mPrevGoodFrame);
        mPrevGoodFrameLength = 32;
      }

      return true;
    }
  }

  return false;
}

// vim: ts=4 sw=4
