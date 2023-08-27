#pragma once
#include <filesystem>

namespace files
{
	inline std::string GetExtension(const std::string& Filename) {
		return std::filesystem::path(Filename).extension().string();
	}

	inline std::string GetAbsolutePath(const std::string& Filename) {
		return absolute(std::filesystem::path(Filename)).string();
	}

	inline std::string GetRelativePath(const std::string& Filename) {
		return std::filesystem::path(Filename).relative_path().string();
	}

	namespace glTF
	{
		inline std::string GetRelativePath(const std::string& Filename) {
			return std::filesystem::path(Filename).relative_path().string();
		}

		inline std::string GetAbsolutePath(const std::string& Filename) {
			return absolute(std::filesystem::path("../" + Filename)).string();
		}
		//+ "/"
		inline std::string GetTexturePath(const std::string& Filename, const std::string& TextureName) {
			return std::filesystem::path(Filename).relative_path().parent_path().string() + "/" + TextureName;
		}
		inline std::string GetTexAbsolutePath(const std::string& Filename, const std::string& TextureName) {
			return absolute(std::filesystem::path(Filename)).parent_path().string() + "\\" + TextureName;
		}
	}
	
	// TODO: Add .fbx texture paths
}