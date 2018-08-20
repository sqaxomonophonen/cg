OCCT_INC=-I/Users/andersstraadt/builds/occt-HEAD-47ba172/build/include/opencascade
OCCT_LINK=-L/Users/andersstraadt/builds/occt-HEAD-47ba172/build/mac64/clang/lib -lTKernel -lTKPrim -lTKMesh -lTKBin -lTKMath -lTKGeomBase -lTKG3d -lTKG2d -lTKTopAlgo -lTKBRep -lTKBO -lTKFillet -lTKOffset

all: test

cg.so: cg.cc cg.h
	clang++ -std=c++11 -Wall -shared -fPIC ${OCCT_INC} $< -o $@ ${OCCT_LINK}

test: test.cc cg.so
	clang++ -std=c++11 -Wall $< -o $@ cg.so

clean:
	rm -f *.so test
