SOURCE_PATH = src
INCLUDE_PATH = include
BUILD_PATH = build/bin
OBJECT_PATH = build/obj
TEST_PATH = build/test

CXX := gcc -I ${INCLUDE_PATH} $(addprefix -D, $(LOG_LEVEL))
ALL_SOURCE := $(wildcard $(SOURCE_PATH)/**)
ALL_OBJECT := $(addprefix $(OBJECT_PATH)/, $(notdir $(addsuffix .obj, $(basename $(ALL_SOURCE)))))

all: build

$(ALL_OBJECT):$(OBJECT_PATH)/%.obj:$(SOURCE_PATH)/$(notdir %.c)
	$(CXX) -c $^ -o $@

server.run: $(ALL_OBJECT) server.c
	$(CXX) $(ALL_OBJECT) server.c -o $(BUILD_PATH)/$@

client.run: $(ALL_OBJECT) client.c
	$(CXX) $(ALL_OBJECT) client.c -o $(BUILD_PATH)/$@

build: make_path server.run client.run
	@echo Build to $(BUILD_PATH) success!

rebuild: clean build

%.test: %.c make_path $(ALL_OBJECT)
	$(CXX) $(ALL_OBJECT) $< -o $(TEST_PATH)/$(notdir $@)
	@echo Build to $(TEST_PATH) success!

clean:	
	rm -rf $(TEST_PATH)
	rm -rf $(OBJECT_PATH)
	rm -rf $(BUILD_PATH)
	rm -rf build

make_path:
	@mkdir -p $(BUILD_PATH) $(OBJECT_PATH) $(TEST_PATH)
