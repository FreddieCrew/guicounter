#include <windows.h>
#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

const wchar_t* kClassName = L"Window";
const wchar_t* kWindowTitle = L"GUICounter";
const wchar_t* kLogFileName = L"log.txt";

HWND hTextArea;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LogMessage(const std::wstring& message) {
    std::wofstream logFile(kLogFileName, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << L"[";
        {
            std::time_t now = std::time(nullptr);
            wchar_t timestamp[50];
            std::tm time_info;
            localtime_s(&time_info, &now);
            std::wcsftime(timestamp, sizeof(timestamp) / sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", &time_info);
            logFile << timestamp;
        }
        logFile << L"] " << message << L"\r\n";
        logFile.close();
    }
}

void OpenFolder(const std::wstring& folderPath) {
    ShellExecute(NULL, L"open", folderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void OpenURL(const std::wstring& url) {
    ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

std::size_t CountLines(const std::wstring& filePath) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + std::string(filePath.begin(), filePath.end()));
    }

    std::size_t lineCount = 0;
    std::wstring line;
    while (std::getline(file, line)) {
        ++lineCount;
    }

    return lineCount;
}

void DisplayFileDetails(const std::wstring& filePath) {
    try {
        std::wifstream file(filePath);
        if (!file.is_open()) {
            SetWindowText(hTextArea, L"Error opening file");
            return;
        }

        std::wstringstream contentStream;
        std::wstring line;
        std::size_t lineCount = 0;

        while (std::getline(file, line)) {
            contentStream << std::setw(4) << ++lineCount << L": " << line << L"\r\n";
        }

        contentStream << L"\r\nFile information:\r\n";
        contentStream << L"\r\nFile Size: " << fs::file_size(filePath) << L" bytes\n";
        contentStream << L"\r\nTotal Lines: \n" << lineCount;

        std::wstring content = contentStream.str();

        SetWindowText(hTextArea, content.c_str());
    }
    catch (const std::exception& ex) {
        SetWindowText(hTextArea, L"Error reading file");
        LogMessage(L"Error counting lines");
    }
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        HWND hButton = CreateWindow(L"BUTTON", L"Open text file", WS_VISIBLE | WS_CHILD, 10, 10, 150, 40, hwnd, reinterpret_cast<HMENU>(1), NULL, NULL);

        HMENU hContextMenu = CreatePopupMenu();
        AppendMenu(hContextMenu, MF_STRING, 2, L"Open Folder");
        AppendMenu(hContextMenu, MF_STRING, 3, L"Visit GitHub");
        AppendMenu(hContextMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hContextMenu, MF_STRING, 4, L"Exit");

        SetMenu(hwnd, hContextMenu);

        hTextArea = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            10, 60, 460, 190, hwnd, NULL, NULL, NULL);

        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: {
            OPENFILENAME ofn;
            wchar_t szFileName[MAX_PATH] = L"";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFileName;
            ofn.lpstrFile[0] = L'\0';
            ofn.nMaxFile = sizeof(szFileName) / sizeof(szFileName[0]);
            ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                std::wstring filePath = ofn.lpstrFile;
                LogMessage(L"Selected file: " + filePath);

                DisplayFileDetails(filePath);
            }
        }
        break;
        case 2:
            OpenFolder(L".");
            break;
        case 3:
            OpenURL(L"https://github.com/FreddieCrew/guicounter");
            break;
        case 4:
            DestroyWindow(hwnd);
            break;
        }
        break;
    case WM_DESTROY:
        DestroyMenu(GetMenu(hwnd));
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int main() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = kClassName;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, kClassName, kWindowTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
