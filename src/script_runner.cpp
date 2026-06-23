#include "script_runner.hpp"
#include "shell.hpp"
#include "parser.hpp"

#include <cstdio>
#include <fstream>
#include <string>

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

    std::string script_path = get_script_path(cmd_line);
    if (script_path.empty())
    {
        std::printf("  [script] Cú pháp: <file>.bat\n");
        return 1;
    }

    std::ifstream file(script_path);
    if (!file.is_open())
    {
        std::printf("  [script] Không mở được file: %s\n", script_path.c_str());
        return 1;
    }

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
