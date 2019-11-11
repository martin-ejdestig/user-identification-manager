daemon_deps = [
    common_dep,
    giomm_dep,
    glib_dep,
    glibmm_dep,
    threads_dep,
    uim_dbus_dep
]

daemon_sources = [
    'arguments.cpp',
    'arguments.h',
    'configuration.cpp',
    'configuration.h',
    'daemon.cpp',
    'daemon.h',
    'dbus_service.cpp',
    'dbus_service.h',
    'id_source.cpp',
    'id_source.h',
    'id_sources/mass_storage_device_id_source.cpp',
    'id_sources/mass_storage_device_id_source.h',
    'idle_queue.h',
    'string.cpp',
    'string.h'
]

if get_option('scard_id_source')
    daemon_deps += libpcsclite_dep
    daemon_sources += [
        'pcsc_context.cpp',
        'pcsc_context.h',
        'id_sources/smart_card_id_source.cpp',
        'id_sources/smart_card_id_source.h'
    ]
endif

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
