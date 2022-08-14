default: lambda.o parser.o lexer.o input.o
	g++ -g -Wall lambda.o parser.o lexer.o input.o -o lambda

lambda.o: lambda.cc
	g++ -g -Wall -c lambda.cc

parser.o: parser.cc parser.hh
	g++ -g -Wall -c parser.cc

lexer.o: lexer.cc lexer.hh
	g++ -g -Wall -c lexer.cc

input.o: input.cc input.hh
	g++ -g -Wall -c input.cc

clean:
	rm *.o lambda 