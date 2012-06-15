/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SPIKEDATA_H
#define SPIKEDATA_H

/*
    These defines and structs were taken from the
    original Spike2 source code.
*/

#include <stdint.h>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>

// Porting from windows!
#define DWORD uint32_t

// HEADER VERSION

#define HEADER_UNKNOWN 0
#define HEADER_50 1
#define HEADER_60 2
#define HEADER_61 3
#define HEADER_62 4

// Stimulus types
#define NOISE		0
#define SINUS		1
#define AMSINUS 	2
#define FMSINUS		3
#define SWEPTSINUS	4
#define WAVEFILE	5
#define FILT_NOISE	6

// Step mode
#define NO_STEP		0
#define FREQ_STEP	1
#define ATT_STEP	2
#define RASTER_STEP	3

// Stimulus
#define EXTERNAL	0
#define INTERNAL	1

//#define AUTOMATIC	0
#define TABLE		1

#define	SEQUENTIAL	0
#define RANDOMIZED	1
#define NONINTERLEAVED  2	// 8/29/00

#define LINEAR		0
#define LOGARITHMIC	1

#define SWEEP_MARK      ((DWORD) -1)
#define EOF_MARK        ((DWORD) -2)
#define MAX_SWEEPS      64

// structure to store state of checkboxes
// use char to insure known size
typedef struct
{
        char bCarFreq;
        char bModFreq;
        char bFreqDev;
        char bAmDepth;
        char bPhase;
        char bAtten;
        char bBegin;
        char bDur;
        char bStimInt;
        char bWav;
} STEP_FLAGS;

typedef struct
{
        float fFreq;
        float fPhase;
} SIN;

typedef struct
{
        float fCarFreq;
        float fModFreq;
        float fAmDepth;
        float fPhase;
} AMSIN;

typedef struct
{
        float fCarFreq;
        float fModFreq;
        float fFreqDev;
} FMSIN;

typedef struct
{
        float fCarFreq;	// 1st 3 fields are same as FMSIN for compat w/ old files
        float fModFreq;
        float fFreqDev;	// 0 = wideband
        float fAmDepth; // >0 = AM
} NOISE_EX;

// g++ doesn't pack to the nearest byte by default
// Here I turn that on for compatibility with the
// spike files which are packed to the nearest byte.
#ifdef WIN32
typedef struct 
#else
typedef struct __attribute__((__packed__)) 
#endif
{
#ifdef WIN32
#pragma pack(push,1)
#endif
        short nType;
        float fCarFreq;
        float fDeltaFreq;
#ifdef WIN32
#pragma pack(pop)
#endif
} SWEPTSIN;

typedef struct
{
        char strFileName[80];
} WAV;

typedef struct
{
        short nPass;
        short nSweep;
        float fTime;
} SPIKESTRUCT;

// g++ doesn't pack to the nearest byte by default
// Here I turn that on for compatibility with the
// spike files which are packed to the nearest byte.
#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((__packed__)) 
#endif
{
#ifdef WIN32
#pragma pack(push,1)
#endif
        short nType;		// stimulus type
        float fBegin;
        float fDur;
        float fRfTime;
        short nStimPerSweep;
        float fStimInt;
        float fAtten;
        float fFreq;	// used for compatibility with old code
	// (28) 
        
        union
        {
                SIN	 sin; // (8)
                AMSIN	 amsin; // (16)
                FMSIN	 fmsin; // (12)
                NOISE_EX noise_ex; // (16)
                SWEPTSIN sweptsin; // (10)
                WAV	 wav;	//(80)	// added for V 6.0
        } params; // (80)
#ifdef WIN32
#pragma pack(pop)
#endif
} STIM; // (108)        

#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((__packed__)) 
#endif
{
#ifdef WIN32
#pragma pack(push,1)
#endif
	short nType;		// stimulus type
	float fBegin;
	float fDur;
	float fRfTime;
	short nStimPerSweep;
	float fStimInt;
	float fAtten;
	float fFreq;	// used for compatibility with old code
	
	union
	{
		SIN		 sin;
		AMSIN	 amsin;
		FMSIN	 fmsin;
		SWEPTSIN sweptsin;
	} params;
#ifdef WIN32
#pragma pack(pop)
#endif
	
}  STIM_50; // 5.0 and earlier

typedef struct
{
        float fBegin;
        float fDur;
        float fStimInt;
        float fAtten;
        float fCarFreq;
        float fPhase;
        float fFreqDev;
        float fAmDepth;
        float fModFreq;
}  DELTA;

typedef struct
{
	// general data
	char	cId[12];		//  (12) file identification / validiation 
	int	    nMagic;			//   (4) for internal use of the spike programs 
	char	cDateTime[16];       //   (8) date of data acquisition 
	short	nOnCh1;			//   (2) stimulus channel1 on/off 
	short	nOnCh2;			//   (2) stimulus channel2 on/off 
	int	    nRepInt;		//   (4) repetition interval
	short	nPasses;		//   (2) number of repetitions of each stimulus 
	short	nSweeps;		//   (2) number of values (=stimuli) used 
	short	nStepMode;		//   (2) frequency, attenuation or both
	short	nScaling;		//   (2) scaling steps -> linear/logarithmic 
	short	nPresentation;	//   (2) presentation -> sequential/randomized 
							//  (50) -> Total
	char	cFill1[6];		//  (6)
							//  (56) -> Total

	STEP_FLAGS stepFlags[2][2];//(40) [0=chan1,1=chan2][0=X,1=Y]
							//  (96) -> Total
	// special data
	STIM_50	stimFirstCh1;	//  (44) first stimulus parameters for channel1
	DELTA	deltaCh1;		//  (36) delta step of stimulus variables for channel1
	STIM_50	stimFirstCh2;	//  (44) first stimulus parameters for channel2
	DELTA	deltaCh2;		//  (36) delta step of stimulus variables for channel2
							// (256) -> Total

	// now follows user specific data 
	// ??
	char	cFill2[768];
}  HEADER50;				// Total: (1024) bytes ?

typedef struct
{

        // int below was long originally
        // Changed to int to work on 32-bit and 64-bit machines

        // general data
        char	cId[12];		//  (12) file identification / validiation 
        int	nMagic;			//   (4) for internal use of the spike programs 
        char	cDateTime[16];       //   (8) date of data acquisition 
//        char	cTime[8];       //   (8) time of data acquisition 
        short	nOnCh1;			//   (2) stimulus channel1 on/off 
        short	nOnCh2;			//   (2) stimulus channel2 on/off 
        int	nRepInt;		//   (4) repetition interval
        short	nPasses;		//   (2) number of repetitions of each stimulus 
        short	nSweeps;		//   (2) number of values (=stimuli) used 
        short	nStepMode;		//   (2) frequency, attenuation or both
        short	nScaling;		//   (2) scaling steps -> linear/logarithmic 
        short	nPresentation;	//   (2) presentation -> sequential/randomized/seq-noninterleaved
        short	isSPL;			//	 (2) 0=atten, 1=SPL
                                                        //  (52) -> Total
        char	cFrozenNoise;	//   (1) v 6.1
        char	cAutoAdjPhase;	//   (1) v 18-nov-2002
        char	cFill1[2];		//   (2)
                                                        //  (56) -> Total

        STEP_FLAGS stepFlags[2][2];//(40) [0=chan1,1=chan2][0=X,1=Y]
                                                        //  (96) -> Total
        // special data
        STIM	stimFirstCh1;	// (108) first stimulus parameters for channel1
        DELTA	deltaCh1;	//  (36) delta step of stimulus variables for channel1
        STIM	stimFirstCh2;	// (108) first stimulus parameters for channel2
        DELTA	deltaCh2;       //  (36) delta step of stimulus variables for channel2
                                                        // (384) -> Total

        char	cFill2[768]; 
}  HEADER;				// Total: (1152) bytes


class SpikeData
{
    public:
		SpikeData();
		~SpikeData();
        bool parse(const char* filename);
        void printfile();
        std::string xVariable();
        std::string type(int channel);
        double xvalue(int sweep);
        double delta();
        int trials();
        double begin(int channel, int sweep);
        double duration(int channel, int sweep);
        double attenuation(int channel, int sweep);
        double frequency(int channel, int sweep);
        bool setHeader(void *header);

        HEADER m_head;
        HEADER50 m_head50;
        std::vector<SPIKESTRUCT> m_spikeArray;
		std::string iso8601(const char* s);
        int headerversion(void *header);
		std::vector<int> m_sweepOrder;

    private:
        std::vector<DWORD> m_dwDataArray;
        int m_nActualPasses[MAX_SWEEPS];
        bool parsedata();
        int headerversion(char *ID);
		void upgradeheader(int version);
};

#endif
