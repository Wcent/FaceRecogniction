// Face Recognition.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

/*****************************************************************************************/
#include <commdlg.h> // common dialogs
#include <vfw.h> // vedio for window library
#pragma comment(lib, "vfw32.lib") // ���linkʱproject����lib����
#include "image.h"
/*****************************************************************************************/

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

/*****************************************************************************************
�ҵ�ȫ�ֱ�������

*****************************************************************************************/
HWND hMainWnd; // handle of main window
HWND hwndCap; // handle of capture window
CAPDRIVERCAPS capDrvCaps; // driver capabilities
bool isRecognition = false; // flag set if is recognizing face
bool isThreadEnd = true;
char curDir[1024]; // current dirtory
char facebasePath[1024]; // facebase abbsolute path
char ImagePath[1024];


// face recognition thread procedure
DWORD WINAPI RecognitionThreadProc(LPVOID lParam);
//����frame callback������frame available����
LRESULT PASCAL FrameCallbackProc(HWND hwnd, LPVIDEOHDR lpVHdr);
//ѡ��Ŀ��ͼ���������ʶ��
void SearchFace();


// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FACERECOGNITION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_FACERECOGNITION);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_FACERECOGNITION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_FACERECOGNITION;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_FACERECOGNITION:
			isRecognition = true;
			// �õ�������ɫCb��Cr�Աȿ�cbcr[cb][cr]
			if ( !FaceCbcrProc() )
				break;

			// Register a frame callback function with the capture window
			capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
			break;

		case IDM_ENTERINGFACE:
			isRecognition = false;
			// �õ�������ɫCb��Cr�Աȿ�cbcr[cb][cr]
			if ( !FaceCbcrProc() )
				break;

			// Register a frame callback function with the capture window
			capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
			break;

		case IDM_DELETE:
			// �õ�������ɫCb��Cr�Աȿ�cbcr[cb][cr]
			if ( !FaceCbcrProc() )
				break;

			DeleteFace();
			break;

		case IDM_SEARCHFACE:
			SearchFace();
			break;

		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
	case WM_CREATE:
		hWinDC = GetDC(hWnd);
		hMainWnd = hWnd;
		// �õ���ǰĿ¼
		GetCurrentDirectory(1024, curDir);

		// ����capture���ڣ�������Ƶ
		hwndCap = capCreateCaptureWindow(
								(LPSTR)"My Capture Window",
								WS_CHILD | WS_VISIBLE,
								0, 0, 640, 480,
								(HWND)hWnd,
								(int)1);

		// Register a frame callback function with the capture window
	//	capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
			
		// connect to the driver 0(NO.1)
		SendMessage(hwndCap, WM_CAP_DRIVER_CONNECT, 0, 0L);
		// update the driver capabilities
		SendMessage(hwndCap, WM_CAP_DRIVER_GET_CAPS,
							sizeof(CAPDRIVERCAPS), (LONG)(LPVOID)&capDrvCaps);
		// set preview rate to 66 milliseconds
		capPreviewRate(hwndCap, 66);
		// start preview vedio
		capPreview(hwndCap, TRUE);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;

	case WM_DESTROY:
			// disable frame callback function
			capSetCallbackOnFrame(hwndCap, NULL);
			// end preview
			capPreview(hwndCap, FALSE);
			// disconnect from driver
			capDriverDisconnect(hwndCap);
			PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

/*************************************************************************************************
	��ʾ����������������״̬

*************************************************************************************************/
void ShowCaptureState()
{
	static int countFrame=0;
	countFrame++;
	if ( !isRecognition )
	{
		if ( countFrame < 5 )
			TextOut(hWinDC, 750, 150, "���ڲ�������     ", strlen("���ڲ�������     "));
		else if ( countFrame < 10 )
			TextOut(hWinDC, 750, 150, "���ڲ�������.    ", strlen("���ڲ�������.    "));
		else if ( countFrame < 15 )
			TextOut(hWinDC, 750, 150, "���ڲ�������..   ", strlen("���ڲ�������..   "));
		else if ( countFrame < 20 )
			TextOut(hWinDC, 750, 150, "���ڲ�������...  ", strlen("���ڲ�������...  "));
		else if ( countFrame < 25 )
			TextOut(hWinDC, 750, 150, "���ڲ�������.... ", strlen("���ڲ�������.... "));
		else if ( countFrame < 30 )
			TextOut(hWinDC, 750, 150, "���ڲ�������.....", strlen("���ڲ�������....."));
		else
			countFrame = 0;
	}
}

/************************************************************************************************
	callback procedure: FrameCallbackProc
						frame callback function
				  hwnd: capture window handle
				lpVHdr: pointer to structure containing captured frame information

************************************************************************************************/
LRESULT PASCAL FrameCallbackProc(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	HANDLE hRecognitionThread; //handle of face recognition thread
	if ( !hwnd )
		return FALSE;

	// ���������״̬
	if ( !isRecognition )
		ShowCaptureState();

// ���߳̽�����Ǻ����¿��߳��´���
	if ( isThreadEnd )
	{
		strcpy(ImagePath, curDir);
		strcat(ImagePath, "\\FaceImage.bmp");
		// ���浱ǰ֡ͼ��ImagePath��bmp�ļ���
		capFileSaveDIB(hwnd, ImagePath);

		// disable frame callback function to end entering face
		//	capSetCallbackOnFrame(hwndCap, NULL);

		// create the video capture thread
		DWORD id;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		
		hRecognitionThread = CreateThread(&sa, (ULONG)0,
									RecognitionThreadProc, //�̺߳���
									(LPVOID)(ULONG)0, (ULONG)0, &id);
		if ( hRecognitionThread == NULL )
			MessageBox(hwnd, "Creation of face recognition thread failed!",
						"Thread", MB_OK | MB_ICONEXCLAMATION);
	}
	
	return (LRESULT)TRUE;
}

/************************************************************************************************
	Face Recognition Thread �̴߳�����

************************************************************************************************/
DWORD WINAPI RecognitionThreadProc(LPVOID lParam)
{
	char facebasePath[1024];

	isThreadEnd = false;
	if ( !ReadBmpFile(ImagePath, Image) )
		// Register a frame callback function with the capture window
		capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
//	ShowBmpImage(Image, ImageWidth, ImageHeight, 665, 20);

	if ( isRecognition )
	{
		strcpy(facebasePath, curDir);
		strcat(facebasePath, "\\Facebase");
		
		// �������в���Ŀ������
		if ( RecognizeFace(Image, ImageWidth, ImageHeight, facebasePath) )
		{
			// disable frame callback function to end entering face
			capSetCallbackOnFrame(hwndCap, NULL);
			MessageBox(hMainWnd, "ƥ��ɹ���", "����ʶ��", 0);
		}
		else if ( MessageBox(hMainWnd, "ƥ��ʧ�ܣ���ȥ¼�����������������ʶ�𼸴Σ�", "����ʶ��", MB_YESNO) == IDYES )
			// Register a frame callback function with the capture window
			capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
		else // disable frame callback function to end entering face
			capSetCallbackOnFrame(hwndCap, NULL);

		// Update Show Rect()
		RECT rt;
		rt.left = 660, rt.top = 20;
		rt.right = 1100, rt.bottom = 500;
		InvalidateRect(hMainWnd, &rt, true);
	}
	else // if ( is Recognition == false )
	{
		if ( !ExtractFace(Image, ImageWidth, ImageHeight, FaceWidth, FaceHeight) )
		{
			// Register a frame callback function with the capture window
			capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
			isThreadEnd = true;
			return 0;
		}

		// normalize face image to 70 70 size
		NormalizeImageSize(NewImage, 70, 70, Image, FaceWidth, FaceHeight);
		ShowBmpImage(NewImage, 70, 70, 770, 200);

		int rtn = MessageBox(hMainWnd, "�ǣ���¼���������������򲶻�������", "Entering Face", MB_YESNOCANCEL);
		if (  rtn == IDNO )
			// Register a frame callback function with the capture window
			capSetCallbackOnFrame(hwndCap, FrameCallbackProc);
		else if ( rtn == IDYES )
		{
			// disable frame callback function to end entering face
			capSetCallbackOnFrame(hwndCap, NULL);

			// ����������Ŀ¼
			strcpy(facebasePath, curDir);
			strcat(facebasePath, "\\Facebase");
			
			// ����ͼ�񱣴����
			if ( EnterFace(ImagePath, facebasePath) )
				MessageBox(hMainWnd, "¼��ɹ���", "Entering Face", 0);
		}
		else // disable frame callback function to end entering face
			capSetCallbackOnFrame(hwndCap, NULL);
	
		// Update Show Rect()
		RECT rt;
		rt.left = 750, rt.top = 150;
		rt.right = 900, rt.bottom = 270;
		InvalidateRect(hMainWnd, &rt, true);	
	}

	isThreadEnd = true;
//	MessageBox(NULL, "Leaving Face Recognition Thread", "Thread", NULL);
	return 0;
}


/*************************************************************************************************
	����ѡ���Ŀ��ͼ�񣬽�������ʶ��

*************************************************************************************************/
void SearchFace()
{
	//������Ƶ���񴰿�
	ShowWindow(hwndCap, SW_HIDE);
	//����֡ͼ�񲶻�ص�����Ϊ��
	capSetCallbackOnFrame(hwndCap, NULL);
	
	// �õ�������ɫCb��Cr�Աȿ�cbcr[cb][cr]
	if ( !FaceCbcrProc() )
		goto BREAK_SEARCH;
	
	OpenImageFile("ѡ������Ŀ������ͼ��");
	if ( !strlen(ImgFileName) || !ReadBmpFile(ImgFileName, Image) )
		goto BREAK_SEARCH;
	ShowBmpImage(Image, ImageWidth, ImageHeight, 300, 200);
	
	strcpy(facebasePath, curDir);
	strcat(facebasePath, "\\Facebase");
	
	// ���������в���Ŀ������
	if ( RecognizeFace(Image, ImageWidth, ImageHeight, facebasePath) )
		MessageBox(hMainWnd, "ƥ��ɹ���", "����ʶ��", 0);
	else 
		MessageBox(hMainWnd, "ƥ��ʧ�ܣ�", "����ʶ��", 0);
BREAK_SEARCH:
	// Update Show window rect
	InvalidateRect(hMainWnd, NULL, true);
	ShowWindow(hwndCap, SW_SHOW);
}


// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
