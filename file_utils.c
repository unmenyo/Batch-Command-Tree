#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "file_utils.h"
#include "platform_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

// 获取不带扩展名的文件名
char* getFileNameWithoutExtension(const char* path) {
    static char result[MAX_PATH_LENGTH];
    char* lastBackslash = strrchr(path, '\\');
    char* filename = (lastBackslash != NULL) ? lastBackslash + 1 : (char*)path;
    
    strcpy(result, filename);
    
    // 移除扩展名
    char* lastDot = strrchr(result, '.');
    if (lastDot != NULL) {
        *lastDot = '\0';
    }
    
    return result;
}

// 处理文件
void processFiles(FileEntry* fileList, const char* inputPath, const char* outputPath, const char* command, int copyOnError) {
    // 首先创建完整的目录树
    createDirectoryTree(inputPath, outputPath);
    
    FileEntry* current = fileList;
    
    while (current != NULL) {
        if (!current->is_directory) {
            // 计算相对路径
            const char* relativePath = current->path + strlen(inputPath);
            
            // 获取输出目录路径
            char outputDir[MAX_PATH_LENGTH];
            snprintf(outputDir, MAX_PATH_LENGTH, "%s%s", outputPath, relativePath);
            
            // 移除文件名，只保留目录部分
            char* lastBackslash = strrchr(outputDir, '\\');
            if (lastBackslash != NULL) {
                *lastBackslash = '\0';
            }
            
            // 获取不带扩展名的文件名
            char* filenameWithoutExt = getFileNameWithoutExtension(current->path);
            
            // 构建命令
            char finalCommand[MAX_COMMAND_LENGTH * 2];
            char escapedInputPath[MAX_PATH_LENGTH * 2];
            char escapedOutputDir[MAX_PATH_LENGTH * 2];
            
            // 转义路径中的空格和特殊字符
            strcpy(escapedInputPath, "\"");
            strcat(escapedInputPath, current->path);
            strcat(escapedInputPath, "\"");
            
            strcpy(escapedOutputDir, "\"");
            strcat(escapedOutputDir, outputDir);
            strcat(escapedOutputDir, "\\");
            strcat(escapedOutputDir, filenameWithoutExt);
            strcat(escapedOutputDir, "\"");
            
            // 替换命令中的占位符
            strcpy(finalCommand, command);
            
            // 替换 %i 占位符
            char* inputPlaceholder = strstr(finalCommand, "%i");
            if (inputPlaceholder != NULL) {
                size_t prefixLen = inputPlaceholder - finalCommand;
                char prefix[MAX_COMMAND_LENGTH];
                strncpy(prefix, finalCommand, prefixLen);
                prefix[prefixLen] = '\0';
                
                size_t suffixLen = strlen(inputPlaceholder + 2);
                char suffix[MAX_COMMAND_LENGTH];
                strcpy(suffix, inputPlaceholder + 2);
                
                snprintf(finalCommand, MAX_COMMAND_LENGTH * 2, "%s%s%s", prefix, escapedInputPath, suffix);
            }
            
            // 替换 %o 占位符
            char* outputPlaceholder = strstr(finalCommand, "%o");
            if (outputPlaceholder != NULL) {
                size_t prefixLen = outputPlaceholder - finalCommand;
                char prefix[MAX_COMMAND_LENGTH];
                strncpy(prefix, finalCommand, prefixLen);
                prefix[prefixLen] = '\0';
                
                size_t suffixLen = strlen(outputPlaceholder + 2);
                char suffix[MAX_COMMAND_LENGTH];
                strcpy(suffix, outputPlaceholder + 2);
                
                snprintf(finalCommand, MAX_COMMAND_LENGTH * 2, "%s%s%s", prefix, escapedOutputDir, suffix);
            }
            
            printf("Executing: %s\n", finalCommand);
            
            // 执行命令
            int result;
            
            #ifdef _WIN32
            // 在Windows上，使用宽字符API执行命令以确保UTF-8路径正确传递
            wchar_t wcommand[MAX_COMMAND_LENGTH * 2];
            int wlen = MultiByteToWideChar(CP_UTF8, 0, finalCommand, -1, wcommand, MAX_COMMAND_LENGTH * 2);
            if (wlen > 0) {
                result = _wsystem(wcommand);
            } else {
                result = system(finalCommand);
            }
            #else
            result = system(finalCommand);
            #endif
            
            if (result != 0) {
                printf("Error: Command execution failed (code: %d)\n", result);
                
                // 如果启用了命令失败时复制源文件的功能
                if (copyOnError) {
                    printf("Attempting to copy source file...\n");
                    
                    // 构建目标文件路径
                    char targetPath[MAX_PATH_LENGTH];
                    snprintf(targetPath, MAX_PATH_LENGTH, "%s%s", outputPath, relativePath);
                    
                    // 复制源文件到目标路径
                    if (!copyFileWithPath(current->path, targetPath)) {
                        printf("Copying source file also failed\n");
                    }
                }
            } else {
                printf("Command executed successfully\n");
            }
        }
        
        current = current->next;
    }
}

// 释放文件列表内存
void freeFileList(FileEntry* list) {
    FileEntry* current = list;
    while (current != NULL) {
        FileEntry* next = current->next;
        free(current);
        current = next;
    }
}