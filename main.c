#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "file_utils.h"
#include "platform_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // 设置控制台输出为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    char inputPath[MAX_PATH_LENGTH];
    char outputPath[MAX_PATH_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char choice[10];
    int copyOnError = 0;
    
    printf("Batch Command Tree (BCT) - File Processing Utility\n");
    printf("Enter the folder path to process: ");
    fgets(inputPath, MAX_PATH_LENGTH, stdin);
    inputPath[strcspn(inputPath, "\n")] = 0; // 移除换行符
    
    // 检查路径是否存在
    if (!pathExists(inputPath)) {
        printf("Error: Path does not exist or cannot be accessed\n");
        return 1;
    }
    
    printf("\nFile tree structure:\n");
    printf("==========================================\n");
    printFileTree(inputPath, 0);
    printf("==========================================\n\n");
    
    printf("Enter processing command (use %%i for input file, %%o for output file base name):\n");
    printf("Example: ffmpeg -i %%i -vcodec libx264 %%o.mp4\n");
    printf("Command: ");
    fgets(command, MAX_COMMAND_LENGTH, stdin);
    command[strcspn(command, "\n")] = 0; // 移除换行符
    
    printf("Enter output file path: ");
    fgets(outputPath, MAX_PATH_LENGTH, stdin);
    outputPath[strcspn(outputPath, "\n")] = 0; // 移除换行符
    
    // 询问用户是否启用命令失败时复制源文件的功能
    printf("Enable copy source file on command failure? (y/n): ");
    fgets(choice, 10, stdin);
    choice[strcspn(choice, "\n")] = 0; // 移除换行符
    if (strcmp(choice, "y") == 0 || strcmp(choice, "Y") == 0) {
        copyOnError = 1;
        printf("Copy on error feature enabled.\n");
    } else {
        printf("Copy on error feature disabled.\n");
    }
    
    // 创建输出目录（如果不存在）
    if (createDirectory(outputPath) != 0 && errno != EEXIST) {
        printf("Error: Cannot create output directory\n");
        return 1;
    }
    
    // 构建文件列表并处理
    FileEntry* fileList = NULL;
    fileList = buildFileList(inputPath, fileList);
    
    printf("\nStarting file processing...\n");
    processFiles(fileList, inputPath, outputPath, command, copyOnError);
    
    // 清理
    freeFileList(fileList);
    
    printf("Processing completed!\n");
    system("pause");
    return 0;
}