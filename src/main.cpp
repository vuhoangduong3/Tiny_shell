// filepath: src/main.cpp
#include <windows.h>   // SetConsoleCP, GetCurrentDirectoryA, MAX_PATH

#include <cstdio>
#include <cstring>
#include <string>

#include "shell.hpp"
#include "parser.hpp"

// Forward decl — do HOÀNG (Signal Handler) implement.
extern void SetupCtrlCHandler(void);

int main(void) {
    // 1. Set UTF-8 cho console để in tiếng Việt có dấu.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // 2. Đăng ký Ctrl+C handler (do Hoàng cung cấp).
    SetupCtrlCHandler();

    // 3. In banner.
    PrintBanner();

    // 4. Vòng lặp REPL.
    char buffer[4096];
    while (true) {
        // In prompt: myShell\<cwd>>
        char cwd[MAX_PATH] = {};
        if (!GetCurrentDirectoryA(MAX_PATH, cwd)) {
            std::strcpy(cwd, "?");
        }
        std::printf("myShell\\%s>", cwd);
        std::fflush(stdout);

        // Đọc 1 dòng lệnh.
        if (std::fgets(buffer, sizeof(buffer), stdin) == nullptr) {
            break;   // EOF (Ctrl+D / Ctrl+Z) -> thoát
        }

        // Trim và bỏ qua dòng rỗng.
        std::string cmd = TrimCommand(buffer);
        if (cmd.empty()) {
            continue;
        }

        // Dispatch. Nếu trả false nghĩa là gặp "exit".
        if (!DispatchCommand(cmd)) {
            break;
        }
    }

    return 0;
}