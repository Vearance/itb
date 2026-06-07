.PHONY: build run test clean all diagram

build:
	mvn clean package

run:
	mvn javafx:run

test:
	mvn test

clean:
	mvn clean

all: build run

diagram:
	mkdir -p docs/uml
	java tools/GenerateDiagram.java src/main/java docs/uml/class-diagram.puml
