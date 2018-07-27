project('paw_print', 'cpp', default_options : ['cpp_std=c++11'])


add_global_arguments('-std=c++11', language : 'cpp')


inc_dirs = []

srcs = run_command('python3', 'find_src.py', 'src').stdout().strip().split('\n')
paw_print_lib = static_library('paw_print', srcs)


srcs = ['main.cpp']
executable('paw_print_test', srcs, link_with :paw_print_lib, include_directories : inc_dirs)

