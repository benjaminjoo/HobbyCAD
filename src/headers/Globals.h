#pragma once


enum class ToolBox
{
    Selection   = 0,
    Line        = 1,
    Polyline    = 2,
    Polygon     = 3
};


extern ToolBox  g_ActiveTool;

extern int      g_LeftClicksIn;