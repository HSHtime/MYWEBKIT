// TEST1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TEST1.h"
#include "WinLauncher.h"
#include "WinLauncherWebHost.h"
#include "PrintWebUIDelegate.h"
#include "ShlObj.h"
//#pragma comment(lib,"ws2_32.lib")
#define MAX_LOADSTRING 100
WinLauncher *gWinLauncher;
// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hMainWnd;
HWND gViewWindow = 0;
WNDPROC DefWebKitProc = nullptr;
// Support moving the transparent window
POINT s_windowPosition = { 100, 100 };
SIZE s_windowSize = { 800, 400 };
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	// Init COM
	OleInitialize(nullptr);
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TEST1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST1));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	//while (1);
	shutDownWebKit();
	// Shut down COM.
	OleUninitialize();
	delete gWinLauncher;
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TEST1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
static bool getAppDataFolder(_bstr_t& directory)
{
	wchar_t appDataDirectory[MAX_PATH];
	if (FAILED(SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, 0, 0, appDataDirectory)))
		return false;

	wchar_t executablePath[MAX_PATH];
	if (!::GetModuleFileNameW(0, executablePath, MAX_PATH))
		return false;

	::PathRemoveExtensionW(executablePath);

	directory = _bstr_t(appDataDirectory) + L"\\" + ::PathFindFileNameW(executablePath);

	return true;
}

static bool setCacheFolder()
{
	IWebCachePtr webCache = gWinLauncher->webCache();
	if (!webCache)
		return false;

	_bstr_t appDataFolder;
	if (!getAppDataFolder(appDataFolder))
		return false;

	appDataFolder += L"\\cache";
	webCache->setCacheFolder(appDataFolder);

	return true;
}
static void resizeSubViews()
{
	if (gWinLauncher->usesLayeredWebView() || !gViewWindow)
		return;

	RECT rcClient;
	GetClientRect(hMainWnd, &rcClient);
	MoveWindow(gViewWindow, 0, 24, rcClient.right, rcClient.bottom - 24, TRUE);
}
static void subclassForLayeredWindow()
{
	hMainWnd = gViewWindow;
#if defined _M_AMD64 || defined _WIN64
	DefWebKitProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hMainWnd, GWLP_WNDPROC));
	::SetWindowLongPtr(hMainWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
#else
	DefWebKitProc = reinterpret_cast<WNDPROC>(::GetWindowLong(hMainWnd, GWL_WNDPROC));
	::SetWindowLong(hMainWnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
#endif
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   hInst = hInstance; // Store instance handle in our global variable

   hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hMainWnd)
   {
      return FALSE;
   }
   RECT containerRect;
   ::GetClientRect(hMainWnd, &containerRect);

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);
  
   
   gWinLauncher = new WinLauncher(hMainWnd, true, hInst, nCmdShow);

   if (!gWinLauncher)
	   goto exit;
   if (!gWinLauncher->seedInitialDefaultPreferences())
	   goto exit;

   if (!gWinLauncher->setToDefaultPreferences())
	   goto exit;

   gWinLauncher->init();
   HRESULT hr = gWinLauncher->setFrameLoadDelegate(new WinLauncherWebHost(gWinLauncher, NULL));
   if (FAILED(hr))
	   goto exit;

   hr = gWinLauncher->setUIDelegate(new PrintWebUIDelegate());
   if (FAILED(hr))
	   goto exit;

   hr = gWinLauncher->setAccessibilityDelegate(new AccessibilityDelegate());
   if (FAILED(hr))
	   goto exit;

  // MessageBox(NULL, szFileName, L"", MB_OK);
   hr = gWinLauncher->prepareViews(hMainWnd, containerRect, NULL, gViewWindow);
   if (FAILED(hr) )
	   goto exit;
  // subclassForLayeredWindow();
   resizeSubViews();

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);
	   return TRUE;
exit:
	   return FALSE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//



static const int dragBarHeight = 30;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//WNDPROC parentProc = (gWinLauncher) ? (gWinLauncher->usesLayeredWebView() ? DefWebKitProc : DefWindowProc) : DefWindowProc;
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
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
