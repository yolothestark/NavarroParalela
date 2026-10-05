# Makefile para Práctica 1.3: Reduce en MPI con OpenMP

CXX = mpicxx
CXXFLAGS = -fopenmp -O3 -Wall -std=c++11

TARGET = practica1_3
SRCS = main.cpp OperacionesArreglos.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) log_equipo_*.txt

run_local: $(TARGET)
	mpirun -np 5 ./$(TARGET)
