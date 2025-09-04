#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define MAX_PATH_LENGTH 1024
#define MAX_COMMAND_LENGTH 2048
#define MAX_EXTENSIONS_LENGTH 256

// 结构体用于存储文件信息
typedef struct FileEntry {
    char path[MAX_PATH_LENGTH];
    int is_directory;
    struct FileEntry* next;
} FileEntry;

// 通用函数声明
void printFileTree(const char* path, int depth);
FileEntry* buildFileList(const char* path, FileEntry* list);
void freeFileList(FileEntry* list);
void processFiles(FileEntry* fileList, const char* inputPath, const char* outputPath, const char* command, int copyOnError, const char* excludeExtensions);
void createDirectoryTree(const char* inputPath, const char* outputPath);
char* getFileNameWithoutExtension(const char* path);
char* getFileExtension(const char* path);
int copyFileWithPath(const char* source, const char* destination);
int shouldExcludeFile(const char* filename, const char* excludeExtensions);

// 新增函数声明
int countFiles(FileEntry* list);

#endif