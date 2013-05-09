// dbcat.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "dbcat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWinApp theApp;

using namespace std;

// flush the buffer before getting killed.
BOOL CtrlHandler( DWORD fdwCtrlType ) {
	printf("----- Terminating -----\n");
	fflush(stdout);
	exit(1);
	return TRUE;
}

int watch() {
	HANDLE hAckEvent, hReadyEvent, hSharedFile;
	LPVOID pSharedMem;
	LPSTR pString;
	DWORD ret;
	LPDWORD pThisPid;
	TCHAR tszTempBuffer[1024];
	BOOL bb;

	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES) ;
	sa.bInheritHandle = TRUE ;
	sa.lpSecurityDescriptor = &sd ;	
	bb = InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	bb = SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

	hAckEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_BUFFER_READY");
	hReadyEvent = CreateEvent(&sa, FALSE, FALSE, "DBWIN_DATA_READY");

	hSharedFile = CreateFileMapping((HANDLE)-1, &sa, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");
	pSharedMem = MapViewOfFile(hSharedFile, FILE_MAP_READ, 0, 0, 512);

	pString = (LPSTR)pSharedMem + sizeof(DWORD);
	pThisPid = (LPDWORD)pSharedMem;

	SetEvent(hAckEvent);

	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );

	DWORD start = 0, time = 0;
	for (;;) {
		ret = WaitForSingleObject(hReadyEvent, INFINITE);
		if (ret == WAIT_OBJECT_0) {
			if (start == 0) {
				start = ::GetTickCount();
			} else {
				time = ::GetTickCount() - start;
			}
			lstrcpyn(tszTempBuffer, pString, 256);
			printf("[%d]\t%s", time, pString);
			SetEvent(hAckEvent);
		}
	}
  return 0;
}


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(NULL);
	//fprintf(stderr, "%d\n", argc);

	if (hModule != NULL) {
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0)) {
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		} else {
			if ( argc == 1 ) {
				nRetCode = watch();
			} else {
				::OutputDebugString(argv[1]);
			}
		}
	} else {
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
