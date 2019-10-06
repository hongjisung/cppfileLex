CC=clang++

all: jsonVer csvVer

jsonVer:
	$(CC) -I"include/" -lclang lx_cpp2json.cpp -o lx_cpp2json.o 

csvVer:
	$(CC) -I"include/" -lclang lx_cpp2csv.cpp -o lx_cpp2csv.o

clean:
	rm lx_cpp2json.o lx_cpp2csv.o
