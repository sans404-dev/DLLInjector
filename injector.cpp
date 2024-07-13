#include <windows.h>
#include <iostream>
#include <tchar.h>

char *buff = (char*) malloc(MAX_PATH);
TCHAR dllname[MAX_PATH];
TCHAR pid[6];

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

TCHAR* GetChosenDll()
{
    OPENFILENAME ofn = {0};
    TCHAR filename[MAX_PATH];
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = _T("*.dll\0*.dll\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if(GetOpenFileName(&ofn)) {
        size_t len = _tcslen(filename) + 1;
        TCHAR* selectedFileName = new TCHAR[len];
        _tcscpy_s(selectedFileName, len, filename);
        return selectedFileName;
    } else {
        return NULL;
    }
}

bool Inject(DWORD pid, TCHAR* dllname)
{
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if(process==NULL) {
        MessageBox(NULL, "Can't open process (not enough rights?)", "Error", MB_OK);
        return false;
    }
    LPVOID fp = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"),"LoadLibraryA");
    if(fp==NULL) {
        MessageBox(NULL, "Can't get proc address", "Error", MB_OK);
        return false;
    }
    LPVOID alloc = (LPVOID)VirtualAllocEx(process,0,lstrlen(dllname), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(alloc==NULL) {
        MessageBox(NULL, "Can't alloc memory", "Error", MB_OK);
        return false;
    }
    BOOL w = WriteProcessMemory(process,(LPVOID)alloc,dllname,lstrlen(dllname),0);
    if(!w){
        MessageBox(NULL, "Can't write dll to process", "Error", MB_OK);
        return false;
    }
    HANDLE thread = CreateRemoteThread(process,0,0,(LPTHREAD_START_ROUTINE)fp,(LPVOID)alloc,0,0);
    if(thread==NULL) return false;
    CloseHandle(process);
    return true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    static TCHAR szWindowClass[] = _T("DesktopApp");
    static TCHAR szTitle[] = _T("C++ Injector by @ClientERR0R");
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(20);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    RegisterClassEx(&wcex);
    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 700, 300, 345, 200, NULL, NULL, hInstance, NULL);
    HWND proc_pid = CreateWindowEx(0, _T("Static"), _T("<~ process pid"), WS_CHILD | WS_VISIBLE | WS_BORDER, 80, 0, 100, 20, hWnd, NULL, hInstance, NULL);
    HWND edit_proc = CreateWindowEx(0, _T("Edit"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 80, 20, hWnd, NULL, hInstance, NULL);
    HWND edit_dll = CreateWindowEx(0, _T("Button"), _T("Choose dll"), WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 20, 80, 20, hWnd, NULL, hInstance, NULL);
    HWND inj_btn = CreateWindowEx(0, _T("Button"), _T("Inject"), WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 120, 80, 20, hWnd, NULL, hInstance, NULL);
    HWND label = CreateWindowEx(0, _T("Static"), _T("DLL INJECTOR"), WS_CHILD | WS_VISIBLE, 0, 140, 330, 15, hWnd, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.hwnd == edit_dll && msg.message == WM_LBUTTONUP)
        {
            TCHAR* dll = GetChosenDll();
            _tcscpy_s(dllname, MAX_PATH, dll);
            SetWindowText(label, dllname);
            delete[] dll;
            dll = nullptr;
        } else if (msg.hwnd == inj_btn && msg.message == WM_LBUTTONUP) {
            GetWindowText(edit_proc, pid, sizeof(pid));
            bool result = Inject(_ttoi(pid), dllname);
            if (result==true)
            {
                MessageBox(NULL, "Injection successful", "Success", MB_OK);
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}