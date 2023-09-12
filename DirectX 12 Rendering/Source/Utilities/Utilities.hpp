#pragma once
#include <string>
#include <stdexcept>
#include <cassert>

#if defined (_DEBUG) || (DEBUG)
#include <debugapi.h>
#endif

// For ComPtr
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) { x.Reset(); x = nullptr; } }
#endif
// For non-ComPtr
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) { x->Release(); x = nullptr; } }
#endif

// Helper macro to determinate where exactly an HRESULT error occurs
#ifndef TraceError
#define TraceError(DebugMessage)											\
	{																		\
		std::string file{ "File: " + std::string(__FILE__) };				\
		std::string func{ "\nFunction: " + std::string(__func__) };			\
		std::string line{ "\nLine: " + std::to_string(__LINE__) + '\n' };	\
		std::string message{ file + func + line + DebugMessage };			\
		::OutputDebugStringA(message.c_str());								\
		::MessageBoxA(nullptr, message.data(), "Error", MB_OK);				\
	}																	
#endif

#ifndef ThrowIfFailed
#define ThrowIfFailed(hResult, ...)									\
		if (FAILED(hResult))										\
		{															\
			std::string msg{ std::string(__VA_ARGS__) };			\
			TraceError(msg);										\
			throw std::runtime_error("HRESULT exception thrown.");	\
		}
#endif
