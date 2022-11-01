SIM_SRC  = rat.cpp rob.cpp pipeline.cpp sim.cpp exeq.cpp
SIM_OBJS = $(SIM_SRC:.cpp=.o)

all: $(SIM_SRC) sim

%.o: %.cpp
	g++ -std=c++14 -Wall -c -o $@ $< -g

sim: $(SIM_OBJS)
	g++ -std=c++14 -Wall -o $@ $^ -g

clean:
	rm sim *.o
