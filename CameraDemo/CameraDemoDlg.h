
// CameraDemoDlg.h : ͷ�ļ�
//
#pragma once

struct frame_container{
	CRect m_VideoRect;
	CDC *m_pVideoDC;
	HDC m_hVideoDC;
	CvvImage m_CvvImage;
};
#define FRAME_MAX		3

// CCameraDemoDlg �Ի���
class CCameraDemoDlg : public CDialogEx
{
// ����
public:
	CCameraDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CAMERADEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	virtual void OnOK() {}
	virtual void OnCancel() {}

// ʵ��
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

	// ���ɵ���Ϣӳ�亯��
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
