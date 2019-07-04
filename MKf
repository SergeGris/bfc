
all: main examples

main:
	cd src/ && $(MAKE) -B
	cd ../

examples:
	cd examples && $(MAKE) -B
	cd ../

clean:
	rm ./bfc && rm ./bft
