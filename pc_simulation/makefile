CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -D _WIN32

SRC = \
    main.cpp \
	HAL_JSON_Manager_WIN.cpp \
	../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.cpp \
	../src/HAL_JSON/HAL_JSON_Operations.cpp \
	../src/HAL_JSON/HAL_JSON_UID_Path.cpp \
	../src/HAL_JSON/HAL_JSON_Value.cpp \
	../src/Support/CharArrayHelpers.cpp \
	../src/Support/Logger.cpp \
	../src/Support/ConvertHelper.cpp
	
OBJ = $(SRC:.cpp=.o)

RuleEngine.exe: $(OBJ)
	$(CXX) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) RuleEngine.exe
