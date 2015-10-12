# ---------------------------------------------------------------------
#      Local configuration
# ---------------------------------------------------------------------

# don't forget to run
# $ use diam
# $ usediam cplex
# in your shell before using this makefile.

## Cheated Makefile

## Putting OS related tag
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
ILOG_ROOT=/Applications/CPLEX_Studio1261
ILOG_SYSTEM=x86-64_osx
ILOG_LIBFORMAT=static_pic
endif

CPLEX_DIR            = $(ILOG_ROOT)/cplex
CONCERT_DIR          = $(ILOG_ROOT)/concert
CPLEX_LIB_DIR        = $(CPLEX_DIR)/lib/$(ILOG_SYSTEM)/$(ILOG_LIBFORMAT)
CONCERT_LIB_DIR      = $(CONCERT_DIR)/lib/$(ILOG_SYSTEM)/$(ILOG_LIBFORMAT)
CPLEX_INCLUDE_DIR    = $(CPLEX_DIR)/include
CONCERT_INCLUDE_DIR  = $(CONCERT_DIR)/include


#  Add -DNDEBUG to those if you need it.

CPLEX_CXX_CFLAGS     = -I$(CPLEX_INCLUDE_DIR) -I$(CONCERT_INCLUDE_DIR) \
                       -m64 -fPIC -fno-strict-aliasing \
                       -fexceptions -O -DIL_STD

CPLEX_C_CFLAGS       = -I$(CPLEX_INCLUDE_DIR) \
                       -m64 -fPIC -fno-strict-aliasing 

CPLEX_CXX_LDDIRS     = -L$(CPLEX_LIB_DIR) -L$(CONCERT_LIB_DIR)
CPLEX_CXX_LDFLAGS    = -lconcert -lilocplex -lcplex -lm
CPLEX_C_LDDIRS       = -L$(CPLEX_LIB_DIR)
CPLEX_C_LDFLAGS      = -lcplex -lm

ifeq ($(UNAME), Linux)
CPLEX_C_LDFLAGS   += -pthread
CPLEX_CXX_LDFLAGS += -pthread
endif


# ---------------------------------------------------------------------

#SRC    := $(wildcard src/*.c)

#C_SRC    := src/data.c  src/solution.c
#C_OBJ    := $(patsubst %.c,   %.o, $(C_SRC))
CXX_SRC  := src/data.cpp src/solution.cpp src/solveur.cpp src/main.cpp
CXX_OBJ  := $(patsubst %.cpp, %.o, $(CXX_SRC))
OBJ      := $(CXX_OBJ) #$(C_OBJ) 

CCXX     := g++
# CC       := gcc
# CFLAGS   += -Wall -ansi -pedantic -g 
CXXFLAGS += -Wall $(CPLEX_CXX_CFLAGS) -g #-DNVERBOSE
LDFLAGS  += $(CPLEX_CXX_LDFLAGS) -g

ifeq ($(UNAME), Darwin)
CXXFLAGS += -stdlib=libstdc++
LDFLAGS  += -stdlib=libstdc++
endif

LDDIRS   += $(CPLEX_CXX_LDDIRS)
LINK     := g++

#$@ is the name of the file to be made.
#$? is the names of the changed dependents.
#$< the name of the related file that caused the action.
#$* the prefix shared by target and dependent files.

gotic: $(OBJ)
	$(LINK) $(LDDIRS) -o $@ $^ $(LDFLAGS) -lm


%.o: %.cpp
	$(CCXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJ)
