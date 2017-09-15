project('myhtml', [ 'c'], version : '0.1.1')

PROJECT_NAME = meson.project_name()
API_VERSION = '0'
PROJECT_VERSION = meson.project_version()
VERSIONED_PROJECT_NAME = PROJECT_NAME+'-'+API_VERSION
CAMEL_CASE_NAME = 'MyHtml'
VERSIONED_CAMEL_CASE_NAME = CAMEL_CASE_NAME +'-'+ API_VERSION

thread_dep = dependency('threads')

inc_rooth = include_directories ('.')
inc_rooth_dep = declare_dependency (include_directories : inc_rooth)

conf = configuration_data()
conf.set('prefix', get_option('prefix'))
conf.set('exec_prefix', get_option('prefix'))
conf.set('libdir', get_option ('libdir'))
conf.set('includedir', get_option ('includedir'))
conf.set('libname', PROJECT_NAME)
conf.set('description', 'HTML parser by https://html.spec.whatwg.org/multipage/')
conf.set('version', PROJECT_VERSION)
conf.set('cflags', '-I${includedir}/@0@-@1@'.format (PROJECT_NAME, API_VERSION))

configure_file(input : 'myhtml.pc.in',
	output : 'myhtml-@0@.pc'.format(API_VERSION),
	configuration : conf,
	install : true,
	install_dir : join_paths (get_option ('libdir'),'pkgconfig')
	)

subdir('source')
