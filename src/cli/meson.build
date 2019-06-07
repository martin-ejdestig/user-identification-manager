cli_deps = [
    common_dep,
    giomm_dep,
    glibmm_dep,
    uim_dbus_dep
]

cli_sources = [
    'main.cpp'
]

executable('uimcli',
    dependencies : cli_deps,
    include_directories : private_include_dir,
    sources : cli_sources,
    install : true)
