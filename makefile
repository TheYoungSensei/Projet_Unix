TARGET = serveur joueur

CC = gcc

normal : $(TARGET)

serveur : serveur.o
	$(CC) serveur.o -o serveur

joueur : joueur.o
	$(CC) joueur.o -o joueur

serveur.o : serveur.h global.h serveur.c
	$(CC) -c serveur.c

joueur.o : joueur.h global.h joueur.c
	$(CC) -c joueur.c
clean :
	$(RM) $(TARGET)

