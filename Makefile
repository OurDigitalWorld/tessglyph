CXX=g++
CFLAGS=-I/usr/local/include/leptonica -I/usr/local/include/tesseract -I/usr/include/libxml2
LIBS=-llept -ltesseract -lxml2
OBJ = tessglyph.o

%.o: %.cpp
	$(CXX) -std=c++11 -c -o $@ $< $(CFLAGS) 

tessglyph: $(OBJ)
	$(CXX)  -std=c++11 -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o tessglyph

