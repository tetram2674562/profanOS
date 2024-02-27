/*
 * Emuxki BeOS Driver for Creative Labs SBLive!/Audigy series
 *
 * Copyright (c) 2002, Jerome Duval (jerome.duval@free.fr)
 *
 * Original code : BeOS Driver for Intel ICH AC'97 Link interface
 * Copyright (c) 2002, Marcus Overhagen <marcus@overhagen.de>
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _AC97_H_
#define _AC97_H_
#include <ktype.h>
#include <stdlib.h>
#include <stdbool.h>
#define INTEL_VID            0x8086           //; vendor ID: Intel
#define ICH_DID              0x2415          //; device ID: 82801AA AC'97 Audio Controller (ICH)
#define ICH0_DID             0x2425           //; device ID: 82801AB AC'97 Audio Controller (ICH0)
#define ICH2_DID             0x2445           //; device ID: 82801BA/BAM AC'97 Audio Controller (ICH2)
#define ICH3_DID             0x2485           //; device ID: 82801CA/CAM AC'97 Audio Controller (ICH3)
#define ICH4_DID             0x24c5           //; device ID: 82801DB/DBL/DBM AC'97 Audio Controller (ICH4)
#define ICH5_DID             0x24d5           //; device ID: 82801EB/ER AC'97 Audio Controller (ICH5)
#define ESB_DID              0x25a6          //; device ID: 6300ESB AC'97 Audio Controller (ESB)
#define ICH6_DID             0x266e          //; device ID: 82801FB/FBM/FR/FW/FRW AC'97 Audio Controller (ICH6)
#define ICH7_DID             0x27de          //; device ID: 82801GB/GBM/GR/GH/GHM AC'97 Audio Controller (ICH7)
#define I440MX_DID           0x7195          //; device ID: 82440MX AC'97 Audio Controller (440MX)

//; Other (non-Intel) vendors that are (mostly) compatible with ICHx AC'97 audio
#define SIS_VID              0x1039          //; vendor ID: Silicon Integrated Systems (SiS)
#define SIS_7012_DID         0x7012        // ; device ID: SiS7012 AC'97 Sound Controller

#define NAMBAR_REG           0x10          // ; native audio mixer BAR
#define NAM_SIZE            256            // ; 256 bytes required.

#define NABMBAR_REG          0x14            // ; native audio bus mastering BAR
#define NABM_SIZE           64             // ; 64 bytes


/*
; BUS master registers, accessed via NABMBAR+offset

; ICH supports 3 different types of register sets for three types of things
; it can do, thus:
;
; PCM in (for recording) aka PI
; PCM out (for playback) aka PO
; MIC in (for recording) aka MC
*/
#define PI_BDBAR_REG                 0      // ; PCM in buffer descriptor BAR
#define PO_BDBAR_REG                 0x10    // ; PCM out buffer descriptor BAR
#define MC_BDBAR_REG                 0x20     //; MIC in buffer descriptor BAR

#define CUSTOM_SIS_7012_REG                  0x4c    ; SiS7012-specific register, required for unmuting output
/*
; each buffer descriptor BAR holds a pointer which has entries to the buffer
; contents of the .WAV file we're going to play.  Each entry is 8 bytes long
; (more on that later) and can contain 32 entries total, so each BAR is
; 256 bytes in length, thus:
*/
#define BDL_SIZE                     32*8    //; Buffer Descriptor List size
#define INDEX_MASK                   31      //; indexes must be 0-31



#define PI_CIV_REG                   4      // ; PCM in current Index value (RO)
#define PO_CIV_REG                   0x14   //; PCM out current Index value (RO)
#define MC_CIV_REG                   0x24     //; MIC in current Index value (RO)
/*
;8bit read only
; each current index value is simply a pointer showing us which buffer
; (0-31) the codec is currently processing.  Once this counter hits 31, it
; wraps back to 0.
; this can be handy to know, as once it hits 31, we're almost out of data to
; play back or room to record!
*/

#define PI_LVI_REG                   5       ; PCM in Last Valid Index
#define PO_LVI_REG                   0x15     ; PCM out Last Valid Index
#define MC_LVI_REG                   0x25     ; MIC in Last Valid Index
/*8bit read/write
 The Last Valid Index is a number (0-31) to let the codec know what buffer
; number to stop on after processing.  It could be very nasty to play audio
; from buffers that aren't filled with the audio we want to play.
*/

#define PI_SR_REG                    6       //; PCM in Status register
#define PO_SR_REG                    0x16     //; PCM out Status register
#define MC_SR_REG                    0x26     //; MIC in Status register
/*
;16bit read/write
; status registers.  Bitfields follow:
*/
#define FIFO_ERR                     BIT4    //; FIFO Over/Underrun W1TC.

#define BCIS                         BIT3    //; buffer completion interrupt status.
                                        //; Set whenever the last sample in ANY
                                        //; buffer is finished.  Bit is only
                                        //; set when the Interrupt on Complete
                                        //; (BIT4 of control reg) is set.

#define LVBCI                        BIT2    //; Set whenever the codec has processed
                                        //; the last buffer in the buffer list.
                                        //; Will fire an interrupt if IOC bit is
                                        //; set. Probably set after the last
                                        //; sample in the last buffer is
                                        //; processed.  W1TC

                                        //; 
#define CELV                         BIT1    //; Current buffer == last valid.
                                        //; Bit is RO and remains set until LVI is
                                        //; cleared.  Probably set up the start
                                        //; of processing for the last buffer.


#define DCH                          BIT0   // ; DMA controller halted.
                                        //; set whenever audio stream is stopped
                                        //; or something else goes wrong.


#define PI_PICB_REG                  8       //; PCM in position in current buffer(RO)
#define PO_PICB_REG                  0x18     //; PCM out position in current buffer(RO)
#define MC_PICB_REG                 0x28     //; MIC in position in current buffer (RO)
#endif