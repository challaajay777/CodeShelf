# CodeShelf Privacy Policy & Data Guarantee

**Effective Date:** November 2, 2026

CodeShelf is built on the principle of **radical privacy and unconditional data ownership**.

---

## 1. Zero Network Transmission

CodeShelf is an offline-first desktop application:
- **No Telemetry**: We do not collect crash reports, usage statistics, hardware metrics, or behavioral analytics.
- **No Accounts**: CodeShelf requires no user registration, email address, password, or authentication credentials.
- **No Cloud Services**: CodeShelf does not communicate with any server, cloud database, or AI API. Your code snippets, technical notes, and configurations never leave your physical workstation.
- **No Automatic Updates**: The application does not poll remote servers for version updates in the background.

---

## 2. Local-First Data Storage

- All application data (snippets, collections, tags, revisions, smart views, and templates) is stored in a single local SQLite database file on your computer:
  - **Windows:** `%LOCALAPPDATA%\CodeShelf\codeshelf.db`
  - **Linux:** `~/.local/share/CodeShelf/codeshelf.db`
- You retain complete ownership and physical custody of this file. You may copy, backup, encrypt, or delete it at any time using standard operating system utilities.

---

## 3. Inert Plain Text

Code snippets and technical notes saved in CodeShelf are treated strictly as inert plain text. The application will never index your files for external marketing, search indexing, or remote analysis.
