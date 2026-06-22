#pragma once
#include <windows.h>

// Cài đặt handler bắt sự kiện Ctrl+C và lưu trạng thái console
void setup_ctrl_c_handler();

// Phục hồi lại trạng thái console nếu tiến trình bị ngắt bằng Ctrl+C
void restore_console_after_cancel();

// Khai báo cho handler biết tiến trình foreground nào đang chạy để ngắt khi cần
void set_foreground_process(HANDLE process, HANDLE job, DWORD pid);

// Xóa thông tin tiến trình foreground sau khi nó chạy xong
void clear_foreground_process();

// Kiểm tra xem tiến trình vừa rồi có bị người dùng ấn Ctrl+C để hủy không
bool was_cancel_requested();