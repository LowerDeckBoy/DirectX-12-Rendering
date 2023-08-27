#pragma once
#include <string>
#include <stdexcept>
#include <cassert>
#include <debugapi.h>

// For ComPtr
#define SAFE_RELEASE(x) { if (x) { x.Reset(); x = nullptr; } }
// For non-ComPtr
#define SAFE_DELETE(x) { if (x) { x->Release(); x = nullptr; } }

#define TraceError(x)													\
	if (FAILED(x)) {													\
		std::string message{ "HRESULT failed\nFile: " + std::string(__FILE__) + "\nLine: " + std::to_string(__LINE__) + '\n' };	\
		::OutputDebugStringA(message.c_str());							\
		::MessageBoxA(nullptr, message.data(), "Error", MB_OK);			\
		throw std::runtime_error("Exception thrown");					\
	}	

inline void ThrowIfFailed(HRESULT hResult)
{
	if (FAILED(hResult))
		throw std::runtime_error("Exception thrown");
}

inline void ThrowIfFailed(HRESULT hResult, const std::string_view& Msg)
{
	if (FAILED(hResult))
	{
		const auto msg{ "Error in: " + std::string(__FILE__) + ", at line: " + std::to_string(__LINE__) };
		::MessageBoxA(nullptr, msg.data(), "Error", MB_OK);
		//::MessageBoxA(nullptr, Msg.data(), "Error", MB_OK);
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
