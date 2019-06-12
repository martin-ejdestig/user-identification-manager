daemon_deps = [
    common_dep,
    giomm_dep,
    glib_dep,
    glibmm_dep,
    uim_dbus_dep
]

daemon_sources = [
    'arguments.cpp',
    'arguments.h',
    'daemon.cpp',
    'daemon.h',
    'dbus_service.cpp',
    'dbus_service.h',
    'id_source.cpp',
    'id_source.h',
]

daemon_main_sources = [
    'main.cpp',
    daemon_sources
]

daemon_exe = executable('user-identification-manager',
    dependencies : daemon_deps,
    include_directories : private_include_dir,
    sources : daemon_main_sources,
    install : true)

subdir('unit_tests')
