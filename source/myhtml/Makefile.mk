myhtml_dirs := .
myhtml_objs := $(call MODEST_UTILS_OBJS,myhtml,$(myhtml_dirs))

myhtml_description := HTML parser by https://html.spec.whatwg.org/multipage/
myhtml_dependencies := mycore myencoding  myport

myhtml_all: $(myhtml_objs)

myhtml_clean: 
	rm -f $(myhtml_objs)

myhtml_clone: 
	$(call MODEST_UTILS_HDRS_CLONE,myhtml,$(myhtml_dirs))
