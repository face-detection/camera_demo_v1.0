
// CameraDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CameraDemo.h"
#include "CameraDemoDlg.h"
#include "afxdialogex.h"
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// control if enable face detection...
#define DETECT_ENABLE		1

// xml file:
static const TCHAR* FRONTAL_FACE_XML_FILE = _T("./xml/haarcascade_frontalface_default.xml");
static const TCHAR* PROFILE_FACE_XML_FILE = _T("./xml/haarcascade_profileface.xml");

// detect parameters:
#define SCALE_FACTOR		(1.1)
#define MIN_NEIGHBOR		3
#define MIN_FACE_WIDTH		100
#define MIN_FACE_HEIGHT		100
#define MAX_FACE_WIDTH		2000
#define MAX_FACE_HEIGHT		2000

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCameraDemoDlg 对话框




CCameraDemoDlg::CCameraDemoDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CCameraDemoDlg::IDD, pParent)
, m_FeatureDetector(400, 0.01, 1.0, 3, true, 0.04)
, m_RNG(12345)
, m_bNeedInit(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCameraDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCameraDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CCameraDemoDlg::OnBnClickedButtonPlay)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_CAMERA, &CCameraDemoDlg::OnBnClickedRadioCamera)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CCameraDemoDlg::OnBnClickedRadioFile)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CCameraDemoDlg::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_CHECK_SAVE, &CCameraDemoDlg::OnBnClickedCheckSave)
END_MESSAGE_MAP()


// CCameraDemoDlg 消息处理程序

BOOL CCameraDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetWindowText(_T("CameraDemo - v2.0"));
	Init();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCameraDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCameraDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// re-draw captured frame:
		for (int i = 0; i < FRAME_MAX; ++i)
			m_Frames[i].m_CvvImage.DrawToHDC(m_Frames[i].m_hVideoDC, &m_Frames[i].m_VideoRect);
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCameraDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCameraDemoDlg::Init()
{
	// init parameters:
	m_bSaveVideo = FALSE;
	m_bCamera = TRUE;
	m_pVideoWriter = NULL;
	m_nTimer = 0;
	CWnd *pWnd = GetDlgItem(IDC_VIDEO);
	CDC *pVideoDC = pWnd->GetDC();
	m_hVideoDC = pVideoDC->GetSafeHdc();
	pWnd->GetClientRect(&m_VideoRect);

	// capture region:
	m_nFrameIndex = 0;
	for (int i = 0; i < FRAME_MAX; ++i)
	{
		pWnd = GetDlgItem(IDC_CAP0 + i);
		pVideoDC = pWnd->GetDC();
		m_Frames[i].m_hVideoDC = pVideoDC->GetSafeHdc();
		pWnd->GetClientRect(&m_Frames[i].m_VideoRect);
	}

	// configure:
	GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(BST_UNCHECKED);

	// open camera:
	m_VideoCap.open(0);
	if (!m_VideoCap.isOpened())
	{
		MessageBox(_T("Error: can not open camera!"));
		return;
	}

	// load cascade:
	m_ImageFaceNumber = 0;
	USES_CONVERSION;
	CString cstr;
	m_FaceFrontalCascade.load(W2A(FRONTAL_FACE_XML_FILE));
	if (m_FaceFrontalCascade.empty())
	{
		cstr.Format(_T("Error: can not load \"%s\""), (FRONTAL_FACE_XML_FILE));
		MessageBox(cstr);
		return;
	}
	m_FaceProfileCascade.load(W2A(PROFILE_FACE_XML_FILE));
	if (m_FaceProfileCascade.empty())
	{
		cstr.Format(_T("Error: can not load \"%s\""), PROFILE_FACE_XML_FILE);
		MessageBox(cstr);
		return;
	}

	// feature detector:
	m_MaskROIs.clear();
	size_t screen_width = (size_t)m_VideoCap.get(CV_CAP_PROP_FRAME_WIDTH);
	size_t screen_height = (size_t)m_VideoCap.get(CV_CAP_PROP_FRAME_HEIGHT);
	m_ScreenCorners.clear();
	m_ScreenCorners.push_back(cvPoint(0, 0));
	m_ScreenCorners.push_back(cvPoint(screen_width, 0));
	m_ScreenCorners.push_back(cvPoint(screen_width, screen_height));
	m_ScreenCorners.push_back(cvPoint(0, screen_height));
}

void CCameraDemoDlg::_render_frame(Mat& MatFrame, HDC& m_hVideoDC, CRect& m_VideoRect)
{
	IplImage IplFrame = MatFrame;
	CvvImage CvvFrame;
	CvvFrame.CopyOf(&IplFrame, 1);
	CvvFrame.DrawToHDC(m_hVideoDC, &m_VideoRect);
}

void CCameraDemoDlg::_save_frame(CvVideoWriter* m_pVideoWriter, Mat& MatFrame)
{
	IplImage IplFrame = MatFrame;
	cvWriteFrame(m_pVideoWriter, &IplFrame);
}

BOOL CCameraDemoDlg::_faces_overlap(Rect& rect, vector<Rect>& target)
{

	for (size_t i = 0; i < target.size(); ++i)
	{
		if (rect.x > target[i].x + target[i].width)
			continue;
		if (rect.y > target[i].y + target[i].height)
			continue;
		if (rect.x + rect.width < target[i].x)
			continue;
		if (rect.y + rect.height < target[i].y)
			continue;
		return TRUE;
	}
	return FALSE;
}

BOOL CCameraDemoDlg::_find_mask_roi(Mat& GrayFrame, vector<Rect>& Faces)
{
	//detect face:
	Mat roi_t;
	Point center;
	Mat EqualizedFrame;
	equalizeHist(GrayFrame, EqualizedFrame);
	if (EqualizedFrame.empty())
		return FALSE;
	m_MaskROIs.clear();
	Faces.clear();
	// Front face:
	m_FaceFrontalCascade.detectMultiScale(EqualizedFrame, Faces, SCALE_FACTOR, MIN_NEIGHBOR, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_WIDTH, MIN_FACE_HEIGHT), Size(MAX_FACE_WIDTH, MAX_FACE_HEIGHT)); // using default parameters...
	if (Faces.size())
	{
		// create mask frame:
		size_t faces = Faces.size();
		if (faces > MAX_FACES)
			faces = MAX_FACES;
		for (size_t i = 0; i < faces; ++i)
		{
			// calculate the center: 	
			roi_t = Mat::zeros(GrayFrame.size(), CV_8U);
			center = Point(Faces[i].x + Faces[i].width / 2, Faces[i].y + Faces[i].height / 2);
			ellipse(roi_t, center, Size(Faces[i].width / 3, Faces[i].height / 3), 0, 0, 360, Scalar(255, 0, 255), -1, 8, 0);
			m_MaskROIs.push_back(roi_t);
		}
	}

	// check if reach max:
	if (m_MaskROIs.size() >= MAX_FACES)
		return TRUE;

	///*
	// profile face:
	vector<Rect> Faces_t;
	m_FaceProfileCascade.detectMultiScale(EqualizedFrame, Faces_t, SCALE_FACTOR, MIN_NEIGHBOR, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_WIDTH, MIN_FACE_HEIGHT), Size(MAX_FACE_WIDTH, MAX_FACE_HEIGHT)); // using default parameters...
	if (Faces_t.size())
	{
		// create mask frame:
		size_t faces = Faces_t.size();
		for (size_t i = 0; i < faces; ++i)
		{
			// check if overlap:
			if (_faces_overlap(Faces_t[i], Faces))
			continue;

			// calculate the center:
			roi_t = Mat::zeros(GrayFrame.size(), CV_8U);
			center = Point(Faces_t[i].x + Faces_t[i].width / 2, Faces_t[i].y + Faces_t[i].height / 2);
			ellipse(roi_t, center, Size(Faces_t[i].width / 3, Faces_t[i].height / 3), 0, 0, 360, Scalar(255, 0, 255), -1, 8, 0);
			m_MaskROIs.push_back(roi_t);
			// store faces vectors:
			Faces.push_back(Faces_t[i]);
			if (Faces.size() >= MAX_FACES)
				break;
		}
	}
	//*/
	// if find ROIs:
	if (m_MaskROIs.size())
		return TRUE;
	else
		return FALSE;
}

void CCameraDemoDlg::_set_radio_sel(UINT sel)
{
	for (UINT i = 0; i < 3; ++i)
	{
		((CButton*)GetDlgItem(IDC_RADIO_F0 + i))->SetCheck((i == sel) ? BST_CHECKED : BST_UNCHECKED);
	}
}

void CCameraDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_nTimer == nIDEvent && m_VideoCap.isOpened())
	{
		Mat MatFrame, GrayFrame;
		m_VideoCap >> MatFrame; // read frame
		if (MatFrame.empty())
		{
			if (!m_bCamera)
			{
				// when playing file, we restart from beginning...
				m_VideoCap.release();
				USES_CONVERSION;
				m_VideoCap.open(W2A(m_cstrVideoPath));
			}
		}
		else
		{
#if (DETECT_ENABLE == 1)
			// conver to gray frame:
			cvtColor(MatFrame, GrayFrame, cv::COLOR_RGB2GRAY);

			if (m_bNeedInit)
			{
				// we find face ROIs first:
				vector<Rect> Faces;
				if (_find_mask_roi(GrayFrame, Faces))
				{
					vector<KeyPoint> keypoints;
					// detect keypoints:
					for (size_t i = 0; i < m_MaskROIs.size(); ++i)
					{
						// detect keypoints within face ROI:
						m_FeatureDetector.detect(MatFrame, keypoints, m_MaskROIs[i]);
						// conver keypoints:
						KeyPoint::convert(keypoints, m_CurKeyPoints[i]);
						//draw init rectangle:
						rectangle(MatFrame, Faces[i], Scalar(0, 255, 0), 2, 8);
						//auto show captured faces:
						m_ImageFace[m_nFrameIndex] = MatFrame(Rect(Faces[i].x, Faces[i].y, Faces[i].width, Faces[i].height));
						_render_frame(m_ImageFace[m_nFrameIndex], m_Frames[m_nFrameIndex].m_hVideoDC, m_Frames[m_nFrameIndex].m_VideoRect);
						_set_radio_sel(m_nFrameIndex);
						m_nFrameIndex++;
						if (m_nFrameIndex == FRAME_MAX)
							m_nFrameIndex = 0;
						//record corners:
						m_OldFaceCorners[i].clear(); // important!!!
						m_OldFaceCorners[i].push_back(cvPoint(Faces[i].x, Faces[i].y));
						m_OldFaceCorners[i].push_back(cvPoint(Faces[i].x + Faces[i].width, Faces[i].y));
						m_OldFaceCorners[i].push_back(cvPoint(Faces[i].x + Faces[i].width, Faces[i].y + Faces[i].height));
						m_OldFaceCorners[i].push_back(cvPoint(Faces[i].x, Faces[i].y + Faces[i].height));
					}
					// only tracking...
					m_bNeedInit = FALSE;
				}
			}
			else // if init complete, keeping tracking...
			{
				for (size_t face_index = 0; face_index < m_MaskROIs.size(); ++face_index)
				{
					if (m_OldKeyPoints[face_index].empty())
						continue;

					// track keypoints:
					vector<uchar> status;
					vector<float> err;
					if (m_PrevGrayFrame.empty())
						GrayFrame.copyTo(m_PrevGrayFrame);
					m_CurKeyPoints[face_index].clear();
					cv::calcOpticalFlowPyrLK(m_PrevGrayFrame, GrayFrame, m_OldKeyPoints[face_index], m_CurKeyPoints[face_index], status, err, Size(21, 21), 2);
					size_t i, k;
					for (i = k = 0; i < m_CurKeyPoints[face_index].size(); i++)
					{
						// remove invalid points:
						if (!status[i])
							continue;

						// remove out of range points:
						if (cv::pointPolygonTest(m_OldFaceCorners[face_index], m_CurKeyPoints[face_index][i], false) < 0)
							continue;

						m_CurKeyPoints[face_index][k] = m_CurKeyPoints[face_index][i];
						m_OldKeyPoints[face_index][k] = m_OldKeyPoints[face_index][i];
						k++;
						circle(MatFrame, m_CurKeyPoints[face_index][i], 3,
							Scalar(m_RNG.uniform(0, 255), m_RNG.uniform(0, 255), m_RNG.uniform(0, 255)), 1, 8);
					}
					m_CurKeyPoints[face_index].resize(k);
					m_OldKeyPoints[face_index].resize(k);

					// affine transform:
					size_t size = k;
					vector<Point2f> cur_corners(4);
					if (size >= MIN_KEY_POINTS)
					{
						vector<Point2f> old_points_t;
						vector<Point2f> cur_points_t;

						for (size_t i = 0; i < 3; ++i)
						{
							old_points_t.push_back(m_OldKeyPoints[face_index][i]);
							cur_points_t.push_back(m_CurKeyPoints[face_index][i]);
						}
						// calculate tranform metrix:
						Mat H = cv::getAffineTransform(old_points_t, cur_points_t);
						cv::transform(m_OldFaceCorners[face_index], cur_corners, H);

						// judge if in screen:
						int number_gate = 0;
						for (size_t i = 0; i < 4; ++i)
						{
							if (cv::pointPolygonTest(m_ScreenCorners, cur_corners[i], false) < 0)
								number_gate++;
						}
						if (number_gate >= 2) // when more than 2 corners out of screen, re-init:
						{
							m_OldKeyPoints[face_index].clear();
							m_CurKeyPoints[face_index].clear();
							m_bNeedInit = TRUE;
						}
						else
						{
							//-- Draw lines between the corners (the mapped object in the scene - image_2 )
							cv::line(MatFrame, cur_corners[0], cur_corners[1], Scalar(0, 255, 0), 2);
							cv::line(MatFrame, cur_corners[1], cur_corners[2], Scalar(0, 255, 0), 2);
							cv::line(MatFrame, cur_corners[2], cur_corners[3], Scalar(0, 255, 0), 2);
							cv::line(MatFrame, cur_corners[3], cur_corners[0], Scalar(0, 255, 0), 2);

							// update previous cornners:
							m_OldFaceCorners[face_index][0] = cur_corners[0];
							m_OldFaceCorners[face_index][1] = cur_corners[1];
							m_OldFaceCorners[face_index][2] = cur_corners[2];
							m_OldFaceCorners[face_index][3] = cur_corners[3];
						}
					}
				}
			}

			// update previous frame:
			cv::swap(m_PrevGrayFrame, GrayFrame);
			if (m_MaskROIs.size() == 0)
			{
				m_bNeedInit = true;
			}
			if (!m_bNeedInit)
			{
				for (size_t face_index = 0; face_index < m_MaskROIs.size(); ++face_index)
				{
					if (m_CurKeyPoints[face_index].size() < MIN_KEY_POINTS)
					{
						m_bNeedInit = true;
						m_OldKeyPoints[face_index].clear();
					}
					else
					{
						std::swap(m_CurKeyPoints[face_index], m_OldKeyPoints[face_index]);
					}
				}
			}
#endif // #if (DETECT_ENABLE == 1)

			// show frame:
			_render_frame(MatFrame, m_hVideoDC, m_VideoRect);

			//check if to be saved:
			if (m_bSaveVideo)
				_save_frame(m_pVideoWriter, MatFrame);

		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CCameraDemoDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cstr;
	GetDlgItemText(IDC_BUTTON_PLAY, cstr);
	if (!cstr.Compare(_T("START")))
	{
		if (!m_VideoCap.isOpened())
		{
			if (((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->GetCheck())
			{
				// open camera:
				m_VideoCap.open(0);
				if (!m_VideoCap.isOpened())
				{
					MessageBox(_T("Error: can not open camera!"));
					return;
				}
			}
			else
			{
				USES_CONVERSION;
				m_VideoCap.open(W2A(m_cstrVideoPath));
				if (!m_VideoCap.isOpened())
				{
					MessageBox(_T("Error: can not open video file!"));
					return;
				}
			}
		}
		// ok, it's time to play...
		if (m_nTimer == 0)
			m_nTimer = SetTimer(600, 16, NULL);
		//enable capture:
		//GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(TRUE);
		//change text:
		SetDlgItemText(IDC_BUTTON_PLAY, _T("STOP"));
	}
	else if (!cstr.Compare(_T("STOP")))
	{
		if (m_nTimer)
		{
			KillTimer(m_nTimer);
			m_nTimer = 0;
		}
		//GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(FALSE);
		//change text:
		SetDlgItemText(IDC_BUTTON_PLAY, _T("START"));
	}
}


void CCameraDemoDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_VideoCap.isOpened())
	{
		m_VideoCap.release();
	}
	if (m_pVideoWriter)
	{
		cvReleaseVideoWriter(&m_pVideoWriter);
	}
	CDialog::OnOK();
}


void CCameraDemoDlg::OnBnClickedRadioCamera()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bCamera)
		return;

	if (m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_CHECKED);
		return;
	}
	if (m_VideoCap.isOpened())
	{
		m_VideoCap.release();
	}
	// open camera:
	m_VideoCap.open(0);
	if (!m_VideoCap.isOpened())
	{
		MessageBox(_T("Error: can not open camera!"));
		((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(TRUE);
		m_bCamera = TRUE;
	}
}


void CCameraDemoDlg::OnBnClickedRadioFile()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
		return;
	}
	if (m_VideoCap.isOpened())
	{
		m_VideoCap.release();
	}
	// select file:
	LPCTSTR lpszDefExt, lpszFilter;
	lpszDefExt = _T(".avi");
	lpszFilter = _T("Video File (*.avi)|*.avi||");
	CFileDialog importDlg(TRUE, lpszDefExt, NULL, OFN_HIDEREADONLY, lpszFilter, NULL);
	int response = importDlg.DoModal();
	if (response == IDOK)
	{
		m_cstrVideoPath = importDlg.GetPathName();
		GetDlgItem(IDC_EDIT_VIDEO_PATH)->SetWindowText(m_cstrVideoPath);
		USES_CONVERSION;
		m_VideoCap.open(W2A(m_cstrVideoPath));
		if (!m_VideoCap.isOpened())
		{
			MessageBox(_T("Error: can not open video file!"));
			goto _ERR;
		}
		else
		{
			GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(FALSE);
			m_bSaveVideo = FALSE;
			((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(BST_UNCHECKED);
		}
		m_bCamera = FALSE;
		return;
	}
_ERR:
	m_bCamera = TRUE;
	GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
}


void CCameraDemoDlg::OnBnClickedButtonCapture()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_nTimer || !m_VideoCap.isOpened())
	{
		return;
	}

#if (DETECT_ENABLE == 1)
	IplImage IplFrame;
	for (int i = 0; i < m_ImageFaceNumber; ++i)
	{
		IplFrame = m_ImageFace[i];
		m_Frames[m_nFrameIndex].m_CvvImage.CopyOf(&IplFrame, 1);
		m_Frames[m_nFrameIndex].m_CvvImage.DrawToHDC(m_Frames[m_nFrameIndex].m_hVideoDC, &m_Frames[m_nFrameIndex].m_VideoRect);
		m_nFrameIndex++;
		if (m_nFrameIndex == FRAME_MAX)
			m_nFrameIndex = 0;
	}
#else
	Mat MatFrame;
	m_VideoCap >> MatFrame;
	IplImage IplFrame = MatFrame;
	m_Frames[m_nFrameIndex].m_CvvImage.CopyOf(&IplFrame, 1);
	m_Frames[m_nFrameIndex].m_CvvImage.DrawToHDC(m_Frames[m_nFrameIndex].m_hVideoDC, &m_Frames[m_nFrameIndex].m_VideoRect);
	m_nFrameIndex++;
	if (m_nFrameIndex == FRAME_MAX)
		m_nFrameIndex = 0;
#endif
}


void CCameraDemoDlg::OnBnClickedCheckSave()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(m_bSaveVideo ? BST_CHECKED : BST_UNCHECKED);
		return;
	}
	if (!m_VideoCap.isOpened())
	{
		MessageBox(_T("Warning: video capture not opened yet!"));
		((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(m_bSaveVideo ? BST_CHECKED : BST_UNCHECKED);
		return;
	}
	m_bSaveVideo = ((CButton*)GetDlgItem(IDC_CHECK_SAVE))->GetCheck() ? TRUE : FALSE;
	if (!m_bSaveVideo)
		return;

	// select file:
	LPCTSTR lpszDefExt, lpszFilter;
	lpszDefExt = _T(".avi");
	lpszFilter = _T("Video File (*.avi)|*.avi||");
	CFileDialog importDlg(FALSE, lpszDefExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, lpszFilter, NULL);
	int response = importDlg.DoModal();
	if (response == IDOK)
	{
		m_cstrSavePath = importDlg.GetPathName();
		GetDlgItem(IDC_EDIT_SAVE_PATH)->SetWindowText(m_cstrSavePath);
		// create writer:
		Mat MatFrame;
		m_VideoCap >> MatFrame;
		IplImage IplFrame = MatFrame;
		if (m_pVideoWriter)
			cvReleaseVideoWriter(&m_pVideoWriter);
		USES_CONVERSION;
		m_pVideoWriter = cvCreateVideoWriter(W2A(m_cstrSavePath), CV_FOURCC('x', 'v', 'I', 'D'), 25, cvSize(IplFrame.width, IplFrame.height));
	}
}

