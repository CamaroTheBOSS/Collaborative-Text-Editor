#include <memory>
#include "window_base.h"
#include "canvas.h"

class Renderer {
public:
	void addToBuffer(const std::unique_ptr<BaseWindow>& window);
	void resize(const COORD& size);
	void clear();
	void render() const;
private:
	void addTextToBuffer(const ScrollableScreenBuffer& buffer, const std::vector<std::string>& linesToRender, const COORD& startPos, const COORD& endPos, const int color = Canvas::defaultColor);
	void addCursorToBuffer(const ScrollableScreenBuffer& buffer, const RenderCursor& cursor, const int color);
	void addSelectionToBuffer(const std::vector<std::string>& linesToRender, const ScrollableScreenBuffer& buffer, const COORD& cursor, const COORD& anchor, const int color);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	Canvas canvas{ COORD{0, 0} };
};