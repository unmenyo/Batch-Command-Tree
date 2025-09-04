#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "file_utils.h"
#include "platform_utils.h"
#include "log_utils.h"

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

// 获取文件扩展名（小写）
char* getFileExtension(const char* path) {
    static char result[16] = {0};
    char* lastDot = strrchr(path, '.');
    
    if (lastDot != NULL) {
        strncpy(result, lastDot + 1, 15);
        result[15] = '\0';
        
        // 转换为小写
        for (int i = 0; result[i]; i++) {
            result[i] = tolower(result[i]);
        }
    } else {
        result[0] = '\0';
    }
    
    return result;
}

// 检查文件是否应该被排除
int shouldExcludeFile(const char* filename, const char* excludeExtensions) {
    if (excludeExtensions == NULL || excludeExtensions[0] == '\0') {
        return 0;
    }
    
    char* ext = getFileExtension(filename);
    if (ext[0] == '\0') {
        return 0;
    }
    
    // 复制排除扩展名字符串以便处理
    char exts[MAX_EXTENSIONS_LENGTH];
    strncpy(exts, excludeExtensions, MAX_EXTENSIONS_LENGTH - 1);
    exts[MAX_EXTENSIONS_LENGTH - 1] = '\0';
    
    // 转换为小写
    for (int i = 0; exts[i]; i++) {
        exts[i] = tolower(exts[i]);
    }
    
    // 检查扩展名是否在排除列表中
    char* token = strtok(exts, " ,;");
    while (token != NULL) {
        // 移除可能的前导点
        if (token[0] == '.') {
            token++;
        }
        
        if (strcmp(ext, token) == 0) {
            return 1;
        }
        
        token = strtok(NULL, " ,;");
    }
    
    return 0;
}

// 计算文件列表中非目录文件的数量
int countFiles(FileEntry* list) {
    int count = 0;
    FileEntry* current = list;
    
    while (current != NULL) {
        if (!current->is_directory) {
            count++;
        }
        current = current->next;
    }
    
    return count;
}

// 处理文件
void processFiles(FileEntry* fileList, const char* inputPath, const char* outputPath, const char* command, int copyOnError, const char* excludeExtensions) {
    // 首先创建完整的目录树
    createDirectoryTree(inputPath, outputPath);
    
    // 计算总文件数和当前处理进度
    int totalFiles = countFiles(fileList);
    int processedFiles = 0;
    int excludedFiles = 0;
    
    FileEntry* current = fileList;
    
    while (current != NULL) {
        if (!current->is_directory) {
            processedFiles++;
            
            // 检查文件是否应该被排除
            if (shouldExcludeFile(current->path, excludeExtensions)) {
                excludedFiles++;
                printf("Excluding file %d/%d: %s (extension excluded)\n", processedFiles, totalFiles, current->path);
                logMessage(LOG_INFO, "Excluding file %d/%d: %s (extension excluded)", processedFiles, totalFiles, current->path);
                
                // 如果启用了复制功能，复制被排除的文件
                if (copyOnError) {
                    // 计算相对路径
                    const char* relativePath = current->path + strlen(inputPath);
                    
                    // 构建目标文件路径
                    char targetPath[MAX_PATH_LENGTH];
                    snprintf(targetPath, MAX_PATH_LENGTH, "%s%s", outputPath, relativePath);
                    
                    printf("Copying excluded file: %s -> %s\n", current->path, targetPath);
                    logMessage(LOG_INFO, "Copying excluded file: %s -> %s", current->path, targetPath);
                    
                    // 复制源文件到目标路径
                    if (!copyFileWithPath(current->path, targetPath)) {
                        printf("Copying excluded file failed\n");
                        logMessage(LOG_ERROR, "Copying excluded file failed");
                    } else {
                        printf("Excluded file copied successfully\n");
                        logMessage(LOG_INFO, "Excluded file copied successfully");
                    }
                }
                
                // 更新进度显示
                printf("Progress: %d/%d files processed (%d excluded)\n\n", processedFiles - excludedFiles, totalFiles - excludedFiles, excludedFiles);
                logMessage(LOG_INFO, "Progress: %d/%d files processed (%d excluded)", processedFiles - excludedFiles, totalFiles - excludedFiles, excludedFiles);
                
                current = current->next;
                continue;
            }
            
            // 更新进度显示
            printf("Processing file %d/%d: %s\n", processedFiles - excludedFiles, totalFiles - excludedFiles, current->path);
            logMessage(LOG_INFO, "Processing file %d/%d: %s", processedFiles - excludedFiles, totalFiles - excludedFiles, current->path);
            
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
            logMessage(LOG_INFO, "Executing: %s", finalCommand);
            
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
                logMessage(LOG_ERROR, "Command execution failed (code: %d)", result);
                logCommandError(finalCommand, current->path, result);
                
                // 如果启用了命令失败时复制源文件的功能
                if (copyOnError) {
                    printf("Attempting to copy source file...\n");
                    logMessage(LOG_INFO, "Attempting to copy source file");
                    
                    // 构建目标文件路径
                    char targetPath[MAX_PATH_LENGTH];
                    snprintf(targetPath, MAX_PATH_LENGTH, "%s%s", outputPath, relativePath);
                    
                    // 复制源文件到目标路径
                    if (!copyFileWithPath(current->path, targetPath)) {
                        printf("Copying source file also failed\n");
                        logMessage(LOG_ERROR, "Copying source file also failed");
                    } else {
                        logMessage(LOG_INFO, "Source file copied successfully");
                    }
                }
            } else {
                printf("Command executed successfully\n");
                logMessage(LOG_INFO, "Command executed successfully");
            }
            
            // 更新进度显示
            printf("Progress: %d/%d files processed (%d excluded)\n\n", processedFiles - excludedFiles, totalFiles - excludedFiles, excludedFiles);
            logMessage(LOG_INFO, "Progress: %d/%d files processed (%d excluded)", processedFiles - excludedFiles, totalFiles - excludedFiles, excludedFiles);
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