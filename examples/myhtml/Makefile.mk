myhtml_dirs := .
myhtml_objs := $(call BINARY_UTILS_OBJS,myhtml,$(myhtml_dirs))

myhtml_all: $(myhtml_objs)

myhtml_clean: 
	rm -f $(myhtml_objs)
