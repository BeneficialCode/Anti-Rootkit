#pragma once
#include "Table.h"
#include "resource.h"
#include "ThreadInfoEx.h"
#include "ProcessManager.h"

class CProcessThreadTable :
	public CTable<std::shared_ptr<WinSys::ThreadInfo>>,
	public CWindowImpl<CProcessThreadTable> {
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	BEGIN_MSG_MAP(CProcessThreadTable)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_USER_VABS, OnUserVabs)
		MESSAGE_HANDLER(WM_USER_VREL, OnUserVrel)
		MESSAGE_HANDLER(WM_USER_CHGS, OnUserChgs)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLBtnDown)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLBtnUp)
		MESSAGE_HANDLER(WM_USER_STS, OnUserSts)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeyDown)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		COMMAND_ID_HANDLER(ID_THREAD_COPY, OnThreadCopy)
		COMMAND_ID_HANDLER(ID_THREAD_EXPORT, OnThreadExport)
	END_MSG_MAP()

	CProcessThreadTable(BarInfo& bars, TableInfo& table, DWORD pid = 0);

	int ParseTableEntry(CString& s, char& mask, int& select, std::shared_ptr<WinSys::ThreadInfo>& info, int column);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lparam, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUserVabs(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUserVrel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUserChgs(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnUserSts(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnSysKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnThreadCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnThreadExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


	enum class ThreadColumn {
		State, Id, ProcessId, ProcessName, CPUTime, CreateTime, Priority, BasePriority, Teb,
		WaitReason, StartAddress, Win32StartAddress, StackBase, StackLimit, ContextSwitches,
		KernelTime, UserTime, IoPriority, MemoryPriority, ComFlags, ComApartment,
		WaitTime, Module
	};

	void Refresh();
	ThreadInfoEx& GetThreadInfoEx(WinSys::ThreadInfo* ti) const;

	static PCWSTR ThreadStateToString(WinSys::ThreadState state);
	static PCWSTR WaitReasonToString(WinSys::WaitReason reason);
private:

	std::wstring GetSingleThreadInfo(std::shared_ptr<WinSys::ThreadInfo>& info);

private:
	WinSys::ProcessManager m_ProcMgr;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_NewThreads;
	std::vector<std::shared_ptr<WinSys::ThreadInfo>> m_TermThreads;
	mutable std::unordered_map<WinSys::ThreadInfo*, ThreadInfoEx> m_ThreadsEx;
	HFONT m_hFont;
	DWORD m_Pid;
};


