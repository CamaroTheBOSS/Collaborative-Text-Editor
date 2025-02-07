#include "screen_buffers.h"

class Renderer {
public:
	void render(ClientSiteDocument& doc, const ScrollableScreenBuffer& buffer) const;
private:
	void renderText(const ScrollableScreenBuffer& buffer, const std::vector<std::string>& linesToRender, const COORD& startPos, const COORD& endPos) const;
	void renderCursor(const ScrollableScreenBuffer& buffer, const RenderCursor& cursor, const int color) const;
	void renderSelection(const std::vector<std::string>& linesToRender, const ScrollableScreenBuffer& buffer, const COORD& cursor, const COORD& anchor, const int color) const;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
};