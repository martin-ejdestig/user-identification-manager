common_deps = [
    glib_dep
]

version_header = vcs_tag(input : 'version.h.in', output : 'version.h')

common_sources = [
    'dbus.h',
    'scoped_silent_log_handler.h',
    'scoped_temp_file.cpp',
    'scoped_temp_file.h',
    version_header
]

common_lib = static_library('common',
    dependencies : common_deps,
    include_directories : private_include_dir,
    sources : common_sources)

common_dep = declare_dependency(link_with : common_lib)

subdir('unit_tests')
