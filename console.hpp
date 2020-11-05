#pragma once

#include "span.hpp"

#include <unordered_map>
#include <string_view>
#include <memory>

enum class CommandExecuteStatus {
	eSucces,
	eFailed,
	eWrong
};

struct ICommand {
	virtual ~ICommand() = default;

	virtual int proccess(span<std::string_view> args) = 0;
};

struct OptionReader {
	OptionReader(span<std::string_view> args) : args(args) {}

	OptionReader& operator++() {
		index++;
		return *this;
	}

	OptionReader operator++(int) {
		OptionReader out = *this;
		index++;
		return out;
	}

	std::string_view operator*() const {
		return args[index];
	}

	void skip() {
		index++;
	}

	bool skip(std::string_view arg) {
		if (args[index] == arg) {
			return index++, true;
		}
		return false;
	}

private:
	span<std::string_view> args;
	size_t index = 0;
};


template <typename T>
struct Command : ICommand {
	Command(T &&command) : command(std::forward<T>(command)) {}

	int proccess(span<std::string_view> args) override {
		return command(args);
	}

private:
	T command;
};

//struct CommandShell {
//	std::string_view args[63];
//
//	std::vector<char *> items;
//	std::vector<char *> history;
//	int history_pos;
//};

struct CommandDispatcher {
//	int fd[2];

	std::unordered_map<std::string_view, std::unique_ptr<ICommand>> commands;

//	CommandDispatcher() {
//		if (pipe(fd) < 0) {
//			perror("init console");
//		}
//	}

	template<typename T>
	void add(std::string_view name, T &&command) {
		commands.emplace(name, new Command<T>(std::forward<T>(command)));
	}

	CommandExecuteStatus execute(span<std::string_view> args) {
		auto command = commands.find(args[0]);
		if (command == commands.end()) {
			return CommandExecuteStatus::eWrong;
		}
		if (command->second->proccess(args) == 0) {
			return CommandExecuteStatus::eSucces;
		}

		return CommandExecuteStatus::eFailed;
	}
};

struct Console {
	char input[256];
	std::string_view args[63];

	CommandDispatcher dispatcher;

	ImVector<char *> Items;
	ImVector<char *> History;
	int HistoryPos;

	ImGuiTextFilter Filter;

	bool AutoScroll;
	bool ScrollToBottom;

	Console() {
		using namespace std::string_view_literals;

		ClearLog();
		memset(input, 0, sizeof(input));
		HistoryPos = -1;
		AutoScroll = true;
		ScrollToBottom = false;

		dispatcher.add("clear", [this] (OptionReader /*args*/) -> int {
			ClearLog();
			return 0;
		});
	}

	~Console() {
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	void ClearLog() {
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
	}

	void log_print(const char *str){
		Items.push_back(strdup(str));
	}

	void log_print_formated(const char *fmt, ...) IM_FMTARGS(2) {
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(strdup(buf));
	}

	void Draw() {
//		ImGui::SetNextWindowPos(ImVec2(0, 0));
//		ImGui::SetNextWindowSize(ImVec2(g_MainWindowData.Width, g_MainWindowData.Height));
		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoTitleBar /*| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/);

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.originY + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

		ImGuiListClipper clipper(Items.Size);
		while (clipper.Step()) {
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				const char *item = Items[i];
				if (!Filter.PassFilter(item))
					continue;

				// Normally you would store more information in your item than just a string.
				// (e.g. make Items[] an array of structure, store color/type etc.)
				ImVec4 color;
				bool has_color = false;
				if (strstr(item, "[error]")) {
					color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
					has_color = true;
				} else if (strncmp(item, "# ", 2) == 0) {
					color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
					has_color = true;
				}
				if (has_color)
					ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::TextUnformatted(item);
				if (has_color)
					ImGui::PopStyleColor();
			}
		}

		if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
			ImGui::SetScrollHereY(1.0f);
		}
		ScrollToBottom = false;

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		ImGuiInputTextFlags input_text_flags =  ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		ImGui::PushItemWidth(-1);
		bool enter = ImGui::InputText("", input, sizeof(input), input_text_flags, &TextEditCallbackStub, (void *) this);
		ImGui::PopItemWidth();
		if (enter) {
			const auto len = strlen(input);
			const char *str = input;
			const char *str_end = str + len;

			HistoryPos = -1;
			History.push_back(strndup(str, len));

			while (str < str_end && *str == ' ') {
				str++;
			}

			if (*str == '/') {
				str++;

				while (str < str_end && str_end[-1] == ' ') {
					str_end--;
				}

				size_t argc = 0;
				for (;str < str_end; str++) {
					const char *ptr = str;
					while (str < str_end && *str != ' ') {
						str++;
					}
					const auto size = str - ptr;
					if (size > 0) {
						args[argc++] = std::string_view(ptr, size);
					}
				}

				if (argc > 0) {
					switch (dispatcher.execute(span(args, argc))) {
					case CommandExecuteStatus::eSucces:
						break;
					case CommandExecuteStatus::eFailed:
						log_print_formated("[error]: no operation specified (use -h for help)");
						break;
					case CommandExecuteStatus::eWrong:
						log_print_formated("[error]: %.*s: command not found", int(args[0].size()), args[0].data());
						break;
					}
				}
			} else {
				Items.push_back(strndup(str, len));
			}

			*input = 0;
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
	static int TextEditCallbackStub(ImGuiInputTextCallbackData *data) {
		return static_cast<Console *>(data->UserData)->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiInputTextCallbackData *data) {
		switch (data->EventFlag) {
		case ImGuiInputTextFlags_CallbackCompletion: {
			break;
		}
		case ImGuiInputTextFlags_CallbackHistory: {
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow) {
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			} else if (data->EventKey == ImGuiKey_DownArrow) {
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos) {
				const char *history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
		}
		return 0;
	}
};