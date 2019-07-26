cli_unit_tests_deps = [
    cli_deps,
    gtest_main_dep
]

cli_unit_tests_sources = [
    'arguments_test.cpp'
]

cli_unit_tests = executable('cli-unit_tests',
    dependencies : cli_unit_tests_deps,
    include_directories : private_include_dir,
    objects : cli_exe.extract_objects(cli_sources),
    sources : cli_unit_tests_sources)

test('cli unit tests', cli_unit_tests)
