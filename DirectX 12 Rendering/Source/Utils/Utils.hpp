#pragma once
//#include <Windows.h>
#include <string>
#include <stdexcept>
#include <cassert>

#define SafeRelease(x) { if (x) { x.Reset(); x = nullptr; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
		//throw std::runtime_error("Error occurred!");
}

inline void ThrowIfFailed(HRESULT hr, const std::string& msg)
{
	if (FAILED(hr))
		throw std::exception(msg.data());
}

