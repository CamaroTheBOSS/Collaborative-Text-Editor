#include "screen_buffers.h"

class Renderer {
public:
	void render(Document& doc, const ScrollableScreenBuffer& buffer) const;
	void renderCursor(Document& doc, const ScrollableScreenBuffer& buffer, const Cursor& cursor, const int color) const;
	void renderDocument(Document& doc, const ScrollableScreenBuffer& buffer) const;
private:
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
};