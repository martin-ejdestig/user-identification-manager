common_unit_tests_deps = [
    common_dep,
    gtest_main_dep
]

common_unit_tests_sources = [
    'scoped_temp_file_test.cpp'
]

common_unit_tests = executable('common-unit_tests',
    dependencies : common_unit_tests_deps,
    include_directories : private_include_dir,
    sources : common_unit_tests_sources)

test('libcommon unit tests', common_unit_tests)
