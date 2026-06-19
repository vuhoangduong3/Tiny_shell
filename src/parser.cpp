#include "parser.hpp"

#include <cctype>
#include <cstring>
#include <string>

namespace {

// Cac helper noi bo, khong xuat ra ngoai.
// Ly do tach rieng: vua de test, vua giam trung lap giua TrimCommand va cac
// ham parse khac (FirstToken, CommandArgs, IsBackgroundCommand...).

inline bool IsWs(char Ch) {
    return Ch == ' ' || Ch == '\t' || Ch == '\r' || Ch == '\n';
}

// BOM UTF-8: 3 byte dau la EF BB BF.
bool HasUtf8Bom(const std::string &S) {
    return S.size() >= 3 &&
           static_cast<unsigned char>(S[0]) == 0xEFu &&
           static_cast<unsigned char>(S[1]) == 0xBBu &&
           static_cast<unsigned char>(S[2]) == 0xBFu;
}

// Bo qua whitespace bat dau tu `From`. Tra ve vi tri dau tien KHONG phai ws.
size_t SkipWsFwd(const std::string &S, size_t From) {
    size_t I = From;
    while (I < S.size() && IsWs(S[I])) {
        ++I;
    }
    return I;
}

// Lui ve phia truoc de bo qua whitespace cuoi. Tra ve vi tri (exclusive) sau
// khi cat. Neu ca chuoi chi toan ws thi tra ve `To`.
size_t SkipWsBwd(const std::string &S, size_t To) {
    size_t I = To;
    while (I > 0 && IsWs(S[I - 1])) {
        --I;
    }
    return I;
}

}  // namespace

// Bo BOM (neu co) + cat khoang trang dau cuoi.
// Day la ham nen tang cho moi ham parse khac.
std::string TrimCommand(const std::string &Text) {
    size_t Lo = HasUtf8Bom(Text) ? 3u : 0u;
    Lo = SkipWsFwd(Text, Lo);
    size_t Hi = SkipWsBwd(Text, Text.size());
    if (Lo >= Hi) {
        return std::string();
    }
    return Text.substr(Lo, Hi - Lo);
}

// Token dau tien sau khi trim. Neu khong co khoang trang -> ca chuoi.
std::string FirstToken(const std::string &Line) {
    const std::string T = TrimCommand(Line);
    const size_t Sp = T.find_first_of(" \t");
    if (Sp == std::string::npos) {
        return T;
    }
    return T.substr(0, Sp);
}

std::string CommandArgs(const std::string &Line) {
    const std::string T = TrimCommand(Line);
    const size_t Sp = T.find_first_of(" \t");
    if (Sp == std::string::npos) {
        return std::string();
    }
    // Args = phan con lai, cat trim them 1 lan cho chac.
    return TrimCommand(T.substr(Sp + 1));
}

// Khong phan biet hoa/thuong khi so verb (list, kill, help...).
std::string ToLower(std::string Text) {
    const size_t N = Text.size();
    for (size_t I = 0; I < N; ++I) {
        const unsigned char Uc = static_cast<unsigned char>(Text[I]);
        Text[I] = static_cast<char>(std::tolower(Uc));
    }
    return Text;
}

// Ten lenh that su, ho tro path co khoang trang nam trong dau nhay kep.
// VD: `"C:\My Tools\app.exe" --foo` -> `C:\My Tools\app.exe`
std::string CommandNameToken(const std::string &Cmd) {
    std::string T = TrimCommand(Cmd);
    if (T.empty() || T.front() != '"') {
        return FirstToken(T);
    }
    const size_t EndQuote = T.find('"', 1);
    if (EndQuote == std::string::npos) {
        // Mo nhay kep ma khong dong -> fallback ve token dau tien.
        return FirstToken(T);
    }
    return T.substr(1, EndQuote - 1);
}

bool EndsWithBat(const std::string &Token) {
    if (Token.size() < 4) {
        return false;
    }
    const std::string Ext = ToLower(Token.substr(Token.size() - 4));
    return Ext == ".bat";
}

bool IsBatCommand(const std::string &Cmd) {
    return EndsWithBat(CommandNameToken(Cmd));
}

// 4 lenh quan ly process: list, kill, stop, resume.
bool IsProcessCommand(const std::string &Cmd) {
    const std::string Verb = ToLower(FirstToken(Cmd));
    return Verb == "kill"  ||
           Verb == "stop"  ||
           Verb == "resume"||
           Verb == "list";
}

// Background mode: cuoi dong (sau khi trim) la `&`.
// Khoang trang truoc `&` duoc bo qua theo spec header.
bool IsBackgroundCommand(const std::string &Line) {
    const std::string T = TrimCommand(Line);
    if (T.empty()) {
        return false;
    }
    return T.back() == '&';
}