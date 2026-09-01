#pragma once

#include "Hooking.Patterns.h"
#include "injector/injector.hpp"

#include <stack>
#include <functional>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

// True when [p, p+n) is committed, readable, non-guard memory.
inline bool IsReadable(const void* p, size_t n)
{
    if(!p)
        return false;
    const uint8_t* a = (const uint8_t*)p;
    const uint8_t* end = a + (n ? n - 1 : 0);
    while(a <= end)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if(VirtualQuery(a, &mbi, sizeof(mbi)) != sizeof(mbi))
            return false;
        if(mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
            return false;
        a = (const uint8_t*)mbi.BaseAddress + mbi.RegionSize;
    }
    return true;
}

// Patches are recorded so a failed init can restore every byte it changed.
// Without this an unloaded ASI leaves its call sites pointing into freed
// memory, and the game crashes later somewhere unrelated.
inline bool gEditorDead = false;

struct SCallPatch { uintptr_t at; uintptr_t prev; };
inline std::vector<SCallPatch> gInstalledCalls;

struct SDwordPatch { uint32_t* at; uint32_t orig; };
inline std::vector<SDwordPatch> gInstalledDwords;

inline uintptr_t PatchCall(void* at, void* dest)
{
    uintptr_t prev = injector::MakeCALL(at, injector::raw_ptr(dest)).as_int();
    gInstalledCalls.push_back({(uintptr_t)at, prev});
    return prev;
}

inline void PatchDword(uint32_t* at, uint32_t value)
{
    gInstalledDwords.push_back({at, *at});
    injector::WriteMemory(at, value, true);
}

inline void RollbackAllPatches()
{
    for(auto it = gInstalledCalls.rbegin(); it != gInstalledCalls.rend(); ++it)
        injector::MakeCALL((void*)it->at, injector::raw_ptr((void*)it->prev));
    gInstalledCalls.clear();

    for(auto it = gInstalledDwords.rbegin(); it != gInstalledDwords.rend(); ++it)
        injector::WriteMemory<uint32_t>(it->at, it->orig, true);
    gInstalledDwords.clear();
}


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
    for(const char* p : patterns)
    {
        hook::pattern pat(p);
        if(!pat.empty())
            return pat;
    }
    return hook::pattern(*patterns.begin());
}
