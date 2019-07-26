cli_deps = [
    common_dep,
    giomm_dep,
    glibmm_dep,
    uim_dbus_dep
]

cli_sources = [
    'arguments.cpp',
    'arguments.h',
    'main.cpp',
    'run.cpp',
    'run.h'
]

executable('uimcli',
    dependencies : cli_deps,
    include_directories : private_include_dir,
    sources : cli_sources,
    install : true)

subdir('unit_tests')
