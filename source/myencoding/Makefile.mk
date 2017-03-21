myencoding_dirs := .
myencoding_objs := $(call MODEST_UTILS_OBJS,myencoding,$(myencoding_dirs))

myencoding_description := work with character encodings, detecting encoding, convert encodings by https://encoding.spec.whatwg.org/
myencoding_dependencies := mycore myport

myencoding_all: $(myencoding_objs)

myencoding_clean: 
	rm -f $(myencoding_objs)

myencoding_clone: 
	$(call MODEST_UTILS_HDRS_CLONE,myencoding,$(myencoding_dirs))
