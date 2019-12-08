
cxx ?= clang++
cxxflags = -std=c++17 -Wall -Wextra -Wno-unused-function
dllflags = -fPIC -rdynamic
ldflags =  #-rpath /usr/local/lib --enable-new-dtags

debug ?= 1

ifeq ($(debug), 1)
cppflags = -DDEBUG -DPROTO_DEBUG
cxxflags += -g
#rdynamic for function names in glibc backtrace()
ldflags += -rdynamic
endif

ifeq ($(OS),Windows_NT)
	platform := WINDOWS
    cppflags += -D WIN32
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        cppflags += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            cppflags += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            cppflags += -D IA32
        endif
    endif
else
    uname_s := $(shell uname -s)
    ifeq ($(uname_s),Linux)
		platform := LINUX
        cppflags += -D LINUX
    endif
    ifeq ($(uname_s),Darwin)
		platform := OSX
        cppflags += -D OSX
    endif
    uname_p := $(shell uname -p)
    ifeq ($(uname_p),x86_64)
        cppflags += -D AMD64
    endif
    ifneq ($(filter %86,$(uname_p)),)
        cppflags += -D IA32
    endif
    ifneq ($(filter arm%,$(uname_p)),)
        cppflags += -D ARM
    endif
endif

# todo get rid of that, no mercy for windows people, get unix tools, scrub!
ifeq ($(platform), LINUX)
define makedir
	mkdir -p $(1)
endef
define remove
	rm -f $(1)
endef
define if-file-not-exist
	if [ ! -f "$(1)" ]; then $(2); fi
endef

else ifeq ($(platform), WINDOWS)
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

src_dir := src/proto
obj_dir := obj
ar_obj_dir := obj/ar
bin_dir := bin
lib_dir := lib
test_src_dir := test
vendor_dir := vendor

ifeq ($(platform), LINUX)
	runtime_name = proto-runtime
	library_name = libproto.a
	runtime      = $(bin_dir)/$(runtime_name)
	library      = $(lib_dir)/$(library_name)

	proto_srcs = $(shell find $(src_dir) -name *.cc)
	proto_objs := $(proto_srcs:$(src_dir)/%.cc=$(obj_dir)/%.o)
	proto_ar_objs := \
		$(addprefix $(ar_obj_dir)/,$(subst /,_,$(proto_objs:$(obj_dir)/%.o=%.o)))
	proto_deps := $(proto_objs:.o=.d)

	test_srcs = $(shell find $(test_src_dir)/proto -name *.cc)
	test_objs := $(test_srcs:$(test_src_dir)/proto/%.cc=$(obj_dir)/test/%.o)

	includes := -I src/ -I vendor/ -I vendor/gl3w/include/
	libs := -L./lib -L/usr/lib -lX11 -lGL -lGLX -lGLEW  -ldl
endif


ifdef client_src_dir
	client_srcs = $(shell find $(client_src_dir) -name *.cc)
	client_obj_dir = $(client_src_dir)/obj
	client_objs = $(client_srcs:$(client_src_dir)/%.cc=$(client_obj_dir)/%.o)
endif

# TARGETS

.PHONY: runtime
runtime: $(runtime)

.PHONY: test
test: $(test)

.PHONY: client
client: $(client_objs) $(library)
	$(if $(client_src_dir),,\
	$(error Client sources directory path variable 'client_src_dir' is not set. ))
	$(if $(client_name),,\
	$(error Client dynamic library name 'client_name' is not set. ))
	$(if $(client_src),,\
	$(error No source files in $(client_src_dir) ))

	$(cxx) $(ldflags) $(dllflags) -o $(client_objs) $(library)

$(client_objs): $(client_obj_dir)/%.o: $(client_src_dir)/%.cc
	@echo $@
	@$(call makedir, $(dir $@))
	$(cxx) -c -fPIC $(cxxflags) -MMD -MP $< $(includes) -o $@

$(library): $(proto_objs)
	ar rcs $(library) $(proto_ar_objs) $(ar_obj_dir)/gl3w.o

$(runtime): $(LIBRARY) $(LIBRARY) Makefile
	$(cxx) $(ldflags) -o $@ $(library) $(libs) 

#$(CATCH2GCH): $(VENDORDIR)/catch2/catch.hpp Makefile
##	Precompilation of rather hefty catch2 header
#	$(CXX) -fPIC -D CATCH_CONFIG_MAIN $(CXXFLAGS) -c $< $(INCLUDES) -o $@

$(test): $(proto_objs) $(test_objs)
#	Filtering out entry-point as catch2 provide its own.
# 	Alternatively recompile all proto_objs with -DPROTO_MAIN=0
	$(eval proto_objs_no_entrypoint = $(filter-out %/entry-point.o, $(proto_objs)))
	$(cxx) $(ldflags) -o $@ $(proto_objs_no_entrypoint) $(test_objs) $(libs)

$(proto_objs): $(obj_dir)/%.o: $(src_dir)/%.cc
	@$(call makedir, $(dir $@))
	$(cxx) -fPIC $(cxxflags) $(cppflags) -MMD -MP -c $< $(includes) -o $@
# 	because ar is stupid
	@$(call makedir, $(ar_obj_dir) )
	$(eval ar_obj_name := $(ar_obj_dir)/$(subst /,_,$(@:$(obj_dir)/%.o=%.o)))
	cp $@ $(ar_obj_name)

$(test_objs): $(obj_dir)/test/%.o: $(test_src_dir)/proto/%.cc
	$(call makedir, $(dir $@))
	$(cxx) -fPIC $(cxxflags) $(cppflags) -MMD -MP -c $< $(includes) -o $@

#obj/gl3w.o: vendor/gl3w/src/gl3w.c
#	$(CXX) -fPIC $(CXXFLAGS) -MMD -MP -c $< $(INCLUDES) -o $@
#	cp $@ $(AROBJDIR)/$@

# TODO(kacper): auto downloading this stuff?
# doesnt everybody have this alredy in /usr/lib?
#$(GL_EXT): $(GL_EXT_DIR)/%.h: Makefile 
#	$(call makedir, $(GL_EXT_DIR))
#	$(call if-file-not-exist, $@,						\
#		echo fetching $(notdir $@))
#	$(call if-file-not-exist, $@,						\
#		wget $(GL_EXT_REPO_URL)/$(notdir $@) -O $@)

.PHONY: clean
clean:
	$(call remove, $(runtime) $(proto_objs) $(proto_ar_objs) $(proto_deps) $(TESTOBJS) $(CATCH2GCH) $(OBJDIR)/glfw3.o)

-include $(proto_deps)
