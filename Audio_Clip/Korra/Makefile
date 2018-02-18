C = gcc
CFLAGS = -Wall -Werror
SPHINX = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad
make:
			$(CC) $(CFLAGS) contkorra.c $(SPHINX) -o contkorra
			$(CC) $(CFLAGS) korra.c $(SPHINX) -o korra
			$(CC) $(CFLAGS) continuous.c $(SPHINX) -o cunt


clean:
		        rm korra
			rm contkorra
			rm cunt
