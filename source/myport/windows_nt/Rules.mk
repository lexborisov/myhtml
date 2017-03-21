#*******************************
# Windows_NT
#*******************
ifeq ($(OS),Windows_NT)
	LIB_NAME_SUFFIX := .dll
	LIB_NAME_SUFFIX_STATIC := .dll.a

	MODEST_LIBRARY_NAME_WITH_VERSION := lib$(LIB_NAME)-$(PROJECT_VERSION_MAJOR)$(LIB_NAME_SUFFIX)

	MODEST_CFLAGS += -Wno-unused-variable -Wno-unused-function -std=c99
    MODEST_LDFLAGS += -Wl,--out-implib,$(call MODEST_LIBRARY_STATIC)

	MODEST_BUILD_SHARED_AFTER += cp $(call MODEST_LIBRARY_WITH_VERSION) $(BINARY_DIR_BASE) $(MODEST_UTILS_NEW_LINE)
	MODEST_BUILD_CLEAN_AFTER += rm -f $(BINARY_DIR_BASE)/$(call MODEST_LIBRARY_NAME_WITH_VERSION) $(MODEST_UTILS_NEW_LINE)

	# Need set
	MODEST_BUILD_OS := $(OS)
	# this name eq source/myport/<namedir>
	MODEST_PORT_NAME := windows_nt
endif
# end of Windows_NT
