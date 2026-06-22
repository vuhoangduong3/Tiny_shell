#include "builtins.hpp"
#include "parser.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
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
        std::cout << "Available built-in commands:\n"
                  << "  help              - Show this help message\n"
                  << "  exit              - Exit the shell\n"
                  << "  date              - Show current date\n"
                  << "  time              - Show current time\n"
                  << "  dir [path]        - List files and directories\n"
                  << "  path              - Show current PATH\n"
                  << "  addpath <dir>     - Add directory to PATH\n";
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
#else
        const char* pathEnv = std::getenv("PATH");
        std::string pathValue = pathEnv ? pathEnv : "";
        if (pathValue.empty()) {
            std::cout << "PATH is not set.\n";
            return 1;
        }
#endif
        std::cout << "PATH=" << pathValue << "\n";
        return 1;
    }

    if (verb == "addpath") {
        if (args.empty()) {
            std::cerr << "addpath: missing directory argument\n"
                      << "Usage: addpath <dir>\n";
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
