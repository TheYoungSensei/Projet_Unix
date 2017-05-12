TARGET = serveur joueur

CC = gcc

normal : $(TARGET)

serveur : serveur.o ipc.o socket.o
	$(CC) serveur.o socket.o ipc.o -o serveur

joueur : joueur.o socket.o ipc.o
	$(CC) joueur.o socket.o ipc.o -o joueur

serveur.o : serveur.h global.h ipc.h socket.h serveur.c
	$(CC) -c serveur.c

joueur.o : joueur.h global.h ipc.h socket.h joueur.c
	$(CC) -c joueur.c

ipc.o : global.h ipc.h ipc.c
	$(CC) -c ipc.c

socket.o : global.h socket.h socket.c
	$(CC) -c socket.c

clean :
	$(RM) $(TARGET)
