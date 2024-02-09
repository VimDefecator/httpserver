CXX := clang++
CXXFLAGS := -std=c++20
CPPFLAGS := -MMD -MP
LDFLAGS := -lpthread

define TARGET_template =
 SRCS_$(1) := $$(foreach name,$(2),src/$$(name).cc)
 OBJS_$(1) := $$(foreach name,$(2),build/$$(name).o)
 build/$(1): $$(OBJS_$(1))
	$(CXX) $(LDFLAGS) -o build/$(1) $$(OBJS_$(1))
endef

ALLUNITS := test imgbrd serverloop tcplistener taskpool http html args

TARGETS := test
UNITS_test := test imgbrd serverloop tcplistener taskpool http html args

all: $(TARGETS:%=build/%)

$(foreach target,$(TARGETS),$(eval $(call TARGET_template,$(target),$(UNITS_$(target)))))

$(ALLUNITS:%=build/%.o): build/%.o: src/%.cc build/.dir
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/.dir:
	mkdir -p build
	touch build/.dir

.PHONY: clean
clean:
	rm -rf build

-include $(ALLUNITS:%=build/%.d)
