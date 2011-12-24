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

#include "stdafx.h"

#include "spikedata.h"

SpikeData::SpikeData()
{
}

SpikeData::~SpikeData()
{
}

bool SpikeData::parse(const char* filename)
{
	std::ifstream in;
	in.open(filename, std::ios::binary);

	if (in.good())
    {
        // Calculate the filesize
        in.seekg(0, std::ios_base::beg);
        std::ifstream::pos_type begin_pos = in.tellg();
        in.seekg(0, std::ios_base::end);
        int fsize = static_cast<int>(in.tellg()-begin_pos);
        in.seekg(0, std::ios_base::beg);

        // Read in the header ID
        char cID[12];
        in.read(reinterpret_cast<char*>(&cID), sizeof(cID));
        int VERSION = headerversion(cID);
        if (VERSION < HEADER_50)
        {
            std::cerr << "ERROR: Spike data file is too old (" << filename << ")" << std::endl;
            return false;
        }

        // Read in the header
        in.seekg(0, std::ios_base::beg);
        int hsize;
        if (VERSION == HEADER_50)
        {
            in.read(reinterpret_cast<char*>(&m_head50), sizeof(m_head50));
            hsize = sizeof(m_head50);
			upgradeheader(VERSION);

        }
        if (VERSION == HEADER_62)
        {
            in.read(reinterpret_cast<char*>(&m_head), sizeof(m_head));
            hsize = sizeof(m_head);
			upgradeheader(VERSION);
        }
		int nbytes = fsize-hsize;

        if (nbytes < 0) {
//            std::cerr << "ERROR: Invalid spike data file." << std::endl;
            return false;
        }
        int size = nbytes / sizeof(DWORD);
        m_dwDataArray.reserve(size);
        int i = 0;
        for (i = 0; i < size; ++i)
        {
            DWORD dw;
            in.read(reinterpret_cast<char*>(&dw), sizeof(dw));
            m_dwDataArray.push_back(dw);
        }
        if (!parsedata())
        {
//          std::cerr << "ERROR: Failed to parse data file." << std::endl;
			in.close();
            return false;
        }
    }
    else
    {
//      std::cerr << "ERROR: Unable to open " << filename << std::endl;
        return false;
    }
	in.close();
    return true;
}

void SpikeData::upgradeheader(int version)
{
	// Upgrade to a HEADER_60
	if (version == HEADER_50)
	{
		memcpy( &m_head, &m_head50, 96 );	
		m_head.stimFirstCh1.nType = m_head50.stimFirstCh1.nType;
		m_head.stimFirstCh1.fBegin = m_head50.stimFirstCh1.fBegin;
		m_head.stimFirstCh1.fDur = m_head50.stimFirstCh1.fDur;
		m_head.stimFirstCh1.fRfTime = m_head50.stimFirstCh1.fRfTime;
		m_head.stimFirstCh1.nStimPerSweep = m_head50.stimFirstCh1.nStimPerSweep;
		m_head.stimFirstCh1.fStimInt = m_head50.stimFirstCh1.fStimInt;
		m_head.stimFirstCh1.fAtten = m_head50.stimFirstCh1.fAtten;
		m_head.stimFirstCh1.fFreq = m_head50.stimFirstCh1.fFreq;
		m_head.stimFirstCh1.params.sin = m_head50.stimFirstCh1.params.sin;
		m_head.stimFirstCh1.params.amsin = m_head50.stimFirstCh1.params.amsin;
		m_head.stimFirstCh1.params.fmsin = m_head50.stimFirstCh1.params.fmsin;
		m_head.stimFirstCh1.params.sweptsin = m_head50.stimFirstCh1.params.sweptsin;
		m_head.stimFirstCh2.nType = m_head50.stimFirstCh2.nType;
		m_head.stimFirstCh2.fBegin = m_head50.stimFirstCh2.fBegin;
		m_head.stimFirstCh2.fDur = m_head50.stimFirstCh2.fDur;
		m_head.stimFirstCh2.fRfTime = m_head50.stimFirstCh2.fRfTime;
		m_head.stimFirstCh2.nStimPerSweep = m_head50.stimFirstCh2.nStimPerSweep;
		m_head.stimFirstCh2.fStimInt = m_head50.stimFirstCh2.fStimInt;
		m_head.stimFirstCh2.fAtten = m_head50.stimFirstCh2.fAtten;
		m_head.stimFirstCh2.fFreq = m_head50.stimFirstCh2.fFreq;
		m_head.stimFirstCh2.params.sin = m_head50.stimFirstCh2.params.sin;
		m_head.stimFirstCh2.params.amsin = m_head50.stimFirstCh2.params.amsin;
		m_head.stimFirstCh2.params.fmsin = m_head50.stimFirstCh2.params.fmsin;
		m_head.stimFirstCh2.params.sweptsin = m_head50.stimFirstCh2.params.sweptsin;
		m_head.deltaCh1 = m_head50.deltaCh1;
		m_head.deltaCh2 = m_head50.deltaCh2;
		m_head.isSPL = 0;
		version = HEADER_60;
	}

	if (version == HEADER_60)
	{
		if (m_head.stimFirstCh1.nType == NOISE) {
			m_head.stimFirstCh1.params.noise_ex.fModFreq = 0.0f;
			m_head.stimFirstCh1.params.noise_ex.fAmDepth = 0.0f;
		}
		if (m_head.stimFirstCh2.nType == NOISE) {
			m_head.stimFirstCh2.params.noise_ex.fModFreq = 0.0f;
			m_head.stimFirstCh2.params.noise_ex.fAmDepth = 0.0f;
		}
		m_head.stepFlags[1][0].bWav = 0;
		m_head.stepFlags[1][1].bWav = 0;
		m_head.stepFlags[0][1].bWav = 0;
		if (m_head.stimFirstCh1.nType == WAVEFILE &&
			m_head.nPasses > 1) {
			m_head.stepFlags[0][0].bWav = 1;
		} else {
			m_head.stepFlags[0][0].bWav = 0;
		}
		m_head.cFrozenNoise = 0;	// was not frozen in the past
		version = HEADER_61;
	}

	if (version == HEADER_61)
	{
		m_head.cAutoAdjPhase = 0;
		version = HEADER_62;
	}
}

int SpikeData::headerversion(char *ID)
{
    if (strcmp(ID, "SPIKE V 6.2") == 0)
        return HEADER_62;
    if (strcmp(ID, "SPIKE V 6.1") == 0)
        return HEADER_61;
    if (strcmp(ID, "SPIKE V 6.0") == 0)
        return HEADER_60;
    if (strcmp(ID, "SPIKE V 5.0") == 0)
        return HEADER_50;

    return HEADER_UNKNOWN;
}

int SpikeData::headerversion(void *header)
{
    // Crudely cast our pointer to a string
    // If it is a header than the \n character at the
    // end of the cID field will truncate the string.
    // Else strcmp will != 0 and we move on with life.
    char *ID = static_cast<char*>(header);
    return headerversion(ID);
}

bool SpikeData::setHeader(void *header)
{
    int VERSION = headerversion(header);
    if (VERSION == HEADER_62 || VERSION == HEADER_61 || VERSION == HEADER_60)
    {
        const HEADER *h = new HEADER(*static_cast<HEADER*>(header));
        m_head = *h;
		upgradeheader(VERSION);
		delete h;
        return true;
    }
    if (VERSION == HEADER_50)
    {
        const HEADER50 *h = new HEADER50(*static_cast<HEADER50*>(header));
        m_head50 = *h;
		upgradeheader(HEADER_50);
		delete h;
        return true;
    }
    return false;
}

bool SpikeData::parsedata()
{
    if (m_dwDataArray.size() == 0) return false;

    if (m_dwDataArray[0] != SWEEP_MARK && m_dwDataArray[0] != EOF_MARK) {
//        std::cerr << "ERROR: Data format error." << std::endl;
        return false;
    }

    SPIKESTRUCT spikeAct;
    int nSweepCount = 0;
    // int nPassCount = 0; // For NONINTERLEAVED
    int nSpikeCount = 0;
    int nSweep = 0;
    int nPass = 0;

    for (int i=0; i<MAX_SWEEPS; i++) {
        m_nActualPasses[i] = 0;
    }

    for (unsigned int i = 0; i < m_dwDataArray.size(); ++i)
    {
        DWORD val = m_dwDataArray[i];
        switch(val)
        {
            case SWEEP_MARK:
                if (m_head.nPresentation != NONINTERLEAVED) {
                    nPass = nSweepCount/m_head.nSweeps + 1;
                    if (nPass < 1 || nPass > m_head.nPasses) {
                        std::cerr << "ERROR: Bad pass number (" << nPass << "~"<<m_head.nPasses<<"). Corrupt data file." << std::endl;
                        nPass = 1;
                    }
                    i++;
                    nSweepCount++;
                    nSweep = m_dwDataArray[i];
                    if (nSweep < 1 || nSweep > m_head.nSweeps) {
                        std::cerr << "ERROR: Bad sweep number found. Corrupt data file." << std::endl;
                        nSweep = 1;
                    }
                    if (nPass <= m_nActualPasses[nSweep-1]) {
                        std::cerr << "ERROR: Unexpected pass number (" << nPass << "). Corrupt data file." << std::endl;
                        std::cerr << "ERROR: Expected > " << m_nActualPasses[nSweep-1] << " at " << nSweep-1 << std::endl;
                    }
                    m_nActualPasses[nSweep-1] = nPass;
                } else {
                    std::cerr << "ERROR: NONINTERLEAVED is not supported." << std::endl;
                    return false;
                }
                break;

            case EOF_MARK:
                break;

            default:    // spike!
                nSpikeCount++;
                spikeAct.nPass = nPass;
                spikeAct.nSweep = nSweep;
                spikeAct.fTime = val / 1000.0f;
                m_spikeArray.push_back(spikeAct);
                break;
        }
    }

    return true;
}

std::string SpikeData::type(int channel)
{
#define NOISE		0
#define SINUS		1
#define AMSINUS 	2
#define FMSINUS		3
#define SWEPTSINUS	4
#define WAVEFILE	5
#define FILT_NOISE	6

    switch(channel)
    {
        case 1:
            switch (m_head.stimFirstCh1.nType)
            {
                case NOISE:
                    return "Noise";
                    break;

                case SINUS:
                    return "Sinus";
                    break;

                case AMSINUS:
                    return "AM Sinus";
                    break;

                case FMSINUS:
                    return "FM Sinus";
                    break;

                case SWEPTSINUS:
                    return "FM Sweep";
                    break;

                case WAVEFILE:
                    return "Wave File";
                    break;

                case FILT_NOISE:
                    return "Filtered Noise";
                    break;

                default:
                    return "Unknown";
                    break;
            }
            break;
        case 2:
            switch (m_head.stimFirstCh2.nType)
            {
                case NOISE:
                    return "Noise";
                    break;

                case SINUS:
                    return "Sinus";
                    break;

                case AMSINUS:
                    return "AM Sinus";
                    break;

                case FMSINUS:
                    return "FM Sinus";
                    break;

                case SWEPTSINUS:
                    return "FM Sweep";
                    break;

                case WAVEFILE:
                    return "Wave File";
                    break;

                case FILT_NOISE:
                    return "Filtered Noise";
                    break;

                default:
                    return "Unknown";
                    break;
            }
            break;
    }
    return "Unknown";
}

std::string SpikeData::xVariable()
{
    if (m_head.nOnCh1 == 1)
    {
        if (m_head.deltaCh1.fBegin != 0) return "Ch 1 Onset";	
        if (m_head.deltaCh1.fDur != 0) return "Ch 1 Dur";	
        if (m_head.deltaCh1.fStimInt != 0) return "Ch 1 Stim Int";	
        if (m_head.deltaCh1.fAtten != 0) return "Ch 1 Atten";	
        if (m_head.deltaCh1.fCarFreq != 0) return "Ch 1 Freq";	
        if (m_head.deltaCh1.fPhase != 0) return "Ch 1 Phase";	
        if (m_head.deltaCh1.fFreqDev != 0) return "Ch 1 Freq Dev";	
        if (m_head.deltaCh1.fAmDepth != 0) return "Ch 1 AM Depth";	
        if (m_head.deltaCh1.fModFreq != 0) return "Ch 1 Mod Freq";	
    }
    if (m_head.nOnCh2 == 1)
    {
        if (m_head.deltaCh2.fBegin != 0) return "Ch 2 Onset";	
        if (m_head.deltaCh2.fDur != 0) return "Ch 2 Dur";	
        if (m_head.deltaCh2.fStimInt != 0) return "Ch 2 Stim Int";	
        if (m_head.deltaCh2.fAtten != 0) return "Ch 2 Atten";	
        if (m_head.deltaCh2.fCarFreq != 0) return "Ch 2 Freq";	
        if (m_head.deltaCh2.fPhase != 0) return "Ch 2 Phase";	
        if (m_head.deltaCh2.fFreqDev != 0) return "Ch 2 Freq Dev";	
        if (m_head.deltaCh2.fAmDepth != 0) return "Ch 2 AM Depth";	
        if (m_head.deltaCh2.fModFreq != 0) return "Ch 2 Mod Freq";	
    }
	return "Unknown";
}

double SpikeData::delta()
{
    if (m_head.nOnCh1 == 1)
	{
		if (m_head.deltaCh1.fBegin != 0) return m_head.deltaCh1.fBegin;	
		if (m_head.deltaCh1.fDur != 0) return m_head.deltaCh1.fDur;	
		if (m_head.deltaCh1.fStimInt != 0) return m_head.deltaCh1.fStimInt;
		if (m_head.deltaCh1.fAtten != 0) return m_head.deltaCh1.fAtten;
		if (m_head.deltaCh1.fCarFreq != 0) return m_head.deltaCh1.fCarFreq;
		if (m_head.deltaCh1.fPhase != 0) return m_head.deltaCh1.fPhase;
		if (m_head.deltaCh1.fFreqDev != 0) return m_head.deltaCh1.fFreqDev;
		if (m_head.deltaCh1.fAmDepth != 0) return m_head.deltaCh1.fAmDepth;
		if (m_head.deltaCh1.fModFreq != 0) return m_head.deltaCh1.fModFreq;
	}

    if (m_head.nOnCh2 == 1)
	{
		if (m_head.deltaCh2.fBegin != 0) return m_head.deltaCh2.fBegin;	
		if (m_head.deltaCh2.fDur != 0) return m_head.deltaCh2.fDur;	
		if (m_head.deltaCh2.fStimInt != 0) return m_head.deltaCh2.fStimInt;
		if (m_head.deltaCh2.fAtten != 0) return m_head.deltaCh2.fAtten;
		if (m_head.deltaCh2.fCarFreq != 0) return m_head.deltaCh2.fCarFreq;
		if (m_head.deltaCh2.fPhase != 0) return m_head.deltaCh2.fPhase;
		if (m_head.deltaCh2.fFreqDev != 0) return m_head.deltaCh2.fFreqDev;
		if (m_head.deltaCh2.fAmDepth != 0) return m_head.deltaCh2.fAmDepth;
		if (m_head.deltaCh2.fModFreq != 0) return m_head.deltaCh2.fModFreq;
	}
    return 1;
}

int SpikeData::trials()
{
    return m_head.nPasses;
}

double SpikeData::xvalue(int sweep)
{
    if (m_head.nOnCh1 == 1)
    {
        if (m_head.deltaCh1.fBegin != 0) {
            return m_head.stimFirstCh1.fBegin+m_head.deltaCh1.fBegin*sweep;
        }
        if (m_head.deltaCh1.fDur != 0) {
            return m_head.stimFirstCh1.fDur+m_head.deltaCh1.fDur*sweep;
        }
        if (m_head.deltaCh1.fStimInt != 0) {
            return m_head.stimFirstCh1.fStimInt+m_head.deltaCh1.fStimInt*sweep;
        }
        if (m_head.deltaCh1.fAtten != 0) {
            return m_head.stimFirstCh1.fAtten+m_head.deltaCh1.fAtten*sweep;
        }
        if (m_head.deltaCh1.fCarFreq != 0) {
            if (m_head.stimFirstCh1.nType == SINUS)
                return m_head.stimFirstCh1.params.sin.fFreq+m_head.deltaCh1.fCarFreq*sweep;
            if (m_head.stimFirstCh1.nType == SWEPTSINUS)
                return m_head.stimFirstCh1.params.sweptsin.fCarFreq+m_head.deltaCh1.fCarFreq*sweep;
        }
        /*
        if (m_head.deltaCh1.fAmDepth != 0) {
            return m_head.stimFirstCh1.fAmDepth+m_head.deltaCh1.fAmDepth*sweep;
        }
        if (m_head.deltaCh1.fModFreq != 0) {
            return m_head.stimFirstCh1.fModFreq+m_head.deltaCh1.fModFreq*sweep;
        }
        */
    }
    if (m_head.nOnCh2 == 1)
    {
        if (m_head.deltaCh2.fBegin != 0) {
            return m_head.stimFirstCh2.fBegin+m_head.deltaCh2.fBegin*sweep;
        }
        if (m_head.deltaCh2.fDur != 0) {
            return m_head.stimFirstCh2.fDur+m_head.deltaCh2.fDur*sweep;
        }
        if (m_head.deltaCh2.fStimInt != 0) {
            return m_head.stimFirstCh2.fStimInt+m_head.deltaCh2.fStimInt*sweep;
        }
        if (m_head.deltaCh2.fAtten != 0) {
            return m_head.stimFirstCh2.fAtten+m_head.deltaCh2.fAtten*sweep;
        }
        if (m_head.deltaCh2.fCarFreq != 0) {
            if (m_head.stimFirstCh2.nType == SINUS)
                return m_head.stimFirstCh2.params.sin.fFreq+m_head.deltaCh2.fCarFreq*sweep;
            if (m_head.stimFirstCh2.nType == SWEPTSINUS)
                return m_head.stimFirstCh2.params.sweptsin.fCarFreq+m_head.deltaCh2.fCarFreq*sweep;
        }
        /*
        if (m_head.deltaCh2.fAmDepth != 0) {
            return m_head.stimFirstCh2.fAmDepth+m_head.deltaCh2.fAmDepth*sweep;
        }
        if (m_head.deltaCh2.fModFreq != 0) {
            return m_head.stimFirstCh2.fModFreq+m_head.deltaCh2.fModFreq*sweep;
        }
        */
    }
    return 0;
}

double SpikeData::begin(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fBegin+m_head.deltaCh1.fBegin*sweep;
            break;
        case 2:
            return m_head.stimFirstCh2.fBegin+m_head.deltaCh2.fBegin*sweep;
            break;
    }
    return 0;
}

double SpikeData::duration(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fDur+m_head.deltaCh1.fDur*sweep;
            break;
        case 2:
            return m_head.stimFirstCh2.fDur+m_head.deltaCh2.fDur*sweep;
            break;
    }
    return 0;
}

double SpikeData::attenuation(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fAtten+m_head.deltaCh1.fAtten*sweep;
            break;
        case 2:
            return m_head.stimFirstCh2.fAtten+m_head.deltaCh2.fAtten*sweep;
            break;
    }
    return 0;
}

double SpikeData::frequency(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            if (m_head.stimFirstCh1.nType == SINUS)
                return m_head.stimFirstCh1.params.sin.fFreq+m_head.deltaCh1.fCarFreq*sweep;
            if (m_head.stimFirstCh1.nType == SWEPTSINUS)
                return m_head.stimFirstCh1.params.sweptsin.fCarFreq+m_head.deltaCh1.fCarFreq*sweep;
            if (m_head.stimFirstCh1.nType == FMSINUS)
                return m_head.stimFirstCh1.params.fmsin.fCarFreq+m_head.deltaCh1.fCarFreq*sweep;
            break;

        case 2:
            if (m_head.stimFirstCh2.nType == SINUS)
                return m_head.stimFirstCh2.params.sin.fFreq+m_head.deltaCh2.fCarFreq*sweep;
            if (m_head.stimFirstCh2.nType == SWEPTSINUS)
                return m_head.stimFirstCh2.params.sweptsin.fCarFreq+m_head.deltaCh2.fCarFreq*sweep;
            if (m_head.stimFirstCh1.nType == FMSINUS)
                return m_head.stimFirstCh2.params.fmsin.fCarFreq+m_head.deltaCh2.fCarFreq*sweep;
            break;
    }
    return 0;
}

std::string SpikeData::iso8601(const char* s)
{
	// Input format: DD.MM.YYHH:MM::SS
	char r[19];

	r[4] = '-';
	r[7] = '-';
	r[10] = ' ';
	if  (s[6] == '9') {
		r[0] = '1';
		r[1] = '9';
	} else {
		r[0] = '2';
		r[1] = '0';
	}
	// Last two year digits
	r[2] = s[6];
	r[3] = s[7];

	// Month
	r[5] = s[3];
	r[6] = s[4];

	// Day
	r[8] = s[0];
	r[9] = s[1];

	// Time
	for (int i = 0; i < 8; ++i) {
		r[i+11] = s[i+8];
	}

	return std::string(r,19);
}

void SpikeData::printfile()
{
    std::cout << "HEADER INFORMATION" << std::endl;
    std::cout << "cId: " << m_head.cId << std::endl;
    std::cout << "nMagic: " << m_head.nMagic << std::endl;
    std::cout << "cDate: " << m_head.cDateTime << std::endl;
    std::cout << "nOnCh1: " << m_head.nOnCh1 << std::endl;
    std::cout << "nOnCh2: " << m_head.nOnCh2 << std::endl;
    std::cout << "nRepInt: " << m_head.nRepInt << std::endl;
    std::cout << "nPasses: " << m_head.nPasses << std::endl;
    std::cout << "nSweeps: " << m_head.nSweeps << std::endl;
    std::cout << "nStepMode: " << m_head.nStepMode << std::endl;
    std::cout << "nScaling: " << m_head.nScaling << std::endl;
    std::cout << "nPresentation: " << m_head.nPresentation << std::endl;
    std::cout << "isSPL: " << m_head.isSPL << std::endl;
    std::cout << "cFrozenNoise: " << m_head.cFrozenNoise << std::endl;
    std::cout << "cAutoAdjPhase: " << m_head.cAutoAdjPhase << std::endl;
    std::cout << "stimFirstCh1.nType: " << m_head.stimFirstCh1.nType << std::endl;
    std::cout << "stimFirstCh1.fBegin: " << m_head.stimFirstCh1.fBegin << std::endl;
    std::cout << "stimFirstCh2.nType: " << m_head.stimFirstCh2.nType << std::endl;
    std::cout << "stimFirstCh2.fBegin: " << m_head.stimFirstCh2.fBegin << std::endl;
}

