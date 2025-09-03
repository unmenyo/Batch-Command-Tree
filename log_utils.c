#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "log_utils.h"

static FILE* logFile = NULL;
static FILE* errorLogFile = NULL;

// 初始化日志系统
void initLogging(LogMode mode) {
    const char* modeStr = (mode == LOG_OVERWRITE) ? "w" : "a";
    
    // 打开运行日志文件
    logFile = fopen("bct.log", modeStr);
    if (logFile == NULL) {
        printf("Warning: Cannot open log file bct.log\n");
    }
    
    // 打开错误日志文件
    errorLogFile = fopen("error.log", modeStr);
    if (errorLogFile == NULL) {
        printf("Warning: Cannot open error log file error.log\n");
    }
    
    // 写入日志头
    if (logFile != NULL) {
        time_t now = time(NULL);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(logFile, "=== BCT Log Started at %s ===\n", timeStr);
        fflush(logFile);
    }
    
    if (errorLogFile != NULL) {
        time_t now = time(NULL);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(errorLogFile, "=== BCT Error Log Started at %s ===\n", timeStr);
        fflush(errorLogFile);
    }
}

// 记录日志消息
void logMessage(LogLevel level, const char* format, ...) {
    if (logFile == NULL) return;
    
    time_t now = time(NULL);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    const char* levelStr;
    switch (level) {
        case LOG_INFO: levelStr = "INFO"; break;
        case LOG_WARNING: levelStr = "WARNING"; break;
        case LOG_ERROR: levelStr = "ERROR"; break;
        default: levelStr = "UNKNOWN"; break;
    }
    
    fprintf(logFile, "[%s] [%s] ", timeStr, levelStr);
    
    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args);
    va_end(args);
    
    fprintf(logFile, "\n");
    fflush(logFile);
}

// 记录命令错误
void logCommandError(const char* command, const char* filename, int errorCode) {
    if (errorLogFile == NULL) return;
    
    time_t now = time(NULL);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(errorLogFile, "[%s] Command failed: %s\n", timeStr, command);
    fprintf(errorLogFile, "File: %s\n", filename);
    fprintf(errorLogFile, "Error code: %d\n\n", errorCode);
    fflush(errorLogFile);
}

// 关闭日志系统
void closeLogging() {
    if (logFile != NULL) {
        time_t now = time(NULL);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(logFile, "=== BCT Log Ended at %s ===\n\n", timeStr);
        fclose(logFile);
        logFile = NULL;
    }
    
    if (errorLogFile != NULL) {
        time_t now = time(NULL);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(errorLogFile, "=== BCT Error Log Ended at %s ===\n\n", timeStr);
        fclose(errorLogFile);
        errorLogFile = NULL;
    }
}