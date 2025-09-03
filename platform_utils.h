#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

// 平台相关函数声明
int pathExists(const char* path);
int createDirectory(const char* path);
void printFileTree(const char* path, int depth);
FileEntry* buildFileList(const char* path, FileEntry* list);
void createDirectoryTree(const char* inputPath, const char* outputPath);
int copyFileWithPath(const char* source, const char* destination);

#endif