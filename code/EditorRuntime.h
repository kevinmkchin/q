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

enum class GridSizeTimes1000 : u32
{
    //Grid0125 = 125,
    //Grid025 = 250,
    //Grid05 = 500,
    Grid1 = 1000,
    Grid2 = 2000,
    Grid4 = 4000,
    Grid8 = 8000,
    Grid16 = 16000,
    Grid32 = 32000,
    Grid64 = 64000,
    Grid128 = 128000
};

class EditorRuntime
{
public:
	void StartEditor();

	void UpdateEditor();
    
public:
    CSGBrush* brush_0 = nullptr;
    
    EditorToolMode activeToolMode = EditorToolMode::None;

private:
    GridSizeTimes1000 activeGridSize = GridSizeTimes1000::Grid1;

    bool handleDragActive = false;
    vec3 selectedDragPoint = vec3();
};
