#include "pch.h"
#include "Memory.h"

// Windows8 ���Ͻ���ʹ��NonPagedPoolNx�滻��ִ�еķǷ�ҳ�ڴ����Ե�NonPagedPool
// tag һ�����������Ų飬���ڴ�й©��ϵͳ�����ȣ�windbg,PoolMon,���������������
void* _cdecl operator new(size_t size, POOL_TYPE type, ULONG tag) {
	auto p = tag == 0 ? ExAllocatePool(type, size) : ExAllocatePoolWithTag(type, size, tag);
	if (p == nullptr) {
		KdPrint(("Falied to allocate %d bytes\n", size));
		return nullptr;
	}
	memset(p, 0, size);
	return p;
}

void* _cdecl operator new(size_t size, POOL_TYPE pool,
	EX_POOL_PRIORITY priority, ULONG tag) {
	return ExAllocatePoolWithTagPriority(pool, size, tag, priority);
}

void* _cdecl operator new(size_t, void* p) {
	return p;
}

void _cdecl operator delete(void* p) {
	if (p)
		ExFreePool(p);
}

void __cdecl operator delete(void* p, size_t) {
	if (p) {
		ExFreePool(p);
		p = nullptr;
	}
}

void* _cdecl operator new[](size_t size, POOL_TYPE type, ULONG tag) {
	auto p = tag == 0 ? ExAllocatePool(type, size) : ExAllocatePoolWithTag(type, size, tag);
	if (p == nullptr) {
		KdPrint(("Falied to allocate %d bytes\n", size));
		return nullptr;
	}
	memset(p, 0, size);
	return p;
}

void _cdecl operator delete[](void* p, size_t) {
	if (p)
		ExFreePool(p);
}


