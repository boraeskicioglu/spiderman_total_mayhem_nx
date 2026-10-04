# Ultimate Spider-Man: Total Mayhem HD -- Nintendo Switch wrapper (AArch32)
TARGET               := spiderman_tm_nx
PORT_NPDM_PROGRAM_ID := 0x01000000000010F0
include runtime/runtime.mk

.PHONY: imports
imports:
	python3 runtime/tools/gen_imports.py --libs .apk_libs

# Stage 1H: stb_vorbis is compiled separately; tm_audio.c includes its API
# in HEADER_ONLY mode.
$(BUILD)/stb_vorbis.o: CFLAGS += -DSTB_VORBIS_NO_PUSHDATA_API -DSTB_VORBIS_MAX_CHANNELS=2 \
	-Wno-unused-function -Wno-unused-variable -Wno-sign-compare

