# Makefile for Linux, WSL, MSYS2
AR = ar
CC = clang
CXX = clang++
VALGRIND = valgrind
PKGCONFIG = pkg-config
RM = rm -rf
MKDIR = mkdir -p

NAME     = reim
OUTDIR   = build
SRCDIR   = src
EXAMDIR  = example
TESTDIR  = test
LIBRARY  = ./$(OUTDIR)/lib$(NAME).a
EXAMBIN  = ./$(OUTDIR)/$(NAME)_example
TESTBIN  = ./$(OUTDIR)/$(NAME)_test
PKGS     = sndfile portaudio-2.0
# PKGS     += fftw3
# FFTFLAG  = -DREIM_USE_FFTW3
# MKLPATH  = /opt/intel/compilers_and_libraries/linux/mkl/
# MPIPATH  = /opt/intel/compilers_and_libraries/linux/mpi/intel64/
# MKLINC   = -I$(MKLPATH)include -I$(MPIPATH)include
# MKLLIB   = -L$(MKLPATH)lib/intel64 -fopenmp -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -ldl
# FFTFLAG  = -DREIM_USE_MKL
INCLUDE  = -Iinclude $(MKLINC)
LIBS     = -lm $(shell $(PKGCONFIG) --libs $(PKGS)) $(MKLLIB)
CFLAGS   = -MMD -MP -O3 -Wall -Wextra -std=c99 $(INCLUDE) $(shell $(PKGCONFIG) --cflags $(PKGS)) $(FFTFLAG)
CXXFLAGS = -MMD -MP -O3 -Wall -Wextra -std=c++11 $(INCLUDE) $(shell $(PKGCONFIG) --cflags $(PKGS)) $(FFTFLAG)
LDFLAGS  = $(LIBS)
SRCS     = $(wildcard $(SRCDIR)/*.c)
EXAMSRCS = $(wildcard $(EXAMDIR)/*.c)
TESTSRCS = $(wildcard $(TESTDIR)/*.cc)
OBJS     = $(addprefix $(OUTDIR)/, $(SRCS:.c=.o))
EXAMOBJS = $(addprefix $(OUTDIR)/, $(EXAMSRCS:.c=.o))
TESTOBJS = $(addprefix $(OUTDIR)/, $(TESTSRCS:.cc=.o))

# Command
# export LD_LIBRARY_PATH=$(MKLPATH)lib/intel64:$LD_LIBRARY_PATH

.PHONY: all lib run test memcheck clean

all: $(LIBRARY) $(EXAMBIN) $(TESTBIN)

lib: $(LIBRARY)

run: $(EXAMBIN)
	$(EXAMBIN)

test: $(TESTBIN)
	$(TESTBIN)

memcheck: $(EXAMBIN)
	$(VALGRIND) --leak-check=full --track-origins=yes $(EXAMBIN)

clean:
	$(RM) $(OUTDIR)

# Executable

$(LIBRARY): $(OBJS) | $(OUTDIR)
	$(AR) rcs -o $@ $^

$(EXAMBIN): $(EXAMOBJS) $(LIBRARY) | $(OUTDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TESTBIN): $(TESTOBJS) $(LIBRARY) | $(OUTDIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Rule

$(OUTDIR)/$(SRCDIR)/%.o: $(SRCDIR)/%.c | $(OUTDIR)/$(SRCDIR)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OUTDIR)/$(EXAMDIR)/%.o: $(EXAMDIR)/%.c | $(OUTDIR)/$(EXAMDIR)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OUTDIR)/$(TESTDIR)/%.o: $(TESTDIR)/%.cc | $(OUTDIR)/$(TESTDIR)
	$(CXX) -o $@ -c $< $(CXXFLAGS) -Itest/doctest/doctest

# Directory

$(OUTDIR):
	$(MKDIR) $@

$(OUTDIR)/$(SRCDIR):
	$(MKDIR) $@

$(OUTDIR)/$(EXAMDIR):
	$(MKDIR) $@

$(OUTDIR)/$(TESTDIR):
	$(MKDIR) $@

-include $(OBJS:.o=.d) $(TESTOBJS:.o=.d)
