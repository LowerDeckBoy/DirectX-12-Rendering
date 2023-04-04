#pragma once
#include <string>
#include <stdexcept>
#include <cassert>

#define SafeRelease(x) { if (x) { x.Reset(); x = nullptr; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::runtime_error("Exception occurred.");
}

inline void ThrowIfFailed(HRESULT hr, const std::string_view& msg)
{
	if (FAILED(hr))
		throw std::runtime_error(msg.data());
}
