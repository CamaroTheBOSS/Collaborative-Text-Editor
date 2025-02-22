#include <memory>
#include "window_base.h"
#include "canvas.h"

class Renderer {
public:
	static void addToCanvas(Canvas& canvas, const BaseWindow& window);
private:
	static void addTextToCanvas(Canvas& canvas, const ScrollableScreenBuffer& buffer, const std::vector<std::string>& linesToRender, const COORD& startPos, const COORD& endPos, const int color = Canvas::defaultColor);
	static void addCursorToCanvas(Canvas& canvas, const ScrollableScreenBuffer& buffer, const RenderCursor& cursor, const int color);
	static void addSelectionToCanvas(Canvas& canvas, const std::vector<std::string>& linesToRender, const ScrollableScreenBuffer& buffer, const COORD& cursor, const COORD& anchor, const int color);
};