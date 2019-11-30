CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Wno-unused-function -DPROTO_DEBUG 

ifeq ($(OS),Windows_NT)
	PLATFORM := WINDOWS
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		PLATFORM := LINUX
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
		PLATFORM := OSX
        CCFLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif


ifeq ($(PLATFORM), LINUX)
INCLUDES := -I src/ -I vendor/ -I src/tools/gl3w/include/
LIBS := -L/usr/lib -lX11 -lGL -lGLX -lGLEW  -ldl
LDFLAGS =  #-rpath /usr/local/lib --enable-new-dtags
else ifeq ($(PLATFORM), WINDOWS)
INCLUDES := -I src/  -I include
LDFLAGS := -LC:/lib -lkernel32 -luser32 -lgdi32 -lopengl32
endif

ifeq ($(PLATFORM), LINUX)
define makedir
	mkdir -p $(1)
endef
define remove
	rm -f $(1)
endef
define if-file-not-exist
	if [ ! -f "$(1)" ]; then $(2); fi
endef

else ifeq ($(PLATFORM), WINDOWS)
define winpath
	$(subst /,\\,$(1))
endef
define unixpath
	$(subst \,/,$(subst \\,/,$(1)))
endef
define makedir
	$(eval ARG := $(call winpath, $(patsubst %/,%,$(1))))
	if not exist $(ARG) mkdir $(ARG)
endef
define remove
	del /Q /F $(call winpath, $(1)) 2> nul
endef
define if-file-exist
	if exist $(call winpath,$(1)) ($(2))
endef
define if-file-not-exist
	if not exist $(call winpath,$(1)) ($(2))
endef

endif

GL_EXT_HEADERS := wglext.h wgl.h glext.h glxext.h glcorearb.h
GL_EXT_DIR := include/gl
GL_EXT = $(GL_EXT_HEADERS:%.h=$(GL_EXT_DIR)/%.h)
GL_EXT_REPO_URL := https://www.khronos.org/registry/OpenGL/api/GL

SRCDIR := src/proto
TESTSRCDIR := test

OBJDIR := obj
VENDORDIR := vendor
AROBJDIR := obj/ar
BINDIR := bin
LIBDIR := lib
TESTDIR := test

LIBS += -L $(LIBDIR)

#todo linux find regex
ifeq ($(PLATFORM), LINUX)
	SRCS = $(shell find $(SRCDIR) -name *.cc)
	TESTSRCS = $(shell find $(TESTSRCDIR)/proto -name *.cc)
	OBJS := $(SRCS:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)
	TESTOBJS := $(TESTSRCS:$(TESTSRCDIR)/proto/%.cc=$(OBJDIR)/test/%.o)
	AROBJS := $(addprefix $(AROBJDIR)/,$(subst /,_,$(OBJS:$(OBJDIR)/%.o=%.o)))
else ifeq ($(PLATFORM), WINDOWS)
	SRCS = 													\
		$(subst $(realpath .),.,							\
		$(subst \,/,										\
		$(shell dir /S /B /A:A $(call winpath, $(SRCDIR)) | \
				findstr /I /R \\[a-zA-Z0-9_-]*\.cc$$ )))
endif


DEPS := $(OBJS:.o=.d)


ifeq ($(PLATFORM), LINUX)
RUNTIME_NAME = proto-runtime
LIBRARY_NAME = libproto.a
TEST_NAME = proto-test
RUNTIME = $(BINDIR)/$(RUNTIME_NAME)
LIBRARY = $(LIBDIR)/$(LIBRARY_NAME)
TEST = $(BINDIR)/proto-test
CATCH2GCH = $(VENDORDIR)/catch2/catch.hpp.gch
else ifeq ($(PLATFORM), WINDOWS)
RUNTIME_NAME = proto-runtime.exe
LIBRARY_NAME = proto.lib
RUNTIME = $(BINDIR) \$(RUNTIME_NAME)
LIBRARY = $(LIBDIR)\$(LIBRARY_NAME)
endif

.PHONY: all
all: runtime

.PHONY: runtime
runtime: $(RUNTIME)

.PHONY: test
test: $(TEST)

.PHONY: demo
demo:
	$(eval OBJS_CLIENT = $(filter-out %-runtime.o, $(OBJS)))
	$(CXX) -fPIC -std=c++17 -c -o src/demos/test/test.o \
    src/demos/test/test.cc -I /shared/projects/proto/src $(INCLUDES)
	$(CXX) -shared -o src/demos/test/libtest.so \
	src/demos/test/test.o $(OBJS_CLIENT)

#-rdynamic is for function names in stacktrace, debug only
$(RUNTIME): $(LIBRARY) Makefile $(LIBRARY) demo
	$(CXX) -g -rdynamic -o $@ $(LIBRARY) $(LIBS) $(LDFLAGS) 

$(LIBRARY): $(OBJS) Makefile obj/gl3w.o
	ar rcs $(LIBRARY) $(AROBJS) $(AROBJDIR)/gl3w.o

$(CATCH2GCH): $(VENDORDIR)/catch2/catch.hpp Makefile
#	Precompilation of rather hefty catch2 header
	$(CXX) -fPIC -D CATCH_CONFIG_MAIN $(CXXFLAGS) -c $< $(INCLUDES) -o $@

$(TEST): $(OBJS) $(TESTOBJS)
#	Filtering out entry-point as catch2 provide its own.
# 	Alternatively recompile all OBJS with -D PROTO_MAIN=0
	$(eval OBJS_NOENTRYPOINT = $(filter-out %/entry-point.o, $(OBJS)))
	$(CXX) -rdynamic -o $@ $(OBJS_NOENTRYPOINT) $(TESTOBJS) $(LIBS) $(LDFLAGS) 

obj/gl3w.o: vendor/gl3w/src/gl3w.c
	$(CXX) -fPIC $(CXXFLAGS) -MMD -MP -c $< $(INCLUDES) -o $@
	cp $@ $(AROBJDIR)/$@

$(GL_EXT): $(GL_EXT_DIR)/%.h: Makefile 
	$(call makedir, $(GL_EXT_DIR))
	$(call if-file-not-exist, $@,						\
		echo fetching $(notdir $@))
	$(call if-file-not-exist, $@,						\
		wget $(GL_EXT_REPO_URL)/$(notdir $@) -O $@)

$(TESTOBJS): $(OBJDIR)/test/%.o: $(TESTSRCDIR)/proto/%.cc
	$(call makedir, $(dir $@))
	$(CXX) -fPIC $(CXXFLAGS) -MMD -MP -c $< $(INCLUDES) -o $@

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@$(call makedir, $(dir $@))
	$(CXX) -fPIC $(CXXFLAGS) -MMD -MP -c $< $(INCLUDES) -o $@
	@$(call makedir, $(AROBJDIR) )
	$(eval AROBJNAME := $(AROBJDIR)/$(subst /,_,$(@:$(OBJDIR)/%.o=%.o)))
	cp $@ $(AROBJNAME)

$(OBJDIR)/glfw3.o: ./src/tools/gl3w/src/gl3w.c
	gcc -MMD -MP -c $< $(INCLUDES) -o $@


.PHONY: clean
clean:
	$(call remove, $(RUNTIME) $(OBJS) $(AROBJS) $(DEPS) $(TESTOBJS) $(CATCH2GCH) $(OBJDIR)/glfw3.o)


-include $(DEPS)
