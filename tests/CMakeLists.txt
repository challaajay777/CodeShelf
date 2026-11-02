cmake_minimum_required(VERSION 3.24)

set(TEST_SOURCES
    main_tests.cpp
    unit/test_validation.cpp
    unit/test_search_parser.cpp
    unit/test_content_hash.cpp
    unit/test_diff.cpp
    unit/test_duplicate.cpp
    unit/test_import_export.cpp
    integration/test_database.cpp
    integration/test_migrations.cpp
    integration/test_backup_restore.cpp
    gui/test_gui.cpp
)

add_executable(codeshelf_tests
    ${TEST_SOURCES}
)

target_link_libraries(codeshelf_tests PRIVATE
    CodeShelfCore
    Qt6::Test
    Qt6::Widgets
    Qt6::Sql
    SQLite3::SQLite3
)

add_test(NAME AllCodeShelfTests COMMAND codeshelf_tests)
