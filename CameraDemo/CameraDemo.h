
// CameraDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCameraDemoApp:
// �йش����ʵ�֣������ CameraDemo.cpp
//

class CCameraDemoApp : public CWinApp
{
public:
	CCameraDemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCameraDemoApp theApp;