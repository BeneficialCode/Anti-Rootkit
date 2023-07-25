#include "stdafx.h"
#include "resource.h"
#include "ProcessMemoryTable.h"
#include <algorithm>
#include "SortHelper.h"
#include "DriverHelper.h"
#include "NtDll.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include "Helpers.h"

LRESULT CProcessMemoryTable::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	m_hProcess = DriverHelper::OpenProcess(m_Pid, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ);
	if (m_hProcess == nullptr)
		return -1;

	m_Tracker.reset(new WinSys::ProcessVMTracker(m_hProcess));
	if (m_Tracker == nullptr)
		return -1;

	m_hReadProcess.reset(DriverHelper::OpenProcess(m_Pid, PROCESS_VM_READ));
	Refresh();

	return 0;
}
LRESULT CProcessMemoryTable::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lparam, BOOL& /*bHandled*/) {
	return 0;
}
LRESULT CProcessMemoryTable::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	PaintTable(m_hWnd);
	return 0;
}

LRESULT CProcessMemoryTable::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	Tablefunction(m_hWnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT CProcessMemoryTable::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	Tablefunction(m_hWnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT CProcessMemoryTable::OnUserVabs(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	Tablefunction(m_hWnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT CProcessMemoryTable::OnUserVrel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	Tablefunction(m_hWnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT CProcessMemoryTable::OnUserChgs(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	Tablefunction(m_hWnd, uMsg, wParam, lParam);
	return 0;
}

LRESULT CProcessMemoryTable::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	return Tablefunction(m_hWnd, WM_VSCROLL, zDelta >= 0 ? 0 : 1, wParam);
}
LRESULT CProcessMemoryTable::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnLBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnLBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnRBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	CMenu menu;
	CMenuHandle hSubMenu;
	menu.LoadMenu(IDR_PROC_CONTEXT);
	hSubMenu = menu.GetSubMenu(2);
	POINT pt;
	::GetCursorPos(&pt);
	bool show = Tablefunction(m_hWnd, uMsg, wParam, lParam);
	if (show) {
		auto id = (UINT)TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, nullptr);
		if (id) {
			PostMessage(WM_COMMAND, id);
		}
	}

	return 0;
}
LRESULT CProcessMemoryTable::OnUserSts(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}
LRESULT CProcessMemoryTable::OnSysKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return Tablefunction(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CProcessMemoryTable::OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return DLGC_WANTARROWS;
}


CProcessMemoryTable::CProcessMemoryTable(BarInfo& bars, TableInfo& table, DWORD pid)
	: CTable(bars, table), m_Pid(pid) {
	SetTableWindowInfo(bars.nbar);
}

int CProcessMemoryTable::ParseTableEntry(CString& s, char& mask, int& select, std::shared_ptr<WinSys::MemoryRegionItem>& info, int column) {
	switch (column) {
	case 0:
		s = StateToString(info->State);
		break;
	case 1:
		s.Format(L"0x%p", info->BaseAddress);
		break;
	case 2:
		s.Format(L"0x%X", info->RegionSize);
		s += L" (" + FormatWithCommas(info->RegionSize >> 10) + L" KB) ";
		break;
	case 3:
		s = info->State != MEM_COMMIT ? L"" : TypeToString(info->Type);
		break;
	case 4:
		s = info->State != MEM_COMMIT ? CString() : ProtectionToString(info->Protect);
		break;
	case 5:
		s = info->State == MEM_FREE ? CString() : ProtectionToString(info->AllocationProtect);
		break;
	case 6:
		s = UsageToString(info);
		break;
	case 7:
		s = GetDetails(info).Details;
		break;
	}
	return s.GetLength();
}

void CProcessMemoryTable::Refresh() {
	if (m_Details.empty())
		m_Details.reserve(128);

	m_Tracker->EnumRegions();
	m_Table.data.info = m_Tracker->GetRegions();
	m_Details.clear();
	m_Details.reserve(m_Table.data.info.size() / 2);

	// enum threads
	m_ProcMgr.EnumProcessAndThreads(m_Pid);
	m_Threads = m_ProcMgr.GetThreads();

	// enum heaps
	m_Heaps.clear();
	m_Heaps.reserve(8);
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, m_Pid));
	if (hSnapshot) {
		HEAPLIST32 list;
		list.dwSize = sizeof(list);
		HEAPENTRY32 entry;
		entry.dwSize = sizeof(entry);
		int index = 1;
		::Heap32ListFirst(hSnapshot.get(), &list);
		do {
			HeapInfo hi;
			hi.Address = list.th32HeapID;
			hi.Flags = list.dwFlags;
			hi.Id = index++;
			m_Heaps.push_back(hi);
		} while (::Heap32ListNext(hSnapshot.get(), &list));
	}

	for (auto& mi : m_Table.data.info) {
		GetDetails(mi);
	}

	auto count = static_cast<int>(m_Table.data.info.size());
	m_Table.data.n = count;

	return;
}

PCWSTR CProcessMemoryTable::StateToString(DWORD state) {
	switch (state) {
	case MEM_COMMIT: return L"Committed";
	case MEM_FREE: return L"Free";
	case MEM_RESERVE: return L"Reserved";
	}
	ATLASSERT(false);
	return nullptr;
}

CString CProcessMemoryTable::ProtectionToString(DWORD protection) {
	static const struct {
		PCWSTR Text;
		DWORD Value;
	} prot[] = {
		{ L"", 0 },
		{ L"Execute", PAGE_EXECUTE },
		{ L"Execute/Read", PAGE_EXECUTE_READ },
		{ L"Execute/Read/Write", PAGE_EXECUTE_READWRITE },
		{ L"WriteCopy", PAGE_WRITECOPY },
		{ L"Execute/WriteCopy", PAGE_EXECUTE_WRITECOPY },
		{ L"No Access", PAGE_NOACCESS },
		{ L"Read", PAGE_READONLY },
		{ L"Read/Write", PAGE_READWRITE },
	};

	CString text = L"";
	auto it = std::find_if(std::begin(prot), std::end(prot), [protection](auto& p) {
		return (p.Value & protection) != 0;
		});
	if (it != std::end(prot)) {
		text = it->Text;
	}
	else {
		WCHAR buffer[20];
		wsprintf(buffer, L"0x%x", protection);
		std::wstring msg = L"Failed to get the prot: ";
		msg += buffer;
		AtlMessageBox(nullptr, msg.c_str(), IDS_TITLE, MB_ICONERROR);
	}

	static const struct {
		PCWSTR Text;
		DWORD Value;
	} extra[] = {
		{ L"Guard", PAGE_GUARD },
		{ L"No Cache", PAGE_NOCACHE },
		{ L"Write Combine", PAGE_WRITECOMBINE },
		{ L"Targets Invalid", PAGE_TARGETS_INVALID },
		{ L"Targets No Update", PAGE_TARGETS_NO_UPDATE },
	};

	std::for_each(std::begin(extra), std::end(extra), [&text, protection](auto& p) {
		if (p.Value & protection) ((text += L"/") += p.Text);
		});

	return text;
}

PCWSTR CProcessMemoryTable::TypeToString(DWORD type) {
	switch (type) {
	case MEM_IMAGE: return L"Image";
	case MEM_PRIVATE: return L"Private";
	case MEM_MAPPED: return L"Mapped";
	}
	return L"";
}

CString CProcessMemoryTable::HeapFlagsToString(DWORD flags) {
	CString text;
	if (flags & HF32_DEFAULT)
		text += L" [Default]";
	if (flags & HF32_SHARED)
		text += L" [Shared]";
	return text;
}

CProcessMemoryTable::ItemDetails CProcessMemoryTable::GetDetails(const std::shared_ptr<WinSys::MemoryRegionItem>& mi) const {
	if (auto it = m_Details.find(mi->AllocationBase ? mi->AllocationBase : mi->BaseAddress); it != m_Details.end()) {
		return it->second;
	}

	ItemDetails details;
	details.Usage = MemoryUsage::Unknown;
	if (mi->State == MEM_FREE) {
		if (mi->RegionSize < (1 << 16)) {
			details.Usage = MemoryUsage::Unusable;
		}
		m_Details.insert({ mi->BaseAddress, details });
		return details;
	}

	if (mi->State == MEM_COMMIT) {
		if (mi->Type == MEM_IMAGE || mi->Type == MEM_MAPPED) {
			WCHAR filename[MAX_PATH];
			if (::GetMappedFileName(m_hProcess, mi->BaseAddress, filename, MAX_PATH) > 0) {
				details.Details = Helpers::GetDosNameFromNtName(filename).c_str();
				details.Usage = mi->Type == MEM_IMAGE ? MemoryUsage::Image : MemoryUsage::Mapped;
			}
		}
		else if (m_hReadProcess) {
			// try threads
			for (auto& t : m_Threads) {
				NT_TIB tib;
				if (::ReadProcessMemory(m_hReadProcess.get(), t->TebBase, &tib, sizeof(tib), nullptr)) {
					if (mi->BaseAddress >= tib.StackLimit && mi->BaseAddress < tib.StackBase) {
						details.Details.Format(L"Thread %u (0x%X) stack", t->Id, t->Id);
						details.Usage = MemoryUsage::ThreadStack;
						break;
					}
				}
			}
		}
	}
	if (details.Usage == MemoryUsage::Unknown) {
		for (auto& heap : m_Heaps) {
			if (mi->AllocationBase <= (PVOID)heap.Address && mi->AllocationBase != nullptr && (BYTE*)mi->AllocationBase + mi->RegionSize > (PVOID)heap.Address) {
				details.Usage = MemoryUsage::Heap;
				details.Details.Format(L"Heap %u %s", heap.Id, HeapFlagsToString(heap.Flags));
				break;
			}
		}
	}

	if (details.Usage != MemoryUsage::Unknown)
		m_Details.insert({ mi->AllocationBase, details });

	return details;
}

PCWSTR CProcessMemoryTable::UsageToString(const std::shared_ptr<WinSys::MemoryRegionItem>& item) const {
	auto it = m_Details.find(item->AllocationBase ? item->AllocationBase : item->BaseAddress);
	MemoryUsage usage;
	if (it != m_Details.end())
		usage = it->second.Usage;
	else
		usage = item->State == MEM_FREE ? MemoryUsage::Unknown : MemoryUsage::PrivateData;

	switch (usage) {
	case MemoryUsage::ThreadStack: return L"Stack";
	case MemoryUsage::Image: return L"Image File";
	case MemoryUsage::Mapped: return L"Mapped File";
	case MemoryUsage::Heap: return L"Heap";
	case MemoryUsage::PrivateData: return L"Data";
	case MemoryUsage::Unusable: return L"Unusable";
	}
	return L"";
}

CString CProcessMemoryTable::FormatWithCommas(long long size) {
	CString result;
	result.Format(L"%lld", size);
	int i = 3;
	while (result.GetLength() - i > 0) {
		result = result.Left(result.GetLength() - i) + L"," + result.Right(i);
		i += 4;
	}
	return result;
}

LRESULT CProcessMemoryTable::OnMemoryDump(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int selected = m_Table.data.selected;
	ATLASSERT(selected >= 0);
	auto& info = m_Table.data.info[selected];

	std::unique_ptr<byte[]> data = std::make_unique<byte[]>(info->RegionSize);
	SIZE_T dummy;
	bool success = ::ReadProcessMemory(m_hProcess, info->BaseAddress, data.get(), info->RegionSize, &dummy);
	if (!success) {
		AtlMessageBox(m_hWnd, L"Dump memory failed");
		return FALSE;
	}
	CSimpleFileDialog dlg(FALSE, nullptr, nullptr, OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT,
		L"All Files\0*.*\0", m_hWnd);
	if (IDOK == dlg.DoModal()) {
		HANDLE hFile = ::CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) {
			AtlMessageBox(m_hWnd, L"Failed to create file", IDS_TITLE, MB_ICONERROR);
			return 0;
		}
		DWORD bytes;
		if (!::WriteFile(hFile, data.get(), (ULONG)dummy, &bytes, nullptr)) {
			AtlMessageBox(m_hWnd, L"Failed to write data", IDS_TITLE, MB_ICONERROR);
		}
		::CloseHandle(hFile);
	}
	return 0;
}