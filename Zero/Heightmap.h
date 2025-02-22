#pragma once

namespace Zero{

	class Heightmap
	{
	public:
		Heightmap() = default;
		Heightmap(const char* path);
		~Heightmap() = default;

		void LoadHeightmap(const char* path);

	private:
		int m_Width;
		int m_Height;
		int m_NumChannels;
	};

};

