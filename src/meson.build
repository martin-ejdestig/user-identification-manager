private_include_dir = include_directories('.')

config_data = configuration_data()
config_data.set('sysconfdir', join_paths(get_option('prefix'), get_option('sysconfdir')))
config_data.set10('msd_id_source', get_option('msd_id_source'))
config_data.set10('scard_id_source', get_option('scard_id_source'))

config_header = configure_file(configuration : config_data,
    input : 'config.h.in',
    output : 'config.h')

subdir('common')
subdir('generated')

subdir('cli')
subdir('daemon')
