#pragma once
#include <optional>
#include <cstdlib>
#include <string>
#include <vector>
std::optional<std::string> get_env_safe(const std::string& key);