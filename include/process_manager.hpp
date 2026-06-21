#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>

// Cấu trúc lưu thông tin background process
struct BackgroundProcess {
    uint32_t pid;
    char command[512]; // Lưu lệnh chạy process
    HANDLE process_handle;
    HANDLE thread_handle;
    HANDLE job_handle;
    bool is_stopped;
};

// Khai báo danh sách process dạng extern để dùng chung toàn chương trình
extern std::vector<BackgroundProcess> background_processes;

// Các hàm giao tiếp chính ra bên ngoài
int handle_process_command(const char *cmd_line);
void cleanup_finished_processes();
void print_running_processes();