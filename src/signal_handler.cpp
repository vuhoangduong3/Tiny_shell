#include "signal_handler.hpp"
#include <cstdio>
#include <atomic>

// Trạng thái của tiến trình foreground đang chạy
// Dùng atomic để đảm bảo an toàn khi luồng chính và luồng nhận tín hiệu (Ctrl+C) truy cập đồng thời
static std::atomic<HANDLE> g_foreground_process(nullptr);
static std::atomic<HANDLE> g_foreground_job(nullptr);
static std::atomic<DWORD>  g_foreground_pid(0);
static std::atomic<bool>   g_cancel_requested(false);

// Trạng thái console ban đầu để phục hồi
static DWORD g_original_input_mode = 0;
static DWORD g_original_output_mode = 0;
static bool  g_console_modes_saved = false;

void restore_console_after_cancel() {
    if (!g_console_modes_saved) return;

    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (input_handle != INVALID_HANDLE_VALUE) {
        SetConsoleMode(input_handle, g_original_input_mode);
        FlushConsoleInputBuffer(input_handle); // Xóa bộ đệm đầu vào tránh kẹt phím
    }
    if (output_handle != INVALID_HANDLE_VALUE) {
        SetConsoleMode(output_handle, g_original_output_mode);
    }
}

// Hàm callback được Windows gọi độc lập trên một luồng khác khi user ấn Ctrl+C
static BOOL WINAPI ctrl_c_handler(DWORD ctrl_type) {
    // Chỉ xử lý Ctrl+C và Ctrl+Break
    if (ctrl_type != CTRL_C_EVENT && ctrl_type != CTRL_BREAK_EVENT) {
        return FALSE; 
    }

    HANDLE process = g_foreground_process.load();
    HANDLE job = g_foreground_job.load();

    if (process != nullptr) {
        g_cancel_requested.store(true);
        // Ưu tiên tắt bằng Job Object để diệt cả chùm tiến trình con (nếu có)
        if (job != nullptr) {
            TerminateJobObject(job, 1);
        } else {
            TerminateProcess(process, 1);
        }
    } else {
        // Nếu không có tiến trình nào đang chạy, đánh dấu không có cancel
        g_cancel_requested.store(false);
    }

    return TRUE; // Trả về TRUE để báo với Windows: "Tôi đã xử lý tín hiệu này, đừng tắt Shell của tôi"
}

void setup_ctrl_c_handler() {
    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Lưu lại trạng thái console trước khi Shell hoạt động
    if (input_handle != INVALID_HANDLE_VALUE && output_handle != INVALID_HANDLE_VALUE &&
        GetConsoleMode(input_handle, &g_original_input_mode) &&
        GetConsoleMode(output_handle, &g_original_output_mode)) {
        g_console_modes_saved = true;
    }

    // Đăng ký hàm bắt sự kiện
    if (!SetConsoleCtrlHandler(ctrl_c_handler, TRUE)) {
        std::printf("  [Ctrl+C] Lỗi: Không thể đăng ký handler (mã lỗi: %lu).\n", GetLastError());
    }
}

void set_foreground_process(HANDLE process, HANDLE job, DWORD pid) {
    g_foreground_process.store(process);
    g_foreground_job.store(job);
    g_foreground_pid.store(pid);
    g_cancel_requested.store(false);
}

void clear_foreground_process() {
    g_foreground_process.store(nullptr);
    g_foreground_job.store(nullptr);
    g_foreground_pid.store(0);
}

bool was_cancel_requested() {
    return g_cancel_requested.load();
}