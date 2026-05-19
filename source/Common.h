#pragma once

#include "Hooking.Patterns.h"

#include <stack>
#include <functional>

struct UndoRedoAction
{
    std::function<void()> Undo;
    std::function<void()> Redo;
};

inline std::stack<UndoRedoAction> sUndoStack;
inline std::stack<UndoRedoAction> sRedoStack;

inline void PushUndo(std::function<void()> undo, std::function<void()> redo)
{
    sUndoStack.push({undo, redo});

    while(!sRedoStack.empty())
        sRedoStack.pop();
}


inline hook::pattern FindPattern(std::initializer_list<const char*> patterns)
{
    for(const auto& pattern : patterns)
    {
        auto result = hook::pattern(pattern);
        if(!result.empty())
            return result;
    }
    return {};
}