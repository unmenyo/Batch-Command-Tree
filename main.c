#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "file_utils.h"
#include "platform_utils.h"
#include "log_utils.h"

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
    
    // 询问日志模式
    printf("Log file mode:\n");
    printf("1. Overwrite existing log files\n");
    printf("2. Append to existing log files\n");
    printf("Choose (1/2): ");
    fgets(choice, 10, stdin);
    choice[strcspn(choice, "\n")] = 0;
    
    LogMode logMode = (choice[0] == '1') ? LOG_OVERWRITE : LOG_APPEND;
    initLogging(logMode);
    
    logMessage(LOG_INFO, "BCT started");
    
    printf("Batch Command Tree (BCT) - File Processing Utility\n");
    logMessage(LOG_INFO, "Batch Command Tree (BCT) - File Processing Utility");
    
    printf("Enter the folder path to process: ");
    fgets(inputPath, MAX_PATH_LENGTH, stdin);
    inputPath[strcspn(inputPath, "\n")] = 0;
    logMessage(LOG_INFO, "Input path: %s", inputPath);
    
    // 检查路径是否存在
    if (!pathExists(inputPath)) {
        printf("Error: Path does not exist or cannot be accessed\n");
        logMessage(LOG_ERROR, "Path does not exist or cannot be accessed: %s", inputPath);
        closeLogging();
        return 1;
    }
    
    printf("\nFile tree structure:\n");
    printf("==========================================\n");
    logMessage(LOG_INFO, "File tree structure:");
    logMessage(LOG_INFO, "==========================================");
    printFileTree(inputPath, 0);
    logMessage(LOG_INFO, "==========================================");
    printf("==========================================\n\n");
    
    printf("Enter processing command (use %%i for input file, %%o for output file base name):\n");
    printf("Example: ffmpeg -i %%i -vcodec libx264 %%o.mp4\n");
    printf("Command: ");
    fgets(command, MAX_COMMAND_LENGTH, stdin);
    command[strcspn(command, "\n")] = 0;
    logMessage(LOG_INFO, "Command: %s", command);
    
    printf("Enter output file path: ");
    fgets(outputPath, MAX_PATH_LENGTH, stdin);
    outputPath[strcspn(outputPath, "\n")] = 0;
    logMessage(LOG_INFO, "Output path: %s", outputPath);
    
    // 询问用户是否启用命令失败时复制源文件的功能
    printf("Enable copy source file on command failure? (y/n): ");
    fgets(choice, 10, stdin);
    choice[strcspn(choice, "\n")] = 0;
    if (strcmp(choice, "y") == 0 || strcmp(choice, "Y") == 0) {
        copyOnError = 1;
        printf("Copy on error feature enabled.\n");
        logMessage(LOG_INFO, "Copy on error feature enabled");
    } else {
        printf("Copy on error feature disabled.\n");
        logMessage(LOG_INFO, "Copy on error feature disabled");
    }
    
    // 创建输出目录（如果不存在）
    if (createDirectory(outputPath) != 0 && errno != EEXIST) {
        printf("Error: Cannot create output directory\n");
        logMessage(LOG_ERROR, "Cannot create output directory: %s", outputPath);
        closeLogging();
        return 1;
    }
    
    // 构建文件列表并处理
    FileEntry* fileList = NULL;
    fileList = buildFileList(inputPath, fileList);
    
    printf("\nStarting file processing...\n");
    logMessage(LOG_INFO, "Starting file processing");
    processFiles(fileList, inputPath, outputPath, command, copyOnError);
    
    // 清理
    freeFileList(fileList);
    
    printf("Processing completed!\n");
    logMessage(LOG_INFO, "Processing completed");
    closeLogging();
    
    system("pause");
    return 0;
}