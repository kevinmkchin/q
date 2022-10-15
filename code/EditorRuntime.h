#pragma once

struct CSGBrush;

enum class EditorToolMode
{
    None,
    EditVertex,
    EditEdge,
    EditFace
};

class EditorRuntime
{
public:
	void StartEditor();

	void UpdateEditor();
    
public:
    CSGBrush* brush_0 = nullptr;
    
    EditorToolMode activeToolMode = EditorToolMode::None;
};
