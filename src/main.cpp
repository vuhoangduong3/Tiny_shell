// filepath: src/main.cpp
#include <windows.h>   // SetConsoleCP, GetCurrentDirectoryA, MAX_PATH

#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

#include "shell.hpp"
#include "parser.hpp"

#include "signal_handler.hpp"

namespace {

void ConfigureConsole(void) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
}

void PrintPrompt(void) {
    char cwd[MAX_PATH] = {};
    if (!GetCurrentDirectoryA(MAX_PATH, cwd)) {
        std::strcpy(cwd, "?");
    }
    std::printf("myShell\\%s>", cwd);
    std::fflush(stdout);
}

// std::nullopt  = EOF (Ctrl+D / Ctrl+Z)
// chuỗi rỗng   = dòng trống sau trim, bỏ qua
// còn lại      = lệnh cần dispatch
std::optional<std::string> ReadCommandLine(void) {
    char buffer[4096];
    if (std::fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        return std::nullopt;
    }
    return TrimCommand(buffer);
}

void RunRepl(void) {
    for (;;) {
        PrintPrompt();

        const std::optional<std::string> cmd = ReadCommandLine();
        if (!cmd.has_value()) {
            break;
        }
        if (cmd->empty()) {
            continue;
        }
        if (!DispatchCommand(*cmd)) {
            break;
        }
    }
}

}  // namespace

int main(void) {
    ConfigureConsole();
    setup_ctrl_c_handler();
    PrintBanner();
    RunRepl();
    return 0;
}
