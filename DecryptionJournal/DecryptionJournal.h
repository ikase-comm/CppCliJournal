#pragma once
#include <string>
#include <string>
#include <sstream>
#include <iomanip>
namespace jenc {
	std::string encryptJournal(std::string text, std::string key);
	std::string decryptJournal(std::string text, std::string key);
}