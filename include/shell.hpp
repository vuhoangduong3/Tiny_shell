
// filepath: include/shell.hpp
#pragma once

#include <string>

// In banner khi shell khởi động.
void PrintBanner(void);

// Dispatcher: phân luồng lệnh tới đúng module xử lý.
//
// Thứ tự ưu tiên (giống IT3070):
//   1. Built-in  (help/exit/date/time/dir/path/addpath)
//   2. Process   (list/kill/stop/resume)
//   3. Script    (file *.bat)
//   4. External  (foreground hoặc background với `&`)
//
// Giá trị trả về:
//   true  — tiếp tục vòng lặp REPL (đã xử lý xong, kể cả khi lệnh lỗi).
//   false — gặp lệnh `exit` (cả khi gõ tay lẫn khi chạy trong script),
//            main() phải thoát vòng lặp và kết thúc chương trình.
bool DispatchCommand(const std::string &cmd);