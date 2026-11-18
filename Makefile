NAME    = diorama
CC      = gcc

SRC_DIR = src
OBJ_DIR = obj

SRC     = $(SRC_DIR)/main.c \
          $(SRC_DIR)/core/camera.c \
          $(SRC_DIR)/core/settings.c \
          $(SRC_DIR)/world/blocktype.c \
          $(SRC_DIR)/world/level.c \
          $(SRC_DIR)/world/movement.c \
          $(SRC_DIR)/world/platform.c \
          $(SRC_DIR)/render/block_render.c \
          $(SRC_DIR)/render/lighting.c \
          $(SRC_DIR)/render/vegetation.c \
          $(SRC_DIR)/render/sky.c \
          $(SRC_DIR)/render/backdrop.c \
          $(SRC_DIR)/entities/player.c \
          $(SRC_DIR)/entities/crank.c \
          $(SRC_DIR)/ui/ui.c \
          $(SRC_DIR)/ui/settings_menu.c \
          $(SRC_DIR)/game/game.c \
          $(SRC_DIR)/game/game_draw.c
OBJ     = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP     = $(OBJ:.o=.d)

ifeq ($(OS),Windows_NT)
# Windows: MSYS2 UCRT64 (pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make)
EXE     = .exe
INCS    =
LDFLAGS = -lraylib -lglfw3 -lopengl32 -lgdi32 -lwinmm -lm
else
# Linux / X11: raylib installed in /usr/local
EXE     =
INCS    = -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

BIN     = $(NAME)$(EXE)
CFLAGS  = -Wall -Wextra -std=c99 -g -I$(SRC_DIR) $(INCS) -MMD -MP

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

TEST_SRC = tests/test_movement.c $(SRC_DIR)/core/settings.c $(SRC_DIR)/world/level.c $(SRC_DIR)/world/movement.c $(SRC_DIR)/world/platform.c \
           $(SRC_DIR)/world/blocktype.c $(SRC_DIR)/render/block_render.c $(SRC_DIR)/render/lighting.c

test:
	$(CC) -Wall -Wextra -std=c99 -g -I$(SRC_DIR) $(INCS) $(TEST_SRC) -o test_movement$(EXE) $(LDFLAGS)
	./test_movement$(EXE)
	rm -f test_movement$(EXE)

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(BIN)

re: fclean all

-include $(DEP)

.PHONY: all run test clean fclean re
