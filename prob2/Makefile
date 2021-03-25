target := part2
objs := part2.o


CC := g++
CFLAGS := -O2 -std=c++11 -pthread

all: $(target)

deps := $(patsubt %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

part2: $(objs)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(DEPFLAGS)

testall: test1 test2 test3 test4

test1:
	./part2 -b 1 -p 5 -c 5 -i 10

test2:
	./part2 -b 4 -p 30 -c 30 -i 10

test3:
	./part2 -b 10 -p 1 -c 10 -i 20
	
test4:
	./part2 -b 1000 -p 20 -c 20 -i 10000

clean:
	rm -rf $(targets) $(objs) $(deps)