#include "spikedata.h"

void SpikeData::parse(const char* filename)
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
            std::cerr << "ERROR: Invalid spike data file." << std::endl;
            return;
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
            std::cerr << "ERROR: Failed to parse data file." << std::endl;
        }
    }
    else
    {
        std::cerr << "ERROR: Unable to open " << filename << std::endl;
    }
}

bool SpikeData::parsedata()
{
    if (m_dwDataArray.size() == 0) return false;

    if (m_dwDataArray[0] != SWEEP_MARK && m_dwDataArray[0] != EOF_MARK) {
        std::cerr << "ERROR: Data format error." << std::endl;
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
                        std::cerr << "ERROR: Bad pass number (" << nPass << "). Corrupt data file." << std::endl;
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
    std::cout << "cFill1: " << m_head.cFill1 << std::endl;

    std::cout << "STIMULUS INFORMATION" << std::endl;

    
    std::cout << "SPIKE INFORMATION" << std::endl;
    for (int i = 0; i < MAX_SWEEPS; ++i)
    {
        if (m_nActualPasses[i] > 0) {
            std::cout << "Stimulus\t" << (i+1) << ":" << std::endl;
            for (int p = 0; p < m_head.nPasses; ++p)
            {
                std::cout << "\t\t";
                for (unsigned int s = 0; s < m_spikeArray.size(); ++s)
                {
                    if (m_spikeArray[s].nSweep == i && m_spikeArray[s].nPass == p)
                    {
                        std::cout <<  m_spikeArray[s].fTime << ",";
                    }
                }
                std::cout << std::endl;
            }
        }
    }
    
}

