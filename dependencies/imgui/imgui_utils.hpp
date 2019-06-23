#pragma once

#include "imgui.h"
#include "imgui_internal.h"

inline ImVec2 operator+(const ImVec2& left, const ImVec2& right) noexcept {
	return ImVec2(left.x + right.x, left.y + right.y);
}

inline ImVec2 operator-(const ImVec2& left, const ImVec2& right) noexcept {
	return ImVec2(left.x - right.x, left.y - right.y);
}

inline ImVec2 operator+=(ImVec2& left, const ImVec2& right) noexcept {
	return ImVec2(left.x += right.x, left.y += right.y);
}

inline ImVec2 operator-=(ImVec2& left, const ImVec2& right) noexcept {
	return ImVec2(left.x -= right.x, left.y -= right.y);
}

inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}