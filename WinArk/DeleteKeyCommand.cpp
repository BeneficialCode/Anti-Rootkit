#include "stdafx.h"
#include "DeleteKeyCommand.h"
#include "Registry.h"
#include "Helpers.h"

DeleteKeyCommand::DeleteKeyCommand(PCWSTR path, PCWSTR name, AppCommandCallback<DeleteKeyCommand> cb)
	:RegAppCommandBase(L"Delete Key " + CString(name), path, name, cb) {
}

bool DeleteKeyCommand::Execute() {
	auto key = Registry::OpenKey(_path, KEY_ENUMERATE_SUB_KEYS | DELETE | KEY_QUERY_VALUE);
	if (!key)
		return false;

	if (_savePath.IsEmpty()) {
		LARGE_INTEGER li;
		::QueryPerformanceCounter(&li);
		_savePath.Format(L"%llX", li.QuadPart);
	}
	CRegKey keyBacukup;
	auto error = keyBacukup.Create(HKEY_CURRENT_USER, DeletedPathBackup + _savePath, nullptr, 0, KEY_ALL_ACCESS);
	::SetLastError(error);
	if (!keyBacukup)
		return false;
	::SetLastError(error = ::RegCopyTree(key.Get(), _name, keyBacukup));
	if (ERROR_SUCCESS != error)
		return false;

	::SetLastError(error = ::RegDeleteTree(key.Get(), _name));
	if (ERROR_SUCCESS != error) {
		return false;
	}
	return InvokeCallback(true);
}

bool DeleteKeyCommand::Undo() {
	auto key = Registry::OpenKey(_path, KEY_CREATE_SUB_KEY);
	if (!key)
		return false;

	DWORD error;
	CRegKey keyBackup;
	::SetLastError(error = keyBackup.Open(HKEY_CURRENT_USER, DeletedPathBackup + _savePath, KEY_READ));
	if (!keyBackup)
		return false;

	CRegKey newKey;
	DWORD disp;
	error = newKey.Create(key.Get(), _name, nullptr, 0, KEY_ALL_ACCESS, nullptr, &disp);
	::SetLastError(error);
	if (error != ERROR_SUCCESS)
		return false;

	::SetLastError(error = ::RegCopyTree(keyBackup, nullptr, newKey));
	if (error != ERROR_SUCCESS)
		return false;

	return InvokeCallback(false);
}