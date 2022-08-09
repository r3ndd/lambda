default: lambda.o parser.o lexer.o input.o
	g++ -g lambda.o parser.o lexer.o input.o -o lambda

lambda.o: lambda.cc
	g++ -g -c lambda.cc

parser.o: parser.cc parser.hh
	g++ -g -c parser.cc

lexer.o: lexer.cc lexer.hh
	g++ -g -c lexer.cc

input.o: input.cc input.hh
	g++ -g -c input.cc

clean:
	rm *.o lambda 