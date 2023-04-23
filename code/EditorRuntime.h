#pragma once

#include <gmath.h>
#include "Common.h"

struct CSGBrush;

enum class EditorToolMode
{
    None,
    EditVertex,
    EditEdge,
    EditFace
};

enum class EditorGridSize : u32
{
    Grid1 = 1,
    Grid2 = 2,
    Grid4 = 4,
    Grid8 = 8,
    Grid16 = 16,
    Grid32 = 32,
    Grid64 = 64,
    Grid128 = 128
};

class EditorRuntime
{
public:
	void StartEditor();

	void UpdateEditor();

public:
    CSGBrush* brush_0 = nullptr;
    
    EditorToolMode activeToolMode = EditorToolMode::EditFace;

private:
    EditorGridSize activeGridSize = EditorGridSize::Grid1;

    bool handleDragActive = false;
    vec3 handleDragPoint = vec3();
    //vec3 selectedDragPoint = vec3();
};
