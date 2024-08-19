// SysRightMenu_Win.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SysRightMenu_Win.h"
#include "WindowSubclassWrapper.h"
#include "MenuHelper.h"

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

 


std::string WstrToStr(const wchar_t* wstr)
{
    if (wstr == nullptr)
        return std::string();

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded == 0)
        return std::string();

    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], sizeNeeded, nullptr, nullptr);
    return strTo;
}

HRESULT GetRootPidl(PIDLIST_ABSOLUTE* pidl)
{
    // While using SHGetKnownFolderIDList() with FOLDERID_Desktop would be simpler than the method
    // used here, that method fails in Windows PE (with ERROR_FILE_NOT_FOUND). That failure is
    // unusual, since although the filesystem desktop folder doesn't exist, the virtual desktop
    // folder at the root of the shell namespace is still accessible and the pidl returned by
    // SHGetKnownFolderIDList() represents the root folder.
    // Retrieving the pidl using the method below works consistently, however.
    // Example:
    // unique_pidl_absolute rootPidl;
    // HRESULT hr = GetRootPidl(wil::out_param(rootPidl));

    wil::com_ptr_nothrow<IShellFolder> desktop;
    RETURN_IF_FAILED(SHGetDesktopFolder(&desktop));
    return SHGetIDListFromObject(desktop.get(), pidl);
}

HRESULT BindToIdl(PIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv)
{
    IShellFolder* pDesktop = nullptr;
    HRESULT hr = SHGetDesktopFolder(&pDesktop);

    if (SUCCEEDED(hr))
    {
        /* See http://blogs.msdn.com/b/oldnewthing/archive/2011/08/30/10202076.aspx. */
        if (pidl->mkid.cb)
        {
            hr = pDesktop->BindToObject(pidl, nullptr, riid, ppv);
        }
        else
        {
            hr = pDesktop->QueryInterface(riid, ppv);
        }

        pDesktop->Release();
    }

    return hr;
}

//ShellContextMenu::ShellContextMenu(PIDLIST_ABSOLUTE *pidlParent,const std::vector<PCITEMID_CHILD>& pidlItems) :
//    /*m_pidlParent(pidlParent ? ILCloneFull(*pidlParent) : nullptr),*/
//    m_pidlItems(pidlItems.begin(), pidlItems.end())
//{
//    
//}


//void UpdateBackgroundContextMenu(HMENU menu, PCIDLIST_ABSOLUTE folderPidl,
//    IContextMenu* contextMenu) {
//    RemoveNonFunctionalItemsFromBackgroundContextMenu(menu, contextMenu);
//
//    UINT position = 0;
//
//    auto viewsMenu = BuildViewsMenu();
//    std::wstring text =
//        ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_VIEW);
//    MenuHelper::AddSubMenuItem(menu, 0, text, std::move(viewsMenu), position++, true);
//
//    SortMenuBuilder sortMenuBuilder(m_resourceInstance);
//    auto sortMenus =
//        sortMenuBuilder.BuildMenus(GetActivePane()->GetTabContainer()->GetSelectedTab());
//    text = ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_SORT_BY);
//    MenuHelper::AddSubMenuItem(menu, 0, text, std::move(sortMenus.sortByMenu), position++, true);
//
//    text = ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_GROUP_BY);
//    MenuHelper::AddSubMenuItem(menu, 0, text, std::move(sortMenus.groupByMenu), position++, true);
//
//    text = ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_REFRESH);
//    MenuHelper::AddStringItem(menu, IDM_BACKGROUND_CONTEXT_MENU_REFRESH, text, position++, true);
//
//    MenuHelper::AddSeparator(menu, position++, true);
//
//    if (CanCustomizeDirectory(folderPidl))
//    {
//        text =
//            ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_CUSTOMIZE);
//        MenuHelper::AddStringItem(menu, IDM_BACKGROUND_CONTEXT_MENU_CUSTOMIZE, text, position++,
//            true);
//        MenuHelper::AddSeparator(menu, position++, true);
//    }
//
//    text = ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_PASTE);
//    MenuHelper::AddStringItem(menu, IDM_BACKGROUND_CONTEXT_MENU_PASTE, text, position++, true);
//
//    if (!CanPasteInDirectory(folderPidl, PasteType::Normal))
//    {
//        MenuHelper::EnableItem(menu, IDM_BACKGROUND_CONTEXT_MENU_PASTE, false);
//    }
//
//    text =
//        ResourceHelper::LoadString(m_resourceInstance, IDS_BACKGROUND_CONTEXT_MENU_PASTE_SHORTCUT);
//    MenuHelper::AddStringItem(menu, IDM_BACKGROUND_CONTEXT_MENU_PASTE_SHORTCUT, text, position++,
//        true);
//
//    if (!CanPasteInDirectory(folderPidl, PasteType::Shortcut))
//    {
//        MenuHelper::EnableItem(menu, IDM_BACKGROUND_CONTEXT_MENU_PASTE_SHORTCUT, false);
//    }
//
//    MenuHelper::AddSeparator(menu, position++, true);
//}

void ShellContextMenu::RemoveTencentDeskGoMenu(HMENU hmenu)
{

    // 找到菜单项的位置
    int menuPos = MenuHelper::FindMenuItem(hmenu, L"开启桌面整理");
    // TODO :删除腾讯桌面整理的菜单
    // 如果找到了，删除该菜单项
    if (menuPos != -1)
    {
        
        bool res = DeleteMenu(hmenu, menuPos, MF_BYPOSITION);
        if (res) {
            MenuHelper::RemoveAdjacentSeparators(hmenu);
        }
        

    }
}

// It's possible for a folder to not provide any IContextMenu instance (for example, the Home folder
// in Windows 11 doesn't provide any IContextMenu instance for the background menu). So, this method
// may return null.
wil::com_ptr_nothrow<IContextMenu> ShellContextMenu::MaybeGetShellContextMenu(HWND hwnd) const
{
    //1.获取桌面的PIDL
    //2.获取绑定桌面的IShellFolder 接口
    //3.获取IContextMenu接口
    unique_pidl_absolute pidlDesktop;
    HRESULT hr = SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, wil::out_param(pidlDesktop));
    if (SUCCEEDED(hr))
    {
        // 获取 IShellFolder 接口
        wil::com_ptr_nothrow<IShellFolder> shellFolder;
        hr = BindToIdl(m_pidlParent.get(), IID_PPV_ARGS(&shellFolder));

        if (SUCCEEDED(hr))
        {
            wil::com_ptr_nothrow<IContextMenu> contextMenu;

            hr = shellFolder->CreateViewObject(hwnd, IID_PPV_ARGS(&contextMenu));

            if (FAILED(hr))
            {
                return nullptr;
            }
            return contextMenu;
        }
    }
    return nullptr;
}

// Returns the parsing path for the current directory, but only if it's a filesystem path.
std::optional<std::string> ShellContextMenu::MaybeGetFilesystemDirectory() const
{
    wil::com_ptr_nothrow<IShellItem> shellItem;
    unique_pidl_absolute pidlDesktop;
    HRESULT hr = SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, wil::out_param(pidlDesktop));

    hr = SHCreateItemFromIDList(pidlDesktop.get(), IID_PPV_ARGS(&shellItem));

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    wil::unique_cotaskmem_string parsingPath;
    hr = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &parsingPath);

    if (SUCCEEDED(hr))
    {
        return WstrToStr(parsingPath.get());
    }

    // In Windows Explorer, it appears that when a menu item is invoked from the background context
    // menu in a library folder, the directory that's used is the default directory for that
    // library. The functionality here tries to mimic that behavior.
    // Without this, menu items like "Git Bash Here" wouldn't work when invoked from a library
    // directory (since a library doesn't have a filesystem path, even though it may be filesystem
    // backed).
    wil::com_ptr_nothrow<IShellLibrary> shellLibrary;
    hr = SHLoadLibraryFromItem(shellItem.get(), STGM_READ, IID_PPV_ARGS(&shellLibrary));

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    wil::com_ptr_nothrow<IShellItem> defaultLocation;
    hr = shellLibrary->GetDefaultSaveFolder(DSFT_DETECT, IID_PPV_ARGS(&defaultLocation));

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    hr = defaultLocation->GetDisplayName(SIGDN_FILESYSPATH, &parsingPath);

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    return WstrToStr(parsingPath.get());
}

wil::unique_hmenu ShellContextMenu::BuildViewMenu()
{

    return wil::unique_hmenu();
}

wil::unique_hmenu ShellContextMenu::BuildSortMenu()
{

    return wil::unique_hmenu();
}

ShellContextMenu::ShellContextMenu(PCIDLIST_ABSOLUTE pidlParent) :
    m_pidlParent(pidlParent ? ILCloneFull(pidlParent) : nullptr)
{

}

LRESULT ShellContextMenu::ParentWindowSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_INITMENUPOPUP:
    case WM_MENUCHAR:
        // wParam is 0 if this item was sent by a menu.
        if ((msg == WM_MEASUREITEM || msg == WM_DRAWITEM) && wParam != 0)
        {
            break;
        }

        if (!m_contextMenu)
        {
            break;
        }

        if (auto contextMenu3 = m_contextMenu.try_query<IContextMenu3>())
        {
            LRESULT result;
            HRESULT hr = contextMenu3->HandleMenuMsg2(msg, wParam, lParam, &result);

            if (SUCCEEDED(hr))
            {
                return result;
            }
        }
        else if (auto contextMenu2 = m_contextMenu.try_query<IContextMenu2>())
        {
            contextMenu2->HandleMenuMsg(msg, wParam, lParam);
        }
        break;

    //case WM_MENUSELECT:
    //{
    //    if (!m_statusBar)
    //    {
    //        break;
    //    }

    //    if (HIWORD(wParam) == 0xFFFF && lParam == 0)
    //    {
    //        m_statusBar->HandleStatusBarMenuClose();
    //    }
    //    else
    //    {
    //        m_statusBar->HandleStatusBarMenuOpen();

    //        UINT menuItemId = LOWORD(wParam);

    //        if (WI_IsAnyFlagSet(HIWORD(wParam), MF_POPUP | MF_SEPARATOR))
    //        {
    //            m_statusBar->SetPartText(0, L"");
    //        }
    //        else if (menuItemId >= MIN_SHELL_MENU_ID && menuItemId <= MAX_SHELL_MENU_ID)
    //        {
    //            //CHECK(m_contextMenu);

    //            TCHAR helpString[512];
    //            HRESULT hr = m_contextMenu->GetCommandString(menuItemId - MIN_SHELL_MENU_ID,
    //                GCS_HELPTEXT, nullptr, reinterpret_cast<LPSTR>(helpString),
    //                static_cast<UINT>(std::size(helpString)));

    //            if (FAILED(hr))
    //            {
    //                StringCchCopy(helpString, std::size(helpString), L"");
    //            }

    //            m_statusBar->SetPartText(0, helpString);
    //        }
    //        else
    //        {
    //            std::wstring helpString = m_handler->GetHelpTextForItem(menuItemId);
    //            m_statusBar->SetPartText(0, helpString.c_str());
    //        }
    //    }

    //    // Prevent the message from been passed onto the original window.
    //    return 0;
    //}
    break;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void ShellContextMenu::show(HWND hwnd) {

    m_contextMenu = MaybeGetShellContextMenu(hwnd);
    if (m_contextMenu) {
        // 创建空的弹出式菜单
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) {
            OutputDebugString(L"Failed to create popup menu.");
            return;
        }

        /*IUnknown* site = NULL;
        if (site; auto objectWithSite = m_contextMenu.try_query<IObjectWithSite>())
        {
            objectWithSite->SetSite(site);
        }*/

        m_contextMenu->QueryContextMenu(hMenu, 0, MIN_SHELL_MENU_ID, MAX_SHELL_MENU_ID,
            CMF_NORMAL);
        RemoveTencentDeskGoMenu(hMenu);

        // 使用 std::bind 绑定 this 指针
        auto boundFunction = std::bind(&ShellContextMenu::ParentWindowSubclass, this, hwnd, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        // Subclass the owner window, so that menu messages can be handled.
        // 使用 lambda 表达式绑定 this 指针
        auto handlerFunction = [this](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
            return this->ParentWindowSubclass(hWnd, msg, wParam, lParam);
            };
        // 创建 WindowSubclassWrapper 实例
        auto winWrapper = std::make_unique<WindowSubclassWrapper>(hwnd, handlerFunction);

        UINT cmd =
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD, 200, 200, 0, hwnd, nullptr);
        if (cmd == 0)
        {
            return;
        }

        if (cmd >= MIN_SHELL_MENU_ID && cmd <= MAX_SHELL_MENU_ID)
        {

            TCHAR verb[64] = _T("");
            HRESULT hr = m_contextMenu->GetCommandString(cmd - MIN_SHELL_MENU_ID, GCS_VERB, nullptr,
                reinterpret_cast<LPSTR>(verb), static_cast<UINT>(std::size(verb)));

            bool handled = false;
            wprintf_s(&verb[0]);
            // Pass the menu back to the caller to give it the chance to handle it.
            //if (SUCCEEDED(hr))
            //{
            //    handled = m_handler->HandleShellMenuItem(m_pidlParent.Raw(), m_pidlItems, verb);
            //}

            if (!handled)
            {
                std::optional<std::string> parsingPathOpt = MaybeGetFilesystemDirectory();

                // Note that some menu items require the directory field to be set. For example, the
                // "Git Bash Here" item (which is added by Git for Windows) requires that.
                CMINVOKECOMMANDINFO commandInfo = {};
                commandInfo.cbSize = sizeof(commandInfo);
                commandInfo.fMask = 0;
                commandInfo.hwnd = hwnd;
                commandInfo.lpVerb = reinterpret_cast<LPCSTR>(MAKEINTRESOURCE(cmd - MIN_SHELL_MENU_ID));
                commandInfo.lpDirectory = parsingPathOpt ? parsingPathOpt->c_str() : nullptr;
                commandInfo.nShow = SW_SHOWNORMAL;
                m_contextMenu->InvokeCommand(&commandInfo);
            }
        }
        else
        {
            //m_handler->HandleCustomMenuItem(m_pidlParent.Raw(), m_pidlItems, cmd);
        }
    }
}
void ShellContextMenu::ShowMenu(HWND hwnd, const POINT* pt, IUnknown* site, Flags flags) {
    m_contextMenu = MaybeGetShellContextMenu(hwnd);
    if (m_contextMenu) {
        // 创建空的弹出式菜单
        wil::unique_hmenu hMenu(CreatePopupMenu());
        if (!hMenu) {
            OutputDebugString(L"Failed to create popup menu.");
            return;
        }

        IUnknown* site = NULL;
        if (site; auto objectWithSite = m_contextMenu.try_query<IObjectWithSite>())
        {
            objectWithSite->SetSite(site);
        }

        m_contextMenu->QueryContextMenu(hMenu.get(), 0, MIN_SHELL_MENU_ID, MAX_SHELL_MENU_ID,
            CMF_NODEFAULT);
        UINT cmd =
            TrackPopupMenu(hMenu.get(), TPM_LEFTALIGN | TPM_RETURNCMD, 200, 200, 0, hwnd, nullptr);
        if (cmd == 0)
        {
            return;
        }

        if (cmd >= MIN_SHELL_MENU_ID && cmd <= MAX_SHELL_MENU_ID)
        {

            TCHAR verb[64] = _T("");
            HRESULT hr = m_contextMenu->GetCommandString(cmd - MIN_SHELL_MENU_ID, GCS_VERB, nullptr,
                reinterpret_cast<LPSTR>(verb), static_cast<UINT>(std::size(verb)));

            bool handled = false;
            wprintf_s(&verb[0]);
            // Pass the menu back to the caller to give it the chance to handle it.
            //if (SUCCEEDED(hr))
            //{
            //    handled = m_handler->HandleShellMenuItem(m_pidlParent.Raw(), m_pidlItems, verb);
            //}
            
            //auto viewMenus = BuildViewsMenu();

            if (!handled)
            {
                std::optional<std::string> parsingPathOpt = MaybeGetFilesystemDirectory();

                // Note that some menu items require the directory field to be set. For example, the
                // "Git Bash Here" item (which is added by Git for Windows) requires that.
                CMINVOKECOMMANDINFO commandInfo = {};
                commandInfo.cbSize = sizeof(commandInfo);
                commandInfo.fMask = 0;
                commandInfo.hwnd = hwnd;
                commandInfo.lpVerb = reinterpret_cast<LPCSTR>(MAKEINTRESOURCE(cmd - MIN_SHELL_MENU_ID));
                commandInfo.lpDirectory = parsingPathOpt ? parsingPathOpt->c_str() : nullptr;
                commandInfo.nShow = SW_SHOWNORMAL;
                m_contextMenu->InvokeCommand(&commandInfo);
            }
        }
        else
        {
            //m_handler->HandleCustomMenuItem(m_pidlParent.Raw(), m_pidlItems, cmd);
        }
    }
}
void ShellContextMenu::DisplayContextMenu(HWND hWnd)
{
    
    // 初始化 COM ...
    CoInitialize(NULL);
    // 桌面路径
    TCHAR folderPath[MAX_PATH] = L"D:\\Desktop\\a.txt";
    // 解析路径获取 ITEMIDLIST
    LPITEMIDLIST pidl = nullptr;
    HRESULT hr = SHParseDisplayName(folderPath, nullptr, &pidl, 0, nullptr);
    if (FAILED(hr) || pidl == nullptr) {
        // 处理错误，例如路径无效
        OutputDebugString(L"----------input path error");
    }
    wchar_t displayName[MAX_PATH];
    SHGetPathFromIDList(pidl, displayName);
    OutputDebugString(L"========================PIDL represents path: \n");
    OutputDebugString(displayName);
    OutputDebugString(L"========================PIDL represents path: \n");

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
    //hr = pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL);
    hr = pContextMenu->QueryContextMenu(hMenu, 0, MIN_SHELL_MENU_ID, MAX_SHELL_MENU_ID, CMF_NODEFAULT);
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

unique_pidl_absolute pidlDesktop;
HRESULT hr = SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, wil::out_param(pidlDesktop));
ShellContextMenu ssd(pidlDesktop.get());

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
                ssd.show(hWnd);
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

HMENU SysMenuHelper::AddSubMenuItem(HMENU hMenu, UINT id, const std::wstring& text, HMENU subMenu, UINT pos, bool enable)
{
    return HMENU();
}

void SysMenuHelper::AddStringItem(HMENU hMenu, UINT id, const std::wstring& text, UINT pos, bool enable)
{
}

void SysMenuHelper::AddSeparator(HMENU hMenu, UINT pos, bool enable)
{
}

void SysMenuHelper::EnableItem(HMENU hMenu, UINT id, bool enable)
{
}
