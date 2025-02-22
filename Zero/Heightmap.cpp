#include "Heightmap.h"
#include <stb_image.h>
#include <vector>
#include <Model/Vertex.h>

Zero::Heightmap::Heightmap(const char* path)
{
    unsigned char* data = stbi_load("resources/heightmaps/iceland_heightmap.png",
        &m_Width, &m_Height, &m_NumChannels, 0);

    if (!data) 
    {
		printf("Failed to load heightmap\n");
		return;
	}

    std::vector<Vertex> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;  // apply a scale+shift to the height data
    for (unsigned int i = 0; i < m_Height; i++)
    {
        for (unsigned int j = 0; j < m_Width; j++)
        {
            Vertex v;

            // retrieve texel for (i,j) tex coord
            unsigned char* texel = data + (j + m_Width * i) * m_NumChannels;
            // raw height at coordinate
            unsigned char y = texel[0];

            // vertex
            v.Position.x = (-m_Height / 2.0f + i);      // v.x
            v.Position.y = ((int)y * yScale - yShift);  // v.y
            v.Position.z = (-m_Width / 2.0f + j);       // v.z

            vertices.emplace_back(v);
        }
    }

    stbi_image_free(data);

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < m_Height - 1; i++)       // for each row a.k.a. each strip
    {
        for (unsigned int j = 0; j < m_Width; j++)      // for each column
        {
            for (unsigned int k = 0; k < 2; k++)      // for each side of the strip
            {
                indices.push_back(j + m_Width * (i + k));
            }
        }
    }
}
