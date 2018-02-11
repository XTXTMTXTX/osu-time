#include<cstdio>
#include<cstring>
#include<Windows.h>
#include<TlHelp32.h>
#define ABS(x) ((x)<0?-(x):(x))
using namespace std;
HANDLE hProcess;
DWORD PID;
unsigned char aob[]= {0xA3,0x00,0x00,0x00,0x00,0x8B,0x35,0x00,0x00,0x00,0x00,0x85,0xF6},mask[]= {1,0,0,0,0,1,1,0,0,0,0,1,1};
/*
HANDLE OpenProcess(
    DWORD      dwDesiredAccess, //希望获得的访问权限
    BOOL       bInheritHandle, //指明是否希望所获得的句柄可以继承
    DWORD      dwProcessId //要访问的进程ID
);
BOOL WriteProcessMemory(
    HANDLE     hProcess,   //要写进程的句柄
    LPVOID     lpBaseAddress,   //写内存的起始地址
    LPVOID     lpBuffer,   //写入数据的地址
    DWORD      nSize,   //要写的字节数
    LPDWORD    lpNumberOfBytesWritten   //实际写入的子节数
);
BOOL ReadProcessMemory(
    HANDLE     hProcess,   //要读进程的句柄
    LPCVOID    lpBaseAddress,   //读内存的起始地址
    LPVOID     lpBuffer,   //读入数据的地址
    DWORD      nSize,   //要读入的字节数
    LPDWORD    lpNumberOfBytesRead   //实际读入的子节数
);
*/
DWORD getPID(LPCSTR ProcessName) {
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)return 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap,&pe32)) {
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return 0;
	}

	DWORD dwPid=0;
	do {
		if(!strcmp(ProcessName, pe32.szExeFile)) {
			dwPid=pe32.th32ProcessID;
			break;
		}

	} while(Process32Next(hProcessSnap,&pe32));

	CloseHandle(hProcessSnap);
	return dwPid;
}
LPVOID AOB() {
	int len=sizeof(aob),pp=0;
	unsigned int faddr=0;
	unsigned char arBytes[2][4096],tmpByte[len*2+1];
	for(unsigned int addr=(unsigned int)0x00400000; (unsigned int)addr<=(unsigned int)2*1024*1024*1024; addr=(unsigned int)addr+4*1024) {
		ReadProcessMemory(hProcess,(LPVOID)addr,arBytes[0],4096,NULL);
		ReadProcessMemory(hProcess,(LPVOID)(addr+4*1024),arBytes[1],4096,NULL);
		for(int i=4096-len; i<4096; i++)tmpByte[i-4096+len]=arBytes[0][i];
		for(int i=0; i<len-1; i++)tmpByte[len+i]=arBytes[1][i];
		for(int i=(addr==0x00400000?0:len-1); i<4096; i++) {
			if(arBytes[0][i]*mask[pp]==aob[pp]*mask[pp]) {
				if(pp==len-1) {
					faddr=addr+i;
					return LPVOID(faddr-len+1);
				} else pp++;
			} else {
				i=i-pp;
				pp=0;
			}
		}
		for(int i=0; i<len-1; i++) {
			if(tmpByte[len+i]*mask[pp]==aob[pp]*mask[pp]) {
				if(pp==len-1) {
					faddr=addr+4096+i;
					return LPVOID(faddr-len+1);
				} else pp++;
			} else {
				i=i-pp;
				pp=0;
			}
		}

	}
	return 0;
}
int main() {
	PID=getPID("osu!.exe");
	if(!(hProcess=OpenProcess(PROCESS_VM_READ,0,PID))) {
		printf("Openning Failed\n");
		system("pause");
	}
	LPVOID timeaddraddr=LPVOID((unsigned int)(AOB())+1),timeaddr=0;
	ReadProcessMemory(hProcess,timeaddraddr,&timeaddr,4,NULL);
	int time=0,hh,mm,ss,ms;
	while(1) {
		ReadProcessMemory(hProcess,timeaddr,&time,4,NULL);
		ms=ABS(time%1000);time/=1000;
		ss=ABS(time%60);time/=60;
		mm=ABS(time%60);time/=60;
		hh=ABS(time);
		printf("\r%02d:%02d:%02d.%03d",hh,mm,ss,ms);
		//Sleep(50);
	}
	system("pause");
	CloseHandle(hProcess);
	return 0;
}
