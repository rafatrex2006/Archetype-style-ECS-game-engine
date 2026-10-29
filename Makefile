CC      = cc
CFLAGS  = -I. -Wall -Wextra -g $(shell pkg-config --cflags sdl3 2>/dev/null)
ECS_SRC = $(wildcard data_structures/*.c) $(wildcard metadata/*.c) $(wildcard components/*.c) $(wildcard System_Handler/*.c) $(wildcard systems/*.c)

test: tests/test_ecs.c $(ECS_SRC)
	$(CC) $(CFLAGS) -fsanitize=address,undefined $^ -o test_ecs
	./test_ecs

clean:
	rm -f app test_ecs
