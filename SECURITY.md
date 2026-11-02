# CodeShelf Security Policy

## 1. Security Architecture & Threat Model

CodeShelf is architected to eliminate entire classes of vulnerabilities by design:

| Threat Category | CodeShelf Defense Architecture |
| :--- | :--- |
| **Remote Code Execution (RCE)** | Snippets are stored strictly as inert UTF-8 strings. The application contains zero interpreters, eval functions, or shell execution APIs. |
| **SQL Injection** | 100% of database interactions use parameterized SQLite prepared statements (`sqlite3_bind_*`). Dynamic string concatenation of SQL queries is strictly prohibited. |
| **Protocol & URL Injection** | External links in snippet records are strictly verified by `PlatformUtils::openSafeUrl`. Only `http://` and `https://` schemes are permitted. Dangerous schemes (`file://`, `javascript:`, `cmd:`, `ms-settings:`) are blocked. |
| **Markdown Injection** | Markdown exports dynamically scale code fences (using quadruple or quintuple backticks) to prevent unclosed fences or arbitrary code-fence escaping. |
| **Path Traversal** | Import and export paths are resolved against canonical absolute filesystem locations and verified to prevent writing to unauthorized locations. |
| **Remote Data Exfiltration** | Zero network sockets or HTTP requests. The application makes no telemetry, update-checking, or background web calls. |

---

## 2. Reporting a Vulnerability

If you discover a security vulnerability or potential exploit within CodeShelf:

1. **Do not create a public GitHub issue**.
2. Email the maintainer directly at: `challaajay777@gmail.com` with the subject line `[SECURITY] CodeShelf Vulnerability Report`.
3. Include:
   - Detailed description of the vulnerability.
   - Steps to reproduce or proof-of-concept snippet.
   - Expected impact.
4. You will receive an acknowledgment within 48 hours, followed by a security fix and advisory release.
