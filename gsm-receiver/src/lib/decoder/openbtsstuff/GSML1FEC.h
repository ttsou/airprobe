/*
* Copyright 2008 Free Software Foundation, Inc.
*
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* This software is distributed under the terms of the GNU Public License.
* See the COPYING file in the main directory for details.
*/



#ifndef GSML1FEC_H
#define GSML1FEC_H

//#include "Threads.h"
#include "Assert.h"
#include "BitVector.h"

#include "GSMCommon.h"
//#include "GSMTransfer.h"
#include "GSMTDMA.h"
#include "VocoderFrame.h"
#include "RxBurst.h"
//#include "GSM610Tables.h"
#include <stdio.h>



namespace GSM
{


//class SAPMux;
  class L1FEC;
  class L1Decoder;



  /*
   Naming convention for bit vectors follows GSM 05.03 Section 2.2.
   d[k]  data
   u[k]  data bits after first encoding step
   c[k]  data bits after second encoding step
   i[B][k]  interleaved data bits
   e[B][k]  bits in a burst
  */


  enum TCHMode {
    MODE_SPEECH_FR,
    MODE_SPEECH_EFR,
  };


  /** L1 decoder used for full rate TCH and FACCH -- mostly from GSM 05.03 3.1 and 4.2 */
//: public XCCHL1Decoder
  class TCHFACCHL1Decoder
  {

    protected:
      SoftVector mI[8]; ///< deinterleaving history, 8 blocks instead of 4
      SoftVector mC;    ///< c[], as per GSM 05.03 2.2
      BitVector mTCHU;     ///< u[] (uncoded) in the spec
      BitVector mTCHD;     ///< d[] (data) in the spec
      SoftVector mClass1_c;    ///< the class 1 part of c[]
      BitVector mClass1A_d;    ///< the class 1A part of d[]
      SoftVector mClass2_c;    ///< the class 2 part of c[]
      ViterbiR2O4 mVCoder;

      VocoderFrame mVFrame;    ///< buffer for FR vocoder frame
      VocoderAMRFrame mVFrameAMR; ///< buffer for EFR vocoder frame packed in AMR container
      unsigned char mPrevGoodFrame[33]; ///< previous good frame.
      unsigned int  mPrevGoodFrameLength;

      Parity mTCHParity;
      const TDMAMapping& mMapping; ///< multiplexing description

// InterthreadQueue<unsigned char> mSpeechQ;     ///< output queue for speech frames

      static const unsigned mMaxQSize = 3;

      enum TCHMode mMode;

    public:

      TCHFACCHL1Decoder( const TDMAMapping& wMapping );

      ChannelType channelType() const {
        return FACCHType;
      }

      enum TCHMode mode() const {
        return mMode;
      }

      void setMode(enum TCHMode mode) {
        mMode = mode;
      }
 

      /** TCH/FACCH has a special-case writeLowSide. */
      void writeLowSide(const RxBurst& inBurst);

      /**
       Unlike other DCCHs, TCH/FACCH process burst calls
       deinterleave, decode, handleGoodFrame.
      */
      bool processBurst( const RxBurst& );

      /** Deinterleave i[] to c[].  */
      void deinterleave(int blockOffset );

      void replaceFACCH( int blockOffset );

      /**
       Decode a traffic frame from TCHI[] and enqueue it.
       Return true if there's a good frame.
      */
      bool decodeTCH(bool stolen);

      unsigned char * get_voice_frame(){
        return mPrevGoodFrame;
      }
      unsigned int get_voice_frame_length(){
        return mPrevGoodFrameLength;
      }
      /**
       Receive a traffic frame.
       Non-blocking.  Returns NULL if queue is dry.
       Caller is responsible for deleting the returned array.
      */
// unsigned char *recvTCH() { return mSpeechQ.read(0); }

      /** Return count of internally-queued traffic frames. */
// unsigned queueSize() const { return mSpeechQ.size(); }

  };






};  // namespace GSM





#endif

// vim: ts=4 sw=4
