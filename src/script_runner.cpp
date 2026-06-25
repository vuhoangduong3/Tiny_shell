#include "script_runner.hpp"
#include "shell.hpp"
#include "parser.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

static std::string get_script_path(const std::string &cmd_line)
{
    std::string text = TrimCommand(cmd_line);
    if (text.empty())
    {
        return "";
    }

    if (text[0] == '"')
    {
        size_t end_quote = text.find('"', 1);
        if (end_quote == std::string::npos)
        {
            return "";
        }
        return text.substr(1, end_quote - 1);
    }

    return FirstToken(text);
}

// Tim file script trong cac thu muc PATH.
// Neu file da co san (tuong doi hoac tuyet doi) thi tra ve luon.
// Neu khong, duyet tung thu muc trong PATH de tim.
static std::string resolve_script_path(const std::string &filename)
{
    // Thu mo truc tiep truoc (duong dan tuong doi hoac tuyet doi)
    {
        std::ifstream test(filename);
        if (test.is_open())
        {
            return filename;
        }
    }

    // Lay bien moi truong PATH
    std::string path_env;
#ifdef _WIN32
    DWORD size = GetEnvironmentVariableA("PATH", nullptr, 0);
    if (size > 0)
    {
        path_env.resize(size);
        GetEnvironmentVariableA("PATH", &path_env[0], size);
        path_env.pop_back(); // bo null terminator
    }
    const char delimiter = ';';
#else
    const char *env = std::getenv("PATH");
    if (env != nullptr)
    {
        path_env = env;
    }
    const char delimiter = ':';
#endif

    // Duyet tung thu muc trong PATH
    std::istringstream stream(path_env);
    std::string dir;
    while (std::getline(stream, dir, delimiter))
    {
        if (dir.empty())
        {
            continue;
        }
        // Tao duong dan day du: dir/filename hoac dir\filename
        std::string full_path = dir;
        char last = full_path.back();
        if (last != '/' && last != '\\')
        {
            full_path += '\\';
        }
        full_path += filename;

        std::ifstream test(full_path);
        if (test.is_open())
        {
            return full_path;
        }
    }

    // Khong tim thay
    return "";
}

static void remove_utf8_bom(std::string &line)
{
    if (line.size() >= 3 &&
        static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF)
    {
        line.erase(0, 3);
    }
}

int HandleScriptCommand(const char *cmd_line)
{
    if (cmd_line == nullptr || cmd_line[0] == '\0')
    {
        return 1;
    }

    std::string script_name = get_script_path(cmd_line);
    if (script_name.empty())
    {
        std::printf("  [script] Cú pháp: <file>.bat\n");
        return 1;
    }

    // Tim file trong thu muc hien tai hoac trong PATH
    std::string script_path = resolve_script_path(script_name);
    if (script_path.empty())
    {
        std::printf("  [script] Không mở được file: %s\n", script_name.c_str());
        return 1;
    }

    std::ifstream file(script_path);
    std::string line;
    int line_number = 0;
    while (std::getline(file, line))
    {
        line_number++;
        if (line_number == 1)
        {
            remove_utf8_bom(line);
        }

        std::string command = TrimCommand(line);
        if (command.empty())
        {
            continue;
        }

        if (!DispatchCommand(command))
        {
            return 0;
        }
    }

    return 1;
}
