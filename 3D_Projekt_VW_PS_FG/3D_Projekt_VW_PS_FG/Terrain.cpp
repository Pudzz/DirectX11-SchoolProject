#include "Terrain.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Terrain::Terrain()
{
	this->mesh = new Models;

	// Cellspace for how large we want the grid to be
	this->cellSpace = 1.0f;
}

Terrain::~Terrain()
{
}

float Terrain::GetTriangleHeight(const float x, const float z)
{
	// EXEMPEL OCH F�RKLARING TILL FUNKTIONEN FINNS I KAPITEL 19.5 I "Introduction to real time rendering"
	// Values for which cell in the terrains grid we are in and transform from terrain local space to "cell" space.
	float valueX = x - (int)(x / cellSpace);
	float valueZ = z - (int)(z / cellSpace);

	// Which column and row we are in
	int column = (int)floorf(x);
	int row = (int)floorf(z);

	// If we are outside of the terrain, we step off of it
	if (row < 0 || column < 0 || row >= width - 1 || column >= height - 1)
	{
		return 0;
	}

	// Height values for each unique vertex in the quad. We work in quads because our terrain is divided into a grid with "cells"
	float topRight = GetMesh()->GetVertices()[(row * (long long)width + (long long)column)].pos.y;
	float topLeft = GetMesh()->GetVertices()[(row * (long long)width + (long long)column + 1)].pos.y;
	float bottomLeft = GetMesh()->GetVertices()[((row + (long long)1) * (long long)width + (long long)column)].pos.y;
	float bottomRight = GetMesh()->GetVertices()[((row + (long long)1) * (long long)width + (long long)column + 1)].pos.y;

	float height = 0;

	
	// Upper triangle in the quad
	if (valueX + valueZ <= 1.0f)
	{
		float uy = topLeft - topRight;
		float vy = bottomLeft - topRight;
		height = topRight + valueX * uy + valueZ * vy;
	}
	else // If we are inside the lower triangle in the quad
	{
		float uy = bottomLeft - bottomRight;
		float vy = topLeft - bottomRight;
		height = bottomRight + (1.0f - valueX) * uy + (1.0f - valueZ) * vy;
	}

	// Return the height
	return height;
}

void Terrain::CreateTerrain(std::string filename, ID3D11Device* device, HWND hwnd)
{
	// Allocate memory for the image file
	// 4 represents RGBA
	int bpp = sizeof(uint8_t) * 4;
	//Flip the UV coordinates
	stbi_set_flip_vertically_on_load(1);
	uint8_t* image = stbi_load(filename.data(), &width, &height, &bpp, 1);

	// Vectors to hold the vertices and indices
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;

	// Amount of indices
	size_t indexCount = 0;

	// Temporary vertex to push into vectors
	Vertex temp;

	// Starting position of the texCoords
	float UIndex = 0.0f;
	float VIndex = 1.0f;

	// Factor of how much to add to the each pixels texCoord
	float uFactor = 1.0f / width;
	float vFactor = 1.0f / height;

	// For each pixel
	for (size_t z = 0; z < height; z++)
	{
		for (size_t x = 0; x < width; x++)
		{
			// Store width and depth values for each pixel
			temp.pos.x = x * cellSpace;
			temp.pos.z = z * cellSpace;

			// Store the height value for each pixel
			// And multiply by a height factor, in this case 15
			temp.pos.y = (float)image[z * width + x + 0] / 255.0f;
			temp.pos.y *= 15.0f;

			// UV and normals
			temp.texCoord = DirectX::XMFLOAT2(UIndex, VIndex);
			temp.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			UIndex += uFactor;

			// Store the vertex
			vertices.push_back(temp);

			// If the whole map is not done, we store the indices for both triangles
			if ((int)z < height - 1 && (int)x < width - 1)
			{
				// Indices for first triangle
				indices.push_back((unsigned int)(indexCount + width));
				indices.push_back((unsigned int)(indexCount + width) + 1);
				indices.push_back((unsigned int)(indexCount + 1));
				
				// Indices for second triangle
				indices.push_back((unsigned int)(indexCount + width));
				indices.push_back((unsigned int)(indexCount + 1));
				indices.push_back((unsigned int)(indexCount));
			}

			indexCount++;
		}

		UIndex = 0.0f;
		VIndex -= vFactor;
	}

	// Deallocation
	delete image;

	// Vector to store temporary normals
	std::vector<DirectX::XMFLOAT3> tempNormal;
	DirectX::XMFLOAT3 unnormalized;

	// Edges to calcuate the normal for each face
	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	// Amount of faces
	int NumFaces = (width - 1) * (height - 1) * 2;
	float vecX, vecY, vecZ;

	// For each face
	for (size_t i = 0; i < NumFaces; ++i)
	{
		// Get the coordinates for the first edge of the current face
		vecX = vertices[indices[i * 3]].pos.x - vertices[indices[(i * 3) + 2]].pos.x;
		vecY = vertices[indices[i * 3]].pos.y - vertices[indices[(i * 3) + 2]].pos.y;
		vecZ = vertices[indices[i * 3]].pos.z - vertices[indices[(i * 3) + 2]].pos.z;
		edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		// Get the coordinates for the second edge of the current face
		vecX = vertices[indices[(i * 3) + 2]].pos.x - vertices[indices[(i * 3) + 1]].pos.x;
		vecY = vertices[indices[(i * 3) + 2]].pos.y - vertices[indices[(i * 3) + 1]].pos.y;
		vecZ = vertices[indices[(i * 3) + 2]].pos.z - vertices[indices[(i * 3) + 1]].pos.z;
		edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		// Store the crossproduct of the 2 edges, which in turn is the face normal
		XMStoreFloat3(&unnormalized, DirectX::XMVector3Cross(edge1, edge2));

		tempNormal.push_back(unnormalized);
	}

	// Compute vertex normals (normal Averaging)
	// This takes one hell of a long time, based on how many vertices it must go through
	// Its just to make a more smooth shading
	DirectX::XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int facesUsing = 0;
	float tX;
	float tY;
	float tZ;

	//// Go through each vertex
	for (int i = 0; i < vertices.size(); ++i)
	{
		//Check which triangles use this vertex
		for (int j = 0; j < NumFaces; ++j)
		{
			if (indices[j * 3] == i ||
				indices[(j * 3) + 1] == i ||
				indices[(j * 3) + 2] == i)
			{
				tX = XMVectorGetX(normalSum) + -tempNormal[j].x;
				tY = XMVectorGetY(normalSum) + -tempNormal[j].y;
				tZ = XMVectorGetZ(normalSum) + -tempNormal[j].z;

				//If a face is using the vertex, add the unormalized face normal to the normalSum
				normalSum = XMVectorSet(tX, tY, tZ, 0.0f);
				facesUsing++;
			}
		}

		//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
		normalSum = normalSum / (float)facesUsing;

		//Normalize the normalSum vector
		normalSum = XMVector3Normalize(normalSum);

		//Store the normal in our current vertex
		vertices[i].normal.x = XMVectorGetX(normalSum);
		vertices[i].normal.y = XMVectorGetY(normalSum);
		vertices[i].normal.z = XMVectorGetZ(normalSum);

		//Clear normalSum and facesUsing for next vertex
		normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		facesUsing = 0;
	}

	// Initialize the buffers for the mesh
	mesh->InitializeFromFbx(vertices, indices, nullptr, device);
}

void Terrain::releaseMesh()
{
	if (mesh) {
		mesh->Shutdown();
		delete mesh;
		mesh = 0;
	}
}

