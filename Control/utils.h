#ifndef UTILS_H
#define UTILS_H

#include "headers.h"

auto ToWstring = [](std::string str) -> std::wstring
{
    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstr(size_needed, L' ');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
    return wstr;
};

#endif // UTILS_H
