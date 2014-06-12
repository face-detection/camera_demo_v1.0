
// CameraDemoDlg.h : 头文件
//
#pragma once


struct frame_container{
	CRect m_VideoRect;
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
	enum { MAX_FACES = 3 };	// detect and track max 3 faces...
	enum { MIN_KEY_POINTS = 10 }; // when keypoints < MIN_KEY_POINTS, so re-init...

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void OnOK() {}
	virtual void OnCancel() {}

	// 实现
protected:
	HICON m_hIcon;
	// for playing main video:
	VideoCapture m_VideoCap;
	CRect m_VideoRect;
	HDC m_hVideoDC;
	UINT_PTR m_nTimer;
	// additional function:
	CString m_cstrVideoPath, m_cstrSavePath;
	BOOL m_bSaveVideo, m_bCamera;
	struct frame_container m_Frames[FRAME_MAX];
	int m_nFrameIndex;
	CvVideoWriter* m_pVideoWriter;
	// face detection helper:
	CascadeClassifier m_FaceFrontalCascade;
	CascadeClassifier m_FaceProfileCascade;
	Mat m_ImageFace[FRAME_MAX];
	int m_ImageFaceNumber;
	// initial routines:
	void Init();
	// feature detector:
	GoodFeaturesToTrackDetector m_FeatureDetector; // using GRIF Detector
	vector<Mat> m_MaskROIs;
	Mat m_PrevGrayFrame;
	BOOL m_bNeedInit;
	RNG m_RNG;
	vector<Point2f> m_OldKeyPoints[MAX_FACES];
	vector<Point2f> m_CurKeyPoints[MAX_FACES];
	vector<Point2f> m_OldFaceCorners[MAX_FACES];
	vector<Point2f> m_ScreenCorners;
	BOOL _faces_overlap(Rect& rect, vector<Rect>& target);
	BOOL _find_mask_roi(Mat& GrayFrame, vector<Rect>& Faces);
	void _render_frame(Mat& MatFrame, HDC& m_hVideoDC, CRect& m_VideoRect);
	void _save_frame(CvVideoWriter* m_pVideoWriter, Mat& MatFrame);
	void _set_radio_sel(UINT sel);

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
