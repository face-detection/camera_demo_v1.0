
// CameraDemoDlg.h : 头文件
//
#pragma once

struct frame_container{
	CRect m_VideoRect;
	CDC *m_pVideoDC;
	HDC m_hVideoDC;
	CvvImage m_CvvImage;
};
#define FRAME_MAX		3

// CCameraDemoDlg 对话框
class CCameraDemoDlg : public CDialogEx
{
// 构造
public:
	CCameraDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CAMERADEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void OnOK() {}
	virtual void OnCancel() {}

// 实现
protected:
	HICON m_hIcon;
	CvCapture *m_pCapture;
	CRect m_VideoRect;
	CDC *m_pVideoDC;
	HDC m_hVideoDC;
	UINT_PTR m_nTimer;
	CString m_cstrVideoPath, m_cstrSavePath;
	BOOL m_bSaveVideo, m_bCamera;
	struct frame_container m_Frames[FRAME_MAX];
	int m_nFrameIndex;
	CvVideoWriter* m_pVideoWriter;
	void Init();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnClose();
	afx_msg void OnBnClickedRadioCamera();
	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedCheckSave();
};
