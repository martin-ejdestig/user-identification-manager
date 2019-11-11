daemon_unit_tests_deps = [
    daemon_deps,
    gtest_main_dep
]

daemon_unit_tests_sources = [
    'arguments_test.cpp',
    'configuration_test.cpp',
    'id_source_test.cpp',
    'id_sources/mass_storage_device_id_source_test.cpp',
    'idle_queue_test.cpp',
    'string_test.cpp'
]

daemon_unit_tests = executable('daemon-unit_tests',
    dependencies : daemon_unit_tests_deps,
    include_directories : private_include_dir,
    objects : daemon_exe.extract_objects(daemon_sources),
    sources : daemon_unit_tests_sources)

test('daemon unit tests', daemon_unit_tests)
