
// CameraDemoDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
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


// CCameraDemoDlg �Ի���




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


// CCameraDemoDlg ��Ϣ�������

BOOL CCameraDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetWindowText(_T("CameraDemo - v1.2.1"));
	Init();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCameraDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	m_FaceFrontalCascade.load(W2A(FRONTAL_FACE_XML_FILE));
	if (m_FaceFrontalCascade.empty())
	{
		MessageBox(_T("Error: can not load \"%s\""), (FRONTAL_FACE_XML_FILE));
		return;
	}
	m_FaceProfileCascade.load(W2A(PROFILE_FACE_XML_FILE));
	if (m_FaceProfileCascade.empty())
	{
		MessageBox(_T("Error: can not load \"%s\""), PROFILE_FACE_XML_FILE);
		return;
	}
}

void CCameraDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_nTimer == nIDEvent && m_VideoCap.isOpened())
	{
		Mat MatFrame;
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
			//detect face:
			Mat GrayFrame, EqualizedFrame;
			vector<Rect> Faces;
			cvtColor(MatFrame, GrayFrame, cv::COLOR_RGB2GRAY);
			equalizeHist(GrayFrame, EqualizedFrame);
			if (EqualizedFrame.empty())
				return;
			// Front face:
			Faces.clear();
			m_FaceFrontalCascade.detectMultiScale(EqualizedFrame, Faces, SCALE_FACTOR, MIN_NEIGHBOR, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_WIDTH, MIN_FACE_HEIGHT), Size(MAX_FACE_WIDTH, MAX_FACE_HEIGHT)); // using default parameters...
			if (Faces.size())
			{
				for (size_t i = 0; i < Faces.size(); i++)
				{
					if (i < FRAME_MAX)
						m_ImageFace[i] = MatFrame(Rect(Faces[i].x, Faces[i].y, Faces[i].width, Faces[i].height));
					rectangle(MatFrame, Faces[i], Scalar(0, 255, 0), 2);
				}
				m_ImageFaceNumber = Faces.size();
				if (m_ImageFaceNumber > FRAME_MAX)
					m_ImageFaceNumber = FRAME_MAX;
			}
			// profile face:
			Faces.clear();
			m_FaceProfileCascade.detectMultiScale(EqualizedFrame, Faces, SCALE_FACTOR, MIN_NEIGHBOR, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_WIDTH, MIN_FACE_HEIGHT), Size(MAX_FACE_WIDTH, MAX_FACE_HEIGHT)); // using default parameters...
			if (Faces.size())
			{
				for (size_t i = 0; i < Faces.size(); i++)
				{
					if (i < FRAME_MAX)
						m_ImageFace[i] = MatFrame(Rect(Faces[i].x, Faces[i].y, Faces[i].width, Faces[i].height));
					rectangle(MatFrame, Faces[i], Scalar(255, 0, 0), 2);
				}
				m_ImageFaceNumber = Faces.size();
				if (m_ImageFaceNumber > FRAME_MAX)
					m_ImageFaceNumber = FRAME_MAX;
			}
#endif
			//draw:
			IplImage IplFrame = MatFrame;
			CvvImage CvvFrame;
			CvvFrame.CopyOf(&IplFrame, 1);
			CvvFrame.DrawToHDC(m_hVideoDC, &m_VideoRect);
			//check if to be saved:
			if (m_bSaveVideo)
			{
				cvWriteFrame(m_pVideoWriter, &IplFrame);
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CCameraDemoDlg::OnBnClickedButtonPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(TRUE);
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
		GetDlgItem(IDC_BUTTON_CAPTURE)->EnableWindow(FALSE);
		//change text:
		SetDlgItemText(IDC_BUTTON_PLAY, _T("START"));
	}
}


void CCameraDemoDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
