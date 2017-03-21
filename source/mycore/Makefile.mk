mycore_dirs := . utils
mycore_objs := $(call MODEST_UTILS_OBJS,mycore,$(mycore_dirs))

mycore_description := base module, it is used by all other modules
mycore_dependencies :=

mycore_all: $(mycore_objs)

mycore_clean: 
	rm -f $(mycore_objs)

mycore_clone: 
	$(call MODEST_UTILS_HDRS_CLONE,mycore,$(mycore_dirs))
