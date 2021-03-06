// Reader.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Reader.h"
#include <stdio.h>
#include <shlwapi.h>
#include <CommDlg.h>
#include <commctrl.h>
#include <vector>
#include <shellapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "shell32.lib")


#define MAX_LOADSTRING              100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szStatusClass[MAX_LOADSTRING];			// the status window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Setting(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Proxy(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BgImage(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    JumpProgress(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    UpgradeProc(HWND, UINT, WPARAM, LPARAM);



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

    // init gdiplus
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // load cache file
    if (!Init())
    {
        return FALSE;
    }

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_READER, szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDC_STATUSBAR, szStatusClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_READER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
        if (!_hFindDlg || !IsDialogMessage(_hFindDlg, &msg))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		    {
			    TranslateMessage(&msg);
			    DispatchMessage(&msg);
		    }
        }
	}

    Exit();

    // uninit gdiplus
    GdiplusShutdown(gdiplusToken);

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW /*| CS_DBLCLKS*/;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BOOK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(_header->bg_color);//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_READER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_BOOK));

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_LAYERED, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      _header->rect.left, _header->rect.top, 
      _header->rect.right - _header->rect.left,
      _header->rect.bottom - _header->rect.top,
      NULL, NULL, hInstance, NULL);

   SetLayeredWindowAttributes(hWnd, 0, _header->alpha, LWA_ALPHA);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
    LRESULT hit;
    POINT pt;
    RECT rc;

    if (message == _uFindReplaceMsg)
    {
        OnFindText(hWnd, message, wParam, lParam);
        return 0;
    }

	switch (message)
	{
	case WM_COMMAND:
        //PauseAutoPage(hWnd);
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
		// Parse the menu selections:
        if (wmId >= IDM_CHAPTER_BEGIN && wmId <= IDM_CHAPTER_END)
        {
            if (_ChapterMap.end() != _ChapterMap.find(wmId))
            {
                _item->index = _ChapterMap[wmId];
                _PageCache.Reset(hWnd);
            }
            ResumeAutoPage(hWnd);
            break;
        }
        else if (wmId >= IDM_OPEN_BEGIN && wmId <= IDM_OPEN_END)
        {
            int item_id = wmId - IDM_OPEN_BEGIN;
            OnOpenItem(hWnd, item_id);
            ResumeAutoPage(hWnd);
            break;
        }
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
        case IDM_OPEN:
            OnOpenFile(hWnd, message, wParam, lParam);
            break;
        case IDM_FONT:
            OnSetFont(hWnd, message, wParam, lParam);
            break;
        case IDM_COLOR:
            OnSetBkColor(hWnd, message, wParam, lParam);
            break;
        case IDM_IMAGE:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_BG_IMAGE), hWnd, BgImage);
            break;
        case IDM_CONFIG:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTING), hWnd, Setting);
            break;
        case IDM_PROXY:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_PROXY), hWnd, Proxy);
            break;
        case IDM_DEFAULT:
            OnRestoreDefault(hWnd, message, wParam, lParam);
            break;
		default:
            ResumeAutoPage(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
        ResumeAutoPage(hWnd);
		break;
    case WM_CREATE:
        OnCreate(hWnd);
        // register hot key
        RegisterHotKey(hWnd, ID_HOTKEY_SHOW_HIDE_WINDOW, _header->hk_show_1 | _header->hk_show_2 | MOD_NOREPEAT, _header->hk_show_3);
        break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
        OnPaint(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
        StopAutoPage(hWnd);
        UnregisterHotKey(hWnd, ID_HOTKEY_SHOW_HIDE_WINDOW);
        UnregisterHotKey(hWnd, ID_HOTKEY_TOP_WINDOW);
		PostQuitMessage(0);
        // save rect
        if (_WndInfo.bHideBorder && !_WndInfo.bFullScreen)
        {
            GetClientRectExceptStatusBar(hWnd, &rc);
            ClientToScreen(hWnd, reinterpret_cast<POINT*>(&rc.left));
            ClientToScreen(hWnd, reinterpret_cast<POINT*>(&rc.right));
            rc.left = rc.left - _WndInfo.hbRect.left;
            rc.right = rc.right + _WndInfo.hbRect.right;
            rc.top = rc.top - _WndInfo.hbRect.top;
            rc.bottom = rc.bottom + _WndInfo.hbRect.bottom;
            _header->rect = rc;
        }
		break;
    case WM_QUERYENDSESSION:
        Exit(); // save data when poweroff ?
        return TRUE;
    case WM_NCHITTEST:
        hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit == HTCLIENT && _WndInfo.bHideBorder)
        {
            if (_header->page_mode == 2)
            {
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                GetClientRectExceptStatusBar(hWnd, &rc);
                rc.left = rc.right/3;
                rc.right = rc.right/3 * 2;
                if (PtInRect(&rc, pt))
                    hit = HTCAPTION;
            }
            else
            {
                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                GetClientRectExceptStatusBar(hWnd, &rc);
                rc.bottom = rc.bottom/2 > 80 ? 80 : rc.bottom/2;
                if (PtInRect(&rc, pt))
                    hit = HTCAPTION;
            }
        }
        return hit;
    case WM_SIZE:
        OnSize(hWnd, message, wParam, lParam);
        if (!IsZoomed(hWnd) && !_WndInfo.bHideBorder && !_WndInfo.bFullScreen)
            GetWindowRect(hWnd, &_header->rect);
        break;
    case WM_MOVE:
        OnMove(hWnd);
        if (!IsZoomed(hWnd) && !_WndInfo.bHideBorder && !_WndInfo.bFullScreen)
            GetWindowRect(hWnd, &_header->rect);
        break;
    case WM_KEYDOWN:
        if (VK_LEFT == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                OnGotoPrevChapter(hWnd, message, wParam, lParam);
            }
            else
            {
                OnPageUp(hWnd);
            }
        }
        else if (VK_RIGHT == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                OnGotoNextChapter(hWnd, message, wParam, lParam);
            }
            else
            {
                OnPageDown(hWnd);
            }
        }
        else if (VK_UP == wParam)
        {
            OnLineUp(hWnd);
        }
        else if (VK_DOWN == wParam)
        {
            OnLineDown(hWnd);
        }
        else if ('F' == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                OnFindText(hWnd, message, wParam, lParam);
            }
        }
        else if ('T' == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                // topmost window
                static bool isTopmost = false;
                SetWindowPos(hWnd, isTopmost ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
                isTopmost = !isTopmost;
            }
        }
        else if ('O' == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                OnOpenFile(hWnd, message, wParam, lParam);
            }
        }
        else if ('G' == wParam)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                DialogBox(hInst, MAKEINTRESOURCE(IDD_JUMP_PROGRESS), hWnd, JumpProgress);
                break;
            }
        }
        else if (VK_F12 == wParam)
        {
            // show or hiden border
            OnHideBorder(hWnd);
        }
        else if (VK_F11 == wParam)
        {
            OnFullScreen(hWnd);
        }
        else if (VK_ESCAPE == wParam)
        {
            if (_WndInfo.bFullScreen)
                OnFullScreen(hWnd);
        }
        else if (VK_SPACE == wParam)
        {
            if (_IsAutoPage)
            {
                StopAutoPage(hWnd);
            }
            else
            {
                StartAutoPage(hWnd);
            }
        }
        break;
    case WM_HOTKEY:
        if (ID_HOTKEY_SHOW_HIDE_WINDOW == wParam)
        {
            // show or hide window
            static bool isShow = true;
            ShowWindow(hWnd, isShow ? SW_HIDE : SW_SHOW);
            SetForegroundWindow(hWnd);
            isShow = !isShow;
        }
        break;
    case WM_LBUTTONDOWN:
        if (_header->page_mode == 1)
        {
            OnPageDown(hWnd);
        }
        else if (_header->page_mode == 2)
        {
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            GetClientRectExceptStatusBar(hWnd, &rc);
            rc.left = rc.right/3 * 2;
            if (PtInRect(&rc, pt))
            {
                OnPageDown(hWnd);
            }
            else
            {
                rc.left = 0;
                rc.right = rc.right/3;
                if (PtInRect(&rc, pt))
                {
                    OnPageUp(hWnd);
                }
            }
        }
        break;
    case WM_RBUTTONDOWN:
        if (_header->page_mode == 1)
        {
            OnPageUp(hWnd);
        }
        break;
    case WM_MOUSEWHEEL:
        {
            const BYTE MIN_ALPHA = 0x0F;
            const BYTE MAX_ALPHA = 0xff;
            const BYTE UNIT_STEP = 0x05;
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
            {
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                {
                    if (_header->alpha < MAX_ALPHA - UNIT_STEP)
                        _header->alpha += UNIT_STEP;
                    else
                        _header->alpha = MAX_ALPHA;
                    SetLayeredWindowAttributes(hWnd, 0, _header->alpha, LWA_ALPHA);
                }
                else
                {
                    OnLineUp(hWnd);
                }
            }
            else
            {
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                {
                    if (_header->alpha > MIN_ALPHA + UNIT_STEP)
                        _header->alpha -= UNIT_STEP;
                    else
                        _header->alpha = MIN_ALPHA;
                    SetLayeredWindowAttributes(hWnd, 0, _header->alpha, LWA_ALPHA);
                }
                else
                {
                    OnLineDown(hWnd);
                }
            }
        }
        break;
    case WM_ERASEBKGND:
        if (!_Text)
            DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_DROPFILES:
        OnDropFiles(hWnd, message, wParam, lParam);
        break;
    case WM_TIMER:
        switch (wParam)
        {
        case IDT_TIMER_PAGE:
            OnPageDown(hWnd);
            if (_item->index+_PageCache.GetCurPageSize() == _TextLen)
                StopAutoPage(hWnd);
            break;
        case IDT_TIMER_UPGRADE:
            _Upgrade.Check(UpgradeCallback, hWnd);
            break;
        default:
            break;
        }
        break;
    case WM_NEW_VERSION:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPGRADE), hWnd, UpgradeProc);
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    HWND hWnd;
    RECT rc;

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
    case WM_LBUTTONDOWN:
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        hWnd = GetDlgItem(hDlg, IDC_STATIC_LINK);
        ClientToScreen(hDlg, &pt);
        GetWindowRect(hWnd, &rc);
        if (PtInRect(&rc, pt))
        {
            ShellExecute(NULL, _T("open"), _T("https://github.com/binbyu/Reader"), NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    default:
        break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;
    BOOL bUpdated = FALSE;
    int value = 0;
    int cid;
    LRESULT res;
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_EDIT_LINEGAP, _header->line_gap, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_BORDER, _header->internal_border, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_ELAPSE, _header->uElapse, TRUE);
        if (_header->page_mode == 0)
            cid = IDC_RADIO_MODE1;
        else if (_header->page_mode == 1)
            cid = IDC_RADIO_MODE2;
        else
            cid = IDC_RADIO_MODE3;
        SendMessage(GetDlgItem(hDlg, cid), BM_SETCHECK, BST_CHECKED, NULL);
        WheelSpeedInit(hDlg);
        // init hotkey
        HotkeyInit(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            switch (LOWORD(wParam))
            {
            case IDOK:
                GetDlgItemInt(hDlg, IDC_EDIT_LINEGAP, &bResult, FALSE);
                if (!bResult)
                {
                    MessageBox(hDlg, _T("Invalid line gap value!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                GetDlgItemInt(hDlg, IDC_EDIT_BORDER, &bResult, FALSE);
                if (!bResult)
                {
                    MessageBox(hDlg, _T("Invalid internal border value!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                value = GetDlgItemInt(hDlg, IDC_EDIT_ELAPSE, &bResult, FALSE);
                if (!bResult || value == 0)
                {
                    MessageBox(hDlg, _T("Invalid auto page time value!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                // save hot key
                if (!HotkeySave(hDlg))
                {
                    return (INT_PTR)TRUE;
                }
                if (SendMessage(GetDlgItem(hDlg, IDC_COMBO_SPEED), CB_GETCURSEL, 0, NULL) != -1)
                {
                    _header->wheel_speed = SendMessage(GetDlgItem(hDlg, IDC_COMBO_SPEED), CB_GETCURSEL, 0, NULL) + 1;
                }
                res = SendMessage(GetDlgItem(hDlg, IDC_RADIO_MODE1), BM_GETCHECK, 0, NULL);
                if (res == BST_CHECKED)
                {
                    _header->page_mode = 0;
                }
                else
                {
                    res = SendMessage(GetDlgItem(hDlg, IDC_RADIO_MODE2), BM_GETCHECK, 0, NULL);
                    if (res == BST_CHECKED)
                    {
                        _header->page_mode = 1;
                    }
                    else
                    {
                        _header->page_mode = 2;
                    }
                }
                value = GetDlgItemInt(hDlg, IDC_EDIT_LINEGAP, &bResult, FALSE);
                if (value != _header->line_gap)
                {
                    _header->line_gap = value;
                    bUpdated = TRUE;
                }
                value = GetDlgItemInt(hDlg, IDC_EDIT_BORDER, &bResult, FALSE);
                if (value != _header->internal_border)
                {
                    _header->internal_border = value;
                    bUpdated = TRUE;
                }
                value = GetDlgItemInt(hDlg, IDC_EDIT_ELAPSE, &bResult, FALSE);
                if (value != _header->uElapse)
                {
                    _header->uElapse = value;
                }
                break;
            case IDCANCEL:
                break;
            default:
                break;
            }

            EndDialog(hDlg, LOWORD(wParam));

            if (bUpdated)
            {
                _PageCache.Reset(GetParent(hDlg));
            }

            return (INT_PTR)TRUE;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Proxy(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT res;
    BOOL bResult = FALSE;
    int value = 0;
    TCHAR buf[64] = {0};
    wchar_t *uni = NULL;
    char *utf8 = NULL;
    int len;
    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_PROXY), CB_ADDSTRING, 0, (LPARAM)_T("不使用代理"));
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_PROXY), CB_ADDSTRING, 0, (LPARAM)_T("使用代理"));
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_PROXY), CB_SETCURSEL, _header->proxy.enable ? 1 : 0, NULL);
        uni = Utils::utf8_to_unicode(_header->proxy.addr, &len);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), uni);
        free(uni);
        uni = Utils::utf8_to_unicode(_header->proxy.user, &len);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), uni);
        free(uni);
        uni = Utils::utf8_to_unicode(_header->proxy.pass, &len);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), uni);
        free(uni);
        if (_header->proxy.port == 0)
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), _T(""));
        else
            SetDlgItemInt(hDlg, IDC_EDIT_PROXY_PORT, _header->proxy.port, TRUE);
        if (_header->proxy.enable)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), TRUE);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), FALSE);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), buf, 64-1);
            if (buf[0])
            {
                value = GetDlgItemInt(hDlg, IDC_EDIT_PROXY_PORT, &bResult, FALSE);
                if (!bResult || value <= 0 || value >= 0xFFFF)
                {
                    MessageBox(hDlg, _T("Invalid port value!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                _header->proxy.port = value;
            }
            else
            {
                _header->proxy.port = 0;
            }

            res = SendMessage(GetDlgItem(hDlg, IDC_COMBO_PROXY), CB_GETCURSEL, 0, NULL);
            if (res == 1)
            {
                _header->proxy.enable = TRUE;
            }
            else
            {
                _header->proxy.enable = FALSE;
            }
            GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), buf, 64-1);
            utf8 = Utils::unicode_to_utf8(buf, &len);
            strcpy(_header->proxy.addr, utf8);
            free(utf8);
            GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), buf, 64-1);
            utf8 = Utils::unicode_to_utf8(buf, &len);
            strcpy(_header->proxy.user, utf8);
            free(utf8);
            GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), buf, 64-1);
            utf8 = Utils::unicode_to_utf8(buf, &len);
            strcpy(_header->proxy.pass, utf8);
            free(utf8);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDC_COMBO_PROXY:
            res = SendMessage(GetDlgItem(hDlg, IDC_COMBO_PROXY), CB_GETCURSEL, 0, NULL);
            if (res == 0)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_ADDR), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_USER), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PASS), TRUE);
            }
            break;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK BgImage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT res;
    TCHAR text[MAX_PATH] = {0};
    TCHAR *ext;
    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_BIENABLE), BM_SETCHECK, _header->bg_image.enable ? BST_CHECKED : BST_UNCHECKED, NULL);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_BIFILE), _header->bg_image.file_name);
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_BIMODE), CB_ADDSTRING, 0, (LPARAM)_T("拉伸缩放"));
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_BIMODE), CB_ADDSTRING, 1, (LPARAM)_T("平铺"));
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_BIMODE), CB_ADDSTRING, 2, (LPARAM)_T("旋转平铺"));
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_BIMODE), CB_SETCURSEL, _header->bg_image.mode, NULL);
        if (_header->bg_image.enable)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BIMODE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_BIFILE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BISEL), TRUE);
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BIMODE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_BIFILE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BISEL), FALSE);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            res = SendMessage(GetDlgItem(hDlg, IDC_CHECK_BIENABLE), BM_GETCHECK, 0, NULL);
            if (res == BST_CHECKED)
            {
                GetWindowText(GetDlgItem(hDlg, IDC_EDIT_BIFILE), text, MAX_PATH-1);
                if (!text[0])
                {
                    MessageBox(hDlg, _T("Please select a image!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                ext = PathFindExtension(text);
                if (_tcscmp(ext, _T(".jpg")) != 0
                    && _tcscmp(ext, _T(".png")) != 0
                    && _tcscmp(ext, _T(".bmp")) != 0
                    && _tcscmp(ext, _T(".jpeg")) != 0)
                {
                    MessageBox(hDlg, _T("Invalid image format!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                if (!FileExists(text))
                {
                    MessageBox(hDlg, _T("image is not exists!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                res = SendMessage(GetDlgItem(hDlg, IDC_COMBO_BIMODE), CB_GETCURSEL, 0, NULL);
                if (res == -1)
                {
                    MessageBox(hDlg, _T("Invalid layout mode format!"), _T("Error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)TRUE;
                }
                _header->bg_image.mode = res;
                _header->bg_image.enable = 1;
                _tcscpy(_header->bg_image.file_name, text);
                _PageCache.ReDraw(GetParent(hDlg));
            }
            else
            {
                _header->bg_image.enable = 0;
                _PageCache.ReDraw(GetParent(hDlg));
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDC_CHECK_BIENABLE:
            res = SendMessage(GetDlgItem(hDlg, IDC_CHECK_BIENABLE), BM_GETCHECK, 0, NULL);
            if (res == BST_CHECKED)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BIMODE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_BIFILE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BISEL), TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BIMODE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_BIFILE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BISEL), FALSE);
            }
            break;
        case IDC_BUTTON_BISEL:
            if (IsWindow(_hFindDlg))
            {
                DestroyWindow(_hFindDlg);
                _hFindDlg = NULL;
            }
            TCHAR szFileName[MAX_PATH] = {0};
            TCHAR szTitle[MAX_PATH] = {0};
            OPENFILENAME ofn = {0};
            ofn.lStructSize = sizeof(ofn);  
            ofn.hwndOwner = hDlg;  
            ofn.lpstrFilter = _T("jpg(*.jpg)\0*.jpg\0jpeg(*.jpeg)\0*.jpeg\0png(*.png)\0*.png\0bmp(*.bmp)\0*.bmp\0\0");
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrFile = szFileName; 
            ofn.nMaxFile = sizeof(szFileName)/sizeof(*szFileName);  
            ofn.nFilterIndex = 0;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            BOOL bSel = GetOpenFileName(&ofn);
            if (!bSel)
            {
                return 0;
            }
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_BIFILE), szFileName);
            break;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK JumpProgress(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hWnd;
    TCHAR buf[16] = {0};
    double progress;

    switch (message)
    {
    case WM_INITDIALOG:
        hWnd = GetDlgItem(hDlg, IDC_EDIT_JP);
        progress = (double)(_item->index+_PageCache.GetCurPageSize())*100/_TextLen;
        _stprintf(buf, _T("%0.2f"), progress);
        SetWindowText(hWnd, buf);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (LOWORD(wParam) == IDOK)
            {
                if (_item)
                {
                    hWnd = GetDlgItem(hDlg, IDC_EDIT_JP);
                    GetWindowText(hWnd, buf, 15);
                    progress = _tstof(buf);
                    if (progress < 0.0 || progress > 100.0)
                    {
                        MessageBox(hDlg, _T("Invalid value!"), _T("Error"), MB_OK|MB_ICONERROR);
                        return (INT_PTR)TRUE;
                    }
                    _item->index = (int)(progress * _TextLen / 100);
                    if (_item->index == _TextLen)
                        _item->index--;
                    _PageCache.Reset(GetParent(hDlg));
                }
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK UpgradeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    json_item_data_t *vinfo;
    switch (message)
    {
    case WM_INITDIALOG:
        vinfo = _Upgrade.GetVersionInfo();
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_UPGRADE_VERSION), vinfo->version.c_str());
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_UPGRADE_DESC), vinfo->desc.c_str());
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDC_BUTTON_UPGRADE_INGORE:
            vinfo = _Upgrade.GetVersionInfo();
            wcscpy(_header->ingore_version, vinfo->version.c_str());
            break;
        case IDC_BUTTON_UPGRADE_DOWN:
            vinfo = _Upgrade.GetVersionInfo();
            ShellExecute(NULL, _T("open"), vinfo->url.c_str(), NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT OnCreate(HWND hWnd)
{
    _WndInfo.hMenu = GetMenu(hWnd);
    // create status bar
    _WndInfo.hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, _T("Please open a text."), hWnd, IDC_STATUSBAR);
    // register find text dialog
    _uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
    OnUpdateMenu(hWnd);

    // open the last file
    if (_header->size > 0)
    {
        item_t* item = _Cache.get_item(0);
        OnOpenFile(hWnd, item->file_name);
    }

    // check upgrade
    //CheckUpgrade(hWnd);
    return 0;
}

LRESULT OnUpdateMenu(HWND hWnd)
{
    int menu_begin_id = IDM_OPEN_BEGIN;
    HMENU hMenuBar = GetMenu(hWnd);
    HMENU hFile = GetSubMenu(hMenuBar, 0);
    if (hFile)
    {
        DeleteMenu(hMenuBar, 0, MF_BYPOSITION);
        hFile = NULL;
    }
    hFile = CreateMenu();
    AppendMenu(hFile, MF_STRING, IDM_OPEN, _T("&Open"));
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    for (int i=0; i<_header->size; i++)
    {
        item_t* item = _Cache.get_item(i);
        AppendMenu(hFile, MF_STRING, (UINT_PTR)menu_begin_id++, item->file_name);
    }
    if (_header->size > 0)
        AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_EXIT, _T("E&xit"));
    InsertMenu(hMenuBar, 0, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hFile, L"&File");
    DrawMenuBar(hWnd);

    return 0;
}

LRESULT OnOpenItem(HWND hWnd, int item_id)
{
    if (IsWindow(_hFindDlg))
    {
        DestroyWindow(_hFindDlg);
        _hFindDlg = NULL;
    }
    if (_item && _item->id == item_id)
    {
        return 0;
    }

    BOOL bNotExist = FALSE;
    item_t* item = _Cache.get_item(item_id);
    if (!item)
    {
        bNotExist = TRUE;
    }
    else
    {
        if (PathFileExists(item->file_name))
        {
            if (!ReadAllAndDecode(hWnd, item->file_name, &item))
            {
                return 0;
            }

            // open item
            _item = _Cache.open_item(item->id);

            // get chapter
            if (_hThreadChapter)
            {
                TerminateThread(_hThreadChapter, 0);
                CloseHandle(_hThreadChapter);
                _hThreadChapter = NULL;
            }
            _ChapterMap.clear();
            DWORD dwThreadId;
            _hThreadChapter = CreateThread(NULL, 0, ThreadProcChapter, hWnd, CREATE_SUSPENDED, &dwThreadId);
        }
        else
        {
            bNotExist = TRUE;
        }
    }

    if (bNotExist)
    {
        _Cache.delete_item(item_id);
        // update menu
        OnUpdateMenu(hWnd);
        return 0L;
    }

    // set text
    _PageCache.SetText(hWnd, _Text, _TextLen, &_item->index, &_header->line_gap, &_header->internal_border);
    
    // update menu
    OnUpdateMenu(hWnd);

    // Update title
    TCHAR szFileName[MAX_PATH] = {0};
    memcpy(szFileName, _item->file_name, sizeof(szFileName));
    if (!_szSrcTitle[0])
        GetWindowText(hWnd, _szSrcTitle, MAX_PATH);
    PathRemoveExtension(szFileName);
    _stprintf(szTitle, _T("%s - %s"), _szSrcTitle, PathFindFileName(szFileName));
    SetWindowText(hWnd, szTitle);

    // repaint
    RECT rc;
    GetClientRectExceptStatusBar(hWnd, &rc);
    InvalidateRect(hWnd, &rc, TRUE);

    if (_hThreadChapter)
        ResumeThread(_hThreadChapter);
	return 0;
}

LRESULT OnOpenFile(HWND hWnd, TCHAR *filename)
{
    item_t* item = NULL;
    TCHAR szFileName[MAX_PATH] = {0};

    memcpy(szFileName, filename, MAX_PATH);

    if (!ReadAllAndDecode(hWnd, szFileName, &item))
    {
        return 0;
    }

    // open item
    _item = _Cache.open_item(item->id);

    // set text
    _PageCache.SetText(hWnd, _Text, _TextLen, &_item->index, &_header->line_gap, &_header->internal_border);

    // get chapter
    if (_hThreadChapter)
    {
        TerminateThread(_hThreadChapter, 0);
        CloseHandle(_hThreadChapter);
        _hThreadChapter = NULL;
    }
    _ChapterMap.clear();
    DWORD dwThreadId;
    _hThreadChapter = CreateThread(NULL, 0, ThreadProcChapter, hWnd, CREATE_SUSPENDED, &dwThreadId);

    // update menu
    OnUpdateMenu(hWnd);

    // Update title
    if (!_szSrcTitle[0])
        GetWindowText(hWnd, _szSrcTitle, MAX_PATH);
    PathRemoveExtension(szFileName);
    _stprintf(szTitle, _T("%s - %s"), _szSrcTitle, PathFindFileName(szFileName));
    SetWindowText(hWnd, szTitle);

    // repaint
    RECT rc;
    GetClientRectExceptStatusBar(hWnd, &rc);
    InvalidateRect(hWnd, &rc, TRUE);

    if (_hThreadChapter)
        ResumeThread(_hThreadChapter);

    return 0;
}

LRESULT OnOpenFile(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (IsWindow(_hFindDlg))
    {
        DestroyWindow(_hFindDlg);
        _hFindDlg = NULL;
    }
    TCHAR szFileName[MAX_PATH] = {0};
    TCHAR szTitle[MAX_PATH] = {0};
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);  
    ofn.hwndOwner = hWnd;  
    ofn.lpstrFilter = _T("txt(*.txt)\0*.txt\0");
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFile = szFileName; 
    ofn.nMaxFile = sizeof(szFileName)/sizeof(*szFileName);  
    ofn.nFilterIndex = 0;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    BOOL bSel = GetOpenFileName(&ofn);
    if (!bSel)
    {
        return 0;
    }

    return OnOpenFile(hWnd, szFileName);
}

LRESULT OnSetFont(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CHOOSEFONT cf;            // common dialog box structure
    LOGFONT logFont;
    static DWORD rgbCurrent;   // current text color
    BOOL bUpdate = FALSE;

    // Initialize CHOOSEFONT
    ZeroMemory(&cf, sizeof(cf));
    memcpy(&logFont, &_header->font, sizeof(LOGFONT));
    cf.lStructSize = sizeof (cf);
    cf.hwndOwner = hWnd;
    cf.lpLogFont = &logFont;
    cf.rgbColors = _header->font_color;
    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS;

    if (ChooseFont(&cf))
    {
        if (_header->font_color != cf.rgbColors)
        {
            _header->font_color = cf.rgbColors;
            bUpdate = TRUE;
        }

        if (0 != memcmp(&logFont, &_header->font, sizeof(LOGFONT)))
        {
            memcpy(&_header->font, &logFont, sizeof(LOGFONT));
            bUpdate = TRUE;
        }
        if (bUpdate)
            _PageCache.Reset(hWnd);
    }

    return 0;
}

LRESULT OnSetBkColor(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CHOOSECOLOR cc;                 // common dialog box structure 
    static COLORREF acrCustClr[16]; // array of custom colors 

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = _header->bg_color;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc))
    {
        if (_header->bg_color != cc.rgbResult)
        {
            _header->bg_color = cc.rgbResult;
            _PageCache.ReDraw(hWnd);
        }
    }
    
    return 0;
}

LRESULT OnRestoreDefault(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (IsZoomed(hWnd))
        SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    _Cache.default_header();
    SetLayeredWindowAttributes(hWnd, 0, _header->alpha, LWA_ALPHA);
    SetWindowPos(hWnd, NULL,
        _header->rect.left, _header->rect.top,
        _header->rect.right - _header->rect.left,
        _header->rect.bottom - _header->rect.top,
        SWP_DRAWFRAME);
    _PageCache.Reset(hWnd);
    RegisterHotKey(hWnd, ID_HOTKEY_SHOW_HIDE_WINDOW, _header->hk_show_1 | _header->hk_show_2 | MOD_NOREPEAT, _header->hk_show_3);
    return 0;
}

LRESULT OnPaint(HWND hWnd, HDC hdc)
{
    RECT rc;
    HBRUSH hBrush = NULL;
    HDC memdc = NULL;
    HBITMAP hBmp = NULL;
    HFONT hFont = NULL;
    Bitmap *image;

    GetClientRectExceptStatusBar(hWnd, &rc);

    // memory dc
    memdc = CreateCompatibleDC(hdc);

    // load bg image
    image = LoadBGImage(rc.right-rc.left,rc.bottom-rc.top);
    if (image)
    {
        image->GetHBITMAP(Color(255, 255, 255), &hBmp);
        SelectObject(memdc, hBmp);
    }
    else
    {
        hBmp = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);
        SelectObject(memdc, hBmp);

        // set bg color
        hBrush = CreateSolidBrush(_header->bg_color);
        SelectObject(memdc, hBrush);
        FillRect(memdc, &rc, hBrush);
    }

    // set font
    hFont = CreateFontIndirect(&_header->font);
    SelectObject(memdc, hFont);
    SetTextColor(memdc, _header->font_color);
    
    SetBkMode(memdc, TRANSPARENT);
    _PageCache.DrawPage(memdc);

    BitBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, memdc, rc.left, rc.top, SRCCOPY);

    DeleteObject(hBmp);
    DeleteObject(hFont);
    DeleteObject(hBrush);
    DeleteDC(memdc);
    UpdateProgess();
    return 0;
}

LRESULT OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    GetClientRectExceptStatusBar(hWnd, &rc);
    _PageCache.SetRect(&rc);
    SendMessage(_WndInfo.hStatusBar, message, wParam, lParam);
    return 0;
}

LRESULT OnMove(HWND hWnd)
{
    // save rect to cache file when exit app
    return 0;
}

LRESULT OnPageUp(HWND hWnd)
{
    _PageCache.PageUp(hWnd);
    return 0;
}

LRESULT OnPageDown(HWND hWnd)
{
    _PageCache.PageDown(hWnd);
    return 0;
}

LRESULT OnLineUp(HWND hWnd)
{
    _PageCache.LineUp(hWnd, _header->wheel_speed);
    return 0;
}

LRESULT OnLineDown(HWND hWnd)
{
    _PageCache.LineDown(hWnd, _header->wheel_speed);
    return 0;
}

LRESULT OnGotoPrevChapter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!_Text || _item->index == 0)
        return 0;

    std::map<int, int>::reverse_iterator itor;
    for (itor = _ChapterMap.rbegin(); itor != _ChapterMap.rend(); itor++)
    {
        if (itor->second < _item->index)
        {
            _item->index = itor->second;
            _PageCache.Reset(hWnd);
            break;
        }
    }
    return 0;
}

LRESULT OnGotoNextChapter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!_Text || _TextLen == _item->index+_PageCache.GetCurPageSize())
        return 0;

    std::map<int, int>::iterator itor;
    for (itor = _ChapterMap.begin(); itor != _ChapterMap.end(); itor++)
    {
        if (itor->second > _item->index)
        {
            _item->index = itor->second;
            _PageCache.Reset(hWnd);
            break;
        }
    }
    return 0;
}

LRESULT OnDropFiles(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDROP hDropInfo = (HDROP)wParam;
    UINT  nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
    TCHAR szFileName[MAX_PATH] = _T("");
    DWORD dwAttribute;

    for (UINT i = 0; i < nFileCount; i++)
    {
        ::DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));
        dwAttribute = ::GetFileAttributes(szFileName);

        if (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)
        {          
            continue;
        }

        // check is txt file
        if (_tcscmp(PathFindExtension(szFileName), _T(".txt")))
        {
            continue;
        }

        // open file
        OnOpenFile(hWnd, szFileName);

        // just open first file
        break;
    }

    ::DragFinish(hDropInfo);
    return 0;
}

LRESULT OnHideBorder(HWND hWnd)
{
    RECT rc;

    if (_WndInfo.bFullScreen)
        return 0;

    GetClientRectExceptStatusBar(hWnd, &rc);
    ClientToScreen(hWnd, reinterpret_cast<POINT*>(&rc.left));
    ClientToScreen(hWnd, reinterpret_cast<POINT*>(&rc.right));

    // save status
    if (!_WndInfo.bHideBorder)
    {
        _WndInfo.hbStyle = (DWORD)GetWindowLong(hWnd, GWL_STYLE);
        _WndInfo.hbExStyle = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
        GetWindowRect(hWnd, &_WndInfo.hbRect);
        _WndInfo.hbRect.left = rc.left - _WndInfo.hbRect.left;
        _WndInfo.hbRect.right = _WndInfo.hbRect.right - rc.right;
        _WndInfo.hbRect.top = rc.top - _WndInfo.hbRect.top;
        _WndInfo.hbRect.bottom = _WndInfo.hbRect.bottom - rc.bottom;
    }
    else
    {
        rc.left = rc.left - _WndInfo.hbRect.left;
        rc.right = rc.right + _WndInfo.hbRect.right;
        rc.top = rc.top - _WndInfo.hbRect.top;
        rc.bottom = rc.bottom + _WndInfo.hbRect.bottom;
    }

    // set new status
    _WndInfo.bHideBorder = !_WndInfo.bHideBorder;

    // hide border
    if (_WndInfo.bHideBorder)
    {
        SetWindowLong(hWnd, GWL_STYLE, _WndInfo.hbStyle & (~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)));
        SetWindowLong(hWnd, GWL_EXSTYLE, _WndInfo.hbExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
        SetMenu(hWnd, NULL);
        ShowWindow(_WndInfo.hStatusBar, SW_HIDE);
        SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_DRAWFRAME);
    }
    // show border
    else
    {
        SetWindowLong(hWnd, GWL_STYLE, _WndInfo.hbStyle);
        SetWindowLong(hWnd, GWL_EXSTYLE, _WndInfo.hbExStyle);
        SetMenu(hWnd, _WndInfo.hMenu);
        ShowWindow(_WndInfo.hStatusBar, SW_SHOW);
        SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_DRAWFRAME);
    }
    return 0;
}

LRESULT OnFullScreen(HWND hWnd)
{
    MONITORINFO mi;
    RECT rc;

    if (!_WndInfo.bFullScreen)
    {
        _WndInfo.fsStyle = (DWORD)GetWindowLong(hWnd, GWL_STYLE);
        _WndInfo.fsExStyle = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
        GetWindowRect(hWnd, &_WndInfo.fsRect);
    }

    _WndInfo.bFullScreen = !_WndInfo.bFullScreen;
    
    if (_WndInfo.bFullScreen)
    {
        SetWindowLong(hWnd, GWL_STYLE, _WndInfo.fsStyle & (~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)));
        SetWindowLong(hWnd, GWL_EXSTYLE, _WndInfo.fsExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
        SetMenu(hWnd, NULL);
        ShowWindow(_WndInfo.hStatusBar, SW_HIDE);

        
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST),&mi);
        rc = mi.rcMonitor;
        SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_DRAWFRAME);
    }
    else
    {
        SetWindowLong(hWnd, GWL_STYLE, _WndInfo.fsStyle);
        SetWindowLong(hWnd, GWL_EXSTYLE, _WndInfo.fsExStyle);
        if (!_WndInfo.bHideBorder)
        {
            SetMenu(hWnd, _WndInfo.hMenu);
            ShowWindow(_WndInfo.hStatusBar, SW_SHOW);
        }
        SetWindowPos(hWnd, NULL, _WndInfo.fsRect.left, _WndInfo.fsRect.top, 
            _WndInfo.fsRect.right-_WndInfo.fsRect.left, _WndInfo.fsRect.bottom-_WndInfo.fsRect.top, SWP_DRAWFRAME);
    }
    return 0;
}

LRESULT OnFindText(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static FINDREPLACE fr;       // common dialog box structure
    static TCHAR szFindWhat[80] = {0}; // buffer receiving string

    if (message == _uFindReplaceMsg)
    {
        // do search
        if (!_Text)
            return 0;
        int len = _tcslen(szFindWhat);
        if (fr.Flags & FR_DIALOGTERM)
        {
            // close dlg
        }
        else if (fr.Flags & FR_DOWN) // back search
        {
            for (int i=_item->index+1; i<_TextLen-len+1; i++)
            {
                if (0 == memcmp(szFindWhat, _Text+i, len*sizeof(TCHAR)))
                {
                    _item->index = i;
                    _PageCache.Reset(hWnd);
                    break;
                }
            }
        }
        else // front search
        {
            for (int i=_item->index-3; i>=0; i--)
            {
                if (0 == memcmp(szFindWhat, _Text+i, len*sizeof(TCHAR)))
                {
                    _item->index = i;
                    _PageCache.Reset(hWnd);
                    break;
                }
            }
        }
    }
    else
    {
        if (!IsWindow(_hFindDlg))
        {
            // Initialize FINDREPLACE
            ZeroMemory(&fr, sizeof(fr));
            fr.lStructSize = sizeof(fr);
            fr.hwndOwner = hWnd;
            fr.hInstance = hInst;
            fr.lpstrFindWhat = szFindWhat;
            fr.wFindWhatLen = 80;
            fr.Flags = FR_DOWN;

            _hFindDlg = FindText(&fr);
        }
    }

    return 0;
}

UINT GetCacheVersion(void)
{
    // Not a real version, just used to flag whether you need to update the cache.dat file.
    char version[4] = {'1','4','0','0'};
    UINT ver = 0;

    ver = version[0] << 24 | version[1] << 16 | version[2] << 8 | version[3];
    return ver;
}

BOOL Init(void)
{
    if (!_Cache.init())
    {
        MessageBox(NULL, _T("Init Cache fail."), _T("Error"), MB_OK);
        return FALSE;
    }

    _header = _Cache.get_header();

    // delete not exist items
    std::vector<int> delVec;
    for (int i=0; i<_header->size; i++)
    {
        item_t* item = _Cache.get_item(i);
        if (!PathFileExists(item->file_name))
        {
            delVec.push_back(i);
        }
    }
    for (size_t i=0; i<delVec.size(); i++)
    {
        _Cache.delete_item(delVec[i]);
    }

    return TRUE;
}

void Exit(void)
{
    if (_Text)
        free(_Text);
    
    if (_hThreadChapter)
    {
        TerminateThread(_hThreadChapter, 0);
        CloseHandle(_hThreadChapter);
        _hThreadChapter = NULL;
    }
    _ChapterMap.clear();

    if (!_Cache.exit())
    {
        MessageBox(NULL, _T("Save Cache fail."), _T("Error"), MB_OK);
    }
}

BOOL ReadAllAndDecode(HWND hWnd, TCHAR* szFileName, item_t** item)
{
    // read full file context
    FILE* fp = _tfopen(szFileName, _T("rb"));
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    char* buf = (char*)malloc(len+1);
    if (!buf)
    {
        // Memory is not enough 
        MessageBox(hWnd, _T("Memory is not enough."), _T("Error"), MB_OK);
        return FALSE;
    }
    fseek(fp, 0, SEEK_SET);
    int size = fread(buf, 1, len, fp);
    buf[size] = 0;
    fclose(fp);

    // get md5
    u128_t md5;
    memset(&md5, 0, sizeof(u128_t));
    Utils::get_md5(buf, size, &md5);

    // check cache
    if (NULL == *item)
    {
        *item = _Cache.find_item(&md5, szFileName);
        if (*item) // already exist
        {
            if (_item && (*item)->id == _item->id) // current is opened
            {
                free(buf);
                return FALSE;
            }
        }
        else
        {
            *item = _Cache.new_item(&md5, szFileName);
            _header = _Cache.get_header(); // after realloc the header address has been changed
        }
    }

    // decode
    type_t bom = Unknown;
    char* tmp = buf;
    wchar_t* result = (wchar_t*)buf;
    if (Unknown != (bom = Utils::check_bom(buf, size)))
    {
        if (utf8 == bom)
        {
            tmp += 3;
            size -= 3;
            result = Utils::utf8_to_unicode(tmp, &_TextLen);
            free(buf);
        }
        else if (utf16_le == bom)
        {
            //tmp += 2;
            //size -= 2;
            result = (wchar_t*)tmp;
            _TextLen = size/2;
        }
        else if (utf16_be == bom)
        {
            //tmp += 2;
            //size -= 2;
            tmp = Utils::be_to_le(tmp, size);
            result = (wchar_t*)tmp;
            _TextLen = size/2;
        }
        else if (utf32_le == bom || utf32_be == bom)
        {
            tmp += 4;
            size -= 4;
            // not support
            free(buf);
            MessageBox(hWnd, _T("This file encoding is not support."), _T("Error"), MB_OK);
            return FALSE;
        }
    }
    else if (Utils::is_ascii(buf, size > 1024 ? 1024 : size))
    {
        result = Utils::utf8_to_unicode(tmp, &_TextLen);
        free(buf);
    }
    else if (Utils::is_utf8(buf, size > 1024 ? 1024 : size))
    {
        result = Utils::utf8_to_unicode(tmp, &_TextLen);
        free(buf);
    }
    else
    {
        result = Utils::ansi_to_unicode(tmp, &_TextLen);
        free(buf);
    }
    if (_Text)
    {
        free(_Text);
    }
    _Text = result;
    FormatText();
    return TRUE;
}

void FormatText(void)
{
    TCHAR *buf = NULL;
    int i, index = 0;
    if (!_Text || _TextLen == 0)
        return;

    buf = (TCHAR *)malloc(_TextLen * sizeof(TCHAR));
    for (i = 0; i < _TextLen; i++)
    {
        if (_Text[i] == 0x0D && _Text[i + 1] == 0x0A)
            i++;
        buf[index++] = _Text[i];
    }
#if 0
    buf[index] = 0;
#else
    index--;
#endif
    _TextLen = index;
    memcpy(_Text, buf, _TextLen * sizeof(TCHAR));
    free(buf);
}

void UpdateProgess(void)
{
    static TCHAR progress[32] = {0};
    double dprog = 0.0;
    int nprog = 0;
    if (_item)
    {
        dprog = (double)(_item->index+_PageCache.GetCurPageSize())*100.0/_TextLen;
        nprog = (int)(dprog * 100);
        dprog = (double)nprog / 100.0;
        _stprintf(progress, _T("Progress: %.2f%%"), dprog);
        SendMessage(_WndInfo.hStatusBar, SB_SETTEXT, (WPARAM)0, (LPARAM)progress);
    }
    else
    {
        SendMessage(_WndInfo.hStatusBar, SB_SETTEXT, (WPARAM)0, (LPARAM)_T(""));
    }
}

BOOL GetClientRectExceptStatusBar(HWND hWnd, RECT* rc)
{
    RECT rect;
    GetClientRect(_WndInfo.hStatusBar, &rect);
    GetClientRect(hWnd, rc);
    if (IsWindowVisible(_WndInfo.hStatusBar))
    {
        rc->bottom -= rect.bottom;
    }
    return TRUE;
}

inline BOOL GetLine(TCHAR* buf, int len, int* line_size)
{
    if (!buf || len <= 0)
        return FALSE;

    for (int i=0; i<len; i++)
    {
        if (buf[i] == 0x0A)
        {
            if (i > 0)
            {
                *line_size = i;
                return TRUE;
            }
        }
        else if (i < len - 1
            && buf[i] == 0x0D && buf[i+1] == 0x0A)
        {
            if (i > 0)
            {
                *line_size = i;
                return TRUE;
            }
        }
    }
    *line_size = len;
    return TRUE;
}

const TCHAR valid_chapter[] = {
    _T(' '), _T('\t'),
    _T('0'), _T('1'), _T('2'), _T('3'), _T('4'),
    _T('5'), _T('6'), _T('7'), _T('8'), _T('9'),
    _T('零'), _T('一'), _T('二'), _T('三'), _T('四'),
    _T('五'), _T('六'), _T('七'), _T('八'), _T('九'), _T('十'),
    _T('壹'), _T('贰'), _T('叁'), _T('肆'),
    _T('伍'), _T('陆'), _T('柒'), _T('捌'), _T('玖'), _T('拾'),
};

inline BOOL IsChapter(TCHAR* buf, int len)
{
    BOOL bFound = FALSE;
    if (!buf || len <= 0)
        return FALSE;

    for (int i=0; i<len; i++)
    {
        bFound = FALSE;
        for (int j=0; j<sizeof(valid_chapter); j++)
        {
            if (buf[i] == valid_chapter[j])
            {
                bFound = TRUE;
                break;
            }
        }
        if (!bFound)
        {
            return FALSE;
        }
    }
    return TRUE;
}

DWORD WINAPI ThreadProcChapter(LPVOID lpParam)
{
    HWND hWnd = (HWND)lpParam;
    TCHAR title[MAX_CHAPTER_LENGTH] = {0};
    int title_len = 0;
    TCHAR* index = _Text;
    int line_size;
    int menu_begin_id = IDM_CHAPTER_BEGIN;

    HMENU hMenuBar = GetMenu(hWnd);
    HMENU hView = CreateMenu();

    while (TRUE)
    {
        if (!GetLine(index, _TextLen-(index-_Text), &line_size))
        {
            break;
        }

        // check format
        BOOL bFound = FALSE;
        int idx_1 = -1, idx_2 = -1;
        for (int i=0; i<line_size; i++)
        {
            if (index[i] == _T('第'))
            {
                idx_1 = i;
            }
            if (idx_1>-1
                && ((line_size > i+1 && index[i+1] == _T(' ') 
                || index[i+1] == _T('\t'))
                || line_size <= i+1))
            {
                if (index[i] == _T('卷')
                    || index[i] == _T('章')
                    || index[i] == _T('部')
                    || index[i] == _T('节'))
                {
                    idx_2 = i;
                    bFound = TRUE;
                    break;
                }
            }
        }
        if (bFound && IsChapter(index+idx_1+1, idx_2-idx_1-1))
        {
            title_len = line_size-idx_1 < MAX_CHAPTER_LENGTH ? line_size-idx_1 : MAX_CHAPTER_LENGTH-1;
            memcpy(title, index+idx_1, title_len*sizeof(TCHAR));
            title[title_len] = 0;
            // update menu
            AppendMenu(hView, MF_STRING, menu_begin_id, title);
            _ChapterMap.insert(std::make_pair(menu_begin_id++, idx_1+(index-_Text)));
        }

        // set index
        index += line_size;
    }

    DeleteMenu(hMenuBar, 1, MF_BYPOSITION);
    InsertMenu(hMenuBar, 1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hView, L"&View");
    DrawMenuBar(hWnd);
    return 0;
}

void WheelSpeedInit(HWND hDlg)
{
    int i;
    HWND hWnd = NULL;
    TCHAR buf[8] = {0};

    hWnd = GetDlgItem(hDlg, IDC_COMBO_SPEED);
    for (i=1; i<=_PageCache.GetOnePageLineCount(); i++)
    {
        _itot(i, buf, 10);
        SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hWnd, CB_SETCURSEL, _header->wheel_speed - 1, NULL);
}

void HotkeyInit(HWND hDlg)
{
    HWND hWnd = NULL;
    TCHAR buf[2] = {0};


    // IDC_COMBO_SHOW_1
    hWnd = GetDlgItem(hDlg, IDC_COMBO_SHOW_1);
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T(""));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Ctrl"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Alt"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Shift"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Win"));
    SendMessage(hWnd, CB_SETCURSEL, HotKeyMap_KeyToIndex(_header->hk_show_1, IDC_COMBO_SHOW_1), NULL);

    // IDC_COMBO_SHOW_2
    hWnd = GetDlgItem(hDlg, IDC_COMBO_SHOW_2);
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Ctrl"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Alt"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Shift"));
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Win"));
    SendMessage(hWnd, CB_SETCURSEL, HotKeyMap_KeyToIndex(_header->hk_show_2, IDC_COMBO_SHOW_2), NULL);

    // IDC_COMBO_SHOW_3
    hWnd = GetDlgItem(hDlg, IDC_COMBO_SHOW_3);
    for (int i='A'; i<='Z'; i++)
    {
        buf[0] = i;
        SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hWnd, CB_SETCURSEL, HotKeyMap_KeyToIndex(_header->hk_show_3, IDC_COMBO_SHOW_3), NULL);
}

BOOL HotkeySave(HWND hDlg)
{
    int vk_1, vk_2, vk_3;
    TCHAR msg[256] = {0};

    vk_1 = HotKeyMap_IndexToKey(SendMessage(GetDlgItem(hDlg, IDC_COMBO_SHOW_1), CB_GETCURSEL, 0, NULL), IDC_COMBO_SHOW_1);
    vk_2 = HotKeyMap_IndexToKey(SendMessage(GetDlgItem(hDlg, IDC_COMBO_SHOW_2), CB_GETCURSEL, 0, NULL), IDC_COMBO_SHOW_2);
    vk_3 = HotKeyMap_IndexToKey(SendMessage(GetDlgItem(hDlg, IDC_COMBO_SHOW_3), CB_GETCURSEL, 0, NULL), IDC_COMBO_SHOW_3);
    
    if (vk_1 != _header->hk_show_1 || vk_2 != _header->hk_show_2 || vk_3 != _header->hk_show_3)
    {
        if (!RegisterHotKey(GetParent(hDlg), ID_HOTKEY_SHOW_HIDE_WINDOW, vk_1 | vk_2 | MOD_NOREPEAT, vk_3))
        {
            if (vk_1 == 0)
            {
                _stprintf(msg, _T("[%s+%s]\r\nis invalid or occupied.\r\nPlease choose another key.")
                    ,HotKeyMap_KeyToString(vk_2, IDC_COMBO_SHOW_2)
                    ,HotKeyMap_KeyToString(vk_3, IDC_COMBO_SHOW_3));
            }
            else
            {
                _stprintf(msg, _T("[%s+%s+%s]\r\nis invalid or occupied.\r\nPlease choose another key.")
                    ,HotKeyMap_KeyToString(vk_1, IDC_COMBO_SHOW_1)
                    ,HotKeyMap_KeyToString(vk_2, IDC_COMBO_SHOW_2)
                    ,HotKeyMap_KeyToString(vk_3, IDC_COMBO_SHOW_3));
            }
            MessageBox(hDlg, msg, _T("Error"), MB_OK|MB_ICONERROR);

            return FALSE;
        }
    }
    
    _header->hk_show_1 = vk_1;
    _header->hk_show_2 = vk_2;
    _header->hk_show_3 = vk_3;
    return TRUE;
}

static int s_hk_map_1[5] = {
    0, MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN
};

static int s_hk_map_2[4] = {
    MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN
};

static int s_hk_map_3[26] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z'
};
int HotKeyMap_IndexToKey(int index, int cid)
{
    if (index < 0)
        return 0;

    if (cid == IDC_COMBO_SHOW_1)
    {
        return s_hk_map_1[index];
    }
    else if (cid == IDC_COMBO_SHOW_2)
    {
        return s_hk_map_2[index];
    }
    else
    {
        return s_hk_map_3[index];
    }
}

int HotKeyMap_KeyToIndex(int key, int cid)
{
    if (cid == IDC_COMBO_SHOW_1)
    {
        for (int i=0; i<5; i++)
        {
            if (key == s_hk_map_1[i])
            {
                return i;
            }
        }
    }
    else if (cid == IDC_COMBO_SHOW_2)
    {
        for (int i=0; i<4; i++)
        {
            if (key == s_hk_map_2[i])
            {
                return i;
            }
        }
    }
    else
    {
        for (int i=0; i<26; i++)
        {
            if (key == s_hk_map_3[i])
            {
                return i;
            }
        }
    }
    return -1;
}

TCHAR* HotKeyMap_KeyToString(int key, int cid)
{
    static TCHAR buf1[8] = {0};
    static TCHAR buf2[8] = {0};
    static TCHAR buf3[8] = {0};


    if (cid == IDC_COMBO_SHOW_1)
    {
        switch (key)
        {
        case 0:
            memset(buf1, 0, sizeof(TCHAR)*8);
            break;
        case MOD_CONTROL:
            _tcscpy(buf1, _T("Ctrl"));
            break;
        case MOD_ALT:
            _tcscpy(buf1, _T("Alt"));
            break;
        case MOD_SHIFT:
            _tcscpy(buf1, _T("Shift"));
            break;
        case MOD_WIN:
            _tcscpy(buf1, _T("Win"));
            break;
        }
        return buf1;
    }
    else if (cid == IDC_COMBO_SHOW_2)
    {
        switch (key)
        {
        case MOD_CONTROL:
            _tcscpy(buf2, _T("Ctrl"));
            break;
        case MOD_ALT:
            _tcscpy(buf2, _T("Alt"));
            break;
        case MOD_SHIFT:
            _tcscpy(buf2, _T("Shift"));
            break;
        case MOD_WIN:
            _tcscpy(buf2, _T("Win"));
            break;
        }
        return buf2;
    }
    else
    {
        memset(buf3, 0, sizeof(TCHAR)*8);
        buf3[0] = key;
        return buf3;
    }
}

Bitmap* LoadBGImage(int w, int h)
{
    static Bitmap *bgimg = NULL;
    static TCHAR curfile[MAX_PATH] = {0};
    static int curWidth = 0;
    static int curHeight = 0;
    static int curmode = Stretch;
    Bitmap *image = NULL;
    Graphics *graphics = NULL;
    ImageAttributes ImgAtt;
    RectF rcDrawRect;

    if (!_header->bg_image.enable || !_header->bg_image.file_name[0])
    {
        return NULL;
    }

    if (_tcscmp(_header->bg_image.file_name, curfile) == 0 && curWidth == w && curHeight == h 
        && curmode == _header->bg_image.mode && bgimg)
    {
        return bgimg;
    }

    if (!FileExists(_header->bg_image.file_name))
    {
        _header->bg_image.file_name[0] = 0;
        return NULL;
    }
    
    if (bgimg)
    {
        delete bgimg;
        bgimg = NULL;
    }
    _tcscpy(curfile, _header->bg_image.file_name);
    curWidth = w;
    curHeight = h;
    curmode = _header->bg_image.mode;
    
    // load image file
    image = Bitmap::FromFile(curfile);
    if (image == NULL)
        return NULL;
    if (Gdiplus::Ok != image->GetLastStatus())
    {
        delete image;
        image = NULL;
        return NULL;
    }

    // create bg image
    bgimg = new Bitmap(curWidth, curHeight);
    rcDrawRect.X=0.0;
    rcDrawRect.Y=0.0;
    rcDrawRect.Width=(float)curWidth;
    rcDrawRect.Height=(float)curHeight;
    graphics = Graphics::FromImage(bgimg);
    graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);

    switch (curmode)
    {
    case Stretch:
        graphics->DrawImage(image,rcDrawRect,0.0,0.0,(float)image->GetWidth(),(float)image->GetHeight(),UnitPixel);
        break;
    case Tile:
        ImgAtt.SetWrapMode(WrapModeTile);
        graphics->DrawImage(image,rcDrawRect,0.0,0.0,(float)curWidth,(float)curHeight,UnitPixel,&ImgAtt);
        break;
    case TileFlip:
        ImgAtt.SetWrapMode(WrapModeTileFlipXY);
        graphics->DrawImage(image,rcDrawRect,0.0,0.0,(float)curWidth,(float)curHeight,UnitPixel,&ImgAtt);
        break;
    default:
        graphics->DrawImage(image,rcDrawRect,0.0,0.0,(float)image->GetWidth(),(float)image->GetHeight(),UnitPixel);
        break;
    }

    delete image;
    delete graphics;
    return bgimg;
}

BOOL FileExists(TCHAR *file)
{
    DWORD dwAttrib = GetFileAttributes(file);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void StartAutoPage(HWND hWnd)
{
    if (_item && !_IsAutoPage)
    {
        SetTimer(hWnd, IDT_TIMER_PAGE, _header->uElapse, NULL);
        _IsAutoPage = TRUE;
    }
}

void StopAutoPage(HWND hWnd)
{
    if (_IsAutoPage)
    {
        KillTimer(hWnd, IDT_TIMER_PAGE);
        _IsAutoPage = FALSE;
    }
}

void PauseAutoPage(HWND hWnd)
{
    if (_IsAutoPage)
    {
        KillTimer(hWnd, IDT_TIMER_PAGE);
    }
}

void ResumeAutoPage(HWND hWnd)
{
    if (_item && _IsAutoPage)
    {
        SetTimer(hWnd, IDT_TIMER_PAGE, _header->uElapse, NULL);
    }
}

void CheckUpgrade(HWND hWnd)
{
    // set proxy & check upgrade
    _Upgrade.SetProxy(&_header->proxy);
    _Upgrade.SetIngoreVersion(_header->ingore_version);
    _Upgrade.Check(UpgradeCallback, hWnd);
    SetTimer(hWnd, IDT_TIMER_UPGRADE, 24 * 60 * 60 * 1000 /*one day*/, NULL);
}

bool UpgradeCallback(void *param, json_item_data_t *item)
{
    HWND hWnd = (HWND)param;

    PostMessage(hWnd, WM_NEW_VERSION, 0, NULL);
    return true;
}
