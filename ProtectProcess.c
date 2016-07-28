/*
*******************************************************************************
*= = �ļ����ƣ�ProtectProcess.h
*= = �ļ�����������ProtectProcess��ͷ�ļ�
*= = ��    �ߣ�indigo
*= = ��дʱ�䣺2016-07-09 19:18:00
*******************************************************************************
*/

#include "ProtectProcess.h"

BOOLEAN unProtectProcessById(ULONG uPid, ULONG user);

//*============================================================================
//*= = �������ƣ�RemoveProtect
//*= = ����������ȥ���ڴ汣������ 
//*= = ��ڲ�����NULL 
//*= = ���ڲ�����VOID
//*============================================================================
VOID RemoveProtect()
{
	_asm
	{
		cli;								//���ж�
		mov eax, cr0;						//�Ĵ���CR0����ϵͳ�Ŀ��Ʊ�־�����ڿ��ƴ������Ĳ���ģʽ��״̬
		and eax, not 10000h;				//ȡeax�ĺ�16λ
		mov cr0, eax;						//д��cr0�Ĵ���
	}
}

//*============================================================================
//*= = �������ƣ�RemoveProtect
//*= = �����������ָ��ڴ汣������ 
//*= = ��ڲ�����NULL 
//*= = ���ڲ�����VOID
//*============================================================================
VOID ResumeProtect()
{
	_asm
	{
		mov eax, cr0;			    
		or  eax, 10000h;					//�ָ�֮ǰand�����Ľ��
		mov cr0, eax;
		sti;								//���ж�
	}
}

//*============================================================================
//*= = �������ƣ�StrEqual
//*= = �����������ַ����ȽϺ��� 
//*= = ��ڲ�����PWSTR,PWSTR
//*= = ���ڲ�����BOOLEAN
//*============================================================================
BOOLEAN StrEqual(PWSTR wstr1, PWSTR wstr2)
{
	PWCHAR PWC1, PWC2;
	PWC1 = wstr1;
	PWC2 = wstr2;

	while (MmIsAddressValid((PVOID)PWC1) && MmIsAddressValid((PVOID)PWC2))	{
		if (*(PWC1) == 0 && *(PWC2) == 0)
			return TRUE;
		if (*(PWC1) != *(PWC2))
			return FALSE;
		PWC1++;
		PWC2++;
	}
	return FALSE;
}

//*============================================================================
//*= = �������ƣ�StrEqual
//*= = �������������������� 
//*= = ��ڲ�����PLISTNODE,PLISTNODE
//*= = ���ڲ�����VOID
//*============================================================================
VOID InsertNode(PLISTNODE *head, PLISTNODE data)//data��ʾ�½ڵ�ĵ�ַ
{
	PLISTNODE p;

	if (*head == NULL){
		*head = data;
	}
	else{
		p = *head;
		while (p->next != NULL){
			p = p->next;
		}
		p->next = data;
	}
}

//*============================================================================
//*= = �������ƣ�RemoveNodeByUser
//*= = �������������û�ɾ������ڵ㺯�� 
//*= = ��ڲ�����PLISTNODE,ULONG
//*= = ���ڲ�����VOID
//*============================================================================
VOID RemoveNodeByUser(PLISTNODE *head, ULONG user)
{
	PLISTNODE p, t;

	p = *head;
	if (p == NULL){
		return;
	}

	//ͷ���
	if (StrEqual((PWSTR)p->data, (PWSTR)user)){
		*head = (*head)->next;
		ExFreePool((PVOID)p->data);
		ExFreePool((PVOID)p);
	}
	else{
	//��ͷ���
		while (p->next != NULL){
			if (StrEqual((PWSTR)p->next->data, (PWSTR)user))
			{
				t = p->next->next;
				ExFreePool((PVOID)p->next->data);
				ExFreePool((PVOID)p->next);
				p->next = t;
				break;
			}
			p = p->next;
		}
	}
}

//*============================================================================
//*= = �������ƣ�RemoveNode
//*= = ����������ɾ������ڵ㺯�� 
//*= = ��ڲ�����PLISTNODE,ULONG,ULONG
//*= = ���ڲ�����VOID
//*============================================================================
VOID RemoveNode(PLISTNODE *head, ULONG ProcessId, ULONG user)
{
	PLISTNODE p, a, b;
	PPROTECTINFO pt;

	ExAcquireFastMutex(&mux_protect);
	p = *head;
	if (p == NULL){								//����Ϊ��
		ExReleaseFastMutex(&mux_protect);
		return;
	}

	if (*(PULONG)(p->data) == ProcessId){		//����ͷ����
		pt = (PPROTECTINFO)p->data;
		a = pt->AllowedUser;					//��Ȩ�û�
		//ɾ�����нڵ�
		if (user == 0){
			while (a){
				b = a->next;
				ExFreePool((PVOID)a->data);		//�ͷ��ڴ�
				ExFreePool((PVOID)a);
				a = b;
			}
			*head = (*head)->next;
			ExFreePool((PVOID)p->data);
			ExFreePool((PVOID)p);
		}
		else{
			RemoveNodeByUser(&pt->AllowedUser, user);
		}
	}
	else{										//����λ�÷���
		while (p->next != NULL){
			if (*(PULONG)p->next->data == ProcessId){
				if (user == 0){
					pt = (PPROTECTINFO)p->next->data;
					a = pt->AllowedUser;
					while (a){
						b = a->next;
						ExFreePool((PVOID)a->data);
						ExFreePool((PVOID)a);
						a = b;
					}
					ExFreePool((PVOID)p->next->data);
					a = p->next->next;
					ExFreePool((PVOID)p->next);
					p->next = a;
				}
				else{
					RemoveNodeByUser(&((PPROTECTINFO)p->next->data)->AllowedUser, user);
				}
				break;
			}
			p = p->next;
		}
	}
	ExReleaseFastMutex(&mux_protect);
}

//*============================================================================
//*= = �������ƣ�IsInProtectList
//*= = �������������������� 
//*= = ��ڲ�����ULONG,ULONG,BOOLEAN
//*= = ���ڲ�����PLISTNODE
//*============================================================================
PLISTNODE IsInProtectList(ULONG ProcessId, ULONG user, BOOLEAN b_ignoreuser)
{
	PLISTNODE p, t;
	ExAcquireFastMutex(&mux_protect);//���뻥�����
	p = ProtectList;

	while (p){
		if (*(PULONG)(p->data) == ProcessId){
			if (user == 0 || b_ignoreuser){//δ���û��ߺ�����Ȩ�û�
				ExReleaseFastMutex(&mux_protect);
				return p;
			}
			t = ((PPROTECTINFO)(p->data))->AllowedUser;
			while (t){
				if (StrEqual((PWSTR)(t->data), (PWSTR)user)){
					ExReleaseFastMutex(&mux_protect);
					return p;
				}
				t = t->next;
			}
			ExReleaseFastMutex(&mux_protect);
			return NULL;
		}
		p = p->next;
	}
	ExReleaseFastMutex(&mux_protect);
	return NULL;
}

//*============================================================================
//*= = �������ƣ�GetListCount
//*= = ������������ȡ���нڵ���Ϣ���� 
//*= = ��ڲ�����PLISTNODE,PULONG
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetListCount(IN PLISTNODE list, OUT PULONG PidTable)
{
	ULONG count = 0;

	while (list){
		PidTable[count] = *(PULONG)list->data;
		count++;
		list = list->next;
	}
	return count;
}

//*============================================================================
//*= = �������ƣ�FreeList
//*= = �����������ͷ����нڵ㺯�� 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����VOID
//*============================================================================
VOID FreeList()
{
	PULONG PidTable;
	ULONG count, i;

	PidTable = (PULONG)ExAllocatePool(NonPagedPool, sizeof(ULONG)*MAX_PROCESS);
	memset((PVOID)PidTable, 0, sizeof(ULONG)*MAX_PROCESS);

	ExAcquireFastMutex(&mux_protect);

	count = GetListCount(ProtectList, PidTable);

	ExReleaseFastMutex(&mux_protect);
	for (i = 0; i < count; i++)
	{
		unProtectProcessById(PidTable[i], 0);
	}
	ExFreePool(PidTable);
}

//*============================================================================
//*= = �������ƣ�GetFuncAddr
//*= = ������������ȡָ�������ڴ��ַ���� 
//*= = ��ڲ�����PWSTR
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetFuncAddr(PWSTR pwszFuncName)
{
	UNICODE_STRING uniFuncName;
	RtlInitUnicodeString(&uniFuncName, pwszFuncName);
	return (ULONG)MmGetSystemRoutineAddress(&uniFuncName);
}

//*============================================================================
//*= = �������ƣ�GetPspTerminateThreadByPointerAddr
//*= = ������������ȡPspTerminateSystemThread������ַ 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetPspTerminateThreadByPointerAddr()
{
	ULONG FunAddress = 0, i = 0;

	//�Ȼ�ȡPsTerminateSystemThread���������ַ.��������PspTerminateThreadByPointer
	//�����������û�б�����,����ͨ����������.��λcall Ȼ��call�ĵ�ַ�������㼴��
	FunAddress = GetFuncAddr(L"PsTerminateSystemThread");

	//�����ȡʧ��
	if (FunAddress == 0){
		return 0L;
	}

	//Ŀ���ַ=����ָ��ĵ�ַ+������E8����������32λ��
	for (i = FunAddress; i < FunAddress + 0xff; i++){
		if (*(PUCHAR)i == 0x50 && *(PUCHAR)(i + 1) == 0xe8){//������ 0x50 0xe8����push eax call
			return (ULONG)(*(PULONG)(i + 2) + i + 2 + 4);
		}
	}
	return 0L;
}

//*============================================================================
//*= = �������ƣ�GetNtOpenProcessAddr
//*= = ������������ȡNtOpenProcess������ַ 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetNtOpenProcessAddr()
{
	ULONG uZwFuncAddr = 0, uIndex = 0;

	uZwFuncAddr = GetFuncAddr(L"ZwOpenProcess");
	if (uZwFuncAddr == 0){
		return 0L;
	}

	uIndex = *(PULONG)(uZwFuncAddr + 1);
	return KeServiceDescriptorTable.ServiceTableBase[uIndex];
}

//*============================================================================
//*= = �������ƣ�GetNtTerminateProcessAddr
//*= = ������������ȡNtTerminateProcess������ַ 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetNtTerminateProcessAddr()
{
	ULONG uZwFuncAddr = 0, uIndex = 0;

	uZwFuncAddr = GetFuncAddr(L"ZwTerminateProcess");
	if (uZwFuncAddr == 0){
		return 0L;
	}

	uIndex = *(PULONG)(uZwFuncAddr + 1);
	return KeServiceDescriptorTable.ServiceTableBase[uIndex];
}

//*============================================================================
//*= = �������ƣ�GetPsGetNextProcessThreadAddr
//*= = ������������ȡPsGetNextProcessThread������ַ 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetPsGetNextProcessThreadAddr()
{
	ULONG uFuncAddr = 0, i = 0;

	uFuncAddr = GetFuncAddr(L"PsResumeProcess");
	if (uFuncAddr == 0){
		return 0L;
	}

	for (i = uFuncAddr; i < uFuncAddr + 0xff; i++){
		if (*(PUCHAR)i == 0x08 && *(PUCHAR)(i + 1) == 0xe8){//������0x08 0xe8
			return (ULONG)(*(PULONG)(i + 2) + 5 + i + 1);
		}
	}
	return 0L;
}

//*============================================================================
//*= = �������ƣ�GetProcessOwner
//*= = ������������ȡ���̵������û� (local unique)
//*= = ��ڲ�����PEPROCESS
//*= = ���ڲ�����ULONG
//*============================================================================
ULONG GetProcessOwner(PEPROCESS Process)
{
	NTSTATUS status;
	PACCESS_TOKEN token;
	LUID luid;
	PSECURITY_USER_DATA secdata;
	ULONG user;

	token = PsReferencePrimaryToken(Process);
	status = SeQueryAuthenticationIdToken(token, &luid);
	if (!NT_SUCCESS(status)){
		PsDereferencePrimaryToken(token);
		return 0L;
	}
	PsDereferencePrimaryToken(token);

	//�����û���luidȡ�û���
	status = GetSecurityUserInfo(&luid, UNDERSTANDS_LONG_NAMES, &secdata);
	if (!NT_SUCCESS(status)){
		return 0L;
	}
	
	//�������Ҫ�Լ��ͷ�
	user = (ULONG)ExAllocatePool(NonPagedPool, secdata->UserName.Length + sizeof(WCHAR));
	if (!user){
		return 0L;
	}
	memset((PVOID)user, 0, secdata->UserName.Length + sizeof(WCHAR));
	memcpy((PVOID)user, (PVOID)secdata->UserName.Buffer, secdata->UserName.Length);
	return user;
}

//*============================================================================
//*= = �������ƣ�GetCsrssEprocess
//*= = ������������ȡcsrss���̾�� 
//*= = ��ڲ�����NULL
//*= = ���ڲ�����PEPROCESS
//*============================================================================
PEPROCESS GetCsrssEprocess()//����ά��Windows�Ŀ���
{
	PEPROCESS Process;
	PLIST_ENTRY List, p;
	PSTR Name;

	Process = PsGetCurrentProcess();
	List = (PLIST_ENTRY)((ULONG)Process + g_ProcessListOffset);
	p = List;
	do
	{
		Name = (PSTR)((ULONG)p + g_ProcessNameOffset - g_ProcessListOffset);
		if (strstr(Name, "csrss.exe"))//�ж��Ƿ���csrss����
			return (PEPROCESS)((ULONG)p - g_ProcessListOffset);
		p = p->Blink;//ָ��ǰһ��Ԫ��
	} while (p != List);
	return NULL;
}

//*============================================================================
//*= = �������ƣ�OriginPspTerminateThreadByPointer
//*= = ������������ת��ԭ���Ĵ�����PspTerminateThreadByPointer
//*= = ��ڲ�����PETHREAD,NTSTATUS,BOOLEAN
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS __declspec(naked) OriginPspTerminateThreadByPointer(
	IN PETHREAD Thread,
	IN NTSTATUS ExitStatus,
	IN BOOLEAN DirectTerminate
	)
{
	_asm
	{
		mov edi, edi;
		push ebp;
		mov ebp, esp;
		mov eax, [g_PspTerminateThreadByPointerAddr];
		add eax, 5;
		jmp eax;
	}
}

//*============================================================================
//*= = �������ƣ�OriginNtOpenProcess
//*= = ������������ת��ԭ���Ĵ�����NtOpenProcess 
//*= = ��ڲ�����PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PCLIENT_ID
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS __declspec(naked) OriginNtOpenProcess(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	)
{
	_asm
	{
		mov edi, edi;
		push ebp;
		mov ebp, esp;
		mov eax, [g_NtOpenProcessAddr];
		add eax, 5;
		jmp eax;
	}
}

//*============================================================================
//*= = �������ƣ�OriginNtTerminateProcess
//*= = ������������ת��ԭ����NtTerminateProcess����
//*= = ��ڲ�����HANDLE,NTSTATUS
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS __declspec(naked) OriginNtTerminateProcess(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	)
{
	_asm
	{
		mov edi, edi;
		push ebp;
		mov ebp, esp;
		mov eax, [g_NtTerminateProcessAddr];
		add eax, 5;
		jmp eax;
	}
}

//*============================================================================
//*= = �������ƣ�HookPspTerminateThreadByPointer
//*= = ����������Hook��PspTerminateThreadByPointer���� 
//*= = ��ڲ�����PETHREAD,NTSTATUS,BOOLEAN
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS HookPspTerminateThreadByPointer(
	IN PETHREAD Thread,
	IN NTSTATUS ExitStatus,
	IN BOOLEAN DirectTerminate
	)
{
	PEPROCESS Killer, Prey;
	ULONG KillerId, PreyId, user;
	Killer = PsGetCurrentProcess();
	Prey = IoThreadToProcess(Thread);
	KillerId = *(PULONG)((ULONG)Killer + g_ProcessIdOffset);
	PreyId = *(PULONG)((ULONG)Prey + g_ProcessIdOffset);
	if (KillerId != PreyId)
	{
		if (IsInProtectList(PreyId, 0, TRUE))//�鿴�����б�
		{
			user = GetProcessOwner(Killer);
			if (!IsInProtectList(PreyId, user, FALSE))//�鿴�����б�
			{
				if (user)
					ExFreePool((PVOID)user);
				KdPrint(("[*]HookPspTerminateThreadByPointer:stop %s killing %s", (PUCHAR)((ULONG)Killer + g_ProcessNameOffset), (PUCHAR)((ULONG)Prey + g_ProcessNameOffset)));
				return STATUS_ACCESS_DENIED;
			}
			if (user)
				ExFreePool((PVOID)user);
		}
	}
	return OriginPspTerminateThreadByPointer(Thread, ExitStatus, DirectTerminate);
}

//*============================================================================
//*= = �������ƣ�HookNtOpenProcess
//*= = ����������Hook��NtOpenProcess���� 
//*= = ��ڲ�����PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PCLIENT_ID
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS HookNtOpenProcess(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
	)
{
	PEPROCESS Prey, Killer;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG KillerId, PreyId, user;

	status = PsLookupProcessByProcessId(ClientId->UniqueProcess, &Prey);
	if (NT_SUCCESS(status))
	{

		ObDereferenceObject(Prey);
		Killer = PsGetCurrentProcess();
		KillerId = *(PULONG)((ULONG)Killer + g_ProcessIdOffset);
		PreyId = *(PULONG)((ULONG)Prey + g_ProcessIdOffset);

		if (KillerId != PreyId)
		{
			if (IsInProtectList(PreyId, 0, TRUE))
			{
				user = GetProcessOwner(Killer);
				KdPrint(("%ws", (PWSTR)user));
				if (!IsInProtectList(PreyId, user, FALSE))
				{
					DesiredAccess &= ~PROCESS_TERMINATE;
					DesiredAccess &= ~PROCESS_CREATE_THREAD;
					DesiredAccess &= ~PROCESS_SET_SESSIONID;
					DesiredAccess &= ~PROCESS_VM_OPERATION;
					DesiredAccess &= ~PROCESS_VM_READ;
					DesiredAccess &= ~PROCESS_VM_WRITE;
					KdPrint(("[*]HookNtOpenProcess: limit %s's using of %s", (PUCHAR)((ULONG)Killer + g_ProcessNameOffset), (PUCHAR)((ULONG)Prey + g_ProcessNameOffset)));
				}
				if (user)

					ExFreePool((PVOID)user);
			}
		}
	}
	return OriginNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);//����֮�󴫻�ϵͳԭ������������
}

//*============================================================================
//*= = �������ƣ�HookNtTerminateProcess
//*= = ����������Hook��NtTerminateProcess����
//*= = ��ڲ�����HANDLE,NTSTATUS
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS HookNtTerminateProcess(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	)
{
	PEPROCESS Killer, Prey;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	GETPREVIOUSMODE KeGetPreviousMode;
	ULONG KillerId, PreyId, user;

	KeGetPreviousMode = (GETPREVIOUSMODE)GetFuncAddr(L"KeGetPreviousMode");
	status = ObReferenceObjectByHandle(ProcessHandle, PROCESS_TERMINATE, *PsProcessType, KeGetPreviousMode(), &Prey, NULL);
	if (NT_SUCCESS(status))
	{
		ObDereferenceObject(Prey);
		Killer = PsGetCurrentProcess();

		KillerId = *(PULONG)((ULONG)Killer + g_ProcessIdOffset);
		PreyId = *(PULONG)((ULONG)Prey + g_ProcessIdOffset);
		if (KillerId != PreyId)
		{
			if (IsInProtectList(PreyId, 0, TRUE))
			{
				user = GetProcessOwner(Killer);
				if (!IsInProtectList(PreyId, user, FALSE))
				{
					if (user)
						ExFreePool((PVOID)user);
					KdPrint(("[*]HookNtTerminateProcess:stop %s killing %s", (PUCHAR)((ULONG)Killer + g_ProcessNameOffset), (PUCHAR)((ULONG)Prey + g_ProcessNameOffset)));
					return STATUS_ACCESS_DENIED;
				}
				if (user)
					ExFreePool((PVOID)user);
			}
		}

	}
	return OriginNtTerminateProcess(ProcessHandle, ExitStatus);
}

//*============================================================================
//*= = �������ƣ�UsePsGetNextProcessThread
//*= = ��������������PsGetNextProcessThread������ȡ��һ���̵߳ľ�� 
//*= = ��ڲ�����PEPROCESS��PETHREAD
//*= = ���ڲ�����PETHREAD
//*============================================================================
PETHREAD UsePsGetNextProcessThread(
	IN PEPROCESS Process,
	IN PETHREAD Thread
	)
{
	PETHREAD nextThread = NULL;

	_asm
	{
		mov  eax, Process;
		push Thread;
		call PsGetNextProcessThread;
		mov  nextThread, eax;
	}
	return nextThread;
}

//*============================================================================
//*= = �������ƣ�HookSystemRoutine
//*= = ������������װ����
//*= = ��ڲ�����ULONG��ULONG
//*= = ���ڲ�����VOID
//*============================================================================
VOID HookSystemRoutine(ULONG uOldAddr, ULONG uNewAddr)
{
	UCHAR jmpcode[5];

	if (!(MmIsAddressValid((PVOID)uOldAddr) && MmIsAddressValid((PVOID)uNewAddr)))
	{
		KdPrint(("[*]Invalid address!"));
		return;
	}
	jmpcode[0] = 0xe9;
	*(PULONG)(jmpcode + 1) = uNewAddr - uOldAddr - 5;
	RemoveProtect();
	memcpy((PVOID)uOldAddr, (PVOID)jmpcode, 5);
	ResumeProtect();
}

//*============================================================================
//*= = �������ƣ�UnhookSystemRoutine
//*= = ����������ж�ع��� 
//*= = ��ڲ�����ULONG��PCHAR
//*= = ���ڲ�����VOID
//*============================================================================
VOID UnhookSystemRoutine(ULONG uOldAddr, PCHAR pOriCode)
{
	if (!MmIsAddressValid((PVOID)uOldAddr))
	{
		KdPrint(("[*]Invalid address"));
		return;
	}
	RemoveProtect();
	memcpy((PVOID)uOldAddr, (PVOID)pOriCode, 5);
	ResumeProtect();
}

//*============================================================================
//*= = �������ƣ�ProtectProcessById
//*= = ������������ӽ��̵��������� 
//*= = ��ڲ�����ULONG��ULONG
//*= = ���ڲ�����BOOLEAN
//*============================================================================
BOOLEAN ProtectProcessById(ULONG uPid, ULONG user)
{
	PLISTNODE node, p;
	PPROTECTINFO protectinfo;

	if (IsInProtectList(uPid, user, FALSE))
	{
		return FALSE;
	}
	else if ((p = IsInProtectList(uPid, user, TRUE)) != NULL)
	{
		protectinfo = (PPROTECTINFO)p->data;
		node = (PLISTNODE)ExAllocatePool(NonPagedPool, sizeof(LISTNODE));
		node->next = NULL;
		node->data = user;
		ExAcquireFastMutex(&mux_protect);
		InsertNode(&protectinfo->AllowedUser, node);
		ExReleaseFastMutex(&mux_protect);
	}
	else
	{
		node = (PLISTNODE)ExAllocatePool(NonPagedPool, sizeof(LISTNODE));
		protectinfo = (PPROTECTINFO)ExAllocatePool(NonPagedPool, sizeof(PROTECTINFO));
		node->next = NULL;
		node->data = (ULONG)protectinfo;
		protectinfo->ProcessId = uPid;
		protectinfo->AllowedUser = NULL;
		if (user != 0)
		{
			p = (PLISTNODE)ExAllocatePool(NonPagedPool, sizeof(LISTNODE));
			p->next = NULL;
			p->data = user;
			InsertNode(&protectinfo->AllowedUser, p);
			protectinfo->AllowedUser = p;
		}
		ExAcquireFastMutex(&mux_protect);
		InsertNode(&ProtectList, node);
		ExReleaseFastMutex(&mux_protect);
	}
	return TRUE;
}

//*============================================================================
//*= = �������ƣ�unProtectProcessById
//*= = �����������ڱ���������ȥ������
//*= = ��ڲ�����ULONG��ULONG
//*= = ���ڲ�����ULONG
//*============================================================================
BOOLEAN unProtectProcessById(ULONG uPid, ULONG user)
{
	//�жϽ����Ƿ�����ڱ���������
	if (IsInProtectList(uPid, user, FALSE) == NULL){
		return FALSE;
	}
	RemoveNode(&ProtectList, uPid, user);
	return TRUE;
}

//*============================================================================
//*= = �������ƣ�KillProcessById
//*= = ����������ͨ��IDɱ������ 
//*= = ��ڲ�����ULONG
//*= = ���ڲ�����BOOLEAN
//*============================================================================
BOOLEAN KillProcessById(ULONG uPid)
{
	PEPROCESS Process;
	PETHREAD Thread;
	NTSTATUS status;

	//�ж�PsGetNextProcessThread������ַ�Ƿ����
	if (!MmIsAddressValid((PVOID)(ULONG)PsGetNextProcessThread))
	{
		KdPrint(("[*]PsGetNextProcessThread address is invalid."));
		return FALSE;
	}

	//��ȡ���̾��
	status = PsLookupProcessByProcessId((HANDLE)uPid, &Process);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[*]Get process handle failed in KillProcessById function."));
		return FALSE;
	}

	unProtectProcessById(uPid, 0);

	//����ɱ�����̵������߳�
	RtlInterlockedSetBitsDiscardReturn((PULONG)((ULONG)Process + g_ProcessFlagsOffset), PS_PROCESS_FLAGS_PROCESS_DELETE);
	for (Thread = UsePsGetNextProcessThread(Process, NULL); Thread != NULL; Thread = UsePsGetNextProcessThread(Process, Thread))
	{
		HookPspTerminateThreadByPointer(Thread, 0, TRUE);
	}
	ObDereferenceObjectDeferDelete(Process);
	return TRUE;
}

//*============================================================================
//*= = �������ƣ�CreateProecssCallback
//*= = �����������������̵�֪ͨ���� 
//*= = ��ڲ�����HANDLE,HANDLE,BOOLEAN
//*= = ���ڲ�����VOID
//*============================================================================
VOID CreateProecssCallback(
	IN HANDLE  ParentId,
	IN HANDLE  ProcessId,
	IN BOOLEAN  Create
	)
{
	NTSTATUS status;
	PPROCESSINFO ProcessInfo = (PPROCESSINFO)((ULONG)g_ShareBuf + sizeof(ULONG));
	PEPROCESS Process;
	UNICODE_STRING NameW;
	ANSI_STRING NameA;
	if (!MmIsAddressValid((PVOID)g_ShareBuf))
		return;
	memset((PVOID)ProcessInfo, 0, sizeof(PROCESSINFO));

	if (Create){
		//��������
		*(PULONG)g_ShareBuf = TRUE;
		ProcessInfo->ProcessId = (ULONG)ProcessId;

		status = PsLookupProcessByProcessId(ProcessId, &Process);
		if (!NT_SUCCESS(status))
		{
			KdPrint(("[*]Get process handle failed in Createcallback."));
		}

		RtlInitAnsiString(&NameA, (PCSZ)((ULONG)Process + g_ProcessNameOffset));
		RtlAnsiStringToUnicodeString(&NameW, &NameA, TRUE);

		ObDereferenceObject(Process);

		memcpy((PVOID)ProcessInfo->Name, (PVOID)NameW.Buffer, NameW.Length);

		//KdPrint(("[*]USER:%wZ", NameW));

		RtlFreeUnicodeString(&NameW);
	}
	else{
		//�ӱ������̺����ؽ���������ȥ��
		unProtectProcessById((ULONG)ProcessId, 0);
	}
}

//*============================================================================
//*= = �������ƣ�Unload
//*= = ����������ж������
//*= = ��ڲ�����PDRIVER_OBJECT
//*= = ���ڲ�����VOID
//*============================================================================
VOID Unload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING Win32Device;

	//�ͷ��б�
	FreeList();

	//ж�����й���
	UnhookSystemRoutine(g_PspTerminateThreadByPointerAddr, "\x8b\xff\x55\x8b\xec");
	UnhookSystemRoutine(g_NtTerminateProcessAddr, "\x8b\xff\x55\x8b\xec");
	UnhookSystemRoutine(g_NtOpenProcessAddr, "\x8b\xff\x55\x8b\xec");

	//�Ƴ�֪ͨ����
	PsSetCreateProcessNotifyRoutine(CreateProecssCallback, TRUE);

	//�ͷ��ڴ�
	if (g_ShareBuf)
	{
		ExFreePool(g_ShareBuf);
	}

	IoUnregisterShutdownNotification(DriverObject->DeviceObject);

	//�Ƴ��豸
	RtlInitUnicodeString(&Win32Device, SYMBOL_NAME);
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(DriverObject->DeviceObject);
}

//*============================================================================
//*= = �������ƣ�CreateClose
//*= = �����������Կ����ر�Irp����ǲ���� 
//*= = ��ڲ�����PDEVICE_OBJECT,PIRP
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS CreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//*============================================================================
//*= = �������ƣ�DefaultHandler
//*= = ����������Ĭ����ǲ���� 
//*= = ��ڲ�����PDEVICE_OBJECT,PIRP
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS DefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}

//*============================================================================
//*= = �������ƣ�DeviceControl
//*= = �����������豸������ǲ���� 
//*= = ��ڲ�����PDEVICE_OBJECT,PIRP
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS DeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION INFO = IoGetCurrentIrpStackLocation(Irp);
	ULONG ControlCode = INFO->Parameters.DeviceIoControl.IoControlCode;
	PVOID inbuf = Irp->AssociatedIrp.SystemBuffer;
	PVOID outbuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	ULONG outlen = INFO->Parameters.DeviceIoControl.OutputBufferLength;

	PPROCESSINFO p;
	ULONG len, user, i;
	PLISTNODE list;
	PPROTECTINFO pt;

	//�жϿ�����
	switch (ControlCode)
	{
		case PROTECTBYPID:
			p = (PPROCESSINFO)inbuf;
			len = wcslen(p->Name);
			user = 0;

			//���Name��Ϊ�գ����Ƶ�user��
			if (len){
				user = (ULONG)ExAllocatePool(NonPagedPool, (len + 1)*sizeof(WCHAR));
				wcscpy((PWSTR)user, p->Name);
			}

			*(PULONG)outbuf = ProtectProcessById(p->ProcessId, user) ? 1 : 0;
			break;
		case UNPROTECTBYPID:
			p = (PPROCESSINFO)inbuf;
			len = wcslen(p->Name);
			user = 0;

			//���Name��Ϊ�գ����Ƶ�user��
			if (len){
				user = (ULONG)ExAllocatePool(NonPagedPool, (len + 1)*sizeof(WCHAR));
				wcscpy((PWSTR)user, p->Name);
			}

			*(PULONG)outbuf = unProtectProcessById(p->ProcessId, user) ? 1 : 0;
			if (user){//�ͷ��ڴ�
				ExFreePool((PVOID)user);
			}
			break;
		case KILLBYPID:	
			*(PULONG)outbuf = KillProcessById(*(PULONG)inbuf) ? 1 : 0;
			break;
		default:
			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = outlen;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

//*============================================================================
//*= = �������ƣ�Shutdown
//*= = ��������������ر����� 
//*= = ��ڲ�����PDEVICE_OBJECT,PIRP
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS Shutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PDRIVER_OBJECT DriverObject;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING DriverName;
	OBREFERENCEOBJECTBYNAME ObReferenceObjectByName = (OBREFERENCEOBJECTBYNAME)GetFuncAddr(L"ObReferenceObjectByName");
	RtlInitUnicodeString(&DriverName, DRIVER_NAME);
	if (MmIsAddressValid((PVOID)(ULONG)ObReferenceObjectByName)){
		status = ObReferenceObjectByName(
			&DriverName,
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
			NULL,
			0,
			*IoDriverObjectType,
			KernelMode,
			NULL,
			(PVOID*)&DriverObject
			);
		if (NT_SUCCESS(status)){
			Unload(DriverObject);//ж����������
		}
	}
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//*============================================================================
//*= = �������ƣ�DriverEntry
//*= = ��������������������ں��� 
//*= = ��ڲ�����PDRIVER_OBJECT, PUNICODE_STRING 
//*= = ���ڲ�����NTSTATUS
//*============================================================================
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING DeviceName, Win32Device;
	PDEVICE_OBJECT DeviceObject = NULL;
	NTSTATUS status;
	ULONG i;

	DriverObject->DriverUnload = Unload;
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&Win32Device, SYMBOL_NAME);

	// ������ǲ����
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = DefaultHandler;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;			//�����豸��CreateFile�������IRP
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;			//�ر��豸��CloseHandle�������IRP
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;	//DeviceControl�����������IRP
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = Shutdown;			//�ر�ϵͳǰ�������IRP

	//�����豸
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
		return status;
	if (!DeviceObject)
		return STATUS_UNEXPECTED_IO_ERROR;
	

	// ������������ 
	status = IoCreateSymbolicLink(&Win32Device, &DeviceName);
	if (!NT_SUCCESS(status)){
		IoDeleteDevice(DeviceObject);
		return status;
	}
	//ָ��ͨ�ŷ�ʽ
	DeviceObject->Flags |= DO_DIRECT_IO;
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	IoRegisterShutdownNotification(DeviceObject);

	//��ʼ�������ź���
	ExInitializeFastMutex(&mux_protect);

	//��ȡ���ֺ����ĵ�ַ
	g_PspTerminateThreadByPointerAddr = GetPspTerminateThreadByPointerAddr();//TerminateProcess��ʵ��ͨ���������̵��߳�����Ȼ���PspTerminateThreadByPointer����������������̵ġ�
	g_NtTerminateProcessAddr = GetNtTerminateProcessAddr();
	g_NtOpenProcessAddr = GetNtOpenProcessAddr();
	PsGetNextProcessThread = (PSGETNEXTPROCESSTHREAD)GetPsGetNextProcessThreadAddr();

	//ע���¼�֪ͨ ��������ʱ����CreateProecssCallback
	PsSetCreateProcessNotifyRoutine(CreateProecssCallback, FALSE);

	//��װ���ӵ���������
	HookSystemRoutine(g_PspTerminateThreadByPointerAddr, (ULONG)HookPspTerminateThreadByPointer);
	HookSystemRoutine(g_NtTerminateProcessAddr, (ULONG)HookNtTerminateProcess);
	HookSystemRoutine(g_NtOpenProcessAddr, (ULONG)HookNtOpenProcess);

	//��һ��ULONG�Ŀռ������жϴ������ǽ������̡�
	g_ShareBuf = ExAllocatePool(NonPagedPool, sizeof(ULONG)+sizeof(PROCESSINFO));

	return STATUS_SUCCESS;
}
