
## ======================================================================
## partial Makefile provided to students
##

GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
GTK_LIBS := `pkg-config --libs gtk+-3.0`

.PHONY: clean new style feedback submit1 submit2 submit

CFLAGS += -std=c11 -Wall -pedantic -g

# a bit more checks if you'd like to (uncomment
CFLAGS += -Wextra -Wfloat-equal -Wshadow                         \
-Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings \
-Wunreachable-code \
#-Wconversion

# uncomment if you want to add DEBUG flag
# CPPFLAGS += -DDEBUG

# ----------------------------------------------------------------------
# feel free to update/modifiy this part as you wish

# all those libs are required on Debian, feel free to adapt it to your box
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

#CPPFLAGS += -DBLARGG

UNIT_TESTS = unit-test-bit unit-test-alu unit-test-bus unit-test-component unit-test-memory unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-cartridge unit-test-timer unit-test-alu_ext unit-test-cpu-dispatch unit-test-old-bit-vector unit-test-bit-vector
TERMINAL_TESTS = test-cpu-week08 test-cpu-week09 test-gameboy test-image gbsimulator
ALL_TESTS = $(UNIT_TESTS) $(TERMINAL_TESTS)
LATEST_TEST = unit-test-alu_ext

all:: $(ALL_TESTS)

TARGETS := $(UNIT_TESTS)
CHECK_TARGETS := $(UNIT_TESTS)
OBJS = 
OBJS_NO_STATIC_TESTS =
OBJS_STATIC_TESTS = 
OBJS = $(OBJS_STATIC_TESTS) $(OBJS_NO_STATIC_TESTS)

# ----------------------------------------------------------------------
# This part is to make your life easier. See handouts how to make use of it.

cleanAll: clean
	-@/bin/rm libsid_demo

clean::
	-@/bin/rm -f *.o *~ $(ALL_TESTS)

new: clean all

static-check:
	scan-build -analyze-headers --status-bugs -maxloop 64 make CC=clang new

style:
	astyle -n -o -A8 -xt0 *.[ch]

# all those libs are required on Debian, adapt to your box
$(CHECK_TARGETS): LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

check:: $(CHECK_TARGETS)
	$(foreach target,$(CHECK_TARGETS), LD_LIBRARY_PATH=. ./$(target) &&) true

# target to run tests
check:: all
	@if ls tests/*.*.sh 1> /dev/null 2>&1; then \
	  for file in tests/*.*.sh; do [ -x $$file ] || echo "Launching $$file"; ./$$file || exit 1; done; \
	fi

IMAGE=chappeli/feedback:latest
feedback:
	@docker pull $(IMAGE)
	@docker run -it --rm -v ${PWD}:/home/tester/done $(IMAGE)

SUBMIT_SCRIPT=../provided/submit.sh
submit1: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 1

submit2: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 2

submit:
	@printf 'what "make submit"??\nIt'\''s either "make submit1" or "make submit2"...\n'

libsid.so: sidlib.c
	$(CC) -fPIC -shared $(CPPFLAGS) $(CFLAGS) $(GTK_INCLUDE) $^ -o $@

libsid_demo.o: CFLAGS += $(GTK_INCLUDE)

libsid_demo: LDFLAGS += -L.
libsid_demo: LDLIBS += $(GTK_LIBS) -lsid
libsid_demo: libsid_demo.o libsid.so

###################################### Our Part #################################
unit-test-bit: unit-test-bit.o bit.o
unit-test-alu: unit-test-alu.o alu.o bit.o error.o
unit-test-bus: unit-test-bus.o bus.o bit.o error.o component.o util.o memory.o
unit-test-component: unit-test-component.o error.o bus.o memory.o component.o bit.o
unit-test-memory: unit-test-memory.o error.o bus.o memory.o component.o bit.o
unit-test-cpu: LDFLAGS += -L.
unit-test-cpu: LDLIBS += -lcs212gbcpuext
unit-test-cpu: unit-test-cpu.o error.o alu.o bit.o \
 util.o cpu.o bus.o memory.o component.o cpu-registers.o cpu-storage.o \
 cpu-alu.o opcode.o
unit-test-cpu-dispatch-week08: LDFLAGS += -L.
unit-test-cpu-dispatch-week08: LDLIBS += -lcs212gbcpuext
unit-test-cpu-dispatch-week08: unit-test-cpu-dispatch-week08.o \
 error.o alu.o bit.o bus.o memory.o component.o opcode.o util.o \
 cpu-alu.o cpu-registers.o cpu-storage.o
test-cpu-week08: LDFLAGS += -L.
test-cpu-week08: LDLIBS += -lcs212gbcpuext
test-cpu-week08: test-cpu-week08.o error.o opcode.o bit.o cpu.o alu.o bus.o \
 memory.o component.o cpu-storage.o util.o cpu-registers.o cpu-alu.o
unit-test-cpu-dispatch-week09: LDFLAGS += -L.
unit-test-cpu-dispatch-week09: LDLIBS += -lcs212gbcpuext
unit-test-cpu-dispatch-week09: unit-test-cpu-dispatch-week09.o error.o \
 alu.o bit.o bus.o memory.o component.o opcode.o util.o \
 cpu-alu.o cpu-registers.o cpu-storage.o
test-cpu-week09: LDFLAGS += -L.
test-cpu-week09: LDLIBS += -lcs212gbcpuext
test-cpu-week09: test-cpu-week09.o opcode.o bit.o alu.o bus.o \
 memory.o component.o cpu-storage.o util.o error.o cpu.o cpu-registers.o \
 cpu-alu.o
unit-test-cartridge: LDFLAGS += -L.
unit-test-cartridge: LDLIBS += -lcs212gbcpuext
unit-test-cartridge: unit-test-cartridge.o error.o cartridge.o \
 component.o memory.o bus.o cpu.o alu.o bit.o cpu-storage.o \
 cpu-registers.o cpu-alu.o opcode.o
unit-test-timer: LDFLAGS += -L.
unit-test-timer: LDLIBS += -lcs212gbcpuext
unit-test-timer: unit-test-timer.o util.o error.o timer.o \
 component.o memory.o bit.o cpu.o alu.o bus.o cpu-storage.o \
 cpu-registers.o cpu-alu.o opcode.o
test-gameboy: LDFLAGS += -L.
test-gameboy: LDLIBS += -lcs212gbfinalext
test-gameboy: test-gameboy.o gameboy.o bus.o memory.o component.o cpu.o \
 alu.o bit.o timer.o cartridge.o util.o error.o cpu-storage.o cpu-registers.o\
 opcode.o bootrom.o cpu-alu.o image.o bit_vector.o
unit-test-alu_ext: LDFLAGS += -L.
unit-test-alu_ext: LDLIBS += -lcs212gbcpuext
unit-test-alu_ext: unit-test-alu_ext.o error.o alu.o bit.o \
 cpu-storage.o cpu-registers.o cpu-alu.o bus.o
unit-test-cpu-dispatch: LDFLAGS += -L.
unit-test-cpu-dispatch: LDLIBS += -lcs212gbcpuext
unit-test-cpu-dispatch: unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o\
 alu.o component.o memory.o bus.o bit.o error.o
unit-test-old-bit-vector: unit-test-old-bit-vector.o error.o \
 bit_vector.o bit.o
unit-test-bit-vector: unit-test-bit-vector.o error.o bit_vector.o \
 bit.o
test-image: LDFLAGS += -L.
test-image: LDLIBS += -lsid $(GTK_LIBS)
test-image: test-image.o error.o util.o image.o bit_vector.o bit.o
gbsimulator: LDFLAGS += -L.
gbsimulator: LDLIBS += -lsid -lcs212gbfinalext $(GTK_LIBS)
gbsimulator: CC += -D_DEFAULT_SOURCE
gbsimulator: gbsimulator.o gameboy.o bus.o memory.o bootrom.o\
 component.o cpu.o alu.o bit.o timer.o cartridge.o cpu-storage.o\
 bit_vector.o error.o cpu-registers.o cpu-alu.o opcode.o image.o



alu.o: alu.c alu.h bit.h error.h ourError.h
bit.o: bit.c bit.h ourError.h error.h
bit_vector.o: bit_vector.c bit_vector.h bit.h util.h image.h ourError.h \
 error.h
bootrom.o: bootrom.c bootrom.h bus.h memory.h component.h gameboy.h cpu.h \
 alu.h bit.h timer.h cartridge.h lcdc.h image.h bit_vector.h joypad.h \
 error.h
bus.o: bus.c bus.h memory.h component.h error.h bit.h
cartridge.o: cartridge.c cartridge.h component.h memory.h bus.h error.h \
 ourError.h
component.o: component.c component.h memory.h error.h
cpu-alu.o: cpu-alu.c error.h bit.h alu.h cpu-alu.h opcode.h cpu.h \
 memory.h bus.h component.h cpu-storage.h cpu-registers.h
cpu.o: cpu.c error.h opcode.h bit.h cpu.h alu.h memory.h bus.h \
 component.h cpu-alu.h cpu-registers.h cpu-storage.h util.h ourError.h
cpu-registers.o: cpu-registers.c cpu-registers.h cpu.h alu.h bit.h \
 memory.h bus.h component.h error.h ourError.h
cpu-storage.o: cpu-storage.c error.h ourError.h cpu-storage.h memory.h \
 opcode.h bit.h cpu.h alu.h bus.h component.h cpu-registers.h gameboy.h \
 timer.h cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
error.o: error.c
gameboy.o: gameboy.c component.h memory.h bus.h error.h gameboy.h cpu.h \
 alu.h bit.h timer.h cartridge.h lcdc.h image.h bit_vector.h joypad.h \
 util.h bootrom.h ourError.h
gbsimulator.o: CFLAGS += $(GTK_INCLUDE)
gbsimulator.o: gbsimulator.c sidlib.h gameboy.h bus.h memory.h \
 component.h cpu.h alu.h bit.h timer.h cartridge.h lcdc.h image.h \
 bit_vector.h joypad.h error.h ourError.h
image.o: image.c error.h image.h bit_vector.h bit.h
libsid_demo.o: libsid_demo.c sidlib.h
memory.o: memory.c memory.h error.h util.h
opcode.o: opcode.c opcode.h bit.h
sidlib.o: sidlib.c sidlib.h
test-cpu-week08.o: test-cpu-week08.c opcode.h bit.h cpu.h alu.h memory.h \
 bus.h component.h cpu-storage.h util.h error.h
test-cpu-week09.o: test-cpu-week09.c opcode.h bit.h cpu.h alu.h memory.h \
 bus.h component.h cpu-storage.h util.h error.h
test-gameboy.o: test-gameboy.c gameboy.h bus.h memory.h component.h cpu.h \
 alu.h bit.h timer.h cartridge.h lcdc.h image.h bit_vector.h joypad.h \
 util.h error.h
test-image.o: CFLAGS += $(GTK_INCLUDE)
test-image.o: test-image.c error.h util.h image.h bit_vector.h bit.h \
 sidlib.h
timer.o: timer.c timer.h cpu.h alu.h bit.h memory.h bus.h component.h \
 error.h cpu-storage.h opcode.h gameboy.h cartridge.h lcdc.h image.h \
 bit_vector.h joypad.h
unit-test-alu.o: unit-test-alu.c tests.h error.h alu.h bit.h
unit-test-alu_ext.o: unit-test-alu_ext.c tests.h error.h alu.h bit.h \
 alu_ext.h
unit-test-bit.o: unit-test-bit.c tests.h error.h bit.h
unit-test-bit-vector.o: unit-test-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h
unit-test-bus.o: unit-test-bus.c tests.h error.h bus.h memory.h \
 component.h util.h
unit-test-cartridge.o: unit-test-cartridge.c tests.h error.h cartridge.h \
 component.h memory.h bus.h cpu.h alu.h bit.h
unit-test-component.o: unit-test-component.c tests.h error.h bus.h \
 memory.h component.h
unit-test-cpu.o: unit-test-cpu.c tests.h error.h alu.h bit.h opcode.h \
 util.h cpu.h memory.h bus.h component.h cpu-registers.h cpu-storage.h \
 cpu-alu.h
unit-test-cpu-dispatch.o: unit-test-cpu-dispatch.c tests.h error.h alu.h \
 bit.h cpu.h memory.h bus.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h \
 ourError.h
unit-test-cpu-dispatch-week08.o: unit-test-cpu-dispatch-week08.c tests.h \
 error.h alu.h bit.h cpu.h memory.h bus.h component.h opcode.h gameboy.h \
 timer.h cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h \
 ourError.h
unit-test-cpu-dispatch-week09.o: unit-test-cpu-dispatch-week09.c tests.h \
 error.h alu.h bit.h cpu.h memory.h bus.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h \
 ourError.h
unit-test-memory.o: unit-test-memory.c tests.h error.h bus.h memory.h \
 component.h
unit-test-old-bit-vector.o: unit-test-old-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h
unit-test-timer.o: unit-test-timer.c util.h tests.h error.h timer.h cpu.h \
 alu.h bit.h memory.h bus.h component.h
util.o: util.c
