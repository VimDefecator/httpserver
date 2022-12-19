CXX := clang++
CXXFLAGS := -std=c++20
CPPFLAGS := -MMD -MP

TUNITS := test serverloop tcplistener http html
SRCS := $(foreach name,$(TUNITS),src/$(name).cc)
OBJS := $(foreach name,$(TUNITS),build/$(name).o)
DEPS := $(OBJS:.o=.d)

test: $(OBJS)
	$(CXX) -o test $(OBJS)

$(OBJS): build/%.o: src/%.cc build/.dir
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/.dir:
	mkdir -p build
	touch build/.dir

.PHONY: clean
clean:
	rm -rf build

-include $(DEPS)
