INC=/home/nodog/docs/files/DroneJob/noshake_lowest/include

#source file folder
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := out
LIBS_DIR := libs

#our final executable
EXE := $(OUT_DIR)/noshake_lowest

#source files
SRC := $(wildcard $(SRC_DIR)/*.c)

#object files from source files
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude

CROSS_COMPILER=aarch64-linux-android21-clang

#clean is not a real target
.PHONY: clean

clang:
	armv7a-linux-androideabi16-clang accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
clang64: $(EXE)
gcc:
	arm-linux-gnueabi-gcc -march=armv7-a accel_read.c -o accel_read $(INC) -w android-19/libandroid.so
gcc64:
	aarch64-linux-gnu-gcc -march=armv8-a accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
push:
	adb push out/noshake_lowest scripts/copy_libs.sh /system/files
test:
	armv7a-linux-androideabi21-clang -static -march=armv7-a accel_read.c -o accel_read $(INC) -w android-21/libandroid.so
drm64:
	aarch64-linux-gnu-gcc -static -march=armv8-a drm_low.c -o drm_64 $(INC) -w
clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

#$@ is name of target being generated
$(EXE): $(OBJ) | $(BIN_DIR)
	$(CROSS_COMPILER) $^ -o $(BIN_DIR)$@ -w $(LIBS_DIR)/android-21/libandroid.so -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CROSS_COMPILER) $(CPPFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@