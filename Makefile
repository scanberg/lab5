SGCT_INCLUDES=-I../sgct_0_9_5/include/ 
SGCT_LIBS=-L/usr/lib/ -L../sgct_0_9_5/lib/ -lrt -lGL -lGLU -lX11 -lXrandr -static-libgcc -static-libstdc++ -lglfw -lz -lGLEW -lsgct

all: main
	./main -config single.xml

main: DrawableObject.o ObjLoader.o Tokenizer.o Entity3D.o
	g++ main.cpp  DrawableObject.o ObjLoader.o Tokenizer.o Entity3D.o -o main $(SGCT_INCLUDES) $(SGCT_LIBS)

DrawableObject.o: ObjLoader.o

ObjLoader.o: Tokenizer.o

%.o: %.cpp %.h
	g++ -c $< -o $@ $(SGCT_INCLUDES) $(SGCT_LIBS) $(OSG_LIBS)


