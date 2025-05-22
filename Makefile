# ==== Compiler and flags ====
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Isrc -Ilibs/SDL2/include
LDFLAGS = -Llibs/SDL2/lib -lmingw32 -lSDL2main -lSDL2

# ==== Files ====
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = bin/output.exe

# ==== Build rules ====

all: $(TARGET)

$(TARGET): $(SRC)
	@echo Compiling...
	$(CXX) $(SRC) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo Copying SDL2.dll...
	cp libs/SDL2/SDL2.dll bin/
	@echo Build complete: $(TARGET)

# ==== Clean ====

clean:
	del /Q bin\*.exe 2>nul
	del /Q src\*.o 2>nul
	@echo Cleaned build files.
