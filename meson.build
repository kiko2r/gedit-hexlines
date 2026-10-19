project('gedit-hexlines', 'c',
  version: '1.0',
  default_options: ['warning_level=2'],
)

add_project_arguments('-Wno-unused-parameter', language: 'c')

gedit_dep = dependency('gedit')
plugindir = get_option('libdir') / 'gedit' / 'plugins'

shared_module('hexlines', 'hexlines.c',
  dependencies: gedit_dep,
  install: true,
  install_dir: plugindir,
)

install_data('hexlines.plugin', install_dir: plugindir)
