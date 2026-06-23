#pragma once

// Chạy file script .bat từng dòng, mỗi dòng được chuyển về parser xử lý.
// Trả về: 0 = gặp "exit" trong script (dừng shell)
//          1 = chạy xong / lỗi -> tiếp tục REPL
int HandleScriptCommand(const char *cmd_line);
