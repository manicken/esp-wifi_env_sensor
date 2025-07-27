CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -D _WIN32
BUILD_DIR = build

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
SRC = \
    main.cpp \
	RPN_tools.cpp \
	HAL_JSON_Manager_WIN.cpp \
	../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.cpp \
	../src/HAL_JSON/HAL_JSON_Operations.cpp \
	../src/HAL_JSON/HAL_JSON_UID.cpp \
	../src/HAL_JSON/HAL_JSON_UID_Path.cpp \
	../src/HAL_JSON/HAL_JSON_Value.cpp \
	../src/Support/CharArrayHelpers.cpp \
	../src/Support/ZeroCopyString.cpp \
	../src/Support/Logger.cpp \
	../src/Support/ConvertHelper.cpp
	
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

test.exe: $(OBJ)
	$(CXX) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) test.exe
