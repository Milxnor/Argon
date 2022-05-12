// This file contains the commands and ImGui console.

#pragma once

#include <Windows.h>
#include <ImGui/imgui.h>
#include <stdio.h>
#include <string>
#include <vector>

struct ImGUIConsole
{
    char InputBuf[256];
    std::vector<std::string> Items;
    std::vector<std::string> Commands;
    std::vector <std::string> History;
    int HistoryPos;
    ImGuiTextFilter Filter;
    bool AutoScroll;
    bool ScrollToBottom;

    ImGUIConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back(_("HELP"));
        Commands.push_back(_("HISTORY"));
        Commands.push_back(_("CLEAR"));
        Commands.push_back(_("CLASSIFY"));
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Welcome to Dear ImGui!");
    }
    ~ImGUIConsole()
    {
        ClearLog();
    }
	
    void ClearLog()
    {	
        Items.clear();
    }

    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        Items.push_back(buf);
    }

    static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }

    void Show(bool* p_open)
    {
        /*
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }
        */

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem(_("Close Console")))
                *p_open = false;
			
            ImGui::EndPopup();
        }

        ImGui::TextWrapped(_("Enter 'HELP' for help."));

        if (ImGui::SmallButton(_("Add Debug Text"))) { AddLog("%d some text", Items.size()); AddLog("some more text"); AddLog("display very important message here!"); }
        ImGui::SameLine();
		
        if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        ImGui::SameLine();
		
        if (ImGui::SmallButton("Clear")) { ClearLog(); }
        ImGui::SameLine();
		
        bool copy_to_clipboard = ImGui::SmallButton("Copy");

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup(_("Options")))
        {
            ImGui::Checkbox(_("Auto-scroll"), &AutoScroll);
            ImGui::EndPopup();
        }

        if (ImGui::Button(_("Options")))
            ImGui::OpenPopup(_("Options"));
		
        ImGui::SameLine();
        Filter.Draw(_("Filter (\"incl,-excl\") (\"error\")"), 180);
        ImGui::Separator();

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild(_("ScrollingRegion"), ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable(_("Clear"))) ClearLog();
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        for (auto item : Items)
        {
            if (!Filter.PassFilter(item.c_str()))
                continue;
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();

        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
		
        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		
        if (ImGui::InputText(_("Input"), InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
			
            if (s[0])
                ExecCommand(s);

            strcpy_s(s, 0, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        // ImGui::End();
    }

    void ExecCommand(const char* command_line)
    {
        AddLog(_("# %s\n"), command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.size() - 1; i >= 0; i--)
            if (Stricmp(History[i].c_str(), command_line) == 0)
            {
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(command_line);

        std::string cmd = command_line;

        // Process command
        if (cmd == _("CLEAR"))
        {
            ClearLog();
        }
		
        else if (cmd == _("HELP"))
        {
            AddLog(_("Commands:"));
			
            for (int i = 0; i < Commands.size(); i++)
                AddLog(_("- % s"), Commands[i]);
        }
        else if (cmd == _("HISTORY"))
        {
            int first = History.size() - 10;
            for (int i = first > 0 ? first : 0; i < History.size(); i++)
                AddLog(_("%3d: %s\n"), i, History[i]);
        }

        else if (cmd == _("EXECUTE"))
        {
			
        }
		
        else
        {
            AddLog(_("Unknown command: '%s'\n"), command_line);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        ImGUIConsole* console = (ImGUIConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
			
            for (int i = 0; i < Commands.size(); i++)
                if (Strnicmp(Commands[i].c_str(), word_start, (int)(word_end - word_start)) == 0)
                    candidates.push_back(Commands[i].c_str());

            if (candidates.Size == 0)
            {
                // No match
                AddLog(_("No match for \"%.*s\"!\n"), (int)(word_end - word_start), word_start);
            }
			
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                AddLog(_("Possible matches:\n"));
				
                for (int i = 0; i < candidates.Size; i++)
                    AddLog(_("- %s\n"), candidates[i]);
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
            const int prev_history_pos = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (HistoryPos == -1)
                    HistoryPos = History.size() - 1;
                else if (HistoryPos > 0)
                    HistoryPos--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (HistoryPos != -1)
                    if (++HistoryPos >= History.size())
                        HistoryPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != HistoryPos)
            {
                const char* history_str = ((HistoryPos >= 0) ? History[HistoryPos] : "").c_str();
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 0;
    }
};

static ImGUIConsole console;

static void ShowConsole(bool* p_open)
{
    console.Show(p_open);
}

bool handleCommand(const std::string& Command, bool bFromUE = false)
{
    std::vector<std::string> help = { "" };

    if (bFromUE)
    {
		
    }

    else
    {
        console.ExecCommand(Command.c_str());
    }

    return false;

}