#pragma once
#include <string>
#include <stdexcept>
#include <cassert>

// For ComPtr
#define SAFE_RELEASE(x) { if (x) { x.Reset(); x = nullptr; } }
// For non-ComPtr
#define SAFE_DELETE(x) { if (x) { x->Release(); x = nullptr; } }


inline void ThrowIfFailed(HRESULT hResult)
{
	if (FAILED(hResult))
		throw std::logic_error("Exception thrown");
}

inline void ThrowIfFailed(HRESULT hResult, const std::string_view& Msg)
{
	if (FAILED(hResult))
	{
		::MessageBoxA(nullptr, Msg.data(), "Error", MB_OK);
		throw std::runtime_error(Msg.data());
	}
}

[[maybe_unused]]
inline const wchar_t* ToWchar(const std::string& String)
{
	std::wstring wpath = std::wstring(String.begin(), String.end());
	auto output{ wpath.c_str() };
	return output;
}
