#pragma once

#include "pch.h"

namespace utils
{
    // Thanks to https://medium.com/@batteriesnotincludeddev/indexed-for-each-in-modern-c-7df21fce72a1
    auto enumerate(const auto &data)
    {
        return data | std::views::transform([i = 0](const auto &value) mutable
                                            { return std::make_pair(i++, value); });
    }

    template <typename... Args>
    void print(Args &&...args)
    {
        (std::cout << ... << args);
    }

    template <typename... Args>
    void message(Args &&...args)
    {
        (std::cout << ... << args);
        std::cout << '\n';
    }

    template <typename... Args>
    void status(Args &&...args)
    {
        std::cout << "-- [Status] ";
        (std::cout << ... << args);
        std::cout << " --\n";
    }

    template <typename... Args>
    void info(Args &&...args)
    {
        std::cout << "## [Info] ";
        (std::cout << ... << args);
        std::cout << " ##\n";
    }

    template <typename... Args>
    void error(Args &&...args)
    {
        std::cerr << "!! [Error] ";
        (std::cerr << ... << args);
        std::cout << "!! " << std::endl;
    }
} // namespace utils