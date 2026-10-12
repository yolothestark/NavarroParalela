# Makefile para Practica 1.6: Algoritmo de Fuerza Bruta Distribuido con MPI y OpenMP
# Centro Universitario de Tonala (CUTonala) - UDG
# Alumnos: Anguiano Garcia Angel Yahir Guadalupe, Figueroa Robles Axel Israel

CXX = g++
CXXFLAGS = -O3 -fopenmp -Wall
MSMPI_INC = $(MSMPI_INC)
MSMPI_LIB64 = $(MSMPI_LIB64)

INCLUDES = -I "$(MSMPI_INC)"
LDFLAGS = -L "$(MSMPI_LIB64)" -lmsmpi

TARGET = Practica1_6.exe
SRCS = FuerzaBruta.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.txt

.PHONY: all clean
