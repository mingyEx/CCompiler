#include "Parser.h"

using namespace CoreLib::Basic;

namespace CoreLib
{
	namespace Text
	{
		
		Parser::Parser(String text)
		{
			MetaLexer lexer;
			lexer.SetLexProfile(
				L"#WhiteSpace = {\\s+}\n"\
				L"#SingleLineComment = {//[^\\n]*\\n}\n"\
				L"#MultiLineComment = {/\\*([^*]|\\*[^/])*\\*/}\n"\
				L"Identifier = {[a-zA-Z_]\\w*}\n"\
				L"IntConstant = {\\d+}\n"\
				L"FloatConstant = {\\d*.\\d+|\\d+(.\\d+)?(e(-)?\\d+)?}\n"\
				L"StringConstant = {\"([^\\\\\"]|\\\\\\.)*\"}\n"\
				L"CharConstant = {'[^\\n\\r]*'}\n"\
				L"LParent = {\\(}\n"\
				L"RParent = {\\)}\n"\
				L"LBrace = {{}\n"\
				L"RBrace = {}}\n"\
				L"LBracket = {\\[}\n"\
				L"RBracket = {\\]}\n"\
				L"Dot = {.}\n"\
				L"Semicolon = {;}\n"\
				L"Comma = {,}\n"\
				L"OpAdd = {\\+}\n"\
				L"OpSub = {-}\n"\
				L"OpDiv = {/}\n"\
				L"OpMul = {\\*}\n"\
				L"OpMod = {%}\n"\
				L"OpExp = {^}\n"\
				L"OpGreater = {>}\n"\
				L"OpLess = {<}\n"\
				L"OpEqual = {==}\n"\
				L"OpGEqual = {>=}\n"\
				L"OpLEqual = {<=}\n"\
				L"OpNEqual = {!=}\n"\
				L"OpAnd = {&}\n"\
				L"OpOr = {\\|}\n"\
				L"OpNot = {!}\n"\
				L"OpAssign = {=}\n");
			legal = lexer.Parse(text, stream);
			curNode = stream.FirstNode();
		}
	}
}