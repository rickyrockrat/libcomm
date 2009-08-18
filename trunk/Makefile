DEBUG = yes
SYMBOLS = yes
PROFILING = no

CC = g++
SOURCESDIR = src/
OBJSDIR = objs/
HEADERSDIR = $(OBJSDIR)include/
STRUCTS = structs/
OUTDIR = ../bin/
INCLUDEBASEDIR = ../include/
INCLUDEDIR = ../include/libcomm/

SRC = exception.cpp \
			types_utils.cpp \
			serializable.cpp \
			net_message.cpp \
			net_address.cpp \
			net_socket.cpp \
			tcp_socket.cpp \
			udp_socket.cpp \
      file.cpp\
      stream.cpp\
      input_stream.cpp\
      output_stream.cpp\
			serialization_manager.cpp \
			thread.cpp \
      mutex.cpp \
      condition.cpp \
      timer.cpp \
      logger.cpp \
      participant.cpp \
      config_loader.cpp


SRC_STRUCTS = structs/string_serializable.cpp \
							structs/vector_serializable.cpp \
							structs/buffer_serializable.cpp\
              structs/map_serializable.cpp \
              structs/simple_serializable.cpp \
              structs/auto_serializable.cpp \
              structs/null_placeholder.cpp

SRC_FINAL = libcomm_structs.cpp \
						libcomm.cpp

#SRC_FINAL = libcomm.cpp

OBJS = $(SRC:%.cpp=$(OBJSDIR)%.o)
OBJS += $(SRC_STRUCTS:%.cpp=$(OBJSDIR)%.o)
OBJS += $(SRC_FINAL:%.cpp=$(OBJSDIR)%.o)
CCFLAGS := -Wall
CCFLAGS_TEST := $(CCFLAGS)

COMPILE_DISPLAY = objs/comp_disp
CLEAN_HEADERS = objs/includes

ifeq ($(PROFILING), yes)
	PROF_FLAG= -pg
endif

ifeq ($(SYMBOLS), yes)
	SYMBOL_FLAG= -g
endif

ifeq ($(DEBUG), yes)
	CCFLAGS := $(CCFLAGS) $(SYMBOL_FLAG) -ggdb $(PROF_FLAG)
	CCFLAGS_TEST := $(CCFLAGS_TEST) -I$(INCLUDEBASEDIR) $(SYMBOL_FLAG) -ggdb $(PROF_FLAG)
else
	CCFLAGS := $(CCFLAGS) $(SYMBOL_FLAG) -Werror -ansi -O2 $(PROF_FLAG)
	CCFLAGS_TEST := $(CCFLAGS_TEST) -I$(INCLUDEBASEDIR) $(SYMBOL_FLAG) -Werror -ansi -O2 $(PROF_FLAG)
endif

libcomm: $(OUTDIR)libcomm.a

$(OUTDIR)libcomm.a: $(OBJS)
	@tput setaf 6
	@echo "Coping headers..."
	@if [ ! -d $(INCLUDEDIR) ]; then mkdir -p $(INCLUDEDIR); fi
	@cp -R $(HEADERSDIR)src/* $(INCLUDEDIR)
	@echo "Creating library..."
	@tput sgr0
	@if [ ! -d $(OUTDIR) ]; then mkdir $(OUTDIR); fi
	ar rcs $(OUTDIR)libcomm.a $(OBJS)
	@tput setaf 6
	@echo "Library created in" $(OUTDIR)
	@tput sgr0
	@rm $(COMPILE_DISPLAY)
	@tput setaf 2
	@echo "Done libcomm."
	@tput sgr0

$(OBJSDIR)%.o: $(SOURCESDIR)%.cpp
	@if [ ! -d $(OBJSDIR) ]; then mkdir $(OBJSDIR); mkdir $(OBJSDIR)$(STRUCTS); fi
	@if [ ! -d $(HEADERSDIR) ]; then mkdir $(HEADERSDIR) $(HEADERSDIR)src $(HEADERSDIR)src/structs; fi
	@tput setaf 2
	@if [ ! -e $(COMPILE_DISPLAY) ]; then echo "Compiling libcomm..."; touch $(COMPILE_DISPLAY); fi
	@tput sgr0
	$(CC) -c $(CCFLAGS) $(SOURCESDIR)$*.cpp -o $@
	@cp $(SOURCESDIR)$*.h $(HEADERSDIR)$(SOURCESDIR)$*.h
	@echo "$(INCLUDEDIR)$*.h" >> $(CLEAN_HEADERS)

TESTDIR = test/

SRCTEST = test_libcomm_testautoser.cpp \
          test_libcomm.cpp

OBJSTEST= $(SRCTEST:%.cpp=$(OBJSDIR)%.o)

LDFLAGS = -L$(OUTDIR) -lcomm -pthread -lrt

test: $(OUTDIR)test_libcomm

$(OUTDIR)test_libcomm: $(OBJSTEST)
	@tput setaf 6
	@echo "Linking test..."
	@tput sgr0
	@if [ ! -d $(OUTDIR) ]; then mkdir $(OUTDIR); fi
	$(CC) $(CCFLAGS) $(OBJSTEST) -o $@ $(LDFLAGS) 
	@rm -f $(COMPILE_DISPLAY)
	@tput setaf 6
	@echo "Test binary created in" $(OUTDIR)
	@tput sgr0
	@tput setaf 5
	@echo "Done tests."
	@tput sgr0

SRCTEST2 = test-mutex.cpp

OBJSTEST2= $(SRCTEST2:%.cpp=$(OBJSDIR)%.o)

test2: $(OUTDIR)test_mutex

$(OUTDIR)test_mutex: $(OBJSTEST2)
	@tput setaf 6
	@echo "Linking test..."
	@tput sgr0
	@if [ ! -d $(OUTDIR) ]; then mkdir $(OUTDIR); fi
	$(CC) $(CCFLAGS) $(OBJSTEST2) -o $@ $(LDFLAGS) 
	@rm -f $(COMPILE_DISPLAY)
	@tput setaf 6
	@echo "Test binary created in" $(OUTDIR)
	@tput sgr0
	@tput setaf 5
	@echo "Done tests."
	@tput sgr0

$(OBJSDIR)%.o: $(TESTDIR)%.cpp
	@if [ ! -d objs ]; then mkdir objs; fi
	@tput setaf 5
	@if [ ! -e $(COMPILE_DISPLAY) ]; then echo "Compiling tests..."; touch $(COMPILE_DISPLAY); fi
	@tput sgr0
	$(CC) -c $(CCFLAGS_TEST) $(TESTDIR)$*.cpp -o $@

launch_test:
	@tput setaf 4
	@echo -n "Launching test : "
	@tput sgr0
	if [ `which valgrind | wc -l` -eq 1 ];then valgrind --leak-check=full $(OUTDIR)test_libcomm; else $(OUTDIR)test_libcomm; fi

all: libcomm test launch_test

clean:
	@tput setaf 3
	@echo "Cleaning libcomm..."
	@tput sgr0
	if [ -d $(INCLUDEDIR) ]; then if [ -r $(CLEAN_HEADERS) ]; then rm -f `cat $(CLEAN_HEADERS)`; rm -rf $(INCLUDEDIR); fi; if [ `find $(INCLUDEBASEDIR) -type f | wc -l` -eq 0 ];then rmdir $(INCLUDEBASEDIR); fi; fi
	rm -rf $(OBJSDIR)*
	if [ -d $(OBJSDIR) ]; then rmdir $(OBJSDIR); fi
	rm -f $(OUTDIR)libcomm.a
	rm -f $(OUTDIR)test_libcomm
	rm -f $(OUTDIR)test_mutex
	if [ -d $(OUTDIR) ]; then if [ `ls $(OUTDIR) | wc -l` -eq 0 ]; then rmdir $(OUTDIR); fi; fi
	@tput setaf 3
	@echo "Cleaned libcomm."
	@tput sgr0
