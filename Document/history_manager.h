#pragma once
#include "storage.h"
#include "action_history.h"

namespace history {
	using ActionPtr = Action::ActionPtr;
	static constexpr std::chrono::milliseconds defaultMergeInterval{ 2000 };
	static constexpr int defaultCapacity{ 200 };

	struct HistoryManagerOptions {
		std::chrono::milliseconds mergeInterval{ defaultMergeInterval };
		int capacity{ defaultCapacity };
	};

	class HistoryManager {
	public:
		HistoryManager() = default;
		HistoryManager(const HistoryManagerOptions& options);
		HistoryManager(HistoryManager&&) noexcept;
		HistoryManager& operator=(HistoryManager&&) noexcept;
		HistoryManager(const HistoryManager&) = delete;
		HistoryManager& operator=(const HistoryManager&) = delete;
		~HistoryManager();

		int addHistory();
		bool removeHistory(const int index);
		UndoReturn undo(const int index);
		UndoReturn redo(const int index);

		void pushWriteAction(const int index, const COORD& startPos, std::vector<std::string>& text, TextContainer* target);
		void pushEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target);
	private:
		void affect(ActionPtr& action);
		void push(const int index, ActionPtr& action);

		const std::chrono::milliseconds mergeInterval{ defaultMergeInterval };
		const int capacity{ defaultCapacity };
		std::vector<ActionHistory> histories;
		Storage<ActionPtr> eraseRegistry;
	};
}
