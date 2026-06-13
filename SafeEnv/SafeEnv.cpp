// SafeEnv.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "SafeEnv.h"
// TODO: This is an example of a library function

std::optional<std::string> get_env_safe(const std::string& key) {
    size_t required_size = 0;
#if defined(_MSC_VER) 
    if (getenv_s(&required_size, nullptr, 0, key.c_str()) != 0 || required_size == 0) return std::nullopt;
#else 
    if (getenv_s(&required_size, nullptr, 0, key.c_str()) != 0 || required_size == 0) return std::nullopt;
#endif

    std::vector<char> buffer(required_size);
#if defined(_MSC_VER)
    if (getenv_s(&required_size, buffer.data(), required_size, key.c_str()) != 0) return std::nullopt;
#else
    if (getenv_s(&required_size, buffer.data(), required_size, key.c_str()) != 0) return std::nullopt;
#endif
    return std::string(buffer.data(), required_size - 1);
}
