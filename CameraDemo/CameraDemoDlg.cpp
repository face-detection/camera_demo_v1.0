
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

#define DETECT_ENABLE		1
static CascadeClassifier FaceFrontalCascade;
static CascadeClassifier FaceProfileCascade;
static Mat ImageFace[FRAME_MAX];
static int ImageFaceNumber = 0;

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
	SetWindowText(_T("CameraDemo - v1.1"));
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
		for(int i=0; i<FRAME_MAX; ++i)
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
	m_pCapture = NULL;
	m_pVideoWriter = NULL;
	m_nTimer = 0;
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_VIDEO);
	m_pVideoDC = pWnd->GetDC();
	m_hVideoDC = m_pVideoDC->GetSafeHdc();
	pWnd->GetClientRect(&m_VideoRect);

	// capture region:
	m_nFrameIndex = 0;
	for(int i=0; i<FRAME_MAX; ++i)
	{
		pWnd = GetDlgItem(IDC_CAP0 + i);
		m_Frames[i].m_pVideoDC = pWnd->GetDC();
		m_Frames[i].m_hVideoDC = m_Frames[i].m_pVideoDC->GetSafeHdc();
		pWnd->GetClientRect(&m_Frames[i].m_VideoRect);
	}

	// load cascade:
	FaceFrontalCascade.load("xml\\lbpcascade_frontalface.xml");
	//m_FaceProfileCascade.load("xml\\lbpcascade_profileface.xml");

	// configure:
	GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(BST_UNCHECKED);

	// open camera:
	m_pCapture = cvCaptureFromCAM(0);
	if(!m_pCapture)
	{
		MessageBox(_T("can not open camera!"));
		return;
	}
}

void CCameraDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pCapture)
	{
		IplImage *m_Frame;
		m_Frame = cvQueryFrame(m_pCapture);
		if(!m_Frame)
		{
			if(!m_bCamera)
			{
				cvReleaseCapture(&m_pCapture);
				USES_CONVERSION;
				m_pCapture = cvCaptureFromAVI(W2A(m_cstrVideoPath));
			}
		}
		else
		{		
			//detect face:
#if (DETECT_ENABLE == 1)
			Mat GrayFrame;
			vector<Rect> Faces;
			Mat equalizedImg;
			Mat ReferenceFrame(m_Frame); 
			cvtColor(ReferenceFrame, GrayFrame, COLOR_RGB2GRAY);
			equalizeHist(GrayFrame, equalizedImg);
			FaceFrontalCascade.detectMultiScale(equalizedImg, Faces); // using default parameters...
			if(Faces.size())
			{
				for (size_t i = 0; i < Faces.size(); i++)
				{
					if(i < FRAME_MAX)
						ImageFace[i] = ReferenceFrame(Rect(Faces[i].x, Faces[i].y, Faces[i].width, Faces[i].height));
					rectangle(ReferenceFrame, Faces[i], Scalar(0,255,0), 2);
				}	
				ImageFaceNumber = Faces.size();
				if(ImageFaceNumber > FRAME_MAX)
					ImageFaceNumber = FRAME_MAX;
			}
			
#endif
			//draw:
			CvvImage m_CvvImage;
			m_CvvImage.CopyOf(m_Frame, 1);
			m_CvvImage.DrawToHDC(m_hVideoDC, &m_VideoRect);
			//check if to be saved:
			if(m_bSaveVideo)
			{
				cvWriteFrame(m_pVideoWriter, m_Frame);
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CCameraDemoDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cstr;
	GetDlgItemText(IDC_BUTTON_PLAY, cstr);
	if(!cstr.Compare(_T("START")))
	{
		if(!m_pCapture)
		{
			if(((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->GetCheck())
			{
				m_pCapture = cvCaptureFromCAM(0);
				if(!m_pCapture)
				{
					MessageBox(_T("can not open camera!"));
					return;
				}
			}
			else
			{
				m_pCapture = cvCaptureFromAVI("megamind.avi");
				if(!m_pCapture)
				{
					MessageBox(_T("can not open target video file!"));
					return;
				}
			}
		}
		if(m_nTimer == 0)
			m_nTimer = SetTimer(600, 20, NULL);
		//enable capture:
		GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(TRUE);
		//change text:
		SetDlgItemText(IDC_BUTTON_PLAY, _T("STOP"));
	}
	else if(!cstr.Compare(_T("STOP")))
	{
		if(m_nTimer)
		{
			KillTimer(m_nTimer);
			m_nTimer = 0;
		}
		GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(FALSE);
		//change text:
		SetDlgItemText(IDC_BUTTON_PLAY, _T("START"));
	}
}


void CCameraDemoDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pCapture)
	{
		cvReleaseCapture(&m_pCapture); 
	}
	if(m_pVideoWriter)
	{
		cvReleaseVideoWriter(&m_pVideoWriter);
	}
	CDialog::OnOK();
}


void CCameraDemoDlg::OnBnClickedRadioCamera()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bCamera)
		return;

	if(m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_CHECKED);
		return;
	}
	if(m_pCapture)
		cvReleaseCapture(&m_pCapture);
	m_pCapture = cvCaptureFromCAM(0);
	if(!m_pCapture)
	{
		MessageBox(_T("can not open camera!"));
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
	if(m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
		return;
	}
	if(m_pCapture)
		cvReleaseCapture(&m_pCapture);
	// select file:
	LPCTSTR lpszDefExt, lpszFilter;
	TCHAR CurDir[100];
	::GetCurrentDirectory(100,CurDir);
	lpszDefExt = _T(".avi");
	lpszFilter = _T("Video File (*.avi)|*.avi||");
	CFileDialog importDlg(TRUE , lpszDefExt,NULL, OFN_HIDEREADONLY, lpszFilter, NULL);
	int response = importDlg.DoModal();
	if(response == IDOK)
	{
		m_cstrVideoPath = importDlg.GetPathName();
		GetDlgItem(IDC_EDIT_VIDEO_PATH)->SetWindowText(m_cstrVideoPath);
		USES_CONVERSION;
		m_pCapture = cvCaptureFromAVI(W2A(m_cstrVideoPath));
		if(!m_pCapture)
		{
			MessageBox(_T("can not open target video file!"));
			goto _ERR;
		}
		else
		{
			GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(FALSE);
			m_bSaveVideo = FALSE;
			((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(BST_UNCHECKED);
		}
		m_bCamera = FALSE;
		::SetCurrentDirectory(CurDir); // restore to previous directory...
		return;
	}
_ERR:
	m_bCamera = TRUE;
	GetDlgItem(IDC_CHECK_SAVE)->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_RADIO_CAMERA))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_RADIO_FILE))->SetCheck(BST_UNCHECKED);
	::SetCurrentDirectory(CurDir); // restore to previous directory...
}


void CCameraDemoDlg::OnBnClickedButtonCapture()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_nTimer || !m_pCapture)
	{
		return;
	}

#if (DETECT_ENABLE == 1)
	
	IplImage frame_t;
	for(int i=0; i<ImageFaceNumber; ++i)
	{
		frame_t = ImageFace[i];
		m_Frames[m_nFrameIndex].m_CvvImage.CopyOf(&frame_t, 1);
		m_Frames[m_nFrameIndex].m_CvvImage.DrawToHDC(m_Frames[m_nFrameIndex].m_hVideoDC, &m_Frames[m_nFrameIndex].m_VideoRect);
		m_nFrameIndex++;
		if(m_nFrameIndex == FRAME_MAX)
			m_nFrameIndex = 0;
	}
#else
	IplImage *frame = cvQueryFrame(m_pCapture);
	m_Frames[m_nFrameIndex].m_CvvImage.CopyOf(frame, 1);
	m_Frames[m_nFrameIndex].m_CvvImage.DrawToHDC(m_Frames[m_nFrameIndex].m_hVideoDC, &m_Frames[m_nFrameIndex].m_VideoRect);
	m_nFrameIndex++;
	if(m_nFrameIndex == FRAME_MAX)
		m_nFrameIndex = 0;
#endif
}


void CCameraDemoDlg::OnBnClickedCheckSave()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_nTimer)
	{
		MessageBox(_T("Please stop video first!"));
		((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(m_bSaveVideo ? BST_CHECKED : BST_UNCHECKED);
		return;
	}
	if(!m_pCapture)
	{
		MessageBox(_T("video capture not created yet!"));
		((CButton*)GetDlgItem(IDC_CHECK_SAVE))->SetCheck(m_bSaveVideo ? BST_CHECKED : BST_UNCHECKED);
		return;
	}	
	m_bSaveVideo = ((CButton*)GetDlgItem(IDC_CHECK_SAVE))->GetCheck() ? TRUE : FALSE;
	if(!m_bSaveVideo)
		return;

	// select file:
	LPCTSTR lpszDefExt, lpszFilter;
	TCHAR CurDir[100];
	::GetCurrentDirectory(100,CurDir);
	lpszDefExt = _T(".avi");
	lpszFilter = _T("Video File (*.avi)|*.avi||");
	CFileDialog importDlg(FALSE , lpszDefExt,NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, lpszFilter, NULL);
	int response = importDlg.DoModal();
	if(response == IDOK)
	{
		m_cstrSavePath = importDlg.GetPathName();
		GetDlgItem(IDC_EDIT_SAVE_PATH)->SetWindowText(m_cstrSavePath);
		// create writer:
		IplImage *m_Frame;
		m_Frame = cvQueryFrame(m_pCapture); 
		if(m_pVideoWriter)
			cvReleaseVideoWriter(&m_pVideoWriter);
		USES_CONVERSION;
		m_pVideoWriter = cvCreateVideoWriter(W2A(m_cstrSavePath), CV_FOURCC('x', 'v', 'I', 'D'), 25, cvSize(m_Frame->width, m_Frame->height));
	}
	::SetCurrentDirectory(CurDir); // restore to previous directory...
}
