C = gcc
CFLAGS = -Wall -Werror
SPHINX = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad

main:
	        $(CC) $(CFLAGS) korra.c $(SPHINX) -o korra
			C -o stringsplitter stringsplitter.c

clean:
	        rm korra
