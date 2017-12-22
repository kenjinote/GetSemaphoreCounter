#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>

TCHAR szClassName[] = TEXT("Window");

typedef LONG NTSTATUS;
typedef NTSTATUS(NTAPI *_NtQuerySemaphore)(
	HANDLE SemaphoreHandle,
	DWORD SemaphoreInformationClass,
	PVOID SemaphoreInformation,
	ULONG SemaphoreInformationLength,
	PULONG ReturnLength OPTIONAL);
typedef struct _SEMAPHORE_BASIC_INFORMATION {
	ULONG CurrentCount;
	ULONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hSemaphore;
	static HWND hButton;
	switch (msg)
	{
	case WM_CREATE:
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hSemaphore = CreateSemaphore(NULL, LONG_MAX, LONG_MAX, TEXT("SemaphoreTest1234"));
		if (hSemaphore == NULL)
		{
			MessageBox(hWnd, TEXT("Semaphore の作成に失敗しました"), 0, MB_OK | MB_ICONEXCLAMATION);
			return -1;
		}
		if (WAIT_OBJECT_0 != WaitForSingleObject(hSemaphore, 0)) // カウンタを 1 減らす
		{
			MessageBox(NULL, TEXT("アプリケーションの起動上限に達しました"), 0, MB_OK | MB_ICONINFORMATION);
			return -1;
		}
		break;
	case WM_SIZE:
		MoveWindow(hButton, 10, 10, 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			int nCount = 0;
			{
				_NtQuerySemaphore NtQuerySemaphore = (_NtQuerySemaphore)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQuerySemaphore");
				if (NtQuerySemaphore)
				{
					SEMAPHORE_BASIC_INFORMATION BasicInfo;
					if (NtQuerySemaphore(hSemaphore, 0, &BasicInfo, sizeof(SEMAPHORE_BASIC_INFORMATION), NULL) == ERROR_SUCCESS)
					{
						nCount = BasicInfo.CurrentCount;
					}
				}
			}
			TCHAR szText[1024];
			wsprintf(szText, TEXT("%d"), LONG_MAX - nCount);
			MessageBox(hWnd, szText, TEXT("現在のSemaphoreの内部カウンタ"), 0);
		}
		break;
	case WM_DESTROY:
		ReleaseSemaphore(hSemaphore, 1, NULL);
		CloseHandle(hSemaphore);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Semaphore を使って自分自身のプロセス数を取得する"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
