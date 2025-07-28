CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -D _WIN32 -D _WIN32_WINNT=0x0A00
#LDFLAGS = -s
BUILD_DIR = build

# Header include path for ArduinoJson
INCLUDE_DIRS = \
	../.pio/libdeps/esp32dev/ArduinoJson/src \
	./stubs/Arduino \
	./cpp-httplib \
	./stubs/ESPAsyncWebServer \
	./stubs/Ticker \
	./stubs/LittleFS \
	./stubs/DHTesp \
	./stubs/OneWire \
	./stubs/DallasTemperature

INCLUDES = $(foreach dir,$(INCLUDE_DIRS),-I$(dir))

# Source files
SRC = \
	main.cpp \
	RPN_tools.cpp \
	commandLoop.cpp \
	./stubs/Arduino/Arduino.cpp \
	./stubs/ESPAsyncWebServer/ESPAsyncWebServer.cpp \
	./stubs/Ticker/Ticker.cpp \
	./stubs/LittleFS/LittleFS.cpp \
	./stubs/LittleFS/LittleFS_ext.cpp \
	./stubs/DHTesp/DHTesp.cpp \
	./stubs/OneWire/OneWire.cpp \
	./stubs/DallasTemperature/DallasTemperature.cpp \
	../src/Drivers/REGO600.cpp \
	../src/Drivers/RF433.cpp \
	$(wildcard ../src/HAL_JSON/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/OneWireTemp/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/REGO600/*.cpp) \
	$(wildcard ../src/HAL_JSON/Devices/RF433/*.cpp) \
	$(wildcard ../src/HAL_JSON/RuleEngine/*.cpp) \
	../src/Support/CharArrayHelpers.cpp \
	../src/Support/ZeroCopyString.cpp \
	../src/Support/Logger.cpp \
	../src/Support/ConvertHelper.cpp

# Files to exclude (webserver-related)
EXCLUDE_SRC = \
	../src/HAL_JSON/HAL_JSON_GPIO_Manager_REST.cpp \
	../src/HAL_JSON/HAL_JSON.cpp
	
# Remove excluded files from SRC
SRC := $(filter-out $(EXCLUDE_SRC), $(SRC))


# Convert each .cpp into a .o file in the build directory
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# Main executable
test.exe: $(OBJ)
	$(CXX) $(OBJ) $(INCLUDES) $(LDFLAGS) -lws2_32 -o $@

# Rule to compile each .cpp to .o in the build directory
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean target
.PHONY: clean
clean:
	rm -f $(OBJ) test.exe
