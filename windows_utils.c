#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <errno.h>
#include "file_utils.h"
#include "platform_utils.h"

// 辅助函数：将UTF-8字符串转换为宽字符串
static void utf8_to_wchar(const char* utf8, wchar_t* wstr, size_t wstr_size) {
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, (int)wstr_size);
}

// 辅助函数：将宽字符串转换为UTF-8字符串
static void wchar_to_utf8(const wchar_t* wstr, char* utf8, size_t utf8_size) {
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, (int)utf8_size, NULL, NULL);
}

// 检查路径是否存在
int pathExists(const char* path) {
    wchar_t wpath[MAX_PATH_LENGTH];
    utf8_to_wchar(path, wpath, MAX_PATH_LENGTH);
    return GetFileAttributesW(wpath) != INVALID_FILE_ATTRIBUTES;
}

// 创建目录
int createDirectory(const char* path) {
    wchar_t wpath[MAX_PATH_LENGTH];
    utf8_to_wchar(path, wpath, MAX_PATH_LENGTH);
    return _wmkdir(wpath);
}

// 递归打印文件树
void printFileTree(const char* path, int depth) {
    WIN32_FIND_DATAW findFileData;
    wchar_t searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    wchar_t wpath[MAX_PATH_LENGTH];
    utf8_to_wchar(path, wpath, MAX_PATH_LENGTH);
    
    snwprintf(searchPath, MAX_PATH_LENGTH, L"%s\\*", wpath);
    
    hFind = FindFirstFileW(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (wcscmp(findFileData.cFileName, L".") == 0 || wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }
        
        // 缩进
        for (int i = 0; i < depth; i++) {
            printf("  ");
        }
        
        // 将宽字符文件名转换为UTF-8以便打印
        char utf8FileName[MAX_PATH_LENGTH];
        wchar_to_utf8(findFileData.cFileName, utf8FileName, MAX_PATH_LENGTH);
        
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("[%s]\\\n", utf8FileName);
            // 递归处理子目录
            char subPath[MAX_PATH_LENGTH];
            snprintf(subPath, MAX_PATH_LENGTH, "%s\\%s", path, utf8FileName);
            printFileTree(subPath, depth + 1);
        } else {
            printf("%s\n", utf8FileName);
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);
    
    FindClose(hFind);
}

// 构建文件列表（递归）
FileEntry* buildFileList(const char* path, FileEntry* list) {
    WIN32_FIND_DATAW findFileData;
    wchar_t searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    wchar_t wpath[MAX_PATH_LENGTH];
    utf8_to_wchar(path, wpath, MAX_PATH_LENGTH);
    
    snwprintf(searchPath, MAX_PATH_LENGTH, L"%s\\*", wpath);
    
    hFind = FindFirstFileW(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return list;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (wcscmp(findFileData.cFileName, L".") == 0 || wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }
        
        // 创建新节点
        FileEntry* newEntry = (FileEntry*)malloc(sizeof(FileEntry));
        
        // 将宽字符文件名转换为UTF-8
        char utf8FileName[MAX_PATH_LENGTH];
        wchar_to_utf8(findFileData.cFileName, utf8FileName, MAX_PATH_LENGTH);
        
        snprintf(newEntry->path, MAX_PATH_LENGTH, "%s\\%s", path, utf8FileName);
        
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            newEntry->is_directory = 1;
            // 递归处理子目录
            list = buildFileList(newEntry->path, list);
        } else {
            newEntry->is_directory = 0;
        }
        
        // 添加到链表
        newEntry->next = list;
        list = newEntry;
    } while (FindNextFileW(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    return list;
}

// 创建目录树
void createDirectoryTree(const char* inputPath, const char* outputPath) {
    WIN32_FIND_DATAW findFileData;
    wchar_t searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    wchar_t winputPath[MAX_PATH_LENGTH];
    utf8_to_wchar(inputPath, winputPath, MAX_PATH_LENGTH);
    
    snwprintf(searchPath, MAX_PATH_LENGTH, L"%s\\*", winputPath);
    
    hFind = FindFirstFileW(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (wcscmp(findFileData.cFileName, L".") == 0 || wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }
        
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 将宽字符文件名转换为UTF-8
            char utf8FileName[MAX_PATH_LENGTH];
            wchar_to_utf8(findFileData.cFileName, utf8FileName, MAX_PATH_LENGTH);
            
            // 创建输出目录
            char outputDir[MAX_PATH_LENGTH];
            snprintf(outputDir, MAX_PATH_LENGTH, "%s\\%s", outputPath, utf8FileName);
            
            // 使用createDirectory函数创建目录（该函数已经支持UTF-8）
            if (createDirectory(outputDir) != 0 && errno != EEXIST) {
                printf("Warning: Cannot create directory %s\n", outputDir);
            }
            
            // 递归处理子目录
            char subInputPath[MAX_PATH_LENGTH];
            snprintf(subInputPath, MAX_PATH_LENGTH, "%s\\%s", inputPath, utf8FileName);
            createDirectoryTree(subInputPath, outputDir);
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);
    
    FindClose(hFind);
}

// 复制文件（保留路径结构）
int copyFileWithPath(const char* source, const char* destination) {
    // 将源路径和目标路径转换为宽字符
    wchar_t wsource[MAX_PATH_LENGTH];
    wchar_t wdestination[MAX_PATH_LENGTH];
    utf8_to_wchar(source, wsource, MAX_PATH_LENGTH);
    utf8_to_wchar(destination, wdestination, MAX_PATH_LENGTH);
    
    // 确保目标目录存在
    wchar_t destDir[MAX_PATH_LENGTH];
    wcscpy(destDir, wdestination);
    
    wchar_t* lastBackslash = wcsrchr(destDir, L'\\');
    if (lastBackslash != NULL) {
        *lastBackslash = L'\0';
        
        // 创建目录（如果不存在）
        if (_wmkdir(destDir) != 0 && errno != EEXIST) {
            // 将宽字符目录路径转换为UTF-8以便打印错误信息
            char utf8Dir[MAX_PATH_LENGTH];
            wchar_to_utf8(destDir, utf8Dir, MAX_PATH_LENGTH);
            printf("Error: Cannot create directory %s\n", utf8Dir);
            return 0;
        }
    }
    
    // 复制文件
    if (CopyFileW(wsource, wdestination, FALSE)) {
        printf("Copy successful: %s -> %s\n", source, destination);
        return 1;
    } else {
        printf("Error: Failed to copy file %s -> %s\n", source, destination);
        return 0;
    }
}