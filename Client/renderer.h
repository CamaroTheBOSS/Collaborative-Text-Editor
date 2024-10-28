#include <vector>

#include "Windows.h"
#include "document.h"

using ScreenBuffer = CONSOLE_SCREEN_BUFFER_INFO;
using DocRange = std::pair<int, int>;
using TextLines = std::vector<std::string>;

class Renderer {
public:
	void render(Document& doc, const ScreenBuffer& screenBuffer) const;
	void renderCursor(Document& doc, const COORD& docCursor, const ScreenBuffer& screenBuffer, const int color) const;
	void renderDocument(Document& doc, const ScreenBuffer& screenBuffer) const;
	COORD synchronizeCursor(Document& doc, const COORD& docCursor, const ScreenBuffer& screenBuffer) const;
private:
	TextLines getTextToRender(Document& doc, const ScreenBuffer& screenBuffer) const;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
};