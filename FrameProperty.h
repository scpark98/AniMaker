#pragma once
#include "afxdialogex.h"
#include "Common/CEdit/SCEdit/SCEdit.h"

// CFrameProperty 대화 상자

class CFrameProperty : public CDialogEx
{
	DECLARE_DYNAMIC(CFrameProperty)

	int			m_frame_delay;

public:
	CFrameProperty(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CFrameProperty();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRAME_PROPERTY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CSCEdit m_edit_delay;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
