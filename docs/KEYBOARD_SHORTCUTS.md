# CodeShelf Keyboard Shortcuts Reference

CodeShelf is engineered with a search-first, keyboard-accessible workflow. Most major functions can be executed directly without reaching for a mouse.

---

## 1. Global & Application Shortcuts

| Shortcut (Windows / Linux) | Action | Scope |
| :--- | :--- | :--- |
| <kbd>Ctrl</kbd> + <kbd>N</kbd> | Create New Snippet | Global |
| <kbd>Ctrl</kbd> + <kbd>F</kbd> | Focus Search Bar | Global |
| <kbd>Esc</kbd> | Clear Search / Reset Filter | Global |
| <kbd>Ctrl</kbd> + <kbd>,</kbd> | Open Preferences / Settings | Global |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd> | Exit Application | Global |

---

## 2. Snippet Editing & Management

| Shortcut (Windows / Linux) | Action | Scope |
| :--- | :--- | :--- |
| <kbd>Ctrl</kbd> + <kbd>S</kbd> | Save Changes | Editor / Detail Pane |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> | Copy Entire Snippet Code | Global / List / Detail |
| <kbd>Ctrl</kbd> + <kbd>D</kbd> | Duplicate Snippet | Detail Pane |
| <kbd>Ctrl</kbd> + <kbd>H</kbd> | View Revision History & Diffs | Detail Pane |
| <kbd>Delete</kbd> | Soft Delete / Move to Trash | Snippet List |
| <kbd>Ctrl</kbd> + <kbd>Z</kbd> | Undo Edit | Code Editor |
| <kbd>Ctrl</kbd> + <kbd>Y</kbd> / <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Z</kbd> | Redo Edit | Code Editor |
| <kbd>Ctrl</kbd> + <kbd>A</kbd> | Select All Code | Code Editor |

---

## 3. Advanced Tools & Dialogs

| Shortcut (Windows / Linux) | Action | Scope |
| :--- | :--- | :--- |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>D</kbd> | Open Duplicate Detection Scanner | Global |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>T</kbd> | Open Code Templates Manager | Global |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd> | Open Backup & Restore Manager | Global |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>M</kbd> | Open Database Maintenance Tool | Global |

---

## 4. Search Filter Operators (Inline Keyboard Syntax)

Search supports direct power-user syntax in the search bar (<kbd>Ctrl</kbd> + <kbd>F</kbd>):

| Syntax Example | Filter Semantics |
| :--- | :--- |
| `lang:cpp` | Only snippets where language is C++ |
| `tag:algorithms` | Snippets containing the tag `algorithms` |
| `collection:"Web APIs"` | Snippets within collection `"Web APIs"` |
| `type:sql` | Snippets of type SQL (`code`, `note`, `command`, `config`, `sql`, `pseudocode`) |
| `is:favorite` | Only favorited / starred snippets |
| `is:pinned` | Only pinned snippets |
| `is:trash` | Only items currently residing in Trash |
| `"exact phrase"` | Literal multi-word phrase search |
