inc_headersh = include_directories ('.')
inc_headersh_dep = declare_dependency (include_directories : inc_headersh)

subdir('mycore')
subdir('myencoding')
subdir('myhtml')


inc_libh = include_directories ('.')
inc_libh_dep = declare_dependency (include_directories : inc_libh)

sources = files ([
	'mycore/utils.c',
	'mycore/utils/avl_tree.c',
	'mycore/utils/mchar_async.c',
	'mycore/utils/mcobject_async.c',
	'mycore/utils/mcobject.c',
	'mycore/utils/mcsimple.c',
	'mycore/utils/mcsync.c',
	'mycore/utils/mctree.c',
	'mycore/utils/mhash.c',
	'mycore/incoming.c',
	'mycore/myosi.c',
	'mycore/mystring.c',
	'mycore/mythread.c',
	'mycore/thread_queue.c',
	'myport/posix/mycore/io.c',
	'myport/posix/mycore/memory.c',
	'myport/posix/mycore/perf.c',
	'myport/posix/mycore/thread.c',
	'myport/posix/mycore/utils/mcsync.c',
	'myencoding/detect.c',
	'myencoding/encoding.c',
	'myencoding/mystring.c',
	'myhtml/callback.c',
	'myhtml/charef.c',
	'myhtml/data_process.c',
	'myhtml/myhtml.c',
	'myhtml/mynamespace.c',
	'myhtml/mystring.c',
	'myhtml/parser.c',
	'myhtml/rules.c',
	'myhtml/serialization.c',
	'myhtml/stream.c',
	'myhtml/tag.c',
	'myhtml/tag_init.c',
	'myhtml/token.c',
	'myhtml/tokenizer.c',
	'myhtml/tokenizer_doctype.c',
	'myhtml/tokenizer_end.c',
	'myhtml/tokenizer_script.c',
	'myhtml/tree.c',
])


libmyhtml = library('myhtml',
	sources,
	version : PROJECT_VERSION,
	dependencies : [inc_libh_dep, inc_headersh_dep, thread_dep],
	install : true,
	install_dir : [
		true,
	])


