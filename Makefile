
all: bin/namebo

clean:
	rm -rf bin || true
	rm -rf obj || true

bin/namebo: obj/namebo.o
	mkdir bin && g++ -o $@ $<

obj/%.o: %.cc
	mkdir obj && g++ -o $@ -c $<

