_DEPS = Cpu.h Chip8.h Video.h
_OBJ = main.o Cpu.o Chip8.o Video.o
TARGET = chip8

IDIR = ./include
SDIR = ./source
ODIR = ./obj

CXX = g++
CXX_FLAGS = -Wall -I$(IDIR)
LIBS = -lSDL2

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o : $(SDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXX_FLAGS)

$(TARGET) : $(OBJ)
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
