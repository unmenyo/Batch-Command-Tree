#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdio.h>

// 日志级别
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// 日志模式
typedef enum {
    LOG_OVERWRITE,
    LOG_APPEND
} LogMode;

// 函数声明
void initLogging(LogMode mode);
void logMessage(LogLevel level, const char* format, ...);
void logCommandError(const char* command, const char* filename, int errorCode);
void closeLogging();

#endif