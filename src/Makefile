CXX=g++
CXXFLAGS=-std=c++17 -g -I../include
LDFLAGS=`sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lSDL2_mixer

SOURCES = main.cpp Game.cpp ScreenHome.cpp ScreenLevelSelect.cpp ScreenLevel1.cpp ScreenLevel2.cpp ScreenLevel3.cpp Button.cpp ScreenNameInput.cpp ScreenHelp.cpp ScreenCredits.cpp ScreenHighestScore.cpp

OBJECTS = $(SOURCES:.cpp=.o)
TARGET = precision_kick

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)