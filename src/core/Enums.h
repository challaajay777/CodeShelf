#pragma once

#include <QString>
#include <QStringList>

namespace CodeShelf {

enum class SnippetType {
    Code,
    TechnicalNote,
    CommandReference,
    ConfigurationExample,
    SqlQuery,
    Pseudocode
};

inline QString snippetTypeToString(SnippetType type) {
    switch (type) {
        case SnippetType::Code: return QStringLiteral("Code");
        case SnippetType::TechnicalNote: return QStringLiteral("Technical Note");
        case SnippetType::CommandReference: return QStringLiteral("Command Reference");
        case SnippetType::ConfigurationExample: return QStringLiteral("Configuration Example");
        case SnippetType::SqlQuery: return QStringLiteral("SQL");
        case SnippetType::Pseudocode: return QStringLiteral("Pseudocode");
    }
    return QStringLiteral("Code");
}

inline SnippetType stringToSnippetType(const QString& str) {
    const QString lower = str.trimmed().toLower();
    if (lower == QStringLiteral("technical note") || lower == QStringLiteral("note")) {
        return SnippetType::TechnicalNote;
    }
    if (lower == QStringLiteral("command reference") || lower == QStringLiteral("command")) {
        return SnippetType::CommandReference;
    }
    if (lower == QStringLiteral("configuration example") || lower == QStringLiteral("config")) {
        return SnippetType::ConfigurationExample;
    }
    if (lower == QStringLiteral("sql") || lower == QStringLiteral("sql query")) {
        return SnippetType::SqlQuery;
    }
    if (lower == QStringLiteral("pseudocode")) {
        return SnippetType::Pseudocode;
    }
    return SnippetType::Code;
}

inline QStringList allSupportedLanguages() {
    return {
        QStringLiteral("C"),
        QStringLiteral("C++"),
        QStringLiteral("C#"),
        QStringLiteral("Java"),
        QStringLiteral("Python"),
        QStringLiteral("JavaScript"),
        QStringLiteral("TypeScript"),
        QStringLiteral("Rust"),
        QStringLiteral("Go"),
        QStringLiteral("Kotlin"),
        QStringLiteral("Swift"),
        QStringLiteral("Ruby"),
        QStringLiteral("PHP"),
        QStringLiteral("Dart"),
        QStringLiteral("Lua"),
        QStringLiteral("Shell"),
        QStringLiteral("PowerShell"),
        QStringLiteral("SQL"),
        QStringLiteral("HTML"),
        QStringLiteral("CSS"),
        QStringLiteral("JSON"),
        QStringLiteral("YAML"),
        QStringLiteral("TOML"),
        QStringLiteral("XML"),
        QStringLiteral("Markdown"),
        QStringLiteral("Plain Text"),
        QStringLiteral("Other")
    };
}

inline QString normalizeLanguage(const QString& lang) {
    const QString trimmed = lang.trimmed();
    for (const QString& supported : allSupportedLanguages()) {
        if (supported.compare(trimmed, Qt::CaseInsensitive) == 0) {
            return supported;
        }
    }
    const QString lower = trimmed.toLower();
    if (lower == QStringLiteral("cpp") || lower == QStringLiteral("cxx")) return QStringLiteral("C++");
    if (lower == QStringLiteral("cs") || lower == QStringLiteral("csharp")) return QStringLiteral("C#");
    if (lower == QStringLiteral("py")) return QStringLiteral("Python");
    if (lower == QStringLiteral("js")) return QStringLiteral("JavaScript");
    if (lower == QStringLiteral("ts")) return QStringLiteral("TypeScript");
    if (lower == QStringLiteral("rs")) return QStringLiteral("Rust");
    if (lower == QStringLiteral("golang")) return QStringLiteral("Go");
    if (lower == QStringLiteral("kt")) return QStringLiteral("Kotlin");
    if (lower == QStringLiteral("rb")) return QStringLiteral("Ruby");
    if (lower == QStringLiteral("sh") || lower == QStringLiteral("bash") || lower == QStringLiteral("zsh")) return QStringLiteral("Shell");
    if (lower == QStringLiteral("ps1") || lower == QStringLiteral("pwsh")) return QStringLiteral("PowerShell");
    if (lower == QStringLiteral("md")) return QStringLiteral("Markdown");
    if (lower == QStringLiteral("yml")) return QStringLiteral("YAML");
    if (lower == QStringLiteral("txt") || lower == QStringLiteral("text")) return QStringLiteral("Plain Text");

    return trimmed.isEmpty() ? QStringLiteral("Plain Text") : trimmed;
}

// Deterministic heuristic only; user can always override.
inline QString detectLanguageHeuristic(const QString& code) {
    const QString trimmed = code.trimmed();
    if (trimmed.isEmpty()) return QStringLiteral("Plain Text");

    if (trimmed.startsWith(QStringLiteral("#!/bin/bash")) || trimmed.startsWith(QStringLiteral("#!/bin/sh"))) {
        return QStringLiteral("Shell");
    }
    if (trimmed.startsWith(QStringLiteral("#!/usr/bin/env python")) ||
        (trimmed.contains(QStringLiteral("def ")) && trimmed.contains(QStringLiteral("import ")))) {
        return QStringLiteral("Python");
    }
    if (trimmed.contains(QStringLiteral("#include <")) || trimmed.contains(QStringLiteral("std::cout")) || trimmed.contains(QStringLiteral("nullptr"))) {
        return QStringLiteral("C++");
    }
    if ((trimmed.contains(QStringLiteral("SELECT ")) && trimmed.contains(QStringLiteral(" FROM "))) ||
        (trimmed.contains(QStringLiteral("select ")) && trimmed.contains(QStringLiteral(" from ")))) {
        return QStringLiteral("SQL");
    }
    if (trimmed.startsWith(QStringLiteral("<?xml")) || trimmed.startsWith(QStringLiteral("<!DOCTYPE html>")) || trimmed.contains(QStringLiteral("<html"))) {
        return trimmed.contains(QStringLiteral("<html")) ? QStringLiteral("HTML") : QStringLiteral("XML");
    }
    if ((trimmed.startsWith(QStringLiteral("{")) && trimmed.endsWith(QStringLiteral("}"))) ||
        (trimmed.startsWith(QStringLiteral("[")) && trimmed.endsWith(QStringLiteral("]")))) {
        return QStringLiteral("JSON");
    }
    if (trimmed.contains(QStringLiteral("fn main()")) || trimmed.contains(QStringLiteral("let mut "))) {
        return QStringLiteral("Rust");
    }
    if (trimmed.contains(QStringLiteral("package main")) || trimmed.contains(QStringLiteral("func "))) {
        return QStringLiteral("Go");
    }
    if (trimmed.contains(QStringLiteral("public static void main")) || trimmed.contains(QStringLiteral("System.out.println"))) {
        return QStringLiteral("Java");
    }
    if (trimmed.contains(QStringLiteral("function ")) || trimmed.contains(QStringLiteral("const ")) || trimmed.contains(QStringLiteral("console.log"))) {
        return QStringLiteral("JavaScript");
    }

    return QStringLiteral("Plain Text");
}

enum class SortField {
    Title,
    Language,
    CreatedAt,
    UpdatedAt
};

enum class SortDirection {
    Ascending,
    Descending
};

} // namespace CodeShelf
