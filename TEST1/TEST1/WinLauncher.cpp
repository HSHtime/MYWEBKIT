/*
* Copyright (C) 2006, 2008, 2013, 2014 Apple Inc.  All rights reserved.
* Copyright (C) 2009, 2011 Brent Fulgham.  All rights reserved.
* Copyright (C) 2009, 2010, 2011 Appcelerator, Inc. All rights reserved.
* Copyright (C) 2013 Alex Christensen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "WinLauncher.h"

//#include "DOMDefaultImpl.h"
#include <WebKit/WebKitCOMAPI.h>
#include <private/wtf/ExportMacros.h>
#include <private/wtf/Platform.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <winerror.h>

static const wchar_t* defaultHTML = L"<p style=\"background-color: #00FF00\">Testing</p><img id=\"webkit logo\" src=\"http://webkit.org/images/icon-gold.png\" alt=\"Face\"><div style=\"border: solid blue; background: white;\" contenteditable=\"true\">div with blue border</div><ul><li>foo<li>bar<li>baz</ul>";

static const int maxHistorySize = 10;

typedef _com_ptr_t<_com_IIID<IWebMutableURLRequest, &__uuidof(IWebMutableURLRequest)>> IWebMutableURLRequestPtr;


WinLauncher::WinLauncher(HWND mainWnd, bool useLayeredWebView, HINSTANCE hInstance, int cmdShow)
: m_hMainWnd(mainWnd)
, m_useLayeredWebView(useLayeredWebView)
, m_hInstance(hInstance)
, m_cmdShow(cmdShow)
{

}

void WinLauncher::init()
{
	//m_hMainWnd = ::CreateDialog(m_hInstance, MAKEINTRESOURCE(IDD_TEST1_DIALOG), 0, DialogProc);
	HRESULT hr = WebKitCreateInstance(CLSID_WebView, 0, IID_IWebView, reinterpret_cast<void**>(&m_webView));
	if (FAILED(hr))
		return ;
	hr = m_webView->QueryInterface(IID_IWebViewPrivate, reinterpret_cast<void**>(&m_webViewPrivate));
	if (FAILED(hr))
		return ;

	hr = WebKitCreateInstance(CLSID_WebCoreStatistics, 0, __uuidof(m_statistics), reinterpret_cast<void**>(&m_statistics.GetInterfacePtr()));
	if (FAILED(hr))
		return ;

	hr = WebKitCreateInstance(CLSID_WebCache, 0, __uuidof(m_webCache), reinterpret_cast<void**>(&m_webCache.GetInterfacePtr()));

	return ;
}
IWebFrame* WinLauncher::mainFrame()
{
	IWebFrame* framePtr;
	m_webView->mainFrame(&framePtr);
	return framePtr;
}
bool WinLauncher::seedInitialDefaultPreferences()
{
	IWebPreferences* tmpPreferences;
	if (FAILED(WebKitCreateInstance(CLSID_WebPreferences, 0, IID_IWebPreferences, reinterpret_cast<void**>(&tmpPreferences))))
		return false;

	if (FAILED(tmpPreferences->standardPreferences(&m_standardPreferences)))
		return false;

	return true;
}
BOOL WinLauncher::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WinLauncher* dialog = reinterpret_cast<WinLauncher*>(::GetWindowLongPtr(hDlg, DWLP_USER));
	if (!dialog)
		return FALSE;

	switch (message) {
	case WM_CLOSE:
		::PostQuitMessage(0);
		return FALSE;
	case WM_INITDIALOG:
		
		break;
	//case WM_WINDOWPOSCHANGING: {
	//							   WINDOWPOS* p = reinterpret_cast<WINDOWPOS*>(lParam);
	//							   p->hwndInsertAfter = dialog->getPartnerWnd();
	//							   p->flags &= ~SWP_NOZORDER;
	//							   return TRUE;
	//}
	//	break;
	case WM_MOVE: {
					  int x = (int)(short)LOWORD(lParam);   // horizontal position 
					  int y = (int)(short)HIWORD(lParam);
					  dialog->moveHUD(x, y);
					  return FALSE;
	}
		break;
	case WM_SIZE: {
					  int width = (int)(short)LOWORD(lParam);   // horizontal position 
					  int height = (int)(short)HIWORD(lParam);
					  dialog->resizeHUD(width, height);
					  return FALSE;
	}
		break;
	case WM_COMMAND:
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		switch (wmId) {
		
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			//DestroyWindow(hWnd);
			break;
		case IDM_PRINT:
			//PrintView(hWnd, message, wParam, lParam);
			break;
		default:
			return FALSE;
		}
		break;
	}

	return FALSE;
}
HRESULT WinLauncher::prepareViews(HWND mainWnd, const RECT& clientRect, const BSTR& requestedURL, HWND& viewHwnd)
{
	if (!m_webView)
		return E_FAIL;

	HRESULT hr = m_webView->setHostWindow(mainWnd);
	if (FAILED(hr))
		return hr;

	hr = m_webView->initWithFrame(clientRect, 0, 0);
	if (FAILED(hr))
		return hr;
	if (!requestedURL) {
		loadURL(L"http://www.baidu.com");
	}
	

	
	/*hr = m_webViewPrivate->setTransparent(TRUE);
	if (FAILED(hr))
		goto exit;

	hr = m_webViewPrivate->setUsesLayeredWindow(TRUE);
	if (FAILED(hr))
		goto exit;

	hr = m_webViewPrivate->viewWindow(&viewHwnd);
	if (FAILED(hr) || !viewHwnd)
		goto exit;*/

exit:
	return hr;
}
HRESULT WinLauncher::loadURL(const BSTR& passedURL,int type)
{
	//_bstr_t urlBStr = SysAllocString(L"C:\\Users\\admin\\Desktop\\MYWEBKIT\\TEST1\\Debug\\bin32\\test2.html");
	_bstr_t urlBStr(passedURL);
	//BSTR urlBStr(passedURL);
	if (!!urlBStr && (::PathFileExists(urlBStr) || ::PathIsUNC(urlBStr))) {
		TCHAR fileURL[INTERNET_MAX_URL_LENGTH];
		DWORD fileURLLength = sizeof(fileURL) / sizeof(fileURL[0]);

		if (SUCCEEDED(::UrlCreateFromPath(urlBStr, fileURL, &fileURLLength, 0)))
			urlBStr = fileURL;
	}

	IWebFrame* frame;
	HRESULT hr = m_webView->mainFrame(&frame);
	if (FAILED(hr))
		return hr;

	IWebMutableURLRequest* request;
	hr = WebKitCreateInstance(CLSID_WebMutableURLRequest, 0, IID_IWebMutableURLRequest, (void**)&request);
	if (FAILED(hr))
		return hr;

	hr = request->initWithURL(wcsstr(static_cast<wchar_t*>(urlBStr), L"://") ? urlBStr : _bstr_t(L"http://") + urlBStr, WebURLRequestUseProtocolCachePolicy, 60);
	hr = request->initWithURL(urlBStr, WebURLRequestUseProtocolCachePolicy, 60);
	if (FAILED(hr))
		return hr;

	BSTR methodBStr(L"GET");
	hr = request->setHTTPMethod(methodBStr);
	if (FAILED(hr))
		return hr;

	hr = frame->loadRequest(request);

	return hr;
}
bool WinLauncher::setToDefaultPreferences()
{
	HRESULT hr = m_standardPreferences->QueryInterface(IID_IWebPreferencesPrivate, reinterpret_cast<void**>(&m_prefsPrivate));
	if (!SUCCEEDED(hr))
		return false;

	m_prefsPrivate->setFullScreenEnabled(TRUE);
	m_prefsPrivate->setShowDebugBorders(FALSE);
	m_prefsPrivate->setShowRepaintCounter(FALSE);

	m_standardPreferences->setLoadsImagesAutomatically(TRUE);
	m_prefsPrivate->setAuthorAndUserStylesEnabled(TRUE);
	m_standardPreferences->setJavaScriptEnabled(TRUE);
	m_prefsPrivate->setAllowUniversalAccessFromFileURLs(FALSE);
	m_prefsPrivate->setAllowFileAccessFromFileURLs(TRUE);

	m_prefsPrivate->setDeveloperExtrasEnabled(TRUE);

	return true;
}

HRESULT WinLauncher::setFrameLoadDelegate(IWebFrameLoadDelegate* frameLoadDelegate)
{
	m_frameLoadDelegate = frameLoadDelegate;
	return m_webView->setFrameLoadDelegate(frameLoadDelegate);
}

HRESULT WinLauncher::setUIDelegate(IWebUIDelegate* uiDelegate)
{
	m_uiDelegate = uiDelegate;
	return m_webView->setUIDelegate(uiDelegate);
}

HRESULT WinLauncher::setAccessibilityDelegate(IAccessibilityDelegate* accessibilityDelegate)
{
	m_accessibilityDelegate = accessibilityDelegate;
	return m_webView->setAccessibilityDelegate(accessibilityDelegate);
}