#pragma once

struct CSGBrush;

class EditorRuntime
{
public:
	void StartEditor();

	void UpdateEditor();
    
public:
    CSGBrush* brush_0;
};
