// spikeDoc.h : Schnittstelle der Klasse CSpikeDoc
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPIKEDOC_H__B37A511B_568C_11D1_91CC_00A0244F3C96__INCLUDED_)
#define AFX_SPIKEDOC_H__B37A511B_568C_11D1_91CC_00A0244F3C96__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "defines.h"
#include "header60.h"
#include "header50.h"
#include "header40.h"
#include "header30.h"


class CSpikeDoc : public CDocument
{
protected: // Nur aus Serialisierung erzeugen
	CSpikeDoc();
	DECLARE_DYNCREATE(CSpikeDoc)

// Attribute
public:
	char		m_cHeadId[12];
	HEADER60	m_head;				// for storing header from spike file
	HEADER50	m_head_v50;			// for storing header from spike file
	HEADER40	m_head_v40;			// for storing header from spike file
	HEADER30	m_head_v30;			// for storing header from spike file
	CWordArray	m_wDataArray;		// for storing data from spike file
	CDWordArray	m_dwDataArray;		// for storing data from spike file
	CWordArray	m_wHistoArray;		// for storing histogram
	CArray<SPIKESTRUCT, SPIKESTRUCT> m_spikeArray;	// for storing spikes
	CArray<STIM, STIM> m_stimArrayCh1;	// for storing stimuli
	CArray<STIM, STIM> m_stimArrayCh2;	// for storing stimuli
	int			m_nActualPasses[MAX_SWEEPS];	// how many passes for a given sweep

//protected:
	BOOL	m_bSpikeFile;
	CString	m_strFileName;

// Operationen
public:
	void SetHeader();
	void ConvertOldHead();
	BOOL MakeName();
	BOOL Exist(LPCTSTR lpszFileName);
	BOOL IsOpen(LPCTSTR lpszFileName);
	BOOL GetData();
	BOOL CalcHistogram(int nBegin, int nEnd);
	void SetFirstStimCh1(CStimulusPage* pPgStim);
	void SetFirstStimCh2(CStimulusPage* pPgStim);
	void SetDeltaStimCh1(CStimulusPage* pPgStim);
	void SetDeltaStimCh2(CStimulusPage* pPgStim);
	void CalcStimuliCh1();
	void CalcStimuliCh2();

// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CSpikeDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementierung
public:
	void FileWarExport();
	void FileExcelExport();
	void FileExport();
	void RedrawSpikeView();
	virtual ~CSpikeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generierte Message-Map-Funktionen
protected:
	//{{AFX_MSG(CSpikeDoc)
	afx_msg void OnFileExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	float CalcPreWindow(STIM *pStim0, STIM *pStim1);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // !defined(AFX_SPIKEDOC_H__B37A511B_568C_11D1_91CC_00A0244F3C96__INCLUDED_)
