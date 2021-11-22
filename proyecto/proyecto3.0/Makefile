servidor: servidor.o
	@gcc servidor.o -o servidor -lpthread

servidor.o: servidor.c include/systemv.h
	@gcc -c servidor.c -o servidor.o

proceso: proceso.o
	@gcc proceso.o -o proceso

proceso.o: proceso.c include/systemv.h
	@gcc -c proceso.c -o proceso.o

.PHONY: all clean

all: servidor proceso
	@ipcrm -a
	@rm *.o
	@echo "-*Compilación completada*-"

clean:
	@ipcrm -a
	@rm -f shm* sem* servidor proceso
	@echo "-*Limpieza completada*-"
