#include"GUI.h"
#include"ParserState.h"
#include<vector>

#ifndef _PARSER_H_
#define _PARSER_H_

class Parser {
	public:
		Parser(const char* cmd, ParserState *parserState);
		bool getSpeed();
		void execute(LogoGUI *gui);
		int repcount;
	private:
		const char *text;
		int nextToken();	
		double nextNumber();
		double nextNumber(const char *text);
		double nextNumber(const char *text, int *len);
		char* nextCmdList();
		char* nextString();
		bool isStrNext();
		void skipFunc(const char **text);
		double getFunc(const char *text);
		double getFuncParam(const char **text);
		int numFuncParam(const char *text);
		ParserState *parserState;
};

#endif
