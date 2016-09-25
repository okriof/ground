# makefil för linux & liknande

ground : car.cpp car.h ground.cpp ground.h main.cpp tyre.cpp tyre.h camera.h camera.cpp sysdep.h sysdep.cpp shaderutils.h shaderutils.c car.config demodefines.h helpers.h helpers.cpp loadobj.h loadobj.c
	g++ -Wall -O2 -o ground -lglut main.cpp car.cpp ground.cpp tyre.cpp sysdep.cpp camera.cpp shaderutils.c helpers.cpp loadobj.c
	cp ground ~/Skrivbord/
	
	
grounddbg : car.cpp car.h ground.cpp ground.h main.cpp tyre.cpp tyre.h camera.h camera.cpp sysdep.h sysdep.cpp shaderutils.h shaderutils.c car.config demodefines.h helpers.h helpers.cpp loadobj.h loadobj.c
	g++ -pg -Wall -o ground -lglut main.cpp car.cpp ground.cpp tyre.cpp sysdep.cpp camera.cpp shaderutils.c helpers.cpp loadobj.c
	cp ground ~/Skrivbord/
