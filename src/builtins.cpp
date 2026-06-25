#include "builtins.hpp"
#include "parser.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace fs = std::filesystem;

int HandleBuiltinCommand(const std::string &Cmd) {
    std::string verb = ToLower(FirstToken(Cmd));
    std::string args = CommandArgs(Cmd);

    if (verb == "help") {
        std::cout << "=== TinyShell - Danh sach tat ca cac lenh ===\n\n"

                  << "[Built-in commands]\n"
                  << "  help              - Hien thi danh sach lenh nay\n"
                  << "  exit              - Thoat shell\n"
                  << "  date              - Hien thi ngay hien tai\n"
                  << "  time              - Hien thi gio hien tai\n"
                  << "  dir [path]        - Liet ke file va thu muc\n"
                  << "  path              - Hien thi bien moi truong PATH\n"
                  << "  addpath <dir>     - Them thu muc vao PATH\n\n"

                  << "[Process management]\n"
                  << "  list              - Liet ke cac tien trinh background\n"
                  << "  kill <pid>        - Ket thuc tien trinh theo PID\n"
                  << "  stop <pid>        - Tam dung (suspend) tien trinh theo PID\n"
                  << "  resume <pid>      - Tiep tuc (resume) tien trinh theo PID\n\n"
                  << "  <file>.bat        - Chay file script .bat tung dong\n\n"

                  << "[External commands]\n"
                  << "  <command>         - Chay chuong trinh ngoai (foreground)\n"
                  << "  <command> &       - Chay chuong trinh ngoai (background)\n";
        return 1;
    }

    if (verb == "exit") {
        return 0; // Signal shell to stop
    }

    if (verb == "date") {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::cout << "Current date: " << std::put_time(std::localtime(&now_c), "%Y-%m-%d") << "\n";
        return 1;
    }

    if (verb == "time") {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::cout << "Current time: " << std::put_time(std::localtime(&now_c), "%H:%M:%S") << "\n";
        return 1;
    }

    if (verb == "dir") {
        std::string targetPath = args.empty() ? "." : args;
        
        try {
            if (!fs::exists(targetPath)) {
                std::cerr << "dir: cannot access '" << targetPath << "': No such file or directory\n";
                return 1;
            }

            if (!fs::is_directory(targetPath)) {
                // If it's a file, just print its name
                std::cout << targetPath << "\n";
                return 1;
            }

            std::cout << "Directory listing for " << targetPath << ":\n\n";
            for (const auto& entry : fs::directory_iterator(targetPath)) {
                auto path = entry.path();
                std::string type = entry.is_directory() ? "<DIR>" : "     ";
                std::cout << type << "  " << path.filename().string() << "\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "dir: error: " << e.what() << "\n";
        }
        return 1;
    }

    if (verb == "path") {
#ifdef _WIN32
        DWORD size = GetEnvironmentVariableA("PATH", nullptr, 0);
        if (size == 0) {
            std::cout << "PATH is not set.\n";
            return 1;
        }

        std::string pathValue(size, '\0');
        GetEnvironmentVariableA("PATH", &pathValue[0], size);
        // Remove trailing null terminator added by string constructor
        pathValue.pop_back();
        const char delimiter = ';';
#else
        const char* pathEnv = std::getenv("PATH");
        std::string pathValue = pathEnv ? pathEnv : "";
        if (pathValue.empty()) {
            std::cout << "PATH is not set.\n";
            return 1;
        }
        const char delimiter = ':';
#endif
        std::cout << "=== Current PATH ===\n";
        std::istringstream stream(pathValue);
        std::string dir;
        int index = 1;
        while (std::getline(stream, dir, delimiter)) {
            if (!dir.empty()) {
                std::cout << "  [" << std::setw(2) << index++ << "] " << dir << "\n";
            }
        }
        std::cout << "====================\n";
        std::cout << "Total: " << (index - 1) << " directories\n";
        return 1;
    }

    if (verb == "addpath") {
        if (args.empty()) {
            std::cerr << "addpath: missing directory argument\n"
                      << "Usage: addpath <dir>\n";
            return 1;
        }

        // Validate that the path exists and is a directory
        if (!fs::exists(args)) {
            std::cerr << "addpath: '" << args << "': No such file or directory\n";
            return 1;
        }
        if (!fs::is_directory(args)) {
            std::cerr << "addpath: '" << args << "': Not a directory\n";
            return 1;
        }

#ifdef _WIN32
        DWORD size = GetEnvironmentVariableA("PATH", nullptr, 0);
        std::string currentPath;
        if (size > 0) {
            currentPath.resize(size);
            GetEnvironmentVariableA("PATH", &currentPath[0], size);
            currentPath.pop_back();
        }

        // Add semicolon if currentPath is not empty and doesn't end with one
        if (!currentPath.empty() && currentPath.back() != ';') {
            currentPath += ";";
        }
        currentPath += args;

        if (SetEnvironmentVariableA("PATH", currentPath.c_str())) {
            std::cout << "Successfully added '" << args << "' to PATH.\n";
        } else {
            std::cerr << "addpath: failed to set environment variable. Error code: " << GetLastError() << "\n";
        }
#else
        const char* pathEnv = std::getenv("PATH");
        std::string currentPath = pathEnv ? pathEnv : "";

        // Add colon if currentPath is not empty and doesn't end with one
        if (!currentPath.empty() && currentPath.back() != ':') {
            currentPath += ":";
        }
        currentPath += args;

        if (setenv("PATH", currentPath.c_str(), 1) == 0) {
            std::cout << "Successfully added '" << args << "' to PATH.\n";
        } else {
            std::cerr << "addpath: failed to set environment variable.\n";
        }
#endif
        return 1;
    }

    return -1; // Not a built-in command
}
