CC = gcc
CFLAGS = -O3 -Wall

PROGRAMS = all_in_expectation bm_run_matches dealer example_player tcc_ai_player tcc_ai_player_2

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

all_in_expectation: all_in_expectation.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ all_in_expectation.c game.c rng.c net.c

bm_server: bm_server.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_server.c game.c rng.c net.c

bm_widget: bm_widget.c net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_widget.c net.c

bm_run_matches: bm_run_matches.c net.c net.h
	$(CC) $(CFLAGS) -o $@ bm_run_matches.c net.c

dealer: game.c game.h evalHandTables rng.c rng.h dealer.c net.c net.h
	$(CC) $(CFLAGS) -o $@ game.c rng.c dealer.c net.c

example_player: game.c game.h evalHandTables rng.c rng.h example_player.c net.c net.h
	$(CC) $(CFLAGS) -o $@ game.c rng.c example_player.c net.c

tcc_ai_player: tcc_ai_player.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ tcc_ai_player.c game.c rng.c net.c -L/usr/lib/x86_64-linux-gnu/ -ljson-c

tcc_ai_player_2: tcc_ai_player_2.c game.c game.h rng.c rng.h net.c net.h
	$(CC) $(CFLAGS) -o $@ tcc_ai_player_2.c game.c rng.c net.c -L/usr/lib/x86_64-linux-gnu/ -ljson-c