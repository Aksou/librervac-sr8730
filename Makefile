# LibreRVAC sr8730 — firmware alternatif pour Samsung Navibot SR8730
#
# Cibles :
#   make            build le firmware pour le PIC32MX360F256L (xc32-gcc)
#   make test       compile et exécute les tests host (gcc)
#   make lint       analyse statique (cppcheck)
#   make package    archive le firmware (.hex/.elf) dans dist/
#   make clean      nettoie les artefacts de build
#
# Variables surchargeables :
#   XC32_DIR   chemin vers l'installation XC32 (défaut: /opt/microchip/xc32/current)
#   CC_HOST    compilateur hôte pour les tests (défaut: gcc)
#   CPPCHECK   binaire cppcheck (défaut: cppcheck)

# ---------------------------------------------------------------------------
# Configuration cible
# ---------------------------------------------------------------------------

DEVICE      := 32MX360F256L
XC32_DIR    ?= /opt/microchip/xc32/current
XC32_BIN    := $(XC32_DIR)/bin
CC_TARGET   := $(XC32_BIN)/xc32-gcc
OBJCOPY     := $(XC32_BIN)/xc32-bin2hex

# Drapeaux de compilation cible (hérités du projet MPLAB X d'origine :
# la plib legacy exige -fgnu89-inline)
TARGET_CFLAGS := -G1024 -g -O1 -fgnu89-inline -mno-float -Wall \
                 -mprocessor=$(DEVICE) -I. -Icordlib -Ijsmn
TARGET_LDFLAGS := -mprocessor=$(DEVICE) -Wl,--defsym=__MPLAB_BUILD=1,--script="p$(DEVICE).ld",-Map=dist/firmware.map,--report-mem

# Sources : tout le .c du dépôt racine + cordlib + jsmn
SRCS_TARGET := $(wildcard *.c) $(wildcard cordlib/*.c) jsmn/jsmn.c
OBJS_TARGET := $(patsubst %.c,build/target/%.o,$(SRCS_TARGET))

ELF := dist/firmware.elf
HEX := dist/firmware.hex

# ---------------------------------------------------------------------------
# Configuration hôte (tests + lint)
# ---------------------------------------------------------------------------

CC_HOST     ?= gcc
CPPCHECK    ?= cppcheck
HOST_CFLAGS := -std=gnu99 -Wall -Wextra -g \
               -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare \
               -Itests/stubs -I. -Icordlib

# Bibliothèque cordlib compilée sur l'hôte (pour les tests de protocole)
CORDLIB_SRCS := cordlib/cord_protocol.c cordlib/cord_connection.c \
                cordlib/cord_hw.c cordlib/cord_motors.c cordlib/cord_wheels.c \
                cordlib/cord_sound.c cordlib/cord_bumper.c
STUBS_SRC    := tests/stubs/sim_regs.c

TESTS := test_motors test_battery test_charging test_protocol test_bumper test_uart
TEST_BINS := $(patsubst %,build/host/%,$(TESTS))

# Sources firmware nécessaires par test (en plus des stubs + cordlib)
test_motors_DEPS    := motors.c cordlib/cord_motors.c cordlib/cord_hw.c
test_battery_DEPS   := battery.c charging.c motors.c \
                       cordlib/cord_hw.c cordlib/cord_sound.c cordlib/cord_motors.c
test_charging_DEPS  := charging.c battery.c motors.c \
                       cordlib/cord_hw.c cordlib/cord_sound.c cordlib/cord_motors.c
test_protocol_DEPS  := $(CORDLIB_SRCS) jsmn/jsmn.c
test_bumper_DEPS    := bumper.c $(CORDLIB_SRCS) jsmn/jsmn.c
test_uart_DEPS      := uart.c cordlib/cord_hw.c cordlib/cord_connection.c \
                       cordlib/cord_sound.c cordlib/cord_protocol.c \
                       cordlib/cord_motors.c cordlib/cord_wheels.c \
                       cordlib/cord_bumper.c jsmn/jsmn.c

# ---------------------------------------------------------------------------
# Cibles principales
# ---------------------------------------------------------------------------

.PHONY: all test lint package clean

all: $(HEX)

# --- Build cible (XC32) ---

$(ELF): $(OBJS_TARGET) | dist
	$(CC_TARGET) $(TARGET_LDFLAGS) -o $@ $(OBJS_TARGET)

$(HEX): $(ELF) | dist
	$(OBJCOPY) $(ELF)

build/target/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC_TARGET) $(TARGET_CFLAGS) -c -o $@ $<

# --- Tests host ---

test: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do \
	    echo "=== $$t ==="; \
	    $$t || failed=1; \
	done; \
	if [ $$failed -ne 0 ]; then echo "TESTS FAILED"; exit 1; fi; \
	echo "ALL TESTS PASSED"

build/host/%: tests/%.c $(STUBS_SRC) | build/host
	$(CC_HOST) $(HOST_CFLAGS) -o $@ $< $(STUBS_SRC) $($*_DEPS)

# --- Lint ---

lint:
	$(CPPCHECK) --enable=warning,performance,portability \
	    --inline-suppr --std=c99 --platform=unix32 \
	    --suppress=missingIncludeSystem \
	    --suppress=unusedFunction \
	    --suppress=normalCheckLevelMaxBranches \
	    -Itests/stubs -I. -Icordlib -Ijsmn \
	    --error-exitcode=1 \
	    *.c cordlib/*.c jsmn/jsmn.c tests/*.c tests/stubs/*.c

# --- Package ---

package: $(HEX)
	@mkdir -p dist
	tar -czf dist/librervac-sr8730-firmware.tar.gz -C dist firmware.hex firmware.elf firmware.map 2>/dev/null || \
	tar -czf dist/librervac-sr8730-firmware.tar.gz -C dist firmware.hex firmware.elf
	@ls -la dist/
	@echo "Package: dist/librervac-sr8730-firmware.tar.gz"

# --- Nettoyage ---

clean:
	rm -rf build dist

# --- Répertoires ---

dist build/host:
	mkdir -p $@
