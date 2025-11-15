#ifndef __MOUSE_TRACKER_IMGUI_WINAPIFILEOPERATIONS__
#define __MOUSE_TRACKER_IMGUI_WINAPIFILEOPERATIONS__

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <algorithm>
#include "Loggers/Logger.h"

namespace Mt
{
    class WinApiFileOperations
    {
        public:
            static std::string OpenFileDialog
            (
                const std::string& defaultPath = "",
                const std::vector<std::pair<std::string, std::string>>& filters = {{ "All Files", "*.*" }},
                HWND parentWindow = nullptr
            )
            {
                OPENFILENAMEA ofn;
                char szFile[260] = {0};
                
                if (!defaultPath.empty())
                    strcpy_s(szFile, sizeof(szFile), defaultPath.c_str());

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = parentWindow;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                
                std::string filterString;

                for (const auto& filter : filters)
                    filterString += filter.first + '\0' + filter.second + '\0';
                
                filterString += '\0';
                
                ofn.lpstrFilter = filterString.c_str();
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameA(&ofn) == TRUE)
                {
                    Logger::GetInstance().DebugF("File selected: %s", szFile);

                    return std::string(szFile);
                }

                return "";
            }

            static std::string SaveFileDialog
            (
                const std::string& defaultPath = "",
                const std::vector<std::pair<std::string, std::string>>& filters = {{ "All Files", "*.*" }},
                HWND parentWindow = nullptr
            )
            {
                OPENFILENAMEA ofn;
                char szFile[260] = {0};
                
                if (!defaultPath.empty())
                    strcpy_s(szFile, sizeof(szFile), defaultPath.c_str());

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = parentWindow;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                
                std::string filterString;

                for (const auto& filter : filters)
                    filterString += filter.first + '\0' + filter.second + '\0';
                
                filterString += '\0';
                
                ofn.lpstrFilter = filterString.c_str();
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameA(&ofn) == TRUE)
                {
                    Logger::GetInstance().InfoF("File saved: %s", szFile);

                    return std::string(szFile);
                }

                return "";
            }

            static std::string SelectFolderDialog
            (
                const std::string& defaultPath = "",
                HWND parentWindow = nullptr
            )
            {
                std::string result;
            
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                    if (SUCCEEDED(hr))
                    {
                        DWORD dwOptions;
                        pFileOpen->GetOptions(&dwOptions);
                        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

                        if (!defaultPath.empty())
                        {
                            IShellItem* psiDefault;

                            HRESULT hrSI = SHCreateItemFromParsingName
                            (
                                std::wstring(defaultPath.begin(), defaultPath.end()).c_str(), 
                                NULL, IID_IShellItem, reinterpret_cast<void**>(&psiDefault)
                            );

                            if (SUCCEEDED(hrSI))
                            {
                                pFileOpen->SetFolder(psiDefault);
                                psiDefault->Release();
                            }
                        }

                        hr = pFileOpen->Show(parentWindow);

                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);

                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                                if (SUCCEEDED(hr))
                                {
                                    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                                    std::vector<char> buffer(bufferSize);
                                    WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, buffer.data(), bufferSize, NULL, NULL);
                                    result = buffer.data();
                                    CoTaskMemFree(pszFilePath);
                                }

                                pItem->Release();
                            }
                        }

                        pFileOpen->Release();
                    }

                    CoUninitialize();
                }

                if (!result.empty())
                    Logger::GetInstance().Debug("Folder selected: " + result + ".");

                return result;
            }

            static bool FileExists(const std::string& path)
            {
                DWORD dwAttrib = GetFileAttributesA(path.c_str());

                return (dwAttrib != INVALID_FILE_ATTRIBUTES
                    && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
            }

            static bool DirectoryExists(const std::string& path)
            {
                DWORD dwAttrib = GetFileAttributesA(path.c_str());

                return (dwAttrib != INVALID_FILE_ATTRIBUTES 
                    && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
            }

            static bool CreateDirectoryRecursive(const std::string& path)
            {
                if (DirectoryExists(path))
                    return true;

                size_t pos = 0;
                std::string dir;
                
                if (path[path.size() - 1] != '\\')
                    dir = path + "\\";
                else
                    dir = path;

                while ((pos = dir.find_first_of("\\", pos + 1)) != std::string::npos)
                {
                    std::string subdir = dir.substr(0, pos);

                    if (!DirectoryExists(subdir) && !CreateDirectoryA(subdir.c_str(), NULL))
                        if (GetLastError() != ERROR_ALREADY_EXISTS)
                            return false;
                }
                
                return true;
            }

        private:
            static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
            {
                if (uMsg == BFFM_INITIALIZED && lpData != 0)
                    SendMessageA(hwnd, BFFM_SETSELECTION, TRUE, lpData);

                return 0;
            }
    };
}

#endif
