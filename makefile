TARGET = serveur joueur

CC = gcc

normal : $(TARGET)

serveur : serveur.o global.o socket.o
	$(CC) serveur.o global.o socket.o -o serveur

joueur : joueur.o global.o socket.o
	$(CC) joueur.o global.o socket.o -o joueur

serveur.o : serveur.h global.h socket.h serveur.c
	$(CC) -c serveur.c

joueur.o : joueur.h global.h socket.h joueur.c
	$(CC) -c joueur.c

socket.o : global.h socket.h socket.c
	$(CC) -c socket.c

global.o : global.h global.c
	$(CC) -c global.c
	
clean :
	$(RM) $(TARGET)
