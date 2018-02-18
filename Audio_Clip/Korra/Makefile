C = gcc
CFLAGS = -Wall -Werror
SPHINX = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad
make:
			$(CC) $(CFLAGS) gpio.c korra.c $(SPHINX) -o korra

clean:
			rm korra
