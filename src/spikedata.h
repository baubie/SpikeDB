#ifndef SPIKEDATA_H
#define SPIKEDATA_H


/*
    These defines and structs were taken from the
    original Spike2 source code.
*/

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <string>

// Porting from windows!
#define DWORD uint32_t


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

#define AUTOMATIC	0
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

typedef struct
{
        short nType;
        float fCarFreq;
        float fDeltaFreq;
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
typedef struct __attribute__((__packed__)) 
{
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
} STIM; // (108)        

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

        // int below was long originally
        // Changed to int to work on 32-bit and 64-bit machines

        // general data
        char	cId[12];		//  (12) file identification / validiation 
        int	nMagic;			//   (4) for internal use of the spike programs 
        char	cDate[8];       //   (8) date of data acquisition 
        char	cTime[8];       //   (8) time of data acquisition 
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
        bool parse(const char* filename);
        void printfile();
        std::string xVariable();
        std::string type();
        double xvalue(int sweep);
        double delta();
        int trials();
        double begin(int channel, int sweep);
        double duration(int channel, int sweep);
        double attenuation(int channel, int sweep);
        double frequency(int channel, int sweep);
        HEADER m_head;
        std::vector<SPIKESTRUCT> m_spikeArray;

    private:
        std::vector<DWORD> m_dwDataArray;
        int m_nActualPasses[MAX_SWEEPS];
        bool parsedata();
};

#endif
