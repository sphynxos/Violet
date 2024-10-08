#include <common.h>

#include <drivers/fs/sfs.h>
#include <lib/alloc.h>

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_SNPRINTF_SAFE_TRIM_STRING_ON_OVERFLOW 1
typedef long ssize_t;

#define NANOPRINTF_IMPLEMENTATION
#include <nanoprintf.h>

EFI_HANDLE *imageHandle;
EFI_SYSTEM_TABLE *systemTable;

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stdout;
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *stdin;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stderr;

void _putc(char ch) {
    CHAR16 data[2];
    data[0] = ch;
    data[1] = '\0';

    stdout->OutputString(stdout, data);
}

int printf(const char* fmt, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    int length = npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (length < 0 || length >= (int)sizeof(buffer)) {
        return -1;
    }

    for (int i = 0; i < length; ++i) {
        _putc(buffer[i]);
    }

    return length;
}

void __violet_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    imageHandle = &ImageHandle;
    systemTable = SystemTable;
    stdout = systemTable->ConOut;
    stdin = systemTable->ConIn;
    stderr = systemTable->StdErr;

    stdout->SetAttribute(stdout, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    stdout->SetCursorPosition(stdout, 0, 0);
    stdout->ClearScreen(stdout);
}

void __violet_handle() {
    printf("Violet Running on %s (%s)\r\n", _VIOLET_TARGET, _VIOLET_BUILD);
    SimpleFile test = sfs_open(L"\\test.txt");
    if(EFI_ERROR(test.status)) {
        printf("[ERROR] Failed to open \"\\test.txt\"\r\n");
        for(;;);
    }

    char* buff = (char*)malloc(test.info.PhysicalSize);
    if(buff == NULL) {
        printf("[ERROR] Failed to allocate memory for file buffer\r\n");
        for(;;);
    }

    sfs_read(&test, buff);
    if(EFI_ERROR(test.status)) {
        printf("[ERROR] Failed to read \"\\test.txt\"\r\n");
        for(;;);
    }

    printf("%.*s\r\n", test.info.PhysicalSize, buff);
    sfs_close(&test);
    free(buff);
}