/*
    launcher.c

    This file is a stupid and shit over-complicated attempt to launch Hitman along with a patcher
    and server. Sure it's a bit messy, but then again, nothing in life is ever fucking easy, right?
    
      @echo off
      SET PORT=<port>
      SET LOG_LEVEL_CONSOLE=info
      SET LOG_CATEGORY_DISABLED=
      SET LOG_MAX_FILES=14d
      pushd "%~dp0"
      .\nodedist\node.exe chunk0.js
      PAUSE

    Compile with:
        gcc launcher.c -mwindows -o launcher.exe -lcomctl32

    Place launcher.exe in your game folder.
*/

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00  // Target Windows 10 - because why aim for older crap?
#define _WIN32_IE    0x0A00  // IE 10+ - modern enough for our tastes.

#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

// Control IDs
#define IDC_STATIC_STATUS   101
#define IDC_CHECK_PATCHER   102
#define IDC_CHECK_SERVER    103
#define IDC_BUTTON_LAUNCH   104
#define IDC_EDIT_PORT       105
#define IDC_CHECK_CLEAN     106
#define IDC_STATIC_PORT     107  // Label for "Server Port:"

// Global variables for instance and process handles.
// Nothing beats global state for good ol' fashioned convenience.
HINSTANCE hInst = NULL;
HANDLE hJob = NULL;
HANDLE hServerProcess = NULL; // Global handle for the current server process

// -------------------------------------------------------------------------
// Helper: Check if a file exists.
// Yep, a little function to remind us that sometimes files just vanish.
BOOL FileExistsW(const wchar_t *path)
{
    DWORD attrib = GetFileAttributesW(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// -------------------------------------------------------------------------
// Modified LaunchProcessW:
// Launches a process and, if phProcess is non-NULL, returns its process handle.
// Also (optionally) returns its PID in pPID.
// We try not to reinvent the wheel here – just wrapping a lot of Windows API calls.
BOOL LaunchProcessW(const wchar_t *appName, wchar_t *cmdLine, const wchar_t *workDir, BOOL newConsole, DWORD *pPID, HANDLE *phProcess)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD flags = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (newConsole)
        flags |= CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP;

    if (!CreateProcessW(appName, cmdLine, NULL, NULL, FALSE, flags, NULL, workDir, &si, &pi))
    {
        wchar_t msg[256];
        wsprintfW(msg, L"Failed to launch process:\n%s", cmdLine);
        MessageBoxW(NULL, msg, L"Launch Error", MB_ICONERROR);
        return FALSE;
    }

    if (pPID)
        *pPID = pi.dwProcessId;
    if (phProcess)
        *phProcess = pi.hProcess;
    else
        CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (hJob)
    {
        // Assign the new process to our job object.
        // Because leaving processes orphaned is never a good look.
        if (!AssignProcessToJobObject(hJob, (phProcess ? *phProcess : pi.hProcess)))
        {
            // Optionally log a warning if desired.
        }
    }
    return TRUE;
}

// -------------------------------------------------------------------------
// Cleans up any previous server instance by reading "server.pid" and terminating that process,
// and also deleting the server.pid and server.cmd files.
// Sometimes the past just needs to be cleaned up properly.
void CleanPreviousServer()
{
    FILE *f = _wfopen(L"server.pid", L"r");
    if (f)
    {
        DWORD prevPID = 0;
        if (fwscanf(f, L"%lu", &prevPID) == 1)
        {
            HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, prevPID);
            if (hProc)
            {
                TerminateProcess(hProc, 0);
                CloseHandle(hProc);
            }
        }
        fclose(f);
        DeleteFileW(L"server.pid");
    }
    // Also remove any lingering server.cmd file.
    DeleteFileW(L"server.cmd");
}

// -------------------------------------------------------------------------
// Records the given process ID into "server.pid".
// A little scribble to remember which process we spawned.
void RecordServerPID(DWORD pid)
{
    FILE *f = _wfopen(L"server.pid", L"w");
    if (f)
    {
        fwprintf(f, L"%lu", pid);
        fclose(f);
    }
}

// -------------------------------------------------------------------------
// Thread function: Waits for the server process to terminate and then deletes server.cmd.
// Kind of like waiting for a kettle to boil, only with more process handles.
DWORD WINAPI ServerWaitThread(LPVOID lpParam)
{
    HANDLE hProc = (HANDLE)lpParam;
    WaitForSingleObject(hProc, INFINITE);
    // Once the server process terminates, remove the server.cmd file.
    DeleteFileW(L"server.cmd");
    CloseHandle(hProc);
    hServerProcess = NULL;
    return 0;
}

// -------------------------------------------------------------------------
// Write out a dynamic server.cmd file with the given port.
// This writes a batch file on the fly – because why should things always be static?
BOOL WriteServerCMD(int port)
{
    FILE *fp = _wfopen(L"server.cmd", L"w");
    if (!fp)
        return FALSE;
    fwprintf(fp, L"@echo off\r\n");
    fwprintf(fp, L"SET PORT=%d\r\n", port);
    fwprintf(fp, L"SET LOG_LEVEL_CONSOLE=info\r\n");
    fwprintf(fp, L"SET LOG_CATEGORY_DISABLED=\r\n");
    fwprintf(fp, L"SET LOG_MAX_FILES=14d\r\n");
    fwprintf(fp, L"pushd \"%%~dp0\"\r\n");
    fwprintf(fp, L".\\nodedist\\node.exe chunk0.js\r\n");
    fwprintf(fp, L"PAUSE\r\n");
    fclose(fp);
    return TRUE;
}

// -------------------------------------------------------------------------
// Main window procedure.
// The heart of the GUI, handling user interactions and keeping the chaos under control.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hStatus, hCheckPatcher, hCheckServer, hButton, hEditPort, hCheckClean, hStaticPort;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    switch (msg)
    {
        case WM_CREATE:
        {
            // Create status label.
            hStatus = CreateWindowExW(0, L"STATIC", L"Game Status: Checking...",
                                      WS_CHILD | WS_VISIBLE,
                                      20, 20, 350, 20,
                                      hwnd, (HMENU)IDC_STATIC_STATUS, hInst, NULL);
            SendMessageW(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create "Launch Patcher" checkbox.
            hCheckPatcher = CreateWindowExW(0, L"BUTTON", L"Launch Patcher",
                                      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                      20, 50, 150, 20,
                                      hwnd, (HMENU)IDC_CHECK_PATCHER, hInst, NULL);
            SendMessageW(hCheckPatcher, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create "Start Server" checkbox.
            hCheckServer = CreateWindowExW(0, L"BUTTON", L"Start Server",
                                      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                      20, 80, 150, 20,
                                      hwnd, (HMENU)IDC_CHECK_SERVER, hInst, NULL);
            SendMessageW(hCheckServer, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create "Server Port:" label.
            hStaticPort = CreateWindowExW(0, L"STATIC", L"Server Port:",
                                      WS_CHILD | WS_VISIBLE,
                                      40, 110, 100, 20,
                                      hwnd, (HMENU)IDC_STATIC_PORT, hInst, NULL);
            SendMessageW(hStaticPort, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create edit control for the port with default value "6969".
            hEditPort = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"6969",
                                      WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                      150, 110, 80, 20,
                                      hwnd, (HMENU)IDC_EDIT_PORT, hInst, NULL);
            SendMessageW(hEditPort, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create "Clean Server Start" checkbox.
            hCheckClean = CreateWindowExW(0, L"BUTTON", L"Clean Server Start",
                                      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                      250, 110, 150, 20,
                                      hwnd, (HMENU)IDC_CHECK_CLEAN, hInst, NULL);
            SendMessageW(hCheckClean, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create "Launch Game" button.
            hButton = CreateWindowExW(0, L"BUTTON", L"Launch Game",
                                      WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                      20, 150, 150, 30,
                                      hwnd, (HMENU)IDC_BUTTON_LAUNCH, hInst, NULL);
            SendMessageW(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Check for the game executable in "Retail\HITMAN3.exe".
            {
                wchar_t currentDir[MAX_PATH];
                wchar_t gamePath[MAX_PATH * 2];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                wsprintfW(gamePath, L"%s\\Retail\\HITMAN3.exe", currentDir);
                if (FileExistsW(gamePath))
                    SetWindowTextW(hStatus, L"Game Status: GAME FOUND");
                else
                    SetWindowTextW(hStatus, L"Game Status: GAME NOT FOUND");
            }
        }
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_BUTTON_LAUNCH && HIWORD(wParam) == BN_CLICKED)
            {
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);

                // If "Start Server" and "Clean Server Start" are checked, kill previous server.
                if (IsDlgButtonChecked(hwnd, IDC_CHECK_SERVER) == BST_CHECKED &&
                    IsDlgButtonChecked(hwnd, IDC_CHECK_CLEAN) == BST_CHECKED)
                {
                    CleanPreviousServer();
                }

                // 1. Optionally launch the patcher.
                if (IsDlgButtonChecked(hwnd, IDC_CHECK_PATCHER) == BST_CHECKED)
                {
                    LaunchProcessW(NULL, L"PeacockPatcher.exe", currentDir, TRUE, NULL, NULL);
                }

                // 2. Optionally start the server.
                if (IsDlgButtonChecked(hwnd, IDC_CHECK_SERVER) == BST_CHECKED)
                {
                    // Retrieve port from edit control.
                    wchar_t portStr[16];
                    GetWindowTextW(hEditPort, portStr, 16);
                    int port = _wtoi(portStr);
                    if (port <= 0) port = 6969; // default port

                    // Write out a dynamic server.cmd file.
                    if (!WriteServerCMD(port))
                    {
                        MessageBoxW(hwnd, L"Failed to write server.cmd file.", L"Error", MB_ICONERROR);
                        break;
                    }

                    // Build a command line that launches the newly written server.cmd.
                    // Using "cmd.exe /K server.cmd" to spawn a new persistent window.
                    wchar_t serverCmdLine[256];
                    wsprintfW(serverCmdLine, L"cmd.exe /K server.cmd");

                    DWORD serverPID = 0;
                    // Launch the server and obtain its process handle.
                    if (LaunchProcessW(NULL, serverCmdLine, currentDir, TRUE, &serverPID, &hServerProcess))
                    {
                        RecordServerPID(serverPID);
                        // Create a thread to wait for the server process termination and then delete server.cmd.
                        HANDLE hThread = CreateThread(NULL, 0, ServerWaitThread, hServerProcess, 0, NULL);
                        if (hThread)
                            CloseHandle(hThread);
                    }
                    Sleep(1000); // brief delay for server initialization
                }

                // 3. Launch the game executable from "Retail\HITMAN3.exe".
                {
                    wchar_t gameExePath[MAX_PATH];
                    wsprintfW(gameExePath, L"%s\\Retail\\HITMAN3.exe", currentDir);
                    if (FileExistsW(gameExePath))
                    {
                        wchar_t retailDir[MAX_PATH];
                        wsprintfW(retailDir, L"%s\\Retail", currentDir);
                        LaunchProcessW(gameExePath, gameExePath, retailDir, TRUE, NULL, NULL);
                    }
                    else
                    {
                        MessageBoxW(hwnd, L"Game executable not found!", L"Error", MB_ICONERROR);
                    }
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// -------------------------------------------------------------------------
// Main entry point.
// The starting line where we set up everything, create a window, and pretend we know what we're doing.
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine, int nCmdShow)
{
    wchar_t exePath[MAX_PATH];
    wchar_t *lastSlash;
    hInst = hInstance;

    // Set working directory to the folder containing this launcher.
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash)
        *lastSlash = L'\0';
    SetCurrentDirectoryW(exePath);

    // Create a job object so that when the launcher exits, all spawned processes are terminated.
    // Because leaving stray processes is a surefire way to invite trouble.
    hJob = CreateJobObjectW(NULL, NULL);
    if (hJob)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;
        ZeroMemory(&jeli, sizeof(jeli));
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    }

    // Initialize common controls.
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register window class.
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"LauncherWindowClass";
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    // Create the main window.
    HWND hwnd = CreateWindowExW(0, L"LauncherWindowClass", L"Hitman Launcher",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 420, 230,
                                NULL, NULL, hInstance, NULL);
    if (!hwnd)
        return -1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hJob)
        CloseHandle(hJob);

    return (int)msg.wParam;
}

#ifdef UNICODE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    LPWSTR lpCmdLineW = GetCommandLineW();
    return wWinMain(hInstance, hPrevInstance, lpCmdLineW, nCmdShow);
}
#endif
