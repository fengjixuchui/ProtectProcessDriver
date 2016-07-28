/*
*******************************************************************************
*= = �ļ����ƣ�ProtectProcess.h
*= = �ļ�����������ProtectProcess��ͷ�ļ�
*= = ��    �ߣ�indigo
*= = ��дʱ�䣺2016-07-09 19:18:00
*******************************************************************************
*/

#ifndef __PROTECTPROCESS_H__
#define __PROTECTPROCESS_H__

//*============================================================================ 
//*= = ͷ�ļ����� 
//*============================================================================ 

#include <ntifs.h>
#include <windef.h>
#include "ntddk.h"

#pragma comment (lib,"ksecdd.lib")

//*============================================================================ 
//*= = ����ṹ�� 
//*============================================================================ 

#define DEVICE_NAME			L"\\Device\\ProcessGuard"
#define SYMBOL_NAME			L"\\??\\ProcessGuard"
#define DRIVER_NAME			L"\\Driver\\ProcessGuard"
#define MAX_PROCESS			1000
//���������
#define PROTECTBYPID		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define UNPROTECTBYPID		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define KILLBYPID			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

//��Ȩ�޵Ķ���
#define PROCESS_TERMINATE         (0x0001)  // winnt
#define PROCESS_CREATE_THREAD     (0x0002)  // winnt
#define PROCESS_SET_SESSIONID     (0x0004)  // winnt
#define PROCESS_VM_OPERATION      (0x0008)  // winnt
#define PROCESS_VM_READ           (0x0010)  // winnt
#define PROCESS_VM_WRITE          (0x0020)  // winnt
#define PS_PROCESS_FLAGS_PROCESS_DELETE         0x00000008UL

//��������ָ������
typedef CCHAR(*GETPREVIOUSMODE)();

typedef PETHREAD(*PSGETNEXTPROCESSTHREAD) (PETHREAD Thread);

typedef NTSTATUS(*OBREFERENCEOBJECTBYNAME)(
	__in PUNICODE_STRING ObjectName,
	__in ULONG Attributes,
	__in_opt PACCESS_STATE AccessState,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,
	__in KPROCESSOR_MODE AccessMode,
	__inout_opt PVOID ParseContext,
	__out PVOID *Object
	);

extern POBJECT_TYPE *IoDriverObjectType;


//����ͨ�ýڵ�
typedef struct _LISTNODE {
	struct _LISTNODE* next;
	ULONG data;
} LISTNODE, *PLISTNODE;

//���������Ϣ
typedef struct _PROCESSINFO {
	ULONG ProcessId;
	WCHAR Name[MAX_PATH];
} PROCESSINFO, *PPROCESSINFO;

//���屣�����̽ڵ�
typedef struct _PROTECTINFO
{
	ULONG ProcessId;
	PLISTNODE AllowedUser;
} PROTECTINFO, *PPROTECTINFO;

#pragma pack(1)
typedef struct _SERVICEDESCRIPTORTABLE{
	PULONG ServiceTableBase;
	PULONG ServiceCounterTableBase;
	ULONG  NumberOfServices;
	PULONG ParmTableBase;
} SERVICEDESCRIPTORTABLE, *PSERVICEDESCRIPTORTABLE;
#pragma pack()

__declspec(dllimport) SERVICEDESCRIPTORTABLE KeServiceDescriptorTable;

//��������ͽ��̱�������
PLISTNODE ProtectList = NULL;

//win7 32λ�µĸ���ƫ��
ULONG g_ProcessIdOffset = 0xb4;					//����IDƫ��
ULONG g_ProcessNameOffset = 0x16c;				//������ָ��ƫ��
ULONG g_ProcessListOffset = 0xb8;				//��������ָ��ƫ��
ULONG g_ProcessFlagsOffset = 0x270;				//���̱�ʶƫ��
ULONG g_shadowssdtoffset = 0x50;				//������SSDT����ƫ��

ULONG g_PspTerminateThreadByPointerAddr;		//PspTerminateThreadByPointer������ַ
ULONG g_NtOpenProcessAddr;						//NtOpenProcess������ַ
ULONG g_NtTerminateProcessAddr;					//NtTerminateProcess������ַ
PSGETNEXTPROCESSTHREAD PsGetNextProcessThread;  //PsGetNextProcessThread������ַ

FAST_MUTEX mux_protect;							//���ٻ��������ڲ�������
PVOID g_ShareBuf;

#endif	// End of __PROTECTPROCESS_H__ 

//*============================================================================ 
//*= = �ļ����� 
//*============================================================================ 
