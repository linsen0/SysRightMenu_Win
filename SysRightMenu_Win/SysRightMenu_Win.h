#pragma once
#include "resource.h"
#include "resources.h"
#include "ResourceHelper.h"
#include <fstream>
#include <iostream>
#include <locale>
#include <optional>
#include <shlobj.h>
#include <shtypes.h>
#include <tchar.h>
#include <vector>
#include <wil/com.h>
#include <Windows.h>

class PidlHelper {

public:
	
}; 

using unique_pidl_absolute = wil::unique_cotaskmem_ptr<std::remove_pointer_t<PIDLIST_ABSOLUTE>>;
using unique_pidl_relative = wil::unique_cotaskmem_ptr<std::remove_pointer_t<PIDLIST_RELATIVE>>;
using unique_pidl_child = wil::unique_cotaskmem_ptr<std::remove_pointer_t<PITEMID_CHILD>>; 


class SysMenuHelper {
private:

public:
	static HMENU AddSubMenuItem(HMENU hMenu, UINT id, const std::wstring& text, HMENU subMenu, UINT pos, bool enable);
	static void AddStringItem(HMENU hMenu, UINT id, const std::wstring& text, UINT pos, bool enable);
	static void AddSeparator(HMENU hMenu, UINT pos, bool enable);
	static void EnableItem(HMENU hMenu, UINT id, bool enable);

};

// 视图菜单构建器
class ViewsMenuBuilder {
public:
	HMENU BuildViewsMenu() {
		HMENU hViewsMenu = CreatePopupMenu();

		// 添加不同的视图选项
		SysMenuHelper::AddStringItem(hViewsMenu, IDS_BACKGROUND_CONTEXT_MENU_VIEW, L"Large Icons", 0, true);


		return hViewsMenu;
	}
};

// 视图菜单构建器
class SortMenuBuilder {
public:
	HMENU BuildViewsMenu() {
		HMENU hSortMenu = CreatePopupMenu();

		// 添加不同的视图选项
		SysMenuHelper::AddStringItem(hSortMenu, IDS_SORTBY_NAME, L"Name", 0, true);

		return hSortMenu;
	}
};

static const int MIN_SHELL_MENU_ID = 1;
static const int MAX_SHELL_MENU_ID = 1000;

//class ShellContextMenu : private boost::noncopyable
class ShellContextMenu
{
public:
	enum class Flags
	{
		Standard = 0,
		Rename = 1 << 0,
		ExtendedVerbs = 1 << 1
	};



	/*ShellContextMenu(PIDLIST_ABSOLUTE *pidlParent, const std::vector<PCITEMID_CHILD>& pidlItems):
		m_pidlItems(pidlParent ? ILCloneFull(pidlParent) : nullptr) {
		
	}*/
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pidlParent">桌面PIDL</param>
	ShellContextMenu(PCIDLIST_ABSOLUTE pidlParent);

	void ShowMenu(HWND hwnd, const POINT* pt, IUnknown* site, Flags flags);
	void show(HWND hwnd);

	void DisplayContextMenu(HWND hWnd);
	/// <summary>	
	/// 去掉腾讯桌面整理的菜单
	/// </summary>
	void RemoveTencentDeskGoMenu(HMENU hMenu);

private:
	wil::com_ptr_nothrow<IContextMenu> MaybeGetShellContextMenu(HWND hwnd) const;	//获取文件Shell上下文接口
	std::optional<std::string> MaybeGetFilesystemDirectory() const;		//获取执行菜单Shell时，结构体需要的路径

	LRESULT ParentWindowSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		 
	wil::unique_hmenu BuildViewMenu();
	wil::unique_hmenu BuildSortMenu();


	const unique_pidl_absolute m_pidlParent;
	const std::vector<unique_pidl_child> m_pidlItems;
	//ShellContextMenuHandler* const m_handler;
	wil::com_ptr_nothrow<IContextMenu> m_contextMenu;
};

DEFINE_ENUM_FLAG_OPERATORS(ShellContextMenu::Flags);

std::string WstrToStr(const wchar_t* wstr);		//宽字符串转WString
HRESULT GetRootPidl(PIDLIST_ABSOLUTE* pidl);	//获取根节点的Pidl Desktop_Pidl
HRESULT BindToIdl(PIDLIST_ABSOLUTE pidl, REFIID riid, void** ppv);