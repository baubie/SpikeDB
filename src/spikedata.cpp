#include "spikedata.h"

bool SpikeData::parse(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (in.good())
    {
        // Calculate the filesize
        in.seekg(0, std::ios_base::beg);
        std::ifstream::pos_type begin_pos = in.tellg();
        in.seekg(0, std::ios_base::end);
        int fsize = static_cast<int>(in.tellg()-begin_pos);
        in.seekg(0, std::ios_base::beg);

        // Read in the header
        in.read(reinterpret_cast<char*>(&m_head), sizeof(m_head));
        int hsize = sizeof(m_head);
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
            return false;
        }
    }
    else
    {
//      std::cerr << "ERROR: Unable to open " << filename << std::endl;
        return false;
    }
    return true;
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

std::string SpikeData::type()
{
#define NOISE		0
#define SINUS		1
#define AMSINUS 	2
#define FMSINUS		3
#define SWEPTSINUS	4
#define WAVEFILE	5
#define FILT_NOISE	6

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
            return "Swept Sinus";
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
}

std::string SpikeData::xVariable()
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
	if (m_head.deltaCh2.fBegin != 0) return "Ch 2 Onset";	
	if (m_head.deltaCh2.fDur != 0) return "Ch 2 Dur";	
	if (m_head.deltaCh2.fStimInt != 0) return "Ch 2 Stim Int";	
	if (m_head.deltaCh2.fAtten != 0) return "Ch 2 Atten";	
	if (m_head.deltaCh2.fCarFreq != 0) return "Ch 2 Freq";	
	if (m_head.deltaCh2.fPhase != 0) return "Ch 2 Phase";	
	if (m_head.deltaCh2.fFreqDev != 0) return "Ch 2 Freq Dev";	
	if (m_head.deltaCh2.fAmDepth != 0) return "Ch 2 AM Depth";	
	if (m_head.deltaCh2.fModFreq != 0) return "Ch 2 Mod Freq";	
	return "Unknown";
}

double SpikeData::delta()
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
    if (m_head.deltaCh2.fBegin != 0) return m_head.deltaCh2.fBegin;	
    if (m_head.deltaCh2.fDur != 0) return m_head.deltaCh2.fDur;	
    if (m_head.deltaCh2.fStimInt != 0) return m_head.deltaCh2.fStimInt;
    if (m_head.deltaCh2.fAtten != 0) return m_head.deltaCh2.fAtten;
    if (m_head.deltaCh2.fCarFreq != 0) return m_head.deltaCh2.fCarFreq;
    if (m_head.deltaCh2.fPhase != 0) return m_head.deltaCh2.fPhase;
    if (m_head.deltaCh2.fFreqDev != 0) return m_head.deltaCh2.fFreqDev;
    if (m_head.deltaCh2.fAmDepth != 0) return m_head.deltaCh2.fAmDepth;
    if (m_head.deltaCh2.fModFreq != 0) return m_head.deltaCh2.fModFreq;
    return 1;
}

double SpikeData::xvalue(int sweep)
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
    return 0;
}

double SpikeData::begin(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fBegin+m_head.deltaCh1.fBegin*sweep;
        case 2:
            return m_head.stimFirstCh2.fBegin+m_head.deltaCh2.fBegin*sweep;
    }
    return 0;
}

double SpikeData::duration(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fDur+m_head.deltaCh1.fDur*sweep;
        case 2:
            return m_head.stimFirstCh2.fDur+m_head.deltaCh2.fDur*sweep;
    }
    return 0;
}

double SpikeData::attenuation(int channel, int sweep)
{
    switch(channel)
    {
        case 1:
            return m_head.stimFirstCh1.fAtten+m_head.deltaCh1.fAtten*sweep;
        case 2:
            return m_head.stimFirstCh2.fAtten+m_head.deltaCh2.fAtten*sweep;
    }
    return 0;
}

double SpikeData::frequency(int channel, int sweep)
{
    if (m_head.stimFirstCh1.nType == SINUS)
    {
        switch(channel)
        {
            case 1:
                return m_head.stimFirstCh1.params.sin.fFreq+m_head.deltaCh1.fCarFreq*sweep;
            case 2:
                return m_head.stimFirstCh2.params.sin.fFreq+m_head.deltaCh2.fCarFreq*sweep;
        }
    }
    if (m_head.stimFirstCh1.nType == SWEPTSINUS)
    {
        switch(channel)
        {
            case 1:
                return m_head.stimFirstCh1.params.sweptsin.fCarFreq+m_head.deltaCh1.fCarFreq*sweep;
            case 2:
                return m_head.stimFirstCh2.params.sweptsin.fCarFreq+m_head.deltaCh2.fCarFreq*sweep;
        }
    }
    return 0;
}

void SpikeData::printfile()
{
    std::cout << "HEADER INFORMATION" << std::endl;
    std::cout << "cId: " << m_head.cId << std::endl;
    std::cout << "nMagic: " << m_head.nMagic << std::endl;
    std::cout << "cDate: " << m_head.cDate << std::endl;
    std::cout << "cTime: " << m_head.cTime << std::endl;
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

    
}

