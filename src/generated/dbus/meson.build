gdbus_codegen = find_program('gdbus-codegen-glibmm3')

gdbus_codegen_deps = [
    giomm_dep,
    glibmm_dep
]

interfaces_dir = join_paths(meson.source_root(), 'data')
dbus_build_dir = meson.current_build_dir()

uim_dbus_sources = custom_target('UserIdentificationManager D-Bus source',
    command : [ gdbus_codegen, '--generate-cpp-code=' + join_paths(dbus_build_dir, 'user_identification_manager'), '@INPUT@' ],
    input : join_paths(interfaces_dir, 'com.luxoft.UserIdentificationManager.xml'),
    output : [
        'user_identification_manager_common.cpp',
        'user_identification_manager_common.h',
        'user_identification_manager_proxy.cpp',
        'user_identification_manager_proxy.h',
        'user_identification_manager_stub.cpp',
        'user_identification_manager_stub.h'
    ])

uim_dbus_headers = [ uim_dbus_sources[1], uim_dbus_sources[3], uim_dbus_sources[5] ]

uim_dbus_lib = static_library('user_identification_manager_dbus',
    sources : uim_dbus_sources,
    dependencies : gdbus_codegen_deps)

uim_dbus_dep = declare_dependency(link_with : uim_dbus_lib, sources : uim_dbus_headers)
