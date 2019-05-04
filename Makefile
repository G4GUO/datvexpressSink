CC            = gcc
CXX           = g++

OBJECTS = $(patsubst %.cpp,%.o, $(wildcard *.cpp))

datvexpressSink: $(OBJECTS)
	gcc -o $@ $^ -lpthread -lusb-1.0 -lm -lstdc++

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -f *.o
	rm -f datvexpressSink	

install:
	mkdir -p "/lib/firmware/datvexpress"
	cp datvexpressSink /usr/bin/datvexpressSink
	cp datvexpressraw16.rbf /lib/firmware/datvexpress/datvexpressraw16.rbf
	cp datvexpress16.ihx /lib/firmware/datvexpress/datvexpress16.ihx
	cp 10-datvexpress.rules /etc/udev/rules.d/10-datvexpress.rules
	@echo "======================================"
	@echo "|| You will now have to do a reboot ||"
	@echo "======================================"

