#include"GUI.h"
#include"ParserState.h"
#include<vector>

#ifndef _PARSER_H_
#define _PARSER_H_

class Parser {
	public:
		Parser(const char* cmd, ParserState *parserState, LogoGUI *gui);
		bool getSpeed();
		void execute();
		int repcount;
	private:
		void outError(const char *msg);
		const char *text;
		int nextToken();	
		double nextNumber();
		double nextNumber(const char *text);
		double nextNumber(const char *text, int *len);
		double nextNumber(const char **text);
		char* nextCmdList(const char **text);
		char* nextCmdList();
		char* nextString(const char **text);
		char* nextString();
		char* getStringFunction(const char **text);
		bool isStrNext();
		void skipFunc(const char **text);
		double getFunc(const char *text);
		double getFuncParam(const char **text);
		int numFuncParam(const char *text);
		ParserState *parserState;
		bool isSpace(char c);
		bool expectString(const char *text);
		bool expectNumber(const char *text);
		bool expectCmdList(const char *text);
		bool expectToken(const char *text, const char *expect);
		char *lastProc;
		LogoGUI *gui;
};

#endif
