#pragma once

// ���������˼��ĳһ���߳��Ƿ�ɶ�λ��һ����,��ݹ麯������ʱ

// ��ֹͣ�̵߳�����ŵ�������ж�غ���ʱ��
// ��ע����DriverEntry�а�KeWaitForMultipleObjects����Ҫ����Դ��ǰ�����
struct Thread {
public:
	NTSTATUS CreateSystemThread(PKSTART_ROUTINE routine, PVOID context);
	NTSTATUS Close();

	static NTSTATUS SetPriporty(int id, int priority);
private:
	HANDLE _handle;
};