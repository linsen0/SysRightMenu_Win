// SysRightMenu_Win.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SysRightMenu_Win.h"
#include <tchar.h>
#include <iostream>
#include <objbase.h>
#include <CommCtrl.h>
#include <codecvt>
#include <fstream>
#include <locale>
#include <shlobj.h>
#include <shellapi.h>
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void DisplayContextMenu(HWND hWnd)
{
    // 初始化 COM ...
    CoInitialize(NULL);
    // 桌面路径
    TCHAR folderPath[MAX_PATH] = L"D:\\Desktop";
    // 解析路径获取 ITEMIDLIST
    LPITEMIDLIST pidl = nullptr;
    HRESULT hr = SHParseDisplayName(folderPath, nullptr, &pidl, 0, nullptr);
    if (FAILED(hr) || pidl == nullptr) {
        // 处理错误，例如路径无效
        OutputDebugString(L"----------input path error");
    }
    // 获取IShellFolder文件夹接口...
    IShellFolder* fileIShellFolder = nullptr;
    hr = SHBindToObject(nullptr, pidl, nullptr, IID_IShellFolder, (void**)&fileIShellFolder);
    if (FAILED(hr) || fileIShellFolder == nullptr) {
        // 释放pidl资源
        ILFree(pidl);
        // 处理错误，例如文件夹接口无效
        OutputDebugString(L"----------get IShellFolder error");
    }
    LPCITEMIDLIST relativePIDL = nullptr;
    hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&fileIShellFolder, &relativePIDL);

    IContextMenu* pContextMenu;
    hr = fileIShellFolder->GetUIObjectOf(NULL, 1, &relativePIDL, IID_IContextMenu, nullptr, reinterpret_cast<void**>(&pContextMenu));
    if (FAILED(hr))
    {
        WCHAR szErrMsg[512];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrMsg, _countof(szErrMsg), NULL);
        OutputDebugStringW(L"----------get IContextmenu error: ");
        OutputDebugStringW(szErrMsg);
        return; // 或者 throw exception...
    }
    // 创建空的弹出式菜单
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) {
        OutputDebugString(L"Failed to create popup menu.");
        return;
    }
    // 调用 QueryContextMenu 将上下文菜单项添加到弹出式菜单中
    hr = pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to QueryContextMenu");
        DestroyMenu(hMenu);
        return;
    }
    // 获取最后一个命令ID的位置
    UINT lastCommandID = GetMenuItemID(hMenu, GetMenuItemCount(hMenu) - 1);
    // 获取当前鼠标坐标
    POINT pt;
    GetCursorPos(&pt);
    // 显示上下文菜单
    const int iCmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, pt.x, pt.y, hWnd, nullptr);
    if (iCmd > 0)
    {
        CMINVOKECOMMANDINFO info;
        ::memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        info.hwnd = reinterpret_cast<HWND>(hWnd);
        info.lpVerb = MAKEINTRESOURCEA(iCmd - 1);
        info.nShow = SW_SHOWNORMAL;
        // 执行命令
        pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
    }
    DestroyMenu(hMenu);
    CoUninitialize();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SYSRIGHTMENUWIN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SYSRIGHTMENUWIN));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SYSRIGHTMENUWIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SYSRIGHTMENUWIN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DisplayContextMenu(hWnd);
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
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
