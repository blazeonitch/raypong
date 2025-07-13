CC = gcc
VCPKG_ROOT = /home/blaze/vcpkg

# Linux build flags
CFLAGS_LINUX = -std=c11 -Isrc/include -I$(VCPKG_ROOT)/installed/x64-linux/include
LDFLAGS_LINUX = -L$(VCPKG_ROOT)/installed/x64-linux/lib \
  -lraylib -lopus -lvorbis -lvorbisfile -lm -ldl -lpthread -lglfw3 -lm -ldl -lpthread -lGL -lX11

# Windows build flags (mingw)
CC_WIN = x86_64-w64-mingw32-gcc
CFLAGS_WIN = -std=c11 -Isrc/include -I$(VCPKG_ROOT)/installed/x64-mingw-static/include
LDFLAGS_WIN = -L$(VCPKG_ROOT)/installed/x64-mingw-static/lib \
  -lraylib -lvorbis -lvorbisfile -lm -lpthread -lglfw3 -lopengl32 -lgdi32 -lwinmm

TARGET_LINUX = builds/main
TARGET_WIN = builds/wmain.exe

.PHONY: all linux windows clean

all: linux windows

linux: $(TARGET_LINUX)

windows: $(TARGET_WIN)

$(TARGET_LINUX): src/main.c src/menu.c
	$(CC) $(CFLAGS_LINUX) $^ $(LDFLAGS_LINUX) -o $@

$(TARGET_WIN): src/main.c src/menu.c
	$(CC_WIN) $(CFLAGS_WIN) $^ $(LDFLAGS_WIN) -o $@

clean:
	rm -f $(TARGET_LINUX) $(TARGET_WIN)
