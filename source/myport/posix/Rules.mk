#*******************************
# For unix and linux see POSIX rules in /Makefile.cfg
#*******************
#*******************************
# Darwin, Mac OS X
#*******************
ifeq ($(OS),Darwin)
	MODEST_CLONE_SED_HEADER_COMMAND = find $(INCLUDE_DIR_API) -name "*.h" -exec sed -i '.bak' -E 's/^[ \t]*\#[ \t]*include[ \t]*"([^"]+)"/\#include <\1>/g' {} \;

	MODEST_CFLAGS += -fPIC
	MODEST_CFLAGS += $(PROJECT_OPTIMIZATION_LEVEL) -Wno-unused-variable -Wno-unused-function -std=c99
	LIB_NAME_SUFFIX := .dylib

	MODEST_BUILD_SHARED_AFTER += ln -sf $(call MODEST_LIBRARY_NAME_WITH_VERSION) $(call MODEST_LIBRARY) $(MODEST_UTILS_NEW_LINE)
	MODEST_BUILD_SHARED_AFTER += ln -sf $(call MODEST_LIBRARY_NAME_WITH_VERSION) $(call MODEST_LIBRARY_WITH_VERSION_MAJOR) $(MODEST_UTILS_NEW_LINE)
	MODEST_BUILD_SHARED_AFTER += ln -sf $(call MODEST_LIBRARY_NAME_WITH_VERSION) $(call MODEST_LIBRARY_WITH_VERSION_MAJOR_MINOR) $(MODEST_UTILS_NEW_LINE)
	
	MODEST_BUILD_CLEAN_AFTER += rm -f $(call MODEST_LIBRARY) $(MODEST_UTILS_NEW_LINE)
	MODEST_BUILD_CLEAN_AFTER += rm -f $(call MODEST_LIBRARY_WITH_VERSION_MAJOR) $(MODEST_UTILS_NEW_LINE)
	MODEST_BUILD_CLEAN_AFTER += rm -f $(call MODEST_LIBRARY_WITH_VERSION_MAJOR_MINOR) $(MODEST_UTILS_NEW_LINE)

	# Need set
	MODEST_BUILD_OS := $(OS)
	# this name eq source/myport/<namedir>
	MODEST_PORT_NAME := posix
endif
# end of Darwin, Mac OS X
