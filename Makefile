TARGET   = levenshtein
BUILDDIR = .pio/build/make
OBJS     = $(BUILDDIR)/distance.o $(BUILDDIR)/levenshtein.o
CXXFLAGS = -Wall -Wno-sign-compare -O2

default: $(TARGET)

test: $(TARGET)
	./$(TARGET) -c \( -C \) -g -r 10 -d -b -S d -T 3 -f src/levenshtein.cpp 'prefix(levenshtein)suffix' 2>&1

clean:
	$(RM) -v $(OBJS) $(BUILDDIR)/$(TARGET).unstripped $(TARGET)
	rmdir $(BUILDDIR)

install: $(TARGET)
	sudo install --compare --owner=root --group=root --mode=0755 $(TARGET) /usr/local/bin

$(BUILDDIR):
	mkdir -pv $(BUILDDIR)

$(TARGET): $(BUILDDIR)/$(TARGET).unstripped
	strip $< --strip-all -o $@

$(BUILDDIR)/$(TARGET).unstripped: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: src/%.cpp src/%.h $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<
