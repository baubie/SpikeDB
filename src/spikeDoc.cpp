// spikeDoc.cpp : Implementierung der Klasse CSpikeDoc
//

#include "stdafx.h"
#include "spike.h"

#include "spikeDoc.h"
#include "spikeView.h"
#include <math.h>
#include <memory.h>
#include "markers.h"
#include "hw-dsp2.h"

extern BOOL g_searchMode;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc

IMPLEMENT_DYNCREATE(CSpikeDoc, CDocument)

BEGIN_MESSAGE_MAP(CSpikeDoc, CDocument)
	//{{AFX_MSG_MAP(CSpikeDoc)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc Konstruktion/Destruktion

CSpikeDoc::CSpikeDoc()
{
	m_bSpikeFile = TRUE;
}

CSpikeDoc::~CSpikeDoc()
{
}

extern BOOL g_createDummyFile;

BOOL CSpikeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (g_createDummyFile) {
		m_strFileName = "DummyFile.001";
	} else {
		if (!MakeName())			// Dokument mit diesem Titel ist bereits offen !
			return FALSE;
	}

	SetTitle(m_strFileName);
	SetHeader();				// Aufnahmeeinstellungen in Header schreiben

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc Serialisierung

void CSpikeDoc::Serialize(CArchive& ar)
{
	int		i;
	WORD	wBuf;
	DWORD	dwBuf;

	CObject::Serialize(ar);
	if (ar.IsStoring())
	{
		// store header
		ar.Write(&m_head, sizeof(m_head));

		// store data
		for(int i=0; i<m_dwDataArray.GetSize(); i++)
		{
			dwBuf = m_dwDataArray[i];
			ar.Write(&dwBuf, sizeof(dwBuf));		
		}
		TRACE("Storing Array: %d words\n", m_dwDataArray.GetSize());
	}
	else
	{
		int		retval;
		CFile*	fp = ar.GetFile();

		// check file and read header
		fp->Read(m_cHeadId, sizeof(m_cHeadId));
		fp->SeekToBegin();

		// Read the header. Old versions are converted to the next newer version,
		// until they are at the current version.  This way we don't have to
		// modify old header conversion procedures as new header types are introduced.

		if (strcmp(SPIKE30_ID, m_cHeadId) == 0) {			// old header
			ar.Read(&m_head_v30, sizeof(m_head_v30));
			ConvertOldHead();	// convert from Old to 4.0 (3.0 to 4.0) , and continue
			//AfxMessageBox("Old header !");
		}

		if ( strcmp(SPIKE40_ID, m_cHeadId) == 0 ) { // semi-old header
			ar.Read(&m_head_v40, sizeof(m_head_v40));
		}

		if ( strcmp(SPIKE30_ID, m_cHeadId) == 0 ||
		     strcmp(SPIKE40_ID, m_cHeadId) == 0     ) {

			// convert from 4.0 to 5.0 (m_head_v40 to m_head)

			memcpy( &m_head_v50, &m_head_v40, 50 );	// first 50 bytes are the same (up to cFill1[])

			// 	STEP_FLAGS stepFlags[2][2];//(40) [0=chan1,1=chan2][0=X,1=Y]

			m_head_v50.stimFirstCh1 = m_head_v40.stimFirstCh1;
			m_head_v50.stimFirstCh2 = m_head_v40.stimFirstCh2;

			// convert DELTA of chan 1
			m_head_v50.deltaCh1.fBegin = m_head_v40.deltaCh1.fBegin;
			m_head_v50.deltaCh1.fDur = m_head_v40.deltaCh1.fDur;
			m_head_v50.deltaCh1.fStimInt = m_head_v40.deltaCh1.fStimInt;
			m_head_v50.deltaCh1.fAtten = m_head_v40.deltaCh1.fAtten;

			m_head_v50.deltaCh1.fCarFreq = 0.0f;
			m_head_v50.deltaCh1.fFreqDev = 0.0f;
			m_head_v50.deltaCh1.fModFreq = 0.0f;
			if (m_head_v50.stimFirstCh1.nType == SINUS)
				m_head_v50.deltaCh1.fCarFreq = m_head_v40.deltaCh1.fFreq;
			else if (m_head_v50.stimFirstCh1.nType == SWEPTSINUS)
				m_head_v50.deltaCh1.fFreqDev = m_head_v40.deltaCh1.fFreq;
			else
				m_head_v50.deltaCh1.fModFreq = m_head_v40.deltaCh1.fFreq;

			m_head_v50.deltaCh1.fPhase = 0.0f;
			m_head_v50.deltaCh1.fAmDepth = 0.0f;

			// convert DELTA of chan 2
			m_head_v50.deltaCh2.fBegin = m_head_v40.deltaCh2.fBegin;
			m_head_v50.deltaCh2.fDur = m_head_v40.deltaCh2.fDur;
			m_head_v50.deltaCh2.fStimInt = m_head_v40.deltaCh2.fStimInt;
			m_head_v50.deltaCh2.fAtten = m_head_v40.deltaCh2.fAtten;

			m_head_v50.deltaCh2.fCarFreq = 0.0f;
			m_head_v50.deltaCh2.fFreqDev = 0.0f;
			m_head_v50.deltaCh2.fModFreq = 0.0f;
			if (m_head_v50.stimFirstCh2.nType == SINUS)
				m_head_v50.deltaCh2.fCarFreq = m_head_v40.deltaCh2.fFreq;
			else if (m_head_v50.stimFirstCh2.nType == SWEPTSINUS)
				m_head_v50.deltaCh2.fFreqDev = m_head_v40.deltaCh2.fFreq;
			else
				m_head_v50.deltaCh2.fModFreq = m_head_v40.deltaCh2.fFreq;

			m_head_v50.deltaCh2.fPhase = 0.0f;
			m_head_v50.deltaCh2.fAmDepth = 0.0f;

		}

		if ( strcmp(SPIKE50_ID, m_cHeadId) == 0 ) { // 5.0 header
			long val;
			long newPos = fp->Seek(sizeof(m_head_v50), CFile::begin);	// end of 5.0 header
			if (newPos != sizeof(m_head_v50)) {
				AfxMessageBox("Error in header");
				AfxThrowUserException();
			}
			int nbytes = fp->Read(&val, 4);	// if 5.0 - SWEEP_MARK, if 5a - part of header filler
			if (nbytes != 0 && nbytes != 4) {
				AfxMessageBox("Error in header");
				AfxThrowUserException();
			}
			fp->SeekToBegin(); // put back to where it was

			if (nbytes == 0 || val == SWEEP_MARK || val == EOF_MARK) {
				// normal 5.0 file
				ar.Read(&m_head_v50, sizeof(m_head_v50));
			} else if (val == 0xcdcdcdcd) {
				// file created by spike.exe dated between 5/26/00 and 6/23/00
				// it is the same as V6.0, except that isSPL is undefined
				ar.Read(&m_head, sizeof(m_head));
				m_head.isSPL = 0;
				strcpy(m_cHeadId, SPIKE60_ID);
				strcpy(m_head.cId, SPIKE60_ID);
				goto done_reading_60_header;
			} else {
				// 31-may-2006: some files created in June 2000 are getting
				// stuck here. Warn user, then proceed as if val == 0xcdcdcdcd
				AfxMessageBox("Warning: Looks like May/June 2000 header, but not sure.");
				//AfxThrowUserException();
				ar.Read(&m_head, sizeof(m_head));
				m_head.isSPL = 0;
				strcpy(m_cHeadId, SPIKE60_ID);
				strcpy(m_head.cId, SPIKE60_ID);
				goto done_reading_60_header;
			}
		}

		if ( strcmp(SPIKE30_ID, m_cHeadId) == 0 ||
			 strcmp(SPIKE40_ID, m_cHeadId) == 0 ||
		     strcmp(SPIKE50_ID, m_cHeadId) == 0     ) {
			
			// convert from v5.0 to 6.0
			memcpy( &m_head, &m_head_v50, 96 );	// first 96 bytes are the same

			m_head.stimFirstCh1.nType = m_head_v50.stimFirstCh1.nType;
			m_head.stimFirstCh1.fBegin = m_head_v50.stimFirstCh1.fBegin;
			m_head.stimFirstCh1.fDur = m_head_v50.stimFirstCh1.fDur;
			m_head.stimFirstCh1.fRfTime = m_head_v50.stimFirstCh1.fRfTime;
			m_head.stimFirstCh1.nStimPerSweep = m_head_v50.stimFirstCh1.nStimPerSweep;
			m_head.stimFirstCh1.fStimInt = m_head_v50.stimFirstCh1.fStimInt;
			m_head.stimFirstCh1.fAtten = m_head_v50.stimFirstCh1.fAtten;
			m_head.stimFirstCh1.fFreq = m_head_v50.stimFirstCh1.fFreq;
			m_head.stimFirstCh1.params.sin = m_head_v50.stimFirstCh1.params.sin;
			m_head.stimFirstCh1.params.amsin = m_head_v50.stimFirstCh1.params.amsin;
			m_head.stimFirstCh1.params.fmsin = m_head_v50.stimFirstCh1.params.fmsin;
			m_head.stimFirstCh1.params.sweptsin = m_head_v50.stimFirstCh1.params.sweptsin;

			m_head.stimFirstCh2.nType = m_head_v50.stimFirstCh2.nType;
			m_head.stimFirstCh2.fBegin = m_head_v50.stimFirstCh2.fBegin;
			m_head.stimFirstCh2.fDur = m_head_v50.stimFirstCh2.fDur;
			m_head.stimFirstCh2.fRfTime = m_head_v50.stimFirstCh2.fRfTime;
			m_head.stimFirstCh2.nStimPerSweep = m_head_v50.stimFirstCh2.nStimPerSweep;
			m_head.stimFirstCh2.fStimInt = m_head_v50.stimFirstCh2.fStimInt;
			m_head.stimFirstCh2.fAtten = m_head_v50.stimFirstCh2.fAtten;
			m_head.stimFirstCh2.fFreq = m_head_v50.stimFirstCh2.fFreq;
			m_head.stimFirstCh2.params.sin = m_head_v50.stimFirstCh2.params.sin;
			m_head.stimFirstCh2.params.amsin = m_head_v50.stimFirstCh2.params.amsin;
			m_head.stimFirstCh2.params.fmsin = m_head_v50.stimFirstCh2.params.fmsin;
			m_head.stimFirstCh2.params.sweptsin = m_head_v50.stimFirstCh2.params.sweptsin;

			m_head.deltaCh1 = m_head_v50.deltaCh1;
			m_head.deltaCh2 = m_head_v50.deltaCh2;

			m_head.isSPL = 0;
		}

		if (strcmp(SPIKE60_ID, m_cHeadId) == 0 ||
			strcmp(SPIKE61_ID, m_cHeadId) == 0 ||
			strcmp(SPIKE62_ID, m_cHeadId) == 0)	{ // new header
			ar.Read(&m_head, sizeof(m_head));
		}

done_reading_60_header:

		if (strcmp(SPIKE61_ID, m_cHeadId) != 0 && strcmp(SPIKE62_ID, m_cHeadId) != 0) {
			// convert to 6.1
			// AM noise
			if (m_head.stimFirstCh1.nType == NOISE) {
				m_head.stimFirstCh1.params.noise_ex.fModFreq = 0.0f;
				m_head.stimFirstCh1.params.noise_ex.fAmDepth = 0.0f;
			}
			if (m_head.stimFirstCh2.nType == NOISE) {
				m_head.stimFirstCh2.params.noise_ex.fModFreq = 0.0f;
				m_head.stimFirstCh2.params.noise_ex.fAmDepth = 0.0f;
			}
			// no need to zero the deltas because the VaryModFreq and AmDepths
			// won't be checked

			// wav files
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
		}
		if (strcmp(SPIKE62_ID, m_cHeadId) != 0) {
			// convert to 6.2
			m_head.cAutoAdjPhase = 0;
		}

		// make sure we read a valid header
		if ( strcmp(SPIKE30_ID, m_cHeadId) != 0 &&
			 strcmp(SPIKE40_ID, m_cHeadId) != 0 &&
			 strcmp(SPIKE50_ID, m_cHeadId) != 0 &&
			 strcmp(SPIKE62_ID, m_cHeadId) != 0 &&
			 strcmp(SPIKE61_ID, m_cHeadId) != 0 &&
			 strcmp(SPIKE60_ID, m_cHeadId) != 0 ) {
			CString szMsg = fp->GetFileName() +  " is not a spike file !";
			AfxMessageBox(szMsg);
			m_bSpikeFile = FALSE;
			AfxThrowUserException();
		}

		if (m_head.nOnCh1)
			CalcStimuliCh1();
		if (m_head.nOnCh2)
			CalcStimuliCh2();
		
		// read data
		if (strcmp(SPIKE30_ID, m_cHeadId) == 0)	// alt
		{
			m_dwDataArray.SetSize((fp->GetLength() - sizeof(m_head_v30)) / sizeof(WORD)); 
			//TRACE("Array: %d words\n", m_dwDataArray.GetSize());
			i = 0;
			while ((retval = ar.Read(&wBuf, sizeof(wBuf))) == sizeof(wBuf))
			{
				dwBuf = wBuf;
				m_dwDataArray.SetAt(i, dwBuf);
				i++;
			}
		}
		else									// neu
		{
			int len = fp->GetLength();
			int hsize = 1024;
			if (strcmp(SPIKE60_ID, m_cHeadId) == 0 ||
				strcmp(SPIKE61_ID, m_cHeadId) == 0 ||
				strcmp(SPIKE62_ID, m_cHeadId) == 0) {
				hsize = sizeof(m_head);
			}
			int nbytes = len - hsize;
			if (nbytes < 0) {
				AfxMessageBox("Error in file format");
				AfxThrowUserException();
			}
			int siz = nbytes / sizeof(DWORD);
			m_dwDataArray.SetSize(siz); 
			TRACE("Array: %d words\n", m_dwDataArray.GetSize());
			i = 0;
			while ((retval = ar.Read(&dwBuf, sizeof(dwBuf))) == sizeof(dwBuf))
			{
				m_dwDataArray.SetAt(i, dwBuf);
				i++;
			}
		}
		GetData();		// extract spikes, sweeps and passes
		//TRACE("readend: %d (time %d)\n", i, clock());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc Diagnose

#ifdef _DEBUG
void CSpikeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSpikeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpikeDoc Befehle

BOOL CSpikeDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	if (m_bSpikeFile == FALSE)	// no spike file !
		return FALSE;
	
	return TRUE;
}

BOOL CSpikeDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (strcmp(SPIKE30_ID, m_head.cId) == 0 ||
		strcmp(SPIKE40_ID, m_head.cId) == 0 ||
	    strcmp(SPIKE50_ID, m_head.cId) == 0     )	// alt -> nicht speichern !
	{
		AfxMessageBox("Don't store old file in new format!");
		return FALSE;
	}
	
	return CDocument::OnSaveDocument(lpszPathName);
}


BOOL CSpikeDoc::GetData()
{
	SPIKESTRUCT spikeAct;
	DWORD		val;
	int			nSweepCount = 0;
	int			nPassCount = 0;
	int			nSpikeCount = 0;
	int			nSweep, nPass;		// Numerierung beginnt bei 1 !

	for (int i=0; i<MAX_SWEEPS; i++) {
		m_nActualPasses[i] = 0;
	}

	int	nSize = m_dwDataArray.GetSize();
	if (nSize == 0)					// no data available
		return FALSE;

	if (m_dwDataArray[0] != SWEEP_MARK && m_dwDataArray[0] != EOF_MARK) {
		AfxMessageBox("Error in data format");
		return FALSE;
	}

	for (i=0; i<nSize; i++)
	{
		val = m_dwDataArray[i];
		switch(val)
		{
//			case TRIGGER:
//			case R1_BEGIN:
//			case R1_END:
//			case R2_BEGIN:
//			case R2_END:
//			case V_BEGIN:
//			case V_END:
//				i++;
//				break;

	//		case SWEEP_NR:
			case SWEEP_MARK:
				// Passnummer berechnen
				if (m_head.nPresentation != NONINTERLEAVED) {
					if (strcmp(SPIKE30_ID, m_cHeadId) == 0)		// alt
						nPass = nSweepCount/m_head_v30.numxsteps + 1;
					else										// neu
						nPass = nSweepCount/m_head.nSweeps + 1;
					if (nPass < 1 || nPass > m_head.nPasses) {
						AfxMessageBox("Bad pass number found. Corrupted data file?");
						nPass = 1;	// force to legal value
					}
					i++;
					nSweepCount++;
					nSweep = m_dwDataArray[i];
					if (nSweep < 1 || nSweep > m_head.nSweeps) {
						AfxMessageBox("Bad sweep number found. Corrupted data file?");
						nSweep = 1;	// force to legal value
					}
					if (nPass <= m_nActualPasses[nSweep-1]) {
						AfxMessageBox("unexpected pass number. Corrupted data file?");
					}
					m_nActualPasses[nSweep-1] = nPass;
				} else {
					nSweep = nPassCount/m_head.nPasses + 1;
					if (nSweep < 1 || nSweep > m_head.nSweeps) {
						AfxMessageBox("Bad sweep number found. Corrupted data file?");
						nSweep = 1;	// force to legal value
					}
					i++;
					nPassCount++;
					nPass = m_dwDataArray[i];
					if (nPass < 1 || nPass > m_head.nPasses) {
						AfxMessageBox("Bad pass number found. Corrupted data file?");
						nPass = 1;	// force to legal value
					}
					if (nPass <= m_nActualPasses[nSweep-1]) {
						AfxMessageBox("unexpected pass number. Corrupted data file?");
					}
					m_nActualPasses[nSweep-1] = nPass;
				}
				//TRACE("\n SWEEP %3d (%4.0f)  pass %d", nSweep, m_head.x[nSweep-1], nPass);
				break;

//			case E_O_F:
			case EOF_MARK:
				TRACE("GetData() EOF -> %d sweeps  %d spikes\n", nSweepCount, nSpikeCount);
				break;

			default:		// spike !
				nSpikeCount++;
				spikeAct.nPass = nPass;
				spikeAct.nSweep = nSweep;

				// Speichere Zeitwert in ms !
				if (strcmp(SPIKE30_ID, m_cHeadId) == 0)		// alt
					spikeAct.fTime = val * m_head_v30.time_res;	
				else										// neu
					spikeAct.fTime = val / 1000.0f;	
				m_spikeArray.Add(spikeAct);
				break; 
		}
	} 
	//TRACE("\tGetData() Spike array size %4d \n", m_spikeArray.GetSize());
	return TRUE;
}


BOOL CSpikeDoc::CalcHistogram(int nBegin, int nEnd)
{
	UINT	nSpikeCount = 0;
	int		nSweep;
	int		nSize = m_spikeArray.GetSize();

	if (nSize == 0)
		return FALSE;

	if ((m_head.nSweeps > 0) && (m_head.nSweeps <= MAX_SWEEPS))
		m_wHistoArray.SetSize(m_head.nSweeps+1);
	else 
		return FALSE;
	//TRACE("histosize: %d  (%d)\n", m_wHistoArray.GetSize(), m_head.nSweeps);
	
	// Feldelemente auf 0 setzen
	for (int i=0; i<m_wHistoArray.GetSize(); i++){
		m_wHistoArray[i] = 0;
	}

	for (i=0; i<nSize; i++)
	{
		// Rechnung in clock Mics !
		int nSpikeMics = m_spikeArray[i].fTime * 1000.0;		
		int nBeginMics	= nBegin * 1000.0;
		int nEndMics = nEnd * 1000.0;
		if ((nSpikeMics < nBeginMics) || (nSpikeMics >= nEndMics))
			continue;
		nSweep = m_spikeArray[i].nSweep;
		m_wHistoArray[nSweep]++;	// Sweep und Pass werden ab 1 aufwärts gezählt !
		nSpikeCount++;
	}

	// erstes Element enthält das Maximum von Spikes in einem Sweep
	for (i=1; i<m_wHistoArray.GetSize(); i++)
	{
		if (m_wHistoArray[0] < m_wHistoArray[i])
			m_wHistoArray[0] = m_wHistoArray[i];	// Maximum !
	}

	//TRACE("begin %2d  end %2d\n", nBegin, nEnd);
	//for (i=0; i<m_head.nSweeps; i++)
	//	TRACE("sweep %2d  hist:%2d\n", i, m_wHistoArray[i]);
	return TRUE;
}


void CSpikeDoc::SetHeader()
{
	// Ermittle einen Zeiger auf die Stimulus-Seite
	CSpikeApp* pApp = (CSpikeApp*)AfxGetApp();
	CStimulusPage* pPgStim = (CStimulusPage*)(&pApp->m_pParams->pgStim);
	
	CTime theTime;
	theTime = CTime::GetCurrentTime();
	CString strTime = theTime.Format("%H:%M:%S");
	CString strDate = theTime.Format("%d.%m.%y");
	strncpy(m_head.cTime, strTime, sizeof(m_head.cTime));
	strncpy(m_head.cDate, strDate, sizeof(m_head.cDate));
	strncpy(m_head.cId, SPIKE62_ID, sizeof(m_head.cId));
	m_head.nMagic = pApp->m_pParams->nMagic;
	m_head.nOnCh1 = pPgStim->m_bOnCh1;
	m_head.nOnCh2 = pPgStim->m_bOnCh2;
	m_head.isSPL = pPgStim->m_SPL;
	m_head.nRepInt = pPgStim->m_nRepInt;
	m_head.nPasses = pPgStim->m_nPasses;
	m_head.nScaling = pPgStim->m_nScaling;
	m_head.nPresentation = pPgStim->m_nPresentation;
	m_head.cFrozenNoise = pPgStim->m_bFrozenNoise;
	m_head.cAutoAdjPhase = pPgStim->m_bAutoAdjPhase;
	
	// Step-Modus bei der Reizdarbietung
	if (!pPgStim->m_bVaryX && !pPgStim->m_bVaryY)	// feste Frequenz bei fester Intensität
		m_head.nStepMode = NO_STEP;
	if (pPgStim->m_bVaryX && !pPgStim->m_bVaryY)	// ändere Frequenz bei fester Intensität
		m_head.nStepMode = FREQ_STEP;
	if (pPgStim->m_bVaryY && !pPgStim->m_bVaryX)	// ändere Intensität bei fester Frequenz
		m_head.nStepMode = ATT_STEP;
	if (pPgStim->m_bVaryX && pPgStim->m_bVaryY)		// ändere Intensität und Frequenz
		m_head.nStepMode = RASTER_STEP;

	if (g_searchMode) m_head.nStepMode = NO_STEP;

	if (m_head.nStepMode == ATT_STEP)
		m_head.nSweeps = pPgStim->m_nYsteps;
	else
		m_head.nSweeps = pPgStim->m_nXsteps;

	if (m_head.nStepMode == NO_STEP)
		m_head.nSweeps = 1;

	// init stepFlags
	//STEP_FLAGS stepFlags[2][2];//(40) [0=chan1,1=chan2][0=X,1=Y]

	m_head.stepFlags[0][0].bCarFreq = pPgStim->m_bXCarFreqCh1;
	m_head.stepFlags[0][0].bModFreq = pPgStim->m_bXModFreqCh1;
	m_head.stepFlags[0][0].bFreqDev = pPgStim->m_bXFreqDevCh1;
	m_head.stepFlags[0][0].bAmDepth = pPgStim->m_bXAmDepthCh1;
	m_head.stepFlags[0][0].bPhase = pPgStim->m_bXPhaseCh1;
	m_head.stepFlags[0][0].bAtten = pPgStim->m_bXAttenCh1;
	m_head.stepFlags[0][0].bBegin = pPgStim->m_bXBeginCh1;
	m_head.stepFlags[0][0].bDur = pPgStim->m_bXDurCh1;
	m_head.stepFlags[0][0].bStimInt = pPgStim->m_bXStimIntCh1;
	m_head.stepFlags[0][0].bWav = pPgStim->m_bXWavCh1;

	m_head.stepFlags[0][1].bCarFreq = pPgStim->m_bYCarFreqCh1;
	m_head.stepFlags[0][1].bModFreq = pPgStim->m_bYModFreqCh1;
	m_head.stepFlags[0][1].bFreqDev = pPgStim->m_bYFreqDevCh1;
	m_head.stepFlags[0][1].bAmDepth = pPgStim->m_bYAmDepthCh1;
	m_head.stepFlags[0][1].bPhase = pPgStim->m_bYPhaseCh1;
	m_head.stepFlags[0][1].bAtten = pPgStim->m_bYAttenCh1;
	m_head.stepFlags[0][1].bBegin = pPgStim->m_bYBeginCh1;
	m_head.stepFlags[0][1].bDur = pPgStim->m_bYDurCh1;
	m_head.stepFlags[0][1].bStimInt = pPgStim->m_bYStimIntCh1;
	m_head.stepFlags[0][1].bWav = pPgStim->m_bYWavCh1;

	m_head.stepFlags[1][0].bCarFreq = pPgStim->m_bXCarFreqCh2;
	m_head.stepFlags[1][0].bModFreq = pPgStim->m_bXModFreqCh2;
	m_head.stepFlags[1][0].bFreqDev = pPgStim->m_bXFreqDevCh2;
	m_head.stepFlags[1][0].bAmDepth = pPgStim->m_bXAmDepthCh2;
	m_head.stepFlags[1][0].bPhase = pPgStim->m_bXPhaseCh2;
	m_head.stepFlags[1][0].bAtten = pPgStim->m_bXAttenCh2;
	m_head.stepFlags[1][0].bBegin = pPgStim->m_bXBeginCh2;
	m_head.stepFlags[1][0].bDur = pPgStim->m_bXDurCh2;
	m_head.stepFlags[1][0].bStimInt = pPgStim->m_bXStimIntCh2;
	m_head.stepFlags[1][0].bWav = 0; //pPgStim->m_bXWavCh2;

	m_head.stepFlags[1][1].bCarFreq = pPgStim->m_bYCarFreqCh2;
	m_head.stepFlags[1][1].bModFreq = pPgStim->m_bYModFreqCh2;
	m_head.stepFlags[1][1].bFreqDev = pPgStim->m_bYFreqDevCh2;
	m_head.stepFlags[1][1].bAmDepth = pPgStim->m_bYAmDepthCh2;
	m_head.stepFlags[1][1].bPhase = pPgStim->m_bYPhaseCh2;
	m_head.stepFlags[1][1].bAtten = pPgStim->m_bYAttenCh2;
	m_head.stepFlags[1][1].bBegin = pPgStim->m_bYBeginCh2;
	m_head.stepFlags[1][1].bDur = pPgStim->m_bYDurCh2;
	m_head.stepFlags[1][1].bStimInt = pPgStim->m_bYStimIntCh2;
	m_head.stepFlags[1][1].bWav = 0; //pPgStim->m_bYWavCh2;

	// Berechne Stimuli
	pPgStim->CalcYarray();
	if (pPgStim->m_bOnCh1)
	{
		SetFirstStimCh1(pPgStim);
		SetDeltaStimCh1(pPgStim);
		CalcStimuliCh1();
	}
	if (pPgStim->m_bOnCh2)
	{
		SetFirstStimCh2(pPgStim);
		SetDeltaStimCh2(pPgStim);
		CalcStimuliCh2();
	}

}

void CSpikeDoc::SetFirstStimCh1(CStimulusPage* pPgStim)
{

	m_head.stimFirstCh1.fRfTime = pPgStim->m_fRfTimeCh1;
	m_head.stimFirstCh1.nStimPerSweep = pPgStim->m_nStimPerSweepCh1;

	m_head.stimFirstCh1.fBegin = (!g_searchMode && pPgStim->m_bYBeginCh1 ? pPgStim->m_fYactCh1 :
									pPgStim->m_fFirstBeginCh1);

	m_head.stimFirstCh1.fDur = (!g_searchMode && pPgStim->m_bYDurCh1 ? pPgStim->m_fYactCh1 :
									pPgStim->m_fFirstDurCh1);

	m_head.stimFirstCh1.fStimInt = (!g_searchMode && pPgStim->m_bYStimIntCh1 ? pPgStim->m_fYactCh1 :
									pPgStim->m_fFirstStimIntCh1);

	// Intensität
	if (!g_searchMode && pPgStim->m_bYAttenCh1)
		m_head.stimFirstCh1.fAtten = pPgStim->m_fYactCh1;
	else
		m_head.stimFirstCh1.fAtten = pPgStim->m_fFirstAttenCh1;


	switch (pPgStim->m_nStimTypeCh1)
	{
	case WAVEFILE:
		m_head.stimFirstCh1.nType = WAVEFILE;
		strcpy(m_head.stimFirstCh1.params.wav.strFileName, pPgStim->m_strWavFilesCh1);
		break;
	case SINUS:
		m_head.stimFirstCh1.nType = SINUS;
		m_head.stimFirstCh1.params.sin.fPhase = (!g_searchMode && pPgStim->m_bYPhaseCh1 ? pPgStim->m_fYactCh1 :
												pPgStim->m_fFirstPhaseCh1);

		m_head.stimFirstCh1.params.sin.fFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh1 ? pPgStim->m_fYactCh1 :
												pPgStim->m_fFirstCarFreqCh1);
		break;
	case AMSINUS:
		m_head.stimFirstCh1.nType = AMSINUS;
		m_head.stimFirstCh1.params.amsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstCarFreqCh1);

		m_head.stimFirstCh1.params.amsin.fAmDepth = (!g_searchMode && pPgStim->m_bYAmDepthCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstAmDepthCh1) / 100.0f;		// in Prozent !

		m_head.stimFirstCh1.params.amsin.fPhase = (!g_searchMode && pPgStim->m_bYPhaseCh1 ? pPgStim->m_fYactCh1 :
												pPgStim->m_fFirstPhaseCh1);

		m_head.stimFirstCh1.params.amsin.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstModFreqCh1);
		break;
	case FMSINUS:
		m_head.stimFirstCh1.nType = FMSINUS;
		m_head.stimFirstCh1.params.fmsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstCarFreqCh1);

		m_head.stimFirstCh1.params.fmsin.fFreqDev = (!g_searchMode && pPgStim->m_bYFreqDevCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstFreqDevCh1);

		m_head.stimFirstCh1.params.fmsin.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstModFreqCh1);
		break;
	case SWEPTSINUS:
		m_head.stimFirstCh1.nType = SWEPTSINUS;
		m_head.stimFirstCh1.params.sweptsin.nType = LINEAR;

		m_head.stimFirstCh1.params.sweptsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstCarFreqCh1);

		m_head.stimFirstCh1.params.sweptsin.fDeltaFreq = (!g_searchMode && pPgStim->m_bYFreqDevCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstFreqDevCh1);
		break;
	case FILT_NOISE:
	case NOISE:
		m_head.stimFirstCh1.nType = pPgStim->m_nStimTypeCh1;
		m_head.stimFirstCh1.params.noise_ex.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstCarFreqCh1);
		m_head.stimFirstCh1.params.noise_ex.fFreqDev = (!g_searchMode && pPgStim->m_bYFreqDevCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstFreqDevCh1);
		m_head.stimFirstCh1.params.noise_ex.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstModFreqCh1);
		m_head.stimFirstCh1.params.noise_ex.fAmDepth = (!g_searchMode && pPgStim->m_bYAmDepthCh1 ? pPgStim->m_fYactCh1 :
													pPgStim->m_fFirstAmDepthCh1) / 100.0f;
		break;
	default:
		break;
	}
}


void CSpikeDoc::SetFirstStimCh2(CStimulusPage* pPgStim)
{

	m_head.stimFirstCh2.fRfTime = pPgStim->m_fRfTimeCh2;
	m_head.stimFirstCh2.nStimPerSweep = pPgStim->m_nStimPerSweepCh2;

	m_head.stimFirstCh2.fBegin = (!g_searchMode && pPgStim->m_bYBeginCh2 ? pPgStim->m_fYactCh2 :
									pPgStim->m_fFirstBeginCh2);

	m_head.stimFirstCh2.fDur = (!g_searchMode && pPgStim->m_bYDurCh2 ? pPgStim->m_fYactCh2 :
									pPgStim->m_fFirstDurCh2);

	m_head.stimFirstCh2.fStimInt = (!g_searchMode && pPgStim->m_bYStimIntCh2 ? pPgStim->m_fYactCh2 :
									pPgStim->m_fFirstStimIntCh2);

	// Intensität
	if (!g_searchMode && pPgStim->m_bYAttenCh2)
		m_head.stimFirstCh2.fAtten = pPgStim->m_fYactCh2;
	else
		m_head.stimFirstCh2.fAtten = pPgStim->m_fFirstAttenCh2;


	switch (pPgStim->m_nStimTypeCh2)
	{
	case SINUS:
		m_head.stimFirstCh2.nType = SINUS;
		m_head.stimFirstCh2.params.sin.fPhase = (!g_searchMode && pPgStim->m_bYPhaseCh2 ? pPgStim->m_fYactCh2 :
												pPgStim->m_fFirstPhaseCh2);

		m_head.stimFirstCh2.params.sin.fFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh2 ? pPgStim->m_fYactCh2 :
												pPgStim->m_fFirstCarFreqCh2);
		break;
	case AMSINUS:
		m_head.stimFirstCh2.nType = AMSINUS;
		m_head.stimFirstCh2.params.amsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstCarFreqCh2);

		m_head.stimFirstCh2.params.amsin.fAmDepth = (!g_searchMode && pPgStim->m_bYAmDepthCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstAmDepthCh2) / 100.0f;		// in Prozent !

		m_head.stimFirstCh2.params.amsin.fPhase = (!g_searchMode && pPgStim->m_bYPhaseCh2 ? pPgStim->m_fYactCh2 :
												pPgStim->m_fFirstPhaseCh2);

		m_head.stimFirstCh2.params.amsin.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstModFreqCh2);
		break;
	case FMSINUS:
		m_head.stimFirstCh2.nType = FMSINUS;
		m_head.stimFirstCh2.params.fmsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstCarFreqCh2);

		m_head.stimFirstCh2.params.fmsin.fFreqDev = (!g_searchMode && pPgStim->m_bYFreqDevCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstFreqDevCh2);

		m_head.stimFirstCh2.params.fmsin.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstModFreqCh2);
		break;
	case SWEPTSINUS:
		m_head.stimFirstCh2.nType = SWEPTSINUS;
		m_head.stimFirstCh2.params.sweptsin.nType = LINEAR;

		m_head.stimFirstCh2.params.sweptsin.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstCarFreqCh2);

		m_head.stimFirstCh2.params.sweptsin.fDeltaFreq = (!g_searchMode && pPgStim->m_bYFreqDevCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstFreqDevCh2);
		break;
	case FILT_NOISE:
	case NOISE:
		m_head.stimFirstCh2.nType = pPgStim->m_nStimTypeCh2;
		m_head.stimFirstCh2.params.noise_ex.fCarFreq = (!g_searchMode && pPgStim->m_bYCarFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstCarFreqCh2);

		m_head.stimFirstCh2.params.noise_ex.fFreqDev = (!g_searchMode && pPgStim->m_bYFreqDevCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstFreqDevCh2);
		m_head.stimFirstCh2.params.noise_ex.fModFreq = (!g_searchMode && pPgStim->m_bYModFreqCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstModFreqCh2);
		m_head.stimFirstCh2.params.noise_ex.fAmDepth = (!g_searchMode && pPgStim->m_bYAmDepthCh2 ? pPgStim->m_fYactCh2 :
													pPgStim->m_fFirstAmDepthCh2) / 100.0f;
		break;
	default:
		break;
	}
}


void CSpikeDoc::SetDeltaStimCh1(CStimulusPage* pPgStim)
{
	int nSteps = m_head.nSweeps;
	if (nSteps < 2)			// mindestens 2 Stufen sind notwendig !
		return;

	// Timing
	m_head.deltaCh1.fBegin = (!pPgStim->m_bXBeginCh1 ? 0.0f :
			(pPgStim->m_fLastBeginCh1 - pPgStim->m_fFirstBeginCh1) / (nSteps - 1) );

	m_head.deltaCh1.fDur = (!pPgStim->m_bXDurCh1 ? 0.0f :
			(pPgStim->m_fLastDurCh1 - pPgStim->m_fFirstDurCh1) / (nSteps - 1) );

	m_head.deltaCh1.fStimInt = (!pPgStim->m_bXStimIntCh1 ? 0.0f :
			(pPgStim->m_fLastStimIntCh1 - pPgStim->m_fFirstStimIntCh1) / (nSteps - 1) );
		
	// Intensität
	if (pPgStim->m_bXAttenCh1)
		m_head.deltaCh1.fAtten = (pPgStim->m_fLastAttenCh1 - pPgStim->m_fFirstAttenCh1) / (nSteps - 1);
	else
		m_head.deltaCh1.fAtten = 0.0f;
	//TRACE("delta --->  atten %.1f\n", m_head.deltaCh1.fAtten);


	m_head.deltaCh1.fPhase = (!pPgStim->m_bXPhaseCh1 ? 0.0f :
			(pPgStim->m_fLastPhaseCh1 - pPgStim->m_fFirstPhaseCh1) / (nSteps - 1) );

	m_head.deltaCh1.fAmDepth = (!pPgStim->m_bXAmDepthCh1 ? 0.0f :
			0.01f * (pPgStim->m_fLastAmDepthCh1 - pPgStim->m_fFirstAmDepthCh1) / (nSteps - 1) );

	// Frequenz
	if (!pPgStim->m_bXCarFreqCh1) {
		m_head.deltaCh1.fCarFreq = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh1.fCarFreq = log(pPgStim->m_fLastCarFreqCh1 / pPgStim->m_fFirstCarFreqCh1) / (nSteps - 1);
		} else {
			m_head.deltaCh1.fCarFreq = (pPgStim->m_fLastCarFreqCh1 - pPgStim->m_fFirstCarFreqCh1) / (nSteps - 1);
		}
	}

	if (!pPgStim->m_bXFreqDevCh1) {
		m_head.deltaCh1.fFreqDev = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh1.fFreqDev = log(pPgStim->m_fLastFreqDevCh1 / pPgStim->m_fFirstFreqDevCh1) / (nSteps - 1);
		} else {
			m_head.deltaCh1.fFreqDev = (pPgStim->m_fLastFreqDevCh1 - pPgStim->m_fFirstFreqDevCh1) / (nSteps - 1);
		}
	}

	if (!pPgStim->m_bXModFreqCh1) {
		m_head.deltaCh1.fModFreq = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh1.fModFreq = log(pPgStim->m_fLastModFreqCh1 / pPgStim->m_fFirstModFreqCh1) / (nSteps - 1);
		} else {
			m_head.deltaCh1.fModFreq = (pPgStim->m_fLastModFreqCh1 - pPgStim->m_fFirstModFreqCh1) / (nSteps - 1);
		}
	}

}


void CSpikeDoc::SetDeltaStimCh2(CStimulusPage* pPgStim)
{
	int nSteps = m_head.nSweeps;
	if (nSteps < 2)			// mindestens 2 Stufen sind notwendig !
		return;

	// Timing
	m_head.deltaCh2.fBegin = (!pPgStim->m_bXBeginCh2 ? 0.0f :
			(pPgStim->m_fLastBeginCh2 - pPgStim->m_fFirstBeginCh2) / (nSteps - 1) );

	m_head.deltaCh2.fDur = (!pPgStim->m_bXDurCh2 ? 0.0f :
			(pPgStim->m_fLastDurCh2 - pPgStim->m_fFirstDurCh2) / (nSteps - 1) );

	m_head.deltaCh2.fStimInt = (!pPgStim->m_bXStimIntCh2 ? 0.0f :
			(pPgStim->m_fLastStimIntCh2 - pPgStim->m_fFirstStimIntCh2) / (nSteps - 1) );
		
	// Intensität
	if (pPgStim->m_bXAttenCh2)
		m_head.deltaCh2.fAtten = (pPgStim->m_fLastAttenCh2 - pPgStim->m_fFirstAttenCh2) / (nSteps - 1);
	else
		m_head.deltaCh2.fAtten = 0.0f;
	//TRACE("delta --->  atten %.1f\n", m_head.deltaCh2.fAtten);


	m_head.deltaCh2.fPhase = (!pPgStim->m_bXPhaseCh2 ? 0.0f :
			(pPgStim->m_fLastPhaseCh2 - pPgStim->m_fFirstPhaseCh2) / (nSteps - 1) );

	m_head.deltaCh2.fAmDepth = (!pPgStim->m_bXAmDepthCh2 ? 0.0f :
			0.01f * (pPgStim->m_fLastAmDepthCh2 - pPgStim->m_fFirstAmDepthCh2) / (nSteps - 1) );

	// Frequenz
	if (!pPgStim->m_bXCarFreqCh2) {
		m_head.deltaCh2.fCarFreq = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh2.fCarFreq = log(pPgStim->m_fLastCarFreqCh2 / pPgStim->m_fFirstCarFreqCh2) / (nSteps - 1);
		} else {
			m_head.deltaCh2.fCarFreq = (pPgStim->m_fLastCarFreqCh2 - pPgStim->m_fFirstCarFreqCh2) / (nSteps - 1);
		}
	}

	if (!pPgStim->m_bXFreqDevCh2) {
		m_head.deltaCh2.fFreqDev = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh2.fFreqDev = log(pPgStim->m_fLastFreqDevCh2 / pPgStim->m_fFirstFreqDevCh2) / (nSteps - 1);
		} else {
			m_head.deltaCh2.fFreqDev = (pPgStim->m_fLastFreqDevCh2 - pPgStim->m_fFirstFreqDevCh2) / (nSteps - 1);
		}
	}

	if (!pPgStim->m_bXModFreqCh2) {
		m_head.deltaCh2.fModFreq = 0.0f;
	} else {
		if (pPgStim->m_nScaling == LOGARITHMIC) {
			m_head.deltaCh2.fModFreq = log(pPgStim->m_fLastModFreqCh2 / pPgStim->m_fFirstModFreqCh2) / (nSteps - 1);
		} else {
			m_head.deltaCh2.fModFreq = (pPgStim->m_fLastModFreqCh2 - pPgStim->m_fFirstModFreqCh2) / (nSteps - 1);
		}
	}

}
static void copySubStr(char *dstStr, char *srcStr, int index)
{
	char *dstStr_save=dstStr;

	for (int i=0; i<index; i++) {
		srcStr = strchr(srcStr, ',');
		if (srcStr) srcStr++;
		else return;
	}
	while (*srcStr && *srcStr != ',') {
		if (*srcStr == ' ') {
			srcStr++;
			continue;
		}
		*dstStr++ = *srcStr++;
	}
	*dstStr = 0;
	if (!strstr(dstStr_save, ".wav")) {
		// .wav not found, so add it
		strcat(dstStr_save, ".wav");
	}

}

void CSpikeDoc::CalcStimuliCh1()
{
	m_stimArrayCh1.SetSize(m_head.nSweeps);		// Arraygröße festlegen
	int nSteps = m_head.nSweeps;

	for (int i=0; i<nSteps; i++)
	{
		m_stimArrayCh1[i] = m_head.stimFirstCh1;	// Parameter für ersten Stimulus
		
		// Timing
		m_stimArrayCh1[i].fBegin += m_head.deltaCh1.fBegin * i;
		m_stimArrayCh1[i].fStimInt += m_head.deltaCh1.fStimInt * i;
		if (m_head.stimFirstCh1.nType != WAVEFILE) {
			m_stimArrayCh1[i].fDur += m_head.deltaCh1.fDur * i;
		}

		// Intensität
		m_stimArrayCh1[i].fAtten += m_head.deltaCh1.fAtten * i;

		switch (m_head.stimFirstCh1.nType)
		{
		case WAVEFILE:
			if (m_head.stepFlags[0][0].bWav) {
				copySubStr(m_stimArrayCh1[i].params.wav.strFileName, m_head.stimFirstCh1.params.wav.strFileName, i);
			} else {
				copySubStr(m_stimArrayCh1[i].params.wav.strFileName, m_head.stimFirstCh1.params.wav.strFileName, 0);
			}
			m_stimArrayCh1[i].fDur = (float)(GetWaveNStimSamps(m_stimArrayCh1[i].params.wav.strFileName) / (double)PTS_PER_MS);
			break;
		case SINUS:
			m_stimArrayCh1[i].params.sin.fPhase += m_head.deltaCh1.fPhase * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.sin.fFreq *= exp(m_head.deltaCh1.fCarFreq * i);
			} else {
				m_stimArrayCh1[i].params.sin.fFreq += m_head.deltaCh1.fCarFreq * i;
			}
			m_stimArrayCh1[i].fFreq = m_stimArrayCh1[i].params.sin.fFreq;	// for compatibility
			break;
		case AMSINUS:
			m_stimArrayCh1[i].params.amsin.fPhase += m_head.deltaCh1.fPhase * i;
			m_stimArrayCh1[i].params.amsin.fAmDepth += m_head.deltaCh1.fAmDepth * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.amsin.fCarFreq *= exp(m_head.deltaCh1.fCarFreq * i);
			} else {
				m_stimArrayCh1[i].params.amsin.fCarFreq += m_head.deltaCh1.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.amsin.fModFreq *= exp(m_head.deltaCh1.fModFreq * i);
			} else {
				m_stimArrayCh1[i].params.amsin.fModFreq += m_head.deltaCh1.fModFreq * i;
			}
			m_stimArrayCh1[i].fFreq = m_stimArrayCh1[i].params.amsin.fModFreq;	// for compatibility
			break;
		case FILT_NOISE:
		case NOISE:
			m_stimArrayCh1[i].params.noise_ex.fAmDepth += m_head.deltaCh1.fAmDepth * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.noise_ex.fModFreq *= exp(m_head.deltaCh1.fModFreq * i);
			} else {
				m_stimArrayCh1[i].params.noise_ex.fModFreq += m_head.deltaCh1.fModFreq * i;
			}
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.noise_ex.fCarFreq *= exp(m_head.deltaCh1.fCarFreq * i);
			} else {
				m_stimArrayCh1[i].params.noise_ex.fCarFreq += m_head.deltaCh1.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.noise_ex.fFreqDev *= exp(m_head.deltaCh1.fFreqDev * i);
			} else {
				m_stimArrayCh1[i].params.noise_ex.fFreqDev += m_head.deltaCh1.fFreqDev * i;
			}

			break;
		case FMSINUS:
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.fmsin.fCarFreq *= exp(m_head.deltaCh1.fCarFreq * i);
			} else {
				m_stimArrayCh1[i].params.fmsin.fCarFreq += m_head.deltaCh1.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.fmsin.fFreqDev *= exp(m_head.deltaCh1.fFreqDev * i);
			} else {
				m_stimArrayCh1[i].params.fmsin.fFreqDev += m_head.deltaCh1.fFreqDev * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.fmsin.fModFreq *= exp(m_head.deltaCh1.fModFreq * i);
			} else {
				m_stimArrayCh1[i].params.fmsin.fModFreq += m_head.deltaCh1.fModFreq * i;
			}
			m_stimArrayCh1[i].fFreq = m_stimArrayCh1[i].params.fmsin.fModFreq;	// for compatibility
			break;
		case SWEPTSINUS:
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.sweptsin.fCarFreq *= exp(m_head.deltaCh1.fCarFreq * i);
			} else {
				m_stimArrayCh1[i].params.sweptsin.fCarFreq += m_head.deltaCh1.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh1[i].params.sweptsin.fDeltaFreq *= exp(m_head.deltaCh1.fFreqDev * i);
			} else {
				m_stimArrayCh1[i].params.sweptsin.fDeltaFreq += m_head.deltaCh1.fFreqDev * i;
			}
			m_stimArrayCh1[i].fFreq = m_stimArrayCh1[i].params.sweptsin.fDeltaFreq;	// for compatibility
			break;
		default:
			break;
		}

	}
}


void CSpikeDoc::CalcStimuliCh2()
{
	m_stimArrayCh2.SetSize(m_head.nSweeps);		// Arraygröße festlegen
	int nSteps = m_head.nSweeps;

	for (int i=0; i<nSteps; i++)
	{
		m_stimArrayCh2[i] = m_head.stimFirstCh2;	// Parameter für ersten Stimulus
		
		// Timing
		m_stimArrayCh2[i].fBegin += m_head.deltaCh2.fBegin * i;
		m_stimArrayCh2[i].fDur += m_head.deltaCh2.fDur * i;
		m_stimArrayCh2[i].fStimInt += m_head.deltaCh2.fStimInt * i;
		//TRACE("stim %d ->  dur %.1f\n", i, m_stimArrayCh2[i].fDur);

		// Intensität
		m_stimArrayCh2[i].fAtten += m_head.deltaCh2.fAtten * i;
		//TRACE("stim %d ->  atten %.1f\n", i, m_stimArrayCh2[i].fAtten);

		switch (m_head.stimFirstCh2.nType)
		{
		case SINUS:
			m_stimArrayCh2[i].params.sin.fPhase += m_head.deltaCh2.fPhase * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.sin.fFreq *= exp(m_head.deltaCh2.fCarFreq * i);
			} else {
				m_stimArrayCh2[i].params.sin.fFreq += m_head.deltaCh2.fCarFreq * i;
			}
			m_stimArrayCh2[i].fFreq = m_stimArrayCh2[i].params.sin.fFreq;	// for compatibility
			break;
		case AMSINUS:
			m_stimArrayCh2[i].params.amsin.fPhase += m_head.deltaCh2.fPhase * i;
			m_stimArrayCh2[i].params.amsin.fAmDepth += m_head.deltaCh2.fAmDepth * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.amsin.fCarFreq *= exp(m_head.deltaCh2.fCarFreq * i);
			} else {
				m_stimArrayCh2[i].params.amsin.fCarFreq += m_head.deltaCh2.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.amsin.fModFreq *= exp(m_head.deltaCh2.fModFreq * i);
			} else {
				m_stimArrayCh2[i].params.amsin.fModFreq += m_head.deltaCh2.fModFreq * i;
			}
			m_stimArrayCh2[i].fFreq = m_stimArrayCh2[i].params.amsin.fModFreq;	// for compatibility
			break;
		case FILT_NOISE:
		case NOISE:
			m_stimArrayCh2[i].params.noise_ex.fAmDepth += m_head.deltaCh2.fAmDepth * i;
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.noise_ex.fModFreq *= exp(m_head.deltaCh2.fModFreq * i);
			} else {
				m_stimArrayCh2[i].params.noise_ex.fModFreq += m_head.deltaCh2.fModFreq * i;
			}
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.noise_ex.fCarFreq *= exp(m_head.deltaCh2.fCarFreq * i);
			} else {
				m_stimArrayCh2[i].params.noise_ex.fCarFreq += m_head.deltaCh2.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.noise_ex.fFreqDev *= exp(m_head.deltaCh2.fFreqDev * i);
			} else {
				m_stimArrayCh2[i].params.noise_ex.fFreqDev += m_head.deltaCh2.fFreqDev * i;
			}

			break;
		case FMSINUS:
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.fmsin.fCarFreq *= exp(m_head.deltaCh2.fCarFreq * i);
			} else {
				m_stimArrayCh2[i].params.fmsin.fCarFreq += m_head.deltaCh2.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.fmsin.fFreqDev *= exp(m_head.deltaCh2.fFreqDev * i);
			} else {
				m_stimArrayCh2[i].params.fmsin.fFreqDev += m_head.deltaCh2.fFreqDev * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.fmsin.fModFreq *= exp(m_head.deltaCh2.fModFreq * i);
			} else {
				m_stimArrayCh2[i].params.fmsin.fModFreq += m_head.deltaCh2.fModFreq * i;
			}
			m_stimArrayCh2[i].fFreq = m_stimArrayCh2[i].params.fmsin.fModFreq;	// for compatibility
			break;
		case SWEPTSINUS:
			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.sweptsin.fCarFreq *= exp(m_head.deltaCh2.fCarFreq * i);
			} else {
				m_stimArrayCh2[i].params.sweptsin.fCarFreq += m_head.deltaCh2.fCarFreq * i;
			}

			if (m_head.nScaling == LOGARITHMIC)	{
				m_stimArrayCh2[i].params.sweptsin.fDeltaFreq *= exp(m_head.deltaCh2.fFreqDev * i);
			} else {
				m_stimArrayCh2[i].params.sweptsin.fDeltaFreq += m_head.deltaCh2.fFreqDev * i;
			}
			m_stimArrayCh2[i].fFreq = m_stimArrayCh2[i].params.sweptsin.fDeltaFreq;	// for compatibility
			break;
		default:
			break;
		}

	}
}


BOOL CSpikeDoc::MakeName()
{
	CString strExtension;

	// Ermittle einen Zeiger auf die General-Eigenschaftsseite der PARAMETERS-Struktur !
	CSpikeApp*	pApp = (CSpikeApp *)AfxGetApp();
	CGeneralPage*	pPgGen = (CGeneralPage *)(&pApp->m_pParams->pgGen);
	int nNr = pPgGen->m_nNumber;
    
	// Erzeuge einen neuen Dateinamen im aktuellen Verzeichnis  
	do
	{
		strExtension.Format(".%.3d", nNr);
		m_strFileName = pPgGen->m_strName + strExtension;
		nNr++;
		if (nNr > 999)
		{
			pPgGen->m_nNumber = 0;
			AfxMessageBox("Maximale Extension für Dateiname (999) erreicht !\nÄndern Sie bitte den Dateinamen im Setup !");
			return TRUE;
		}
	}
    while (Exist(m_strFileName));	// existiert Datei bereits ?
	
	if (IsOpen(m_strFileName))		// Dokument ist offen, aber leer !
		return FALSE;

	pPgGen->m_nNumber = nNr - 1;	// aktualisiere Nummer

	return TRUE;
}

BOOL CSpikeDoc::Exist(LPCTSTR lpszFileName)
{
	WIN32_FIND_DATA ffd;

	if (::FindFirstFile(lpszFileName, &ffd) == INVALID_HANDLE_VALUE)
		return FALSE;
	else
		return TRUE;
}

BOOL CSpikeDoc::IsOpen(LPCTSTR lpszFileName)
{
	CDocTemplate* pTemplate = ((CSpikeApp*)AfxGetApp())->m_pDocTemplate; 
	
	// Überprüfe, ob Dokument mit gleichem Titel bereits geöffnet ist !
	POSITION pos = pTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{
		CSpikeDoc* pDoc = (CSpikeDoc *)pTemplate->GetNextDoc(pos);
		if (pDoc->GetTitle() == lpszFileName)
		{
			//AfxMessageBox("already open !");
			return TRUE;
		}
	}
	return FALSE;
}

void CSpikeDoc::ConvertOldHead()
// convert from v 3.0 to 4.0 (like before)
{
	strncpy(m_head_v40.cTime, m_head_v30.time, sizeof(m_head_v30.time));
	strncpy(m_head_v40.cDate, m_head_v30.date, sizeof(m_head_v30.date));
	strncpy(m_head_v40.cId, m_head_v30.id, sizeof(m_head_v30.id));
	m_head_v40.nMagic = m_head_v30.magic;
	m_head_v40.nOnCh1 = 1;
	m_head_v40.nOnCh2 = (m_head_v30.numchannels > 1);
	m_head_v40.nPasses = m_head_v30.numpasses;
	m_head_v40.nSweeps = m_head_v30.numxsteps;
	m_head_v40.nRepInt = 0;
	m_head_v40.nPresentation = RANDOMIZED;

	int nDiff = m_head_v30.x[m_head_v40.nSweeps-1] - m_head_v30.x[0];
	int nDelta = m_head_v30.x[1] - m_head_v30.x[0];
	if (nDiff > 0)
	{
		if (nDiff <= nDelta * (m_head_v40.nSweeps-1))
			m_head_v40.nScaling = LINEAR;
		else
			m_head_v40.nScaling = LOGARITHMIC;
	}
	else
	{
		if (nDiff >= nDelta * (m_head_v40.nSweeps-1))
			m_head_v40.nScaling = LINEAR;
		else
			m_head_v40.nScaling = LOGARITHMIC;
	}

	
	// Step-Modus bei der Reizdarbietung
	if (m_head_v30.y < 99.9)				// ändere Frequenz bei fester Intensität
		m_head_v40.nStepMode = FREQ_STEP;
	else								// ändere Intensität bei fester Frequenz
		m_head_v40.nStepMode = ATT_STEP;
	
	//if (m_head_v30.magic > 0)			// ändere Intensität und Frequenz
	//	m_head_v40.nStepMode = RASTER_STEP;
	
	// Stimuli
	if (m_head_v40.nOnCh1)
	{
		m_head_v40.stimFirstCh1.nType = SINUS;
		m_head_v40.stimFirstCh1.nStimPerSweep = 1;
		m_head_v40.stimFirstCh1.fRfTime = 0;
		m_head_v40.stimFirstCh1.fBegin = m_head_v30.timing[0].channel[0].delay * m_head_v30.timing_fac;
		m_head_v40.stimFirstCh1.fDur = m_head_v30.timing[0].channel[0].duration * m_head_v30.timing_fac;
		m_head_v40.stimFirstCh1.fStimInt = 0;
		if (m_head_v30.y > 99.9)
		{
			m_head_v40.stimFirstCh1.fAtten = m_head_v30.x[0];
			m_head_v40.stimFirstCh1.fFreq = m_head_v30.y;
		}
		else
		{
			m_head_v40.stimFirstCh1.fAtten = m_head_v30.y;
			m_head_v40.stimFirstCh1.fFreq = m_head_v30.x[0];
		}
		m_head_v40.stimFirstCh1.params.sin.fFreq = m_head_v40.stimFirstCh1.fFreq;
		m_head_v40.stimFirstCh1.params.sin.fPhase = 0;

		m_head_v40.deltaCh1.fBegin = (m_head_v30.timing[1].channel[0].delay - m_head_v30.timing[0].channel[0].delay) * m_head_v30.timing_fac;
		m_head_v40.deltaCh1.fDur = (m_head_v30.timing[1].channel[0].duration - m_head_v30.timing[0].channel[0].duration) * m_head_v30.timing_fac;
		m_head_v40.deltaCh1.fStimInt = 0;
		if (m_head_v30.y > 99.9)
		{
			m_head_v40.deltaCh1.fAtten = m_head_v30.x[1] - m_head_v30.x[0];
			m_head_v40.deltaCh1.fFreq = 0;
		}
		else
		{
			m_head_v40.deltaCh1.fFreq= m_head_v30.x[1] - m_head_v30.x[0];
			m_head_v40.deltaCh1.fAtten = 0;
		}

//		CalcStimuliCh1();
	}
	if (m_head_v40.nOnCh2)
	{
		m_head_v40.stimFirstCh2.nType = SINUS;
		m_head_v40.stimFirstCh2.nStimPerSweep = 1;
		m_head_v40.stimFirstCh2.fRfTime = 0;
		m_head_v40.stimFirstCh2.fBegin = m_head_v30.timing[0].channel[1].delay * m_head_v30.timing_fac;
		m_head_v40.stimFirstCh2.fDur = m_head_v30.timing[0].channel[1].duration * m_head_v30.timing_fac;
		m_head_v40.stimFirstCh2.fStimInt = 0;
		if (m_head_v30.y > 99.9)
		{
			m_head_v40.stimFirstCh2.fAtten = m_head_v30.x[0];
			m_head_v40.stimFirstCh2.fFreq = m_head_v30.y;
		}
		else
		{
			m_head_v40.stimFirstCh2.fAtten = m_head_v30.y;
			m_head_v40.stimFirstCh2.fFreq = m_head_v30.x[0];
		}
		m_head_v40.stimFirstCh2.params.sin.fFreq = m_head_v40.stimFirstCh1.fFreq;
		m_head_v40.stimFirstCh2.params.sin.fPhase = 0;

		m_head_v40.deltaCh2.fBegin = (m_head_v30.timing[1].channel[1].delay - m_head_v30.timing[0].channel[1].delay) * m_head_v30.timing_fac;
		m_head_v40.deltaCh2.fDur = (m_head_v30.timing[1].channel[1].duration - m_head_v30.timing[0].channel[1].duration) * m_head_v30.timing_fac;
		m_head_v40.deltaCh2.fStimInt = 0;
		if (m_head_v30.y > 99.9)
		{
			m_head_v40.deltaCh2.fAtten = m_head_v30.x[1] - m_head_v30.x[0];
			m_head_v40.deltaCh2.fFreq = 0;
		}
		else
		{
			m_head_v40.deltaCh2.fFreq= m_head_v30.x[1] - m_head_v30.x[0];
			m_head_v40.deltaCh2.fAtten = 0;
		}

//		CalcStimuliCh2();
	}

}


/*
att  Attenuation
del  Delay
dur  Duration
frq  Frequency
swp  Sweep Depth
fmr  Frequency modulation rate
amr  Amplitude modulation rate
ste  Steps
fmd  Frequency modulation depth
amd  Amplitude modulation depth
*/

#include "ExportDlg.h"

int		g_export_time_mode=TMODE_ABS, g_export_line_end_fmt=MAC_NL, g_export_file_fmt=IGOR_FMT;
BOOL	g_bPlaceHolder=FALSE, g_bHeaderOmit=FALSE, g_bTossPreStim=FALSE;
CString	g_export_strRunMode, g_strPlaceHolder="-1.0";

void CSpikeDoc::OnFileExport() 
{

	// user supplied
	CExportDlg dlg;
	dlg.m_newline = g_export_line_end_fmt;
	dlg.m_spikeTimeMode = g_export_time_mode;
	if (g_export_strRunMode == "") g_export_strRunMode = "att  Attenuation";
	dlg.m_runType = g_export_strRunMode;
	dlg.m_fileType = g_export_file_fmt;
	dlg.m_bPlaceHolder = g_bPlaceHolder;
	dlg.m_strPlaceHolder = g_strPlaceHolder;
	dlg.m_bHeaderOmit = g_bHeaderOmit;
	dlg.m_bTossPreStim = g_bTossPreStim;

	// invoke the dialog box
	if (dlg.DoModal() != IDOK) return;

	// retrieve the dialog data
	g_export_line_end_fmt = dlg.m_newline;
	g_export_time_mode = dlg.m_spikeTimeMode;
	g_export_strRunMode = dlg.m_runType;
	g_export_file_fmt = dlg.m_fileType;
	g_bPlaceHolder = dlg.m_bPlaceHolder;
	g_strPlaceHolder = dlg.m_strPlaceHolder;
	g_bHeaderOmit = dlg.m_bHeaderOmit;
	g_bTossPreStim = dlg.m_bTossPreStim;

	if (g_export_file_fmt == WAR_FMT) {
		FileWarExport();
	} else if (g_export_file_fmt == EXCEL_FMT) {
		FileExcelExport();
	} else {
		FileExport();
	}
}

CString stimTypes[] = {"NOISE", "SINUS", "AMSINUS", "FMSINUS", "SWEPTSINUS", "WAVEFILE", "FILT_NOISE"};

void CSpikeDoc::FileExport()
{

	CString lineTerm;
	if (g_export_line_end_fmt == MAC_NL) {
		lineTerm="\r";
	} else if (g_export_line_end_fmt == UNIX_NL) {
		lineTerm = "\n";
	} else {
		lineTerm="\r\n";	// IBM PC
	}

	int	nSpikes = m_spikeArray.GetSize();
	int	nSpikesExported = 0;
	int nSpikesThisFile, nSpikesAllFiles=0;

	CStdioFile dataFile;
	CFileStatus fStat;
	CFileException e;

	CString exportFileName, strStep, tableOfContentsFN, unitNumStr, origFileName, dirName;
	CString mainFileName;

	origFileName = GetPathName();
	int fn_len = origFileName.GetLength();	// fn_len will be adjusted below
	int iName = origFileName.ReverseFind('\\');
	dirName = origFileName.Left(iName+1);
	origFileName = origFileName.Right(fn_len-iName-1);	// sans directory
	mainFileName = origFileName;
	fn_len = mainFileName.GetLength();

	// convert '.' to '_' and extract unit number from filename
	// ALSO: convert non alpha-numeric to '_'
	int fn_field=0, unitIndx=0;
	for (int i=0; i<fn_len; i++) {
		if (mainFileName.GetAt(i) == '.') {
			mainFileName.SetAt(i, '_');
			fn_field++;
		}
		if (fn_field == 1) {
			// in the 2nd field of filename - unit number
			if (unitIndx > 0) {
				unitNumStr += mainFileName.GetAt(i);
			}
			unitIndx++;
		}
		if ( !isalnum(mainFileName.GetAt(i)) && mainFileName.GetAt(i) != '_' ) {
			mainFileName.SetAt(i, '_');
		}
	}

	// If first char is not a letter, then start the filename with a 'W'
	if (! isalpha(mainFileName.GetAt(0)) ) {
		mainFileName = "W" + mainFileName;
	}

	mainFileName = dirName + mainFileName;	// tack dir back on

	CString outStr;

	if (g_export_file_fmt == IGOR_FMT) {
		// write "Table of Contents" file
		tableOfContentsFN = mainFileName + ".txt";
		// typeBinary so it doesn't fuck with the \n or \r\n
		if ( !dataFile.Open(tableOfContentsFN, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate, &e) ) {
			// file could not be opened
			AfxMessageBox("Data TOC file could not be opened. File name is " + tableOfContentsFN);
			return;
		}
		dataFile.WriteString(unitNumStr+lineTerm);
		outStr.Format("01%s%02d%s%s%s", lineTerm, m_head.nSweeps, lineTerm, g_export_strRunMode.Left(3), lineTerm);
		dataFile.WriteString(outStr);
		dataFile.Close();
	}

	for (int nSweep=1; nSweep <= m_head.nSweeps; nSweep++) {
		strStep.Format("_%.2d", nSweep);
		exportFileName = mainFileName + strStep + ".txt";

		// typeBinary so it doesn't fuck with the \n or \r\n
		if ( !dataFile.Open(exportFileName, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate, &e) ) {
			// file could not be opened
			AfxMessageBox("Data file could not be opened. File name is " + exportFileName);
			break;
		}

		TRY
		{
			// write header. The data will follow.

			STIM *pStim[] = {0,0};	// index is chan - 1
			if (m_head.nOnCh1) {
				pStim[0] = &m_stimArrayCh1[nSweep-1];
			}
			if (m_head.nOnCh2) {
				pStim[1] = &m_stimArrayCh2[nSweep-1];
			}

			float preWindow = CalcPreWindow(pStim[0], pStim[1]);
			if (preWindow < 0.0f) {
				break;
			}

			// count spikes
			int nSpikesThisFile=0;
			for(int i=0; i<nSpikes; i++) {
				if (m_spikeArray[i].nSweep == nSweep) {
					if (m_spikeArray[i].nPass < 1 || m_spikeArray[i].nPass > m_head.nPasses) {
						AfxMessageBox("Warning: invalid pass number detected");
					}
					nSpikesThisFile++;
				}
			}

			nSpikesAllFiles += nSpikesThisFile;

			if (!g_bHeaderOmit) {
				// write first section of header
				dataFile.WriteString("RepInterval\tN_Passes\tTotal_Spikes\tTime_Offset\tfilename"+lineTerm);
				outStr.Format("%d\t%d\t%d\t%.1f\t%s%s",
					m_head.nRepInt, m_head.nPasses, nSpikesThisFile, preWindow, origFileName, lineTerm);
				dataFile.WriteString(outStr);


				dataFile.WriteString("Chan\tStimType\tDelay\tDur\tStimPerSweep\tInterStimInt\tLevel"
						"\tCarFreq\tModFreq\tFreqDev\tAM_Depth\tPhase\twavFileName"+lineTerm);

				for (int ichan=0; ichan<2; ichan++) {
					if (!pStim[ichan]) {
						// inactive
						outStr.Format("%d\tOFF%s", ichan+1, lineTerm);
						dataFile.WriteString(outStr);
					} else {
						// chan ichan+1 is active
						outStr.Format("%d\t%s\t%g\t%g\t%d\t%g\t%g",
							ichan+1, stimTypes[pStim[ichan]->nType], pStim[ichan]->fBegin, pStim[ichan]->fDur, 
							pStim[ichan]->nStimPerSweep, pStim[ichan]->fStimInt, pStim[ichan]->fAtten);
						dataFile.WriteString(outStr);

						// write info that varies with stim type
						// \tCarFreq\tModFreq\tFreqDev\tAM_Depth\tPhase\twavFileName
						switch (pStim[ichan]->nType) {
							case NOISE:
								outStr.Format("\t%g\t%g\t%g\t%g\t\t",
									pStim[ichan]->params.noise_ex.fCarFreq,
									pStim[ichan]->params.noise_ex.fModFreq,
									pStim[ichan]->params.noise_ex.fFreqDev,
									pStim[ichan]->params.noise_ex.fAmDepth);
								dataFile.WriteString(outStr);
								break;
							case SINUS:
								outStr.Format("\t%g\t\t\t\t%g\t",
									pStim[ichan]->params.sin.fFreq, 
									pStim[ichan]->params.sin.fPhase);
								dataFile.WriteString(outStr);
								break;
							case AMSINUS:
								outStr.Format("\t%g\t%g\t\t%g\t%g\t",
									pStim[ichan]->params.amsin.fCarFreq,
									pStim[ichan]->params.amsin.fModFreq,

									pStim[ichan]->params.amsin.fAmDepth, 
									pStim[ichan]->params.amsin.fPhase);
								dataFile.WriteString(outStr);
								break;
							case FMSINUS:
								outStr.Format("\t%g\t%g\t%g\t\t\t",
									pStim[ichan]->params.fmsin.fCarFreq,
									pStim[ichan]->params.fmsin.fModFreq,
									pStim[ichan]->params.fmsin.fFreqDev);
								dataFile.WriteString(outStr);
								break;
							case SWEPTSINUS:
								outStr.Format("\t%g\t\t%g\t\t\t",
									pStim[ichan]->params.sweptsin.fCarFreq,
									
									pStim[ichan]->params.sweptsin.fDeltaFreq);
								dataFile.WriteString(outStr);
								break;
							case WAVEFILE:
								outStr.Format("\t\t\t\t\t\t%s",
									pStim[ichan]->params.wav.strFileName);
								dataFile.WriteString(outStr);
								break;
							default:
								break;
						}

						dataFile.WriteString(lineTerm);
					}
				}

				dataFile.WriteString("END_OF_HEADER"+lineTerm);
			}

			for (int nPass=1; nPass <= m_head.nPasses; nPass++) {
				if (g_export_file_fmt == IGOR_FMT) {
					outStr.Format("%d", nPass);
					dataFile.WriteString(outStr);
				}

				int nSpikesThisPass = 0;

				// write out the spike times
				for(int i=0; i<nSpikes; i++) {
					//int nS = m_spikeArray[i].nSweep;
					//int nP = m_spikeArray[i].nPass;
					if (m_spikeArray[i].nPass < nPass) continue;	// could index this to avoid this
					if (m_spikeArray[i].nPass > nPass) continue; //break is faster, continue is safer
					if (m_spikeArray[i].nSweep == nSweep) {
						outStr.Format(" %.3f", m_spikeArray[i].fTime - preWindow);
						dataFile.WriteString(outStr);
						nSpikesThisPass++;
						nSpikesExported++;
						if (g_export_file_fmt == ALL_IN_1_COL) {
							dataFile.WriteString(lineTerm);
						}
					}
				}
				if (g_bPlaceHolder && !nSpikesThisPass) {
					dataFile.WriteString(" ");
					dataFile.WriteString(g_strPlaceHolder);
				}
				if (g_export_file_fmt != ALL_IN_1_COL) dataFile.WriteString(lineTerm);
			}
			dataFile.Close();
		}
		CATCH( CFileException, e )
		{
			char errStr[132];
			sprintf(errStr, "Error writing to data file. m_cause = %d", e->m_cause);
			AfxMessageBox(errStr);
			break;
		}
		END_CATCH

	} // end sweep loop
	if (nSpikesExported != nSpikes) {
		outStr.Format("Warning: spike array size is %d, but exported %d spikes.", nSpikes, nSpikesExported);
		AfxMessageBox(outStr);
	}
	if (nSpikesExported != nSpikesAllFiles) {
		outStr.Format("Warning: Exported %d spikes, but expected %d spikes", nSpikesExported, nSpikesAllFiles);
		AfxMessageBox(outStr);
	}

}

void CSpikeDoc::FileExcelExport()
{
	CString lineTerm;
	if (g_export_line_end_fmt == MAC_NL) {
		lineTerm="\r";
	} else if (g_export_line_end_fmt == UNIX_NL) {
		lineTerm = "\n";
	} else {
		lineTerm="\r\n";	// IBM PC
	}

	CStdioFile dataFile;
	CFileStatus fStat;
	CFileException e;

	CString exportFileName, unitNumStr, origFileName, dirName;
	CString mainFileName;

	origFileName = GetPathName();
	int fn_len = origFileName.GetLength();	// fn_len will be adjusted below
	int iName = origFileName.ReverseFind('\\');
	dirName = origFileName.Left(iName+1);
	origFileName = origFileName.Right(fn_len-iName-1);	// sans directory
	mainFileName = origFileName;
	fn_len = mainFileName.GetLength();

	// convert '.' to '_' and extract unit number from filename
	// ALSO: convert non alpha-numeric to '_'
	int fn_field=0, unitIndx=0;
	for (int i=0; i<fn_len; i++) {
		if (mainFileName.GetAt(i) == '.') {
			mainFileName.SetAt(i, '_');
			fn_field++;
		}
		if (fn_field == 1) {
			// in the 2nd field of filename - unit number
			if (unitIndx > 0) {
				unitNumStr += mainFileName.GetAt(i);
			}
			unitIndx++;
		}
		if ( !isalnum(mainFileName.GetAt(i)) && mainFileName.GetAt(i) != '_' ) {
			mainFileName.SetAt(i, '_');
		}
	}

	mainFileName = dirName + mainFileName;	// tack dir back on

	CString outStr;

	exportFileName = mainFileName + ".txt";
	// typeBinary so it doesn't fuck with the \n or \r\n
	if ( !dataFile.Open(exportFileName, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate, &e) ) {
		// file could not be opened
		AfxMessageBox("Data file could not be opened. File name is " + exportFileName);
		return;
	}

	STIM *pStim[] = {0,0};	// index is chan - 1

	float *preWindow;
	preWindow = new float[m_head.nSweeps];
	TRY
	{
		if (!g_bHeaderOmit) {
			outStr.Format("Filename\t%s%s", origFileName, lineTerm);
			dataFile.WriteString(outStr);
			outStr.Format("Num_Sweeps\t%d%s", m_head.nSweeps, lineTerm);
			dataFile.WriteString(outStr);
			outStr.Format("Num_Passes\t%d%s", m_head.nPasses, lineTerm);
			dataFile.WriteString(outStr);
			outStr.Format("Rep_Interval\t%d%s", m_head.nRepInt, lineTerm);
			dataFile.WriteString(outStr);

			dataFile.WriteString("Sweep\tTime_Offset\t"
				"Chan\tStimType\tDelay\tDur\tStimPerSweep\tInterStimInt\tLevel"
				"\tCarFreq\tModFreq\tFreqDev\tAM_Depth\tPhase\twavFileName"+lineTerm);
		}

		for (int nSweep=1; nSweep <= m_head.nSweeps; nSweep++) {
			if (m_head.nOnCh1) {
				pStim[0] = &m_stimArrayCh1[nSweep-1];
			}
			if (m_head.nOnCh2) {
				pStim[1] = &m_stimArrayCh2[nSweep-1];
			}
			preWindow[nSweep-1] = CalcPreWindow(pStim[0], pStim[1]);
			if (preWindow[nSweep-1] < 0.0f) {
				return;
			}
			if (!g_bHeaderOmit) {
				for (int ichan=0; ichan<2; ichan++) {
					outStr.Format("%d\t%.1f\t", nSweep, preWindow[nSweep-1]);
					dataFile.WriteString(outStr);
					if (!pStim[ichan]) {
						// inactive
						outStr.Format("%d\tOFF%s", ichan+1, lineTerm);
						dataFile.WriteString(outStr);
					} else {
						// chan ichan+1 is active
						outStr.Format("%d\t%s\t%g\t%g\t%d\t%g\t%g",
							ichan+1, stimTypes[pStim[ichan]->nType], pStim[ichan]->fBegin, pStim[ichan]->fDur, 
							pStim[ichan]->nStimPerSweep, pStim[ichan]->fStimInt, pStim[ichan]->fAtten);
						dataFile.WriteString(outStr);

						// write info that varies with stim type
						// \tCarFreq\tModFreq\tFreqDev\tAM_Depth\tPhase\twavFileName
						switch (pStim[ichan]->nType) {
							case NOISE:
								outStr.Format("\t%g\t%g\t%g\t%g\t\t",
									pStim[ichan]->params.noise_ex.fCarFreq,
									pStim[ichan]->params.noise_ex.fModFreq,
									pStim[ichan]->params.noise_ex.fFreqDev,
									pStim[ichan]->params.noise_ex.fAmDepth);
								dataFile.WriteString(outStr);
								break;
							case SINUS:
								outStr.Format("\t%g\t\t\t\t%g\t",
									pStim[ichan]->params.sin.fFreq, 
									pStim[ichan]->params.sin.fPhase);
								dataFile.WriteString(outStr);
								break;
							case AMSINUS:
								outStr.Format("\t%g\t%g\t\t%g\t%g\t",
									pStim[ichan]->params.amsin.fCarFreq,
									pStim[ichan]->params.amsin.fModFreq,

									pStim[ichan]->params.amsin.fAmDepth, 
									pStim[ichan]->params.amsin.fPhase);
								dataFile.WriteString(outStr);
								break;
							case FMSINUS:
								outStr.Format("\t%g\t%g\t%g\t\t\t",
									pStim[ichan]->params.fmsin.fCarFreq,
									pStim[ichan]->params.fmsin.fModFreq,
									pStim[ichan]->params.fmsin.fFreqDev);
								dataFile.WriteString(outStr);
								break;
							case SWEPTSINUS:
								outStr.Format("\t%g\t\t%g\t\t\t",
									pStim[ichan]->params.sweptsin.fCarFreq,
									
									pStim[ichan]->params.sweptsin.fDeltaFreq);
								dataFile.WriteString(outStr);
								break;
							case WAVEFILE:
								outStr.Format("\t\t\t\t\t\t%s",
									pStim[ichan]->params.wav.strFileName);
								dataFile.WriteString(outStr);
								break;
							default:
								break;
						} // end switch

						dataFile.WriteString(lineTerm);
					} // end if
				} // end for
			} // end if (!g_bHeaderOmit)
		}


		if (!g_bHeaderOmit) {
			dataFile.WriteString("n\tsweep\tX_value\tpass\tSpikeTimes");
		}
		// ==== BEGIN GetData() ==========
		DWORD		val;
		int			nSweepCount = 0;
		int			nPassCount = 0;
		int			nSpikeCount = 0;
		int			nPass;		// Numerierung beginnt bei 1 !

		int	nSize = m_dwDataArray.GetSize();
		if (nSize == 0)					// no data available
			return;

		if (m_dwDataArray[0] != SWEEP_MARK && m_dwDataArray[0] != EOF_MARK) {
			AfxMessageBox("Error in data format");
			return;
		}
		int nLine = 1;
		BOOL bFirstSpikeWritten = 0;
		CString X_Val;
		for (i=0; i<nSize; i++)
		{
			val = m_dwDataArray[i];
			switch(val)
			{
				case SWEEP_MARK:
					// Passnummer berechnen
					if (m_head.nPresentation != NONINTERLEAVED) {
						if (strcmp(SPIKE30_ID, m_cHeadId) == 0)		// alt
							nPass = nSweepCount/m_head_v30.numxsteps + 1;
						else										// neu
							nPass = nSweepCount/m_head.nSweeps + 1;
						if (nPass < 1 || nPass > m_head.nPasses) {
							AfxMessageBox("Bad pass number found. Corrupted data file?");
							nPass = 1;	// force to legal value
						}
						i++;
						nSweepCount++;
						nSweep = m_dwDataArray[i];
						if (nSweep < 1 || nSweep > m_head.nSweeps) {
							AfxMessageBox("Bad sweep number found. Corrupted data file?");
							nSweep = 1;	// force to legal value
						}
					} else {
						nSweep = nPassCount/m_head.nPasses + 1;
						if (nSweep < 1 || nSweep > m_head.nSweeps) {
							AfxMessageBox("Bad sweep number found. Corrupted data file?");
							nSweep = 1;	// force to legal value
						}
						i++;
						nPassCount++;
						nPass = m_dwDataArray[i];
						if (nPass < 1 || nPass > m_head.nPasses) {
							AfxMessageBox("Bad pass number found. Corrupted data file?");
							nPass = 1;	// force to legal value
						}
					}
					if (g_bPlaceHolder && nLine > 1 && !bFirstSpikeWritten) {
						dataFile.WriteString(g_strPlaceHolder);
					}
					BOOL bIsNum;
					X_Val = GetXorYVal(0, nSweep-1, this, &bIsNum);	// axis 0=X, 1=Y
					outStr.Format("%s%d\t%d\t%s\t%d\t", lineTerm, nLine, nSweep, X_Val, nPass);
					dataFile.WriteString(outStr);
					nLine++;
					bFirstSpikeWritten = 0;
					break;

				case EOF_MARK:
					if (g_bPlaceHolder && nLine > 1 && !bFirstSpikeWritten) {
						dataFile.WriteString(g_strPlaceHolder);
					}
					break;

				default:		// spike !
					if (bFirstSpikeWritten) {
						dataFile.WriteString(",");
					}
					bFirstSpikeWritten = 1;
					outStr.Format("%.3f", val/1000.0f - preWindow[nSweep-1]);
					dataFile.WriteString(outStr);
					break; 
			}
		} 
		dataFile.WriteString(lineTerm);
		// ==== END GetData() ==========

		dataFile.Close();
	}
	CATCH( CFileException, e )
	{
		char errStr[132];
		sprintf(errStr, "Error writing to data file. m_cause = %d", e->m_cause);
		AfxMessageBox(errStr);
	}
	END_CATCH

	delete [] preWindow;

}

void CSpikeDoc::RedrawSpikeView()
{
	CView*	pView;
	POSITION pos = GetFirstViewPosition();
	while (pos)
	{
		pView = GetNextView(pos);
		ASSERT_VALID(pView);
		if (pView->IsKindOf(RUNTIME_CLASS(CSpikeView))) {
			pView->Invalidate();
		}
	}

}

extern BOOL g_bDummyFileCreated;

void CSpikeDoc::OnCloseDocument() 
{
	if (m_strFileName == "DummyFile.001") {
		// closing dummy file
		g_bDummyFileCreated = 0;
	}

	CDocument::OnCloseDocument();
}

#include "AnaHeader.h"

void CSpikeDoc::FileWarExport()
{
	CStdioFile dataFile, analogFile;
	CFileStatus fStat;
	CFileException e;

	CString exportFileName, strStep, origFileName, dirName;
	CString mainFileName, analogFileName;

	origFileName = GetPathName(); //m_strFileName ?
	if (origFileName.GetLength() == 0) {
		origFileName = m_strFileName;
	}
	analogFileName = origFileName + ".ana";

	if (!analogFile.Open(analogFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary)) {
		// open failure
		AfxMessageBox("Cannot open Analog File");
		return;
	}

	int fn_len = origFileName.GetLength();	// fn_len will be adjusted below
	int iName = origFileName.ReverseFind('\\');
	dirName = origFileName.Left(iName+1);
	origFileName = origFileName.Right(fn_len-iName-1);	// sans directory
	mainFileName = origFileName;
	fn_len = mainFileName.GetLength();

	// convert '.' to '_' and extract unit number from filename
	// ALSO: convert non alpha-numeric to '_'
	int fn_field=0;
	for (int i=0; i<fn_len; i++) {
		if (mainFileName.GetAt(i) == '.') {
			mainFileName.SetAt(i, '_');
			fn_field++;
		}
		if ( !isalnum(mainFileName.GetAt(i)) && mainFileName.GetAt(i) != '_' ) {
			mainFileName.SetAt(i, '_');
		}
	}

	// If first char is not a letter, then start the filename with a 'W'
	if (! isalpha(mainFileName.GetAt(0)) ) {
		mainFileName = "W" + mainFileName;
	}

	mainFileName = dirName + mainFileName;	// tack dir back on
	//+
	HEADER60 head_copy = m_head;	// for _HDR file

	float fDelayBefore1stStim=0.0f;

	if (m_head.nOnCh1 && m_head.nOnCh2) {
		// 2 active chans
		if (g_bTossPreStim) {
			if (m_head.stimFirstCh1.fBegin < m_head.stimFirstCh2.fBegin) {
				fDelayBefore1stStim = m_head.stimFirstCh1.fBegin;
			} else {
				fDelayBefore1stStim = m_head.stimFirstCh2.fBegin;
			}
			head_copy.stimFirstCh1.fBegin -= fDelayBefore1stStim;
			head_copy.stimFirstCh2.fBegin -= fDelayBefore1stStim;
		}
		if (m_head.deltaCh1.fBegin != 0.0f || m_head.deltaCh2.fBegin != 0.0f) {
			AfxMessageBox("WARNING: the stimulus delay settings are not static.");
		}
	} else if (m_head.nOnCh1) {
		if (g_bTossPreStim) {
			head_copy.stimFirstCh1.fBegin = 0.0f;
		}
		if (m_head.deltaCh1.fBegin != 0.0f) {
			AfxMessageBox("WARNING: the stimulus delay settings are not static.");
		}
	} else {
		if (g_bTossPreStim) {
			head_copy.stimFirstCh2.fBegin = 0.0f;
		}
		if (m_head.deltaCh2.fBegin != 0.0f) {
			AfxMessageBox("WARNING: the stimulus delay settings are not static.");
		}
	}

	if ( !dataFile.Open(mainFileName + "_HDR", CFile::modeWrite | CFile::typeBinary | CFile::modeCreate, &e) ) {
		// file could not be opened
		AfxMessageBox("Data hdr file could not be opened. File name is " + mainFileName + "_HDR");
		return;
	}
	TRY
	{
		dataFile.Write((void*)&head_copy, sizeof(head_copy));
		dataFile.Close();
	}
	CATCH( CFileException, e )
	{
		char errStr[132];
		sprintf(errStr, "Error writing to data file. m_cause = %d", e->m_cause);
		AfxMessageBox(errStr);
		return;
	}
	END_CATCH
	//-

	char zeros[256];
	short *anaBuf;
	ANA_HEAD	anaHeader;

	int version = 0;
	int nAnalogSampsPerTrace = 5000;
	int nAnaSRate = 20000;
	analogFile.Read((void*)&anaHeader, sizeof(anaHeader));
	if (strncmp("Spike waveform file",anaHeader.id, 19) != 0) {
		// no header - the old style
		analogFile.SeekToBegin();
	} else {
		// valid header
		version = anaHeader.version;
		nAnalogSampsPerTrace = anaHeader.nAnaPoints;
		nAnaSRate = anaHeader.srate;
	}

	CString outStr;

	for (i=0; i<256; i++) {
		zeros[i] = 0;
	}

	anaBuf = new short[nAnalogSampsPerTrace];

	// get max, min of whole analog file
	short anaMin=32767, anaMax=-32768;
	for (int nSweep=1; nSweep <= m_head.nSweeps; nSweep++) {
		for (int nPass=1; nPass <= m_head.nPasses; nPass++) {
			analogFile.Read(anaBuf, nAnalogSampsPerTrace*2);
			for (i=0; i<nAnalogSampsPerTrace; i++) {
				if (anaBuf[i] > anaMax) anaMax = anaBuf[i];
				if (anaBuf[i] < anaMin) anaMin = anaBuf[i];
			}
		}
	}
	short anaMult = 1;
	while (anaMax < 16380 && anaMin > -16380 && anaMult < 16) {
		anaMult *= 2;
		anaMin *= 2;
		anaMax *= 2;
	}
//+ BEGIN ARRAY BUILD
	int nMaxNTraces = m_head.nSweeps * m_head.nPasses;
	int *sweep_order = new int[nMaxNTraces];
	int *pass_order = new int[nMaxNTraces];
	int iTrace = 0;

	// ==== BEGIN GetData() ==========
	DWORD		val;
	int			nSweepCount = 0;
	int			nPassCount = 0;
	int			nPass;

	int	nSize = m_dwDataArray.GetSize();
	if (nSize == 0)	{				// no data available
		AfxMessageBox("No data available");
		return;
	}

	if (m_dwDataArray[0] != SWEEP_MARK && m_dwDataArray[0] != EOF_MARK) {
		AfxMessageBox("Error in data format");
		return;
	}
	for (i=0; i<nSize; i++) {
		val = m_dwDataArray[i];
		switch(val) {
			case SWEEP_MARK:
				if (m_head.nPresentation != NONINTERLEAVED) {
					if (strcmp(SPIKE30_ID, m_cHeadId) == 0)		// alt
						nPass = nSweepCount/m_head_v30.numxsteps + 1;
					else										// neu
						nPass = nSweepCount/m_head.nSweeps + 1;
					if (nPass < 1 || nPass > m_head.nPasses) {
						AfxMessageBox("Bad pass number found. Corrupted data file?");
						nPass = 1;	// force to legal value
					}
					i++;
					nSweepCount++;
					nSweep = m_dwDataArray[i];
					if (nSweep < 1 || nSweep > m_head.nSweeps) {
						AfxMessageBox("Bad sweep number found. Corrupted data file?");
						nSweep = 1;	// force to legal value
					}
				} else {
					nSweep = nPassCount/m_head.nPasses + 1;
					if (nSweep < 1 || nSweep > m_head.nSweeps) {
						AfxMessageBox("Bad sweep number found. Corrupted data file?");
						nSweep = 1;	// force to legal value
					}
					i++;
					nPassCount++;
					nPass = m_dwDataArray[i];
					if (nPass < 1 || nPass > m_head.nPasses) {
						AfxMessageBox("Bad pass number found. Corrupted data file?");
						nPass = 1;	// force to legal value
					}
				}
				if (iTrace >= nMaxNTraces) {
					AfxMessageBox("File Format Error - too many passes/sweeps");
					return;
				}
				sweep_order[iTrace] = nSweep;
				pass_order[iTrace] = nPass;
				iTrace++;
				break;

			default:		// spike !
				break; 
		}
	} 
	// ==== END GetData() ==========
//- END ARRAY BUILD

	int analogFileOffset;
	if (version == 0) {
		analogFileOffset = 0;
	} else {
		analogFileOffset = sizeof(anaHeader);
	}

	for (nSweep=1; nSweep <= m_head.nSweeps; nSweep++) {
		strStep.Format("_%.2d", nSweep);
		exportFileName = mainFileName + strStep + ".war";

		if ( !dataFile.Open(exportFileName, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate, &e) ) {
			// file could not be opened
			AfxMessageBox("Data file could not be opened. File name is " + exportFileName);
			break;
		}

		TRY
		{
			// write header. The data will follow.

			STIM *pStim[] = {0,0};	// index is chan - 1
			if (m_head.nOnCh1) {
				pStim[0] = &m_stimArrayCh1[nSweep-1];
			}
			if (m_head.nOnCh2) {
				pStim[1] = &m_stimArrayCh2[nSweep-1];
			}

			dataFile.WriteString("PVWAVEACQ_1.0"); // The program we are emulating
			int nHdrSz = 13; // We must keep track of the header size. (must match above line)

			for (int ichan=0; ichan<2; ichan++) {
				if (pStim[ichan]) {

					int freq = 0;
					switch (pStim[ichan]->nType) {
						case NOISE:
							freq = (int)pStim[ichan]->params.noise_ex.fModFreq;
							break;
						case SINUS:
							freq = (int)pStim[ichan]->params.sin.fFreq;
							break;
						case AMSINUS:
							freq = (int)pStim[ichan]->params.amsin.fCarFreq;
							break;
						case FMSINUS:
							freq = (int)pStim[ichan]->params.fmsin.fCarFreq;
							break;
						case SWEPTSINUS:
							freq = (int)pStim[ichan]->params.sweptsin.fCarFreq;
							break;
						case WAVEFILE:
							break;
						default:
							break;
					}
					if (g_bTossPreStim) {
						outStr.Format(" L%d%d F%d%d D%d%d R%d%d C%dnone.spk",
							ichan+1,(int)pStim[ichan]->fAtten, ichan+1,freq, ichan+1,(int)pStim[ichan]->fDur, ichan+1,(int)pStim[ichan]->fRfTime, ichan+1);
					} else {
						outStr.Format(" L%d%d F%d%d D%d0 R%d0 C%dnone.spk",
							ichan+1,(int)pStim[ichan]->fAtten, ichan+1,freq, ichan+1, ichan+1, ichan+1);
					}
					nHdrSz += outStr.GetLength();
					dataFile.WriteString(outStr);
				}
			}

			fDelayBefore1stStim=0.0f;

			if (pStim[0] && pStim[1]) {
				// 2 active chans
				outStr.Format(" DL%d", (int)(pStim[1]->fBegin - pStim[0]->fBegin));
				nHdrSz += outStr.GetLength();
				dataFile.WriteString(outStr);
				if (g_bTossPreStim) {
					if (pStim[0]->fBegin < pStim[1]->fBegin) {
						fDelayBefore1stStim = pStim[0]->fBegin;
					} else {
						fDelayBefore1stStim = pStim[1]->fBegin;
					}
				}
			} else if (pStim[0]) {
				if (g_bTossPreStim) {
					fDelayBefore1stStim = pStim[0]->fBegin;
				}
			} else {
				if (g_bTossPreStim) {
					fDelayBefore1stStim = pStim[1]->fBegin;
				}
			}

			int nStimPeriodInMs = 1000 * nAnalogSampsPerTrace / nAnaSRate;
			nStimPeriodInMs = (int)((float)nStimPeriodInMs - fDelayBefore1stStim);

			outStr.Format(" SP%d RP%d", nStimPeriodInMs, m_head.nPasses);
			nHdrSz += outStr.GetLength();
			dataFile.WriteString(outStr);

			int nZerosToWrite = 256 - nHdrSz;
			dataFile.Write(zeros, nZerosToWrite);

			int nDelayBefore1stStim = fDelayBefore1stStim * nAnaSRate / 1000;	// ms -> nsamps
			if (nDelayBefore1stStim >= nAnalogSampsPerTrace) {
				AfxMessageBox("Error - nDelayBefore1stStim >= nAnalogSampsPerTrace");
				return;
			}
			
			anaMin=32767;
			anaMax=-32768;
			for (int nPass=1; nPass <= m_head.nPasses; nPass++) {

				// seek
				int bTraceFound=0;
				for (iTrace=0; iTrace<nMaxNTraces; iTrace++) {
					if (sweep_order[iTrace] == nSweep && pass_order[iTrace] == nPass) {
						bTraceFound = 1;
						break;
					}
				}
				if (!bTraceFound) {
					AfxMessageBox("FILE FMT ERROR - trace not found");
					return;
				}
				analogFile.Seek(analogFileOffset+iTrace*nAnalogSampsPerTrace*2, CFile::begin);

				analogFile.Read(anaBuf, nAnalogSampsPerTrace*2);

				for (i=0; i<nAnalogSampsPerTrace; i++) {
					if (anaBuf[i] > anaMax) anaMax = anaBuf[i];
					if (anaBuf[i] < anaMin) anaMin = anaBuf[i];
					anaBuf[i] *= anaMult;
					anaBuf[i] &= 0xFFF0;
				}
				if (anaMax == anaMin) {
					AfxMessageBox("ERROR: the analog trace is FLAT");
					return;
				}
				anaBuf[nDelayBefore1stStim] |= 0x4;
				if (nPass == m_head.nPasses) {
					anaBuf[nAnalogSampsPerTrace-1] |= 0x8;
				}

				dataFile.Write(anaBuf+nDelayBefore1stStim, (nAnalogSampsPerTrace-nDelayBefore1stStim)*2);
			}
			dataFile.Close();
		}
		CATCH( CFileException, e )
		{
			char errStr[132];
			sprintf(errStr, "Error writing to data file. m_cause = %d", e->m_cause);
			AfxMessageBox(errStr);
			break;
		}
		END_CATCH

	} // end sweep loop

	delete [] anaBuf;

}

float CSpikeDoc::CalcPreWindow(STIM *pStim0, STIM *pStim1)
{
	float preWindow = 0.0f;
	if (g_export_time_mode == TMODE_CH1) {
		if (!m_head.nOnCh1) {
			AfxMessageBox("Error: CH1 is not active.");
			return -1.0f;
		}
		preWindow = pStim0->fBegin;
	} else if (g_export_time_mode == TMODE_CH2) {
		if (!m_head.nOnCh2) {
			AfxMessageBox("Error: CH2 is not active.");
			return -1.0f;
		}
		preWindow = pStim1->fBegin;
	} else if (g_export_time_mode == TMODE_FIRST_ACTIVE) {
		if (m_head.nOnCh1) preWindow = pStim0->fBegin;
		else if (m_head.nOnCh2) preWindow = pStim1->fBegin;
		else {
			AfxMessageBox("Error: Neither channel is active.");
			return -1.0f;
		}
	}
	return preWindow;
}
