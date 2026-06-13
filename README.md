# 🔐 Secure Encrypted CLI Journal

A secure bilingual encrypted CLI journal written in C++20.

Features authenticated journal encryption, automatic inactivity locking, backup saves, colorful terminal UI, UTF-8 support, persistent language selection (English/French), SafeEnv integration for secure environment access, and modular encryption/decryption libraries for protected local storage.

---

# ✨ Features

## 🔒 Secure Encryption

* Journal entries are encrypted before being written to disk
* Invalid encryption keys are rejected automatically
* Authentication tag verification prevents corrupted or fake data

## 🌍 Bilingual Interface

* English 🇬🇧
* French 🇫🇷

Language preference is saved automatically on first launch.

## 🎨 Modern Colourful Terminal UI

* ANSI coloured terminal output
* UTF-8 emoji support
* Windows terminal VT processing support

## ⌛ Automatic Security Lock

The application automatically locks after inactivity:

```cpp
constexpr int INACTIVITY_TIMEOUT_SECONDS = 60;
```

Unsaved progress is discarded to protect sensitive data.

## 💾 Transactional File Safety

The journal system uses:

* `.tmp` transaction files
* `.bak` automatic backups

to reduce corruption risk during saves.

## 🛡️ Safe Environment Handling

Uses the SafeEnv library for secure environment variable access.

## 📂 Automatic Storage

Files are stored inside the user's profile directory.

Example:

```txt
C:\Users\<User>\journal.dat
C:\Users\<User>\journal_lang.cfg
```

---

# 📦 Included Libraries

## SafeEnv

Provides secure environment variable access:

```cpp
get_env_safe("USERPROFILE");
```

## Decryption / Encryption Library

Handles encrypted journal storage:

```cpp
jenc::encryptJournal(...)
jenc::decryptJournal(...)
```

---

# 🧠 How It Works

1. User creates an encryption key
2. Journal entries are written locally
3. Entries are encrypted before saving
4. Journal is restored only with the correct key
5. Automatic timeout locks the session after inactivity

---

# 🖥️ Example Interface

```txt
🔑 Create a new encryption key:
🔄 Confirm your encryption key:

📝 Enter your journal entry below.
Type ':wq' to save and exit.
```

---

# ⚙️ Requirements

* C++20 compiler
* Windows Terminal / ANSI-compatible terminal
* SafeEnv library
* Encryption/Decryption library

---

# 🔨 Build Example (MSVC)

```bash
cl /std:c++20 main.cpp
```

---

# 📁 Generated Files

| File               | Purpose                    |
| ------------------ | -------------------------- |
| `journal.dat`      | Encrypted journal storage  |
| `journal.dat.bak`  | Automatic backup           |
| `journal.dat.tmp`  | Temporary transaction save |
| `journal_lang.cfg` | Saved language preference  |

---

# 🚀 Planned Features

* Additional language support
* Password masking
* Secure memory wiping
* Search system
* Journal export/import
* Linux/macOS support improvements
* Optional GUI version

---

# 📜 License

This project is open-source and available under your chosen license.

---

# 🔐 Security Notes

This project is designed for local encrypted storage and privacy-focused journaling.

Always:

* Keep your encryption key safe
* Do not forget your key
* Store backups securely

Lost keys cannot decrypt journal contents.

---
