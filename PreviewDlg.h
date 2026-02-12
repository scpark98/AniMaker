#pragma once
#include "afxdialogex.h"

#include "Common/ResizeCtrl.h"
#include "Common/CDialog/SCD2ImageDlg/SCD2ImageDlg.h"

// CPreviewDlg 대화 상자

class CPreviewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPreviewDlg)

	CResizeCtrl		m_resize;
	CSCD2ImageDlg	m_imgDlg;

	void			set_image();
public:
	CPreviewDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CPreviewDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PREVIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
};
