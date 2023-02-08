#pragma once
//#include <Windows.h>
#include <string>
#include <exception>
#include <cassert>
//#include <cstdint>

#define SafeRelease(x) { if (x) { x.Reset(); x = nullptr; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (!SUCCEEDED(hr))
		throw std::exception();
}

inline void ThrowIfFailed(HRESULT hr, const std::string& msg)
{
	if (!SUCCEEDED(hr))
		throw std::exception(msg.data());
}

