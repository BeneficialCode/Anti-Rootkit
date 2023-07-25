#pragma once
#include "SymbolHandler.h"

class SymbolHelper final {
public:
	static std::unique_ptr<SymbolInfo> GetSymbolFromAddress(DWORD64 address, PDWORD64 offset = nullptr);
	static ULONG64 GetKernelSymbolAddressFromName(PCSTR name);
	static ULONG64 GetWin32kSymbolAddressFromName(PCSTR name);
	static DWORD GetKernelStructMemberOffset(std::string name, std::string memberName);
	static DWORD GetKernelStructMemberSize(std::string name, std::string memberName);
	static DWORD GetKernelStructSize(std::string name);
	static DWORD GetFltmgrStructMemberOffset(std::string name, std::string memberName);
	static void Init();

	struct BitField {
		USHORT Position;
		USHORT Size;
	};
	static DWORD GetKernelBitFieldPos(std::string name, std::string fieldName);
private:

	static inline DWORD _win32kSize, _kernelSize;
	static inline DWORD64 _win32kBase, _kernelBase;
	static inline std::string _win32kPdb, _kernelPdb;
	static inline std::string _win32kModule, _kernelModule;
	static inline SymbolHandler _win32k;
	static inline SymbolHandler _kernel;
	static inline DWORD _fltmgrSize;
	static inline DWORD64 _fltmgrBase;
	static inline std::string _fltmgrPdb;
	static inline std::string _fltmgrModule;
	static inline SymbolHandler _fltmgr;
};