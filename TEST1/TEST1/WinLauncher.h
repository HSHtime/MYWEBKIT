/*
* Copyright (C) 2014 Apple Inc.  All rights reserved.
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
#pragma once
#include <WebKit/WebKit.h>
#include <vector>
#include <comip.h>

typedef _com_ptr_t<_com_IIID<IWebFrame, &__uuidof(IWebFrame)>> IWebFramePtr;
typedef _com_ptr_t<_com_IIID<IWebView, &__uuidof(IWebView)>> IWebViewPtr;
typedef _com_ptr_t<_com_IIID<IWebViewPrivate, &__uuidof(IWebViewPrivate)>> IWebViewPrivatePtr;
typedef _com_ptr_t<_com_IIID<IWebFrameLoadDelegate, &__uuidof(IWebFrameLoadDelegate)>> IWebFrameLoadDelegatePtr;
typedef _com_ptr_t<_com_IIID<IWebHistory, &__uuidof(IWebHistory)>> IWebHistoryPtr;
typedef _com_ptr_t<_com_IIID<IWebHistoryItem, &__uuidof(IWebHistoryItem)>> IWebHistoryItemPtr;
typedef _com_ptr_t<_com_IIID<IWebPreferences, &__uuidof(IWebPreferences)>> IWebPreferencesPtr;
typedef _com_ptr_t<_com_IIID<IWebPreferencesPrivate, &__uuidof(IWebPreferencesPrivate)>> IWebPreferencesPrivatePtr;
typedef _com_ptr_t<_com_IIID<IWebUIDelegate, &__uuidof(IWebUIDelegate)>> IWebUIDelegatePtr;
typedef _com_ptr_t<_com_IIID<IAccessibilityDelegate, &__uuidof(IAccessibilityDelegate)>> IAccessibilityDelegatePtr;
typedef _com_ptr_t<_com_IIID<IWebInspector, &__uuidof(IWebInspector)>> IWebInspectorPtr;
typedef _com_ptr_t<_com_IIID<IWebCoreStatistics, &__uuidof(IWebCoreStatistics)>> IWebCoreStatisticsPtr;
typedef _com_ptr_t<_com_IIID<IWebCache, &__uuidof(IWebCache)>> IWebCachePtr;

class WinLauncher {
public:
	WinLauncher(HWND mainWnd, bool useLayeredWebView, HINSTANCE hInstance, int cmdShow);
	void init();
	HRESULT prepareViews(HWND mainWnd, const RECT& clientRect, const BSTR& requestedURL, HWND& viewHwnd);
	bool seedInitialDefaultPreferences();
	bool setToDefaultPreferences();
	HRESULT WinLauncher::loadURL(const BSTR& passedURL,int type=0);//type:1_file,0_url
	HRESULT setFrameLoadDelegate(IWebFrameLoadDelegate*);
	HRESULT setUIDelegate(IWebUIDelegate*);
	HRESULT setAccessibilityDelegate(IAccessibilityDelegate*);

	IWebPreferences* standardPreferences() { return m_standardPreferences; }
	IWebPreferencesPrivate* privatePreferences() { return m_prefsPrivate; }
	IWebFrame* mainFrame();
	IWebCoreStatistics* statistics() { return m_statistics; }
	IWebCache* webCache() { return m_webCache; }
	IWebView* webView() { return m_webView; }
	void moveHUD(int x, int y)
	{
		RECT rect;
		::GetWindowRect(m_hMainWnd, &rect);
		::MoveWindow(m_hMainWnd, x, y+30, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
	void resizeHUD(int width, int height)
	{
		RECT rect;
		::GetWindowRect(m_hMainWnd, &rect);
		::MoveWindow(m_hMainWnd, rect.left, rect.top, width, height, TRUE);
	}
	//HWND getPartnerWnd() { return m_hudWnd; }
	bool usesLayeredWebView() const { return m_useLayeredWebView; }
	static BOOL CALLBACK  DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
private:
	IWebViewPrivate* m_webViewPrivate;

	IWebInspectorPtr m_inspector;
	IWebPreferences* m_standardPreferences;
	IWebPreferencesPrivate* m_prefsPrivate;

	IWebFrameLoadDelegatePtr m_frameLoadDelegate;
	IWebUIDelegatePtr m_uiDelegate;
	IAccessibilityDelegatePtr m_accessibilityDelegate;

	IWebCoreStatisticsPtr m_statistics;
	IWebCachePtr m_webCache;
	HINSTANCE m_hInstance;
	HWND m_hMainWnd;
	IWebView* m_webView;
	bool m_useLayeredWebView;
	int m_cmdShow;
};
