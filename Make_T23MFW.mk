T23SRC:=$(T23MFW_ROOT)/src

T23MFW_CC_FILES := $(wildcard $(T23SRC)/*.cc) $(wildcard $(T23SRC)/QuantTA/*.cc)
T23MFW_OBJ_FILES := $(addprefix objs/,$(notdir $(T23MFW_CC_FILES:.cc=.o)))

T23MFW_GCC_FLAGS=-pthread
T23MFW_CC_FLAGS=-std=c++11

ifeq ($(TARGET), RELEASE)
	# @echo "Does RELEASE build"
	# -m32 or -m64
	# 	-O3 or -O2
	T23MFW_OPTIMISE_FLAGS=-O2 -fomit-frame-pointer -march=native -ffast-math \
							-Wuninitialized -fomit-frame-pointer -DNDEBUG \
							-flto

	# -o -Xlinker --stack=268435456
	T23MFW_DEBUG_FLAGS=
	CC=g++
else
	# @echo "Does DEBUG build"
	T23MFW_OPTIMISE_FLAGS=-g -O0
	T23MFW_DEBUG_FLAGS=-pedantic
	CC=clang++     # no can do because of static libs linking with dukascopy..
	#CC=g++
endif

ifeq ($(TARGET_TYPE), DLL)
	T23MFW_OPTIMISE_FLAGS+=
	T23MFW_DEBUG_FLAGS+=
	T23MFW_LINK_SPECS=
else
	T23MFW_OPTIMISE_FLAGS+=
	T23MFW_DEBUG_FLAGS+=
	T23MFW_LINK_SPECS=
endif

T23MFW_COMPILE_FLAGS= $(T23MFW_GCC_FLAGS) $(T23MFW_CC_FLAGS) -W -Wall ${T23MFW_OPTIMISE_FLAGS} ${T23MFW_DEBUG_FLAGS}
T23MFW_COMPILE_FLAGS+= -MMD
T23MFW_CC_COMPILE_FLAGS=$(T23MFW_COMPILE_FLAGS)
T23MFW_LINK_FLAGS=${T23MFW_GCC_FLAGS} ${T23MFW_DEBUG_FLAGS} ${T23MFW_LINK_SPECS} -lutil

# t23m-test: $(OBJ_FILES)
# 	g++ $(LINK_FLAGS) -o $@ $^

# obj/%.o: $(T23SRC)/%.cc
# 	g++ $(T23MFW_CC_COMPILE_FLAGS) -c -o $@ $<

# -include $(T23MFW_OBJ_FILES:.o=.d)