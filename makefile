COMPILER = gcc -g
FILESYSTEM_FILES_CLIENT = client.c parser.c map.c
FILESYSTEM_FILES_SERVER = server.c

build: 
	$(COMPILER) $(FILESYSTEM_FILES_CLIENT) -o out_client `pkg-config fuse --cflags --libs`
	$(COMPILER) $(FILESYSTEM_FILES_SERVER) -o out_server # pkg-config fuse --cflags --libs
	@echo 'run client: ./out_client -f -s [config file]'
	@echo 'run server: ./out_server [server port storagedir]'

clean:
	rm out_server
	rm out_client