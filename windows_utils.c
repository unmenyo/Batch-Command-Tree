#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <errno.h>
#include "file_utils.h"
#include "platform_utils.h"

// 检查路径是否存在
int pathExists(const char* path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

// 创建目录
int createDirectory(const char* path) {
    return _mkdir(path);
}

// 递归打印文件树
void printFileTree(const char* path, int depth) {
    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    snprintf(searchPath, MAX_PATH_LENGTH, "%s\\*", path);
    
    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }
        
        // 缩进
        for (int i = 0; i < depth; i++) {
            printf("  ");
        }
        
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("[%s]\\\n", findFileData.cFileName);
            // 递归处理子目录
            char subPath[MAX_PATH_LENGTH];
            snprintf(subPath, MAX_PATH_LENGTH, "%s\\%s", path, findFileData.cFileName);
            printFileTree(subPath, depth + 1);
        } else {
            printf("%s\n", findFileData.cFileName);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);
    
    FindClose(hFind);
}

// 构建文件列表（递归）
FileEntry* buildFileList(const char* path, FileEntry* list) {
    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    snprintf(searchPath, MAX_PATH_LENGTH, "%s\\*", path);
    
    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return list;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }
        
        // 创建新节点
        FileEntry* newEntry = (FileEntry*)malloc(sizeof(FileEntry));
        snprintf(newEntry->path, MAX_PATH_LENGTH, "%s\\%s", path, findFileData.cFileName);
        
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
    } while (FindNextFileA(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    return list;
}

// 创建目录树
void createDirectoryTree(const char* inputPath, const char* outputPath) {
    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH_LENGTH];
    HANDLE hFind;
    
    snprintf(searchPath, MAX_PATH_LENGTH, "%s\\*", inputPath);
    
    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        // 跳过 "." 和 ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }
        
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 创建输出目录
            char outputDir[MAX_PATH_LENGTH];
            snprintf(outputDir, MAX_PATH_LENGTH, "%s\\%s", outputPath, findFileData.cFileName);
            
            if (_mkdir(outputDir) != 0 && errno != EEXIST) {
                printf("Warning: Cannot create directory %s\n", outputDir);
            }
            
            // 递归处理子目录
            char subInputPath[MAX_PATH_LENGTH];
            snprintf(subInputPath, MAX_PATH_LENGTH, "%s\\%s", inputPath, findFileData.cFileName);
            createDirectoryTree(subInputPath, outputDir);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);
    
    FindClose(hFind);
}

// 复制文件（保留路径结构）
int copyFileWithPath(const char* source, const char* destination) {
    // 确保目标目录存在
    char destDir[MAX_PATH_LENGTH];
    strcpy(destDir, destination);
    
    char* lastBackslash = strrchr(destDir, '\\');
    if (lastBackslash != NULL) {
        *lastBackslash = '\0';
        
        // 创建目录（如果不存在）
        if (_mkdir(destDir) != 0 && errno != EEXIST) {
            printf("Error: Cannot create directory %s\n", destDir);
            return 0;
        }
    }
    
    // 复制文件
    if (CopyFileA(source, destination, FALSE)) {
        printf("Copy successful: %s -> %s\n", source, destination);
        return 1;
    } else {
        printf("Error: Failed to copy file %s -> %s\n", source, destination);
        return 0;
    }
}