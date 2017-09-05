
myencodingsources = files ([
	'detect.c',
	'encoding.c',
	'mystring.c'
])
myencodingheaders = files ([
	'detect_resource.h',
	'encoding.h',
	'myosi.h',
	'mystring.h',
	'resource.h'
])

inc_libmyencodingh = include_directories ('.')
inc_libmyencodingh_dep = declare_dependency (include_directories : inc_libmyencodingh)

# libmyencoding = static_library('myencoding',
# 	myencodingsources,
# 	version : PROJECT_VERSION,
# 	link_with : libmycore,
# 	dependencies : [inc_headersh_dep],
# 	install : false
# 	)

install_headers(
	myencodingheaders,
	subdir: join_paths (VERSIONED_PROJECT_NAME,'myencoding')
	)

