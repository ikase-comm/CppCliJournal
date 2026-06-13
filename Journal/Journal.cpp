#include <iostream> 
#include <fstream> 
#include <vector> 
#include <string> 
#include <cstdlib> 
#include <filesystem> 
#include <optional> 
#include <chrono> 
#include <iomanip> 
#include <sstream> 
#include <future> 
#include <atomic> 
#include <DecryptionJournal.h> 
#include <SafeEnv.h> 

#if defined(_WIN32)
#include <windows.h>
#endif
enum class Language {
    English,
    French
};

Language currentLanguage = Language::English;
namespace fs = std::filesystem;

const std::string AUTH_TAG = "JOURNAL_AUTH_OK_V2:";
constexpr int INACTIVITY_TIMEOUT_SECONDS = 60;

// ANSI Escape Sequences for Vibrant Text Colours
const std::string RESET = "\033[0m";
const std::string RED = "\033[91m";
const std::string GREEN = "\033[92m";
const std::string YELLOW = "\033[93m";
const std::string CYAN = "\033[96m";
const std::string MAGENTA = "\033[95m";

std::string getLanguagePath() {
    auto userProfile = get_env_safe("USERPROFILE");

    if (!userProfile) return "journal_lang.cfg";

    fs::path profilePath(*userProfile);

    return (profilePath / "journal_lang.cfg").string();
}

void saveLanguage(Language lang) {
    std::ofstream out(getLanguagePath(), std::ios::trunc);

    if (!out)
        return;

    if (lang == Language::English)
        out << "en";
    else
        out << "fr";
}

Language loadLanguage() {
    std::ifstream in(getLanguagePath());

    if (!in)
        return Language::English;

    std::string code;
    std::getline(in, code);

    if (code == "fr")
        return Language::French;

    return Language::English;
}

std::string tr(const std::string& en, const std::string& fr) {
    return (currentLanguage == Language::French) ? fr : en;
}

std::string getJournalPath() {
    auto userProfile = get_env_safe("USERPROFILE");
    if (!userProfile) return "journal.dat";
    fs::path profilePath(*userProfile);
    return (profilePath / "journal.dat").string();
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
#if defined(_MSC_VER) 
    struct tm buf;
    localtime_s(&buf, &in_time_t);
    ss << MAGENTA << std::put_time(&buf, "[%Y-%m-%d %H:%M:%S]") << RESET << "\n";
#else 
    ss << MAGENTA << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S]") << RESET << "\n";
#endif
    return ss.str();
}

bool encryptAndSaveWithBackup(const std::string& inputText, const std::string& targetPath, const std::string& key) {
    std::string tempPath = targetPath + ".tmp";
    std::string backupPath = targetPath + ".bak";
    std::ofstream outFile(tempPath, std::ios::binary);
    if (!outFile) {
        std::cerr << RED << "❌ Error: Unable to create transaction state file.\n" << RESET;
        return false;
    }
    std::string payload = AUTH_TAG + inputText;
    std::string encryptedText = jenc::encryptJournal(payload, key);
    outFile.write(encryptedText.data(), encryptedText.size());
    outFile.close();
    std::error_code ec;
    if (fs::exists(targetPath)) {
        fs::rename(targetPath, backupPath, ec);
    }
    fs::rename(tempPath, targetPath, ec);
    return !ec;
}

std::optional<std::string> decryptFile(const std::string& inputPath, const std::string& key) {
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile) return std::nullopt;
    std::string buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    std::string decryptedPayload = jenc::decryptJournal(buffer, key);
    if (decryptedPayload.rfind(AUTH_TAG, 0) != 0) {
        return std::nullopt;
    }
    return decryptedPayload.substr(AUTH_TAG.length());
}

std::string getLineAsync() {
    std::string line;
    if (std::getline(std::cin, line)) {
        return line;
    }
    return "";
}

int main() {

#if defined(_WIN32)

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;

    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

#endif

    const std::string langPath = getLanguagePath();

    // FIRST RUN LANGUAGE SETUP
    if (!fs::exists(langPath)) {

        std::cout << CYAN
            << "=====================================\n"
            << " Select Language / Choisir la langue\n"
            << "=====================================\n\n"
            << RESET;

        std::cout << YELLOW
            << "[1] English (United Kingdom)\n"
            << "[2] Français (France)\n\n"
			<< "Hint: You can change this later by deleting the 'journal_lang.cfg' file in your user profile directory.\n"
            << RESET;

        std::cout << GREEN
            << "> "
            << RESET;

        std::string choice;

        std::getline(std::cin, choice);

        if (choice == "2") {
            currentLanguage = Language::French;
        }
        else {
            currentLanguage = Language::English;
        }

        saveLanguage(currentLanguage);

        std::cout << GREEN
            << tr(
                "✅ Language saved successfully!\n\n",
                "✅ Langue enregistrée avec succès !\n\n"
            )
            << RESET;
    }
    else {
        currentLanguage = loadLanguage();
    }

    std::string key;

    const std::string journalPath = getJournalPath();

    bool fileExists = fs::exists(journalPath);

    std::string currentJournalContent = "";

    if (!fileExists) {
        while (true) {
            std::string confirmKey;
            std::cout << YELLOW
                << tr(
                    "🔑 Create a new encryption key: ",
                    "🔑 Créez une nouvelle clé de chiffrement : "
                )
                << RESET;
            std::getline(std::cin, key);
            std::cout << YELLOW
                << tr(
                    "🔄 Confirm your encryption key: ",
                    "🔄 Confirmez votre clé de chiffrement : "
                )
                << RESET;
            std::getline(std::cin, confirmKey);

            if (key.empty()) {
                std::cerr << RED
                    << tr(
                        "⚠️ Error: Key cannot be blank. Try again.\n\n",
                        "⚠️ Erreur : la clé ne peut pas être vide. Réessayez.\n\n"
                    )
                    << RESET;
                continue;
            }
            if (key == confirmKey) {
                std::cout << GREEN
                    << tr(
                        "✅ Key setup verified successfully!\n\n",
                        "✅ Configuration de la clé réussie !\n\n"
                    )
                    << RESET;
                break;
            }
            std::cerr << RED
                << tr(
                    "❌ Error: Keys do not match! Please reset.\n\n",
                    "❌ Erreur : les clés ne correspondent pas ! Réessayez.\n\n"
                )
                << RESET;
        }
    }
    else {
        std::cout << YELLOW
            << tr(
                "🔑 Enter your encryption key (used to decrypt): ",
                "🔑 Entrez votre clé de chiffrement (utilisée pour déchiffrer) : "
            )
            << RESET;
        std::getline(std::cin, key);
        auto decryptedText = decryptFile(journalPath, key);
        if (!decryptedText.has_value()) {
            std::cerr << RED
                << tr(
                    "\n🔒 [SECURITY ERROR]: Invalid encryption key! Access Denied.\n",
                    "\n🔒 [ERREUR DE SÉCURITÉ] : clé invalide ! Accès refusé.\n"
                )
                << RESET;
            return 1;
        }
        currentJournalContent = decryptedText.value();
        std::cout << GREEN
            << tr(
                "\n🔓 File verified successfully. Current content:\n",
                "\n🔓 Fichier vérifié avec succès. Contenu actuel :\n"
            )
            << RESET
            << (currentJournalContent.empty()
                ? tr("[Journal empty]", "[Journal vide]")
                : currentJournalContent) << "\n\n";
    }

    std::cout << CYAN
        << "======================================================================\n"
        << RESET;

    std::cout << CYAN
        << tr(
            "📝 Enter your journal entry below. (Emojis are supported! ✨ 📂 🚀)\n",
            "📝 Entrez votre entrée de journal ci-dessous. (Les emojis sont supportés ! ✨ 📂 🚀)\n"
        )
        << RESET;

    std::cout << CYAN
        << tr(
            "Type ':wq' on a new line and press Enter to save and exit.\n",
            "Tapez ':wq' sur une nouvelle ligne puis appuyez sur Entrée pour sauvegarder et quitter.\n"
        )
        << RESET;

    std::cout << RED
        << tr(
            "⌛ Warning: System will auto-lock after ",
            "⌛ Attention : le système se verrouillera après "
        )
        << INACTIVITY_TIMEOUT_SECONDS
        << tr(
            " seconds of total silence.\n",
            " secondes d'inactivité totale.\n"
        )
        << RESET;

    std::cout << CYAN
        << "======================================================================\n\n"
        << RESET;

    std::string completeNewEntry = "";
    bool timedOut = false;

    while (true) {
        auto futureLine = std::async(std::launch::async, getLineAsync);
        if (futureLine.wait_for(std::chrono::seconds(INACTIVITY_TIMEOUT_SECONDS)) == std::future_status::timeout) {
            timedOut = true;
            break;
        }
        std::string line = futureLine.get();
        if (line == ":wq" || std::cin.eof()) {
            break;
        }
        if (!completeNewEntry.empty()) {
            completeNewEntry += "\n";
        }
        completeNewEntry += line;
    }

    if (timedOut) {
#if defined(_WIN32) 
        std::system("cls");
#else 
        std::system("clear");
#endif 
        std::cerr << RED
            << tr(
                "\n🚨 [SECURITY TIMEOUT]: Program auto-locked due to inactivity. Progress discarded.\n",
                "\n🚨 [TEMPS ÉCOULÉ] : le programme s'est verrouillé automatiquement après inactivité. Progression perdue.\n"
            )
            << RESET;
        return 1;
    }

    if (completeNewEntry.empty()) {
        std::cerr << YELLOW
            << tr(
                "No new inputs detected. Changes discarded.\n",
                "Aucune nouvelle entrée détectée. Modifications annulées.\n"
            )
            << RESET;
        return 0;
    }

    std::string updatedContent = currentJournalContent;
    if (!updatedContent.empty()) updatedContent += "\n";
    updatedContent += getCurrentTimestamp() + completeNewEntry + "\n";

    if (encryptAndSaveWithBackup(updatedContent, journalPath, key)) {
        std::cout << GREEN
            << tr(
                "\n💾 Journal encrypted and saved safely to ",
                "\n💾 Journal chiffré et sauvegardé avec succès dans "
            )
            << journalPath
            << "\n"
            << RESET;
    }
    else {
        std::cerr << RED
            << tr(
                "❌ Critical Error: Failed to write data modifications.\n",
                "❌ Erreur critique : impossible d'enregistrer les modifications.\n"
            )
            << RESET;
    }
    return 0;
}
