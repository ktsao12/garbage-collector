garbage_collector: garbage_collector.c main.c 
	gcc -O0 -g garbage_collector.c main.c -o garbage_collector --std=gnu99

debug: garbage_collector.c debug_main.c
	gcc -O0 -g garbage_collector.c debug_main.c -o garbage_collector --std=gnu99
