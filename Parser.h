#include"GUI.h"
class Parser {
	public:
		Parser(const char* cmd);
		void execute(LogoGUI *gui);
		int repcount;
	private:
		const char *text;
		int nextToken();	
		double nextNumber();
		double nextNumber(const char *text);
		double nextNumber(const char *text, int *len);
		char* nextCmdList();
};
