project('paw_print', 'cpp', default_options : ['cpp_std=c++11'])


add_global_arguments('-std=c++11', language : 'cpp')


inc_dirs = [
    include_directories('../src'),
    include_directories('../external/src'),
    include_directories('../external/src/lalr_parsergen'),
]

link_args = [
	'-L../external/lib',
	'-llalr_parsergen',
]

srcs = run_command('python3', 'find_src.py', 'src').stdout().strip().split('\n')
paw_print_lib = static_library('paw_print', srcs, include_directories : inc_dirs, link_args : link_args)



srcs = ['main.cpp']
executable('test_paw_print', srcs, link_with :paw_print_lib, include_directories : inc_dirs, link_args : link_args)

