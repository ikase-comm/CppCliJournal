#include "pch.h"
#include "framework.h"
#include "DecryptionJournal.h"
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "bcrypt.lib")

namespace jenc {

    // Helper: Hashes variable-length passphrases to a secure 32-byte (256-bit) AES key
    std::vector<BYTE> deriveKeySHA256(const std::string& key) {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_HASH_HANDLE hHash = NULL;
        std::vector<BYTE> hash(32, 0);

        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {
            DWORD cbHashObject = 0, cbData = 0;
            BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
            std::vector<BYTE> hashObject(cbHashObject);

            if (BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbHashObject, NULL, 0, 0) == 0) {
                BCryptHashData(hHash, (PBYTE)key.data(), (ULONG)key.size(), 0);
                BCryptFinishHash(hHash, hash.data(), (ULONG)hash.size(), 0);
                BCryptDestroyHash(hHash);
            }
            BCryptCloseAlgorithmProvider(hAlg, 0);
        }
        return hash;
    }

    std::string encryptJournal(std::string text, std::string key)
    {
        if (text.empty() || key.empty()) return "";

        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_KEY_HANDLE hKey = NULL;
        std::string hexResult = "";

        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0) return "";
        BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);

        std::vector<BYTE> aesKey = deriveKeySHA256(key);

        DWORD cbKeyObject = 0, cbData = 0;
        BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0);
        std::vector<BYTE> keyObject(cbKeyObject);

        if (BCryptGenerateSymmetricKey(hAlg, &hKey, keyObject.data(), cbKeyObject, aesKey.data(), (ULONG)aesKey.size(), 0) == 0) {
            std::vector<BYTE> iv(16, 0xAA);

            DWORD cbCiphertext = 0;
            BCryptEncrypt(hKey, (PBYTE)text.data(), (ULONG)text.size(), NULL, iv.data(), (ULONG)iv.size(), NULL, 0, &cbCiphertext, BCRYPT_BLOCK_PADDING);

            std::vector<BYTE> ciphertext(cbCiphertext);
            iv = std::vector<BYTE>(16, 0xAA);

            if (BCryptEncrypt(hKey, (PBYTE)text.data(), (ULONG)text.size(), NULL, iv.data(), (ULONG)iv.size(), ciphertext.data(), cbCiphertext, &cbData, BCRYPT_BLOCK_PADDING) == 0) {
                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for (BYTE b : ciphertext) {
                    ss << std::setw(2) << (int)b;
                }
                hexResult = ss.str();
            }
            BCryptDestroyKey(hKey);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hexResult;
    }

    std::string decryptJournal(std::string text, std::string key)
    {
        if (text.empty() || key.empty() || text.length() % 2 != 0) return "";

        std::vector<BYTE> ciphertext;
        ciphertext.reserve(text.length() / 2);
        for (size_t i = 0; i < text.length(); i += 2) {
            std::string byteString = text.substr(i, 2);
            ciphertext.push_back(static_cast<BYTE>(strtol(byteString.c_str(), nullptr, 16)));
        }

        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_KEY_HANDLE hKey = NULL;
        std::string plaintextResult = "";

        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0) return "";
        BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);

        std::vector<BYTE> aesKey = deriveKeySHA256(key);
        DWORD cbKeyObject = 0, cbData = 0;
        BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0);
        std::vector<BYTE> keyObject(cbKeyObject);

        if (BCryptGenerateSymmetricKey(hAlg, &hKey, keyObject.data(), cbKeyObject, aesKey.data(), (ULONG)aesKey.size(), 0) == 0) {
            std::vector<BYTE> iv(16, 0xAA);

            DWORD cbPlaintext = 0;
            BCryptDecrypt(hKey, ciphertext.data(), (ULONG)ciphertext.size(), NULL, iv.data(), (ULONG)iv.size(), NULL, 0, &cbPlaintext, BCRYPT_BLOCK_PADDING);

            std::vector<BYTE> plaintext(cbPlaintext);
            iv = std::vector<BYTE>(16, 0xAA);

            if (BCryptDecrypt(hKey, ciphertext.data(), (ULONG)ciphertext.size(), NULL, iv.data(), (ULONG)iv.size(), plaintext.data(), cbPlaintext, &cbData, BCRYPT_BLOCK_PADDING) == 0) {
                plaintextResult = std::string((char*)plaintext.data(), cbData);
            }
            BCryptDestroyKey(hKey);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return plaintextResult;
    }
} // namespace journalencryption
