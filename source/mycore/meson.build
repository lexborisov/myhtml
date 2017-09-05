subdir('utils')

mycoresources = files ([
	'incoming.c',
	'myosi.c',
	'mystring.c',
	'mythread.c',
	'thread_queue.c',
	'utils.c'
])
mycoreheaders = files ([
	'incoming.h',
	'myosi.h',
	'mystring.h',
	'mythread.h',
	'perf.h',
	'thread_queue.h',
	'utils.h'
])

inc_libmycoreh = include_directories ('.')
inc_libmycoreh_dep = declare_dependency (include_directories : inc_libmycoreh)

# libmycore = static_library('mycore',
# 	mycoresources+mycoreutilssources,
# 	version : PROJECT_VERSION,
# 	pic : true,
# 	dependencies : [inc_headersh_dep],
# 	install : false
# 	)

install_headers(
	mycoreheaders,
	subdir: join_paths (VERSIONED_PROJECT_NAME,'mycore')
	)

