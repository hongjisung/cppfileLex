CC=clang++
OUT_CSV=lx_cpp2csv

all: jsonVer csvVer

jsonVer:
	$(CC) -I"include/" -lclang lx_cpp2json.cpp -o lx_cpp2json

csv:
	$(CC) -I"include/" -lclang lx_cpp2csv.cpp -o ${OUT_CSV}

simpletestcsv: csv
	./$(OUT_CSV) simpletest/tc.cpp simpletest/log.txt

clean:
	rm lx_cpp2json.o lx_cpp2csv.o
