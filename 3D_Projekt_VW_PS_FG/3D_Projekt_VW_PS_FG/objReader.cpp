#include "objReader.h"

objReader::objReader()
{
	this->hr = 0;
}

objReader::~objReader()
{
}

bool objReader::loadObj(Models* model, ID3D11Device* device, wstring fileName, bool isRightHanded, bool computeNormals)
{
	wifstream fileIn(fileName.c_str()); // Opens a file
	wstring meshMatLib; // Holds the material library filename

	// Temporary vectors where each pos, normal and texcoord will be stored during reading
	std::vector<DirectX::XMFLOAT3> vertexPos;
	std::vector<DirectX::XMFLOAT3> vertexNorm;
	std::vector<DirectX::XMFLOAT2> vertexTexCoord;
	std::vector<wstring> meshMaterials;

	// Temporary vectors which holds the index for each pos, normal and texcoord during reading
	std::vector<int> vertexPosIndex;
	std::vector<int> vertexNormIndex;
	std::vector<int> vertexTexCoordIndex;
	
	bool hasTexCoord = false;
	bool hasNorm = false;

	// Temporary variables to store into vectors
	wstring tempMeshMaterials;
	int tempVertexPosIndex;
	int tempVertexNormIndex;
	int tempVertexTexCoordIndex;

	wchar_t checkChar; // Stores one char at a time from the file
	wstring face; // Holds the string with the face vertises
	int vIndex = 0; // Index count
	int triangleCount = 0; // Total triangles
	int totalVertices = 0; // Total vertices
	int meshTriangles = 0;

	// Check if we can open the file
	if (fileIn)
	{
		// While its open, we do this
		while (fileIn)
		{
			// Get a char from the file
			checkChar = fileIn.get();

			// Depending on what char we get, we have a switch statement
			switch (checkChar)
			{

				// CASE FOR COMMENTS IN THE FILE
			case '#':
				checkChar = fileIn.get();
				while (checkChar != '\n') // While the file is at the same comment line, it keeps getting the next character until a new line
				{
					checkChar = fileIn.get();
				}
				break;

				// CASE FOR VERTEX INFORMATION
			case 'v': 
				checkChar = fileIn.get();
				if (checkChar == ' ') // If there is a space after the char, we can get the next types, since the values are seperated by ' ' or spaces
				{
					float vz, vy, vx;
					fileIn >> vx >> vy >> vz; // Stores the next three values from the file

					if (isRightHanded) // If model is from a right handed system
					{
						vertexPos.push_back(XMFLOAT3(vx, vy, vz * -1.0f));
					}
					else
					{
						vertexPos.push_back(XMFLOAT3(vx, vy, vz));
					}
				}

				if (checkChar == 't') // vt = tex coord
				{
					float vtcu, vtcv;
					fileIn >> vtcu >> vtcv; // Stores the next 2 tex coord info

					if (isRightHanded)
					{
						vertexTexCoord.push_back(XMFLOAT2(vtcu, 1.0f - vtcv));
					}
					else
					{
						vertexTexCoord.push_back(XMFLOAT2(vtcu, vtcv));
					}
					hasTexCoord = true; // Now we know that the model has texture coordinates
				}

				if (checkChar == 'n') // vn = Normals
				{
					float vnx, vny, vnz;
					fileIn >> vnx >> vny >> vnz; // Stores the next 3 normal values

					if (isRightHanded)
					{
						vertexNorm.push_back(XMFLOAT3(vnx, vny, vnz * -1.0f));
					}
					else
					{
						vertexNorm.push_back(XMFLOAT3(vnx, vny, vnz));
					}
					hasNorm = true; // Now we know that the model has normals
				}
				break;

				// CASE FOR GROUP INFORMATION, NOT NEEDED FOR THIS ASSIGNMENT
			case 'g': 
				checkChar = fileIn.get();
				if (checkChar == ' ')
				{
					// New index start for each group
					model->GetSubsetIndexVector().push_back(vIndex);
					model->GetSubsetCount()++;
				}
				break;

				// CASE FOR FACE INFORMATION
			case 'f': 
				checkChar = fileIn.get();
				if (checkChar == ' ')
				{
					face = L"";
					wstring vertDef; // Holds one vertex definition at a time
					triangleCount = 0;
					checkChar = fileIn.get();

					while (checkChar != '\n') // While the file hasnt changed line
					{
						face += checkChar; // The string that contains the face information gets the next char
						checkChar = fileIn.get(); // Get the next char again
						if (checkChar == ' ') // if its a space
						{
							triangleCount++; // Increase
						}
					}

					if (face[face.length() - 1 == ' ']) // Check for a space in the end of the face string
					{
						triangleCount--; // Since every char adds to the triangle count, we need to substract 1 for every space
					}
					triangleCount -= 1; // Every vertex in the face after the 2 first, makes a new face
					wstringstream ss(face);

					if (face.length() > 0)
					{
						int firstVIndex, lastVIndex; // First and last index of vertices
						for (int i = 0; i < 3; i++) // First 3 vertices
						{
							ss >> vertDef; // Gets the vertex information
							wstring vertPart;
							int whichPart = 0; // Used to see if we are looking for pos, tex coord or normals

							//Go through the string
							for (int j = 0; (unsigned)j < vertDef.length(); j++)
							{
								if (vertDef[j] != '/')
								{
									vertPart += vertDef[j];
								}
								if (vertDef[j] == '/' || j == vertDef.length() - 1) // If the current char is a /, or its the last char in the string
								{
									wistringstream wstringToInt(vertPart); // Converts strings to ints
									if (whichPart == 0) // If its vertex positions
									{
										wstringToInt >> tempVertexPosIndex;
										tempVertexPosIndex -= 1; // Subtract 1 since obj arrays starts with 1, not 0
										if (j == vertDef.length() - 1) // If only positions are used in the file
										{
											tempVertexNormIndex = 0;
											tempVertexTexCoordIndex = 0;
										}
									}

									else if (whichPart == 1) // If its tex coords
									{
										if (vertPart != L"") // Checks if its not an empty string
										{
											wstringToInt >> tempVertexTexCoordIndex;
											tempVertexTexCoordIndex -= 1; // Subtract 1
										}
										else // Default value if there are no tex coords
										{
											tempVertexTexCoordIndex = 0;
										}
										if (j == vertDef.length() - 1) // If the current char is in the end of the string, there are no normals
										{
											tempVertexNormIndex = 0; // Sets default normal value
										}
									}

									else if (whichPart == 2) // If its normals
									{
										wistringstream wstringToInt(vertPart);
										wstringToInt >> tempVertexNormIndex;
										tempVertexNormIndex -= 1; // Subtract 1
									}
									vertPart = L""; // Clear and get ready for next part
									whichPart++; // Move on to the next vertex part
								}
							}

							// STORING THE FACE // 
							//Check to see if there are atleast 1 subset
							// Subset is the same as if the model would be several meshes
							if (model->GetSubsetCount() == 0)
							{
								model->GetSubsetIndexVector().push_back(vIndex); // Start index 
								model->GetSubsetCount()++;
							}

							// Store the vertex information
							vertexPosIndex.push_back(tempVertexPosIndex);
							vertexTexCoordIndex.push_back(tempVertexTexCoordIndex);
							vertexNormIndex.push_back(tempVertexNormIndex);
							totalVertices++; // Adds a new vertex
							model->GetIndices().push_back(totalVertices - 1); // Sets the index for this vertex

							// If this is the first vertex in the face, we need to make sure the rest of the triangles use this 
							if (i == 0)
							{
								firstVIndex = model->GetIndices()[vIndex]; // First index of this face
							}
							// If this was the last vertex in the first triangle, we will make sure the next triangle uses this
							if (i == 2)
							{
								lastVIndex = model->GetIndices()[vIndex]; // Last index of this triangle
							}
							vIndex++; // Increase index
						}
						meshTriangles++; // Increase triangles

					}
				}
				break;

				// CASE FOR MATERIAL FILE
			case 'm': // mtllib
				checkChar = fileIn.get();
				if (checkChar == 't')
				{
					checkChar = fileIn.get();
					if (checkChar == 'l')
					{
						checkChar = fileIn.get();
						if (checkChar == 'l')
						{
							checkChar = fileIn.get();
							if (checkChar == 'i')
							{
								checkChar = fileIn.get();
								if (checkChar == 'b')
								{
									checkChar = fileIn.get();
									if (checkChar == ' ')
									{
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}
				break;

				// CASE FOR WHICH MATERIAL TO USE
			case 'u': // usemtl
				checkChar = fileIn.get();
				if (checkChar == 's')
				{
					checkChar = fileIn.get();
					if (checkChar == 'e')
					{
						checkChar = fileIn.get();
						if (checkChar == 'm')
						{
							checkChar = fileIn.get();
							if (checkChar == 't')
							{
								checkChar = fileIn.get();
								if (checkChar == 'l')
								{
									checkChar = fileIn.get();
									if (checkChar == ' ')
									{
										tempMeshMaterials = L"";
										fileIn >> tempMeshMaterials;
										meshMaterials.push_back(tempMeshMaterials);
									}
								}
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
	else // If we couldnt open the file
	{
		wstring message = L"Could not open file";
		MessageBox(0, message.c_str(), L"Error", MB_OK);

		return false;
	}

	model->GetSubsetIndexVector().push_back(vIndex);

	if (model->GetSubsetIndexVector()[1] == 0)
	{
		model->GetSubsetIndexVector().erase(model->GetSubsetIndexVector().begin() + 1);
		model->GetSubsetCount()--;
	}

	if (!hasNorm)
	{
		vertexNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	if (!hasTexCoord)
	{
		vertexTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));
	}

	// Close the obj file and open the mtl file
	fileIn.close();
	fileIn.open(meshMatLib.c_str());

	wstring lastStringRead;
	int materialCount = (int)model->GetMaterial().size(); // Amount of materials

	bool setDiffuse = false; // Will be used to see if the material has a diffuse or not

	if (fileIn)
	{
		while (fileIn)
		{
			checkChar = fileIn.get();
			switch (checkChar)
			{
				// Check for comment
			case '#':
				checkChar = fileIn.get();
				while (checkChar != '\n')
				{
					checkChar = fileIn.get();
				}
				break;
				// check and set diffuse
			case 'K':
				checkChar = fileIn.get();
				if (checkChar == 'd')
				{
					checkChar = fileIn.get(); // removes space
					fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.x;
					fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.y;
					fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.z;

					setDiffuse = true;
				}
				// set the ambient color as diffuse if there was no diffuse
				if (checkChar == 'a')
				{
					checkChar = fileIn.get();
					if (setDiffuse == false)
					{
						fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.x;
						fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.y;
						fileIn >> model->GetMaterial()[materialCount - (long long)1].diffuseColor.z;
					}
					else
					{
						fileIn >> model->GetMaterial()[materialCount - (long long)1].ambientColor.x;
						fileIn >> model->GetMaterial()[materialCount - (long long)1].ambientColor.y;
						fileIn >> model->GetMaterial()[materialCount - (long long)1].ambientColor.z;
					}
				}
				// Specular
				if (checkChar == 's')
				{
					checkChar = fileIn.get();
					fileIn >> model->GetMaterial()[materialCount - (long long)1].specularColor.x;
					fileIn >> model->GetMaterial()[materialCount - (long long)1].specularColor.y;
					fileIn >> model->GetMaterial()[materialCount - (long long)1].specularColor.z;
				}
				break;

				// Check for transparency
				checkChar = fileIn.get();
				if (checkChar == 'r')
				{
					checkChar = fileIn.get();
					float transparency;
					fileIn >> transparency;

					model->GetMaterial()[materialCount - (long long)1].isTransparent = true;
				}
				break;
				// Specular shine
			case 'N':
				checkChar = fileIn.get();
				if (checkChar == 's')
				{
					checkChar = fileIn.get();
					fileIn >> model->GetMaterial()[0].shine;
					model->GetMaterial()[0].specularColor.w = model->GetMaterial()[0].shine;
				}
				break;

				// Normal / bump mapping
			case 'b':
				checkChar = fileIn.get();
				if (checkChar == 'u')
				{
					checkChar = fileIn.get();
					if (checkChar == 'm')
					{
						checkChar = fileIn.get();
						if (checkChar == 'p')
						{
							fileIn.get();
							fileIn.get();
							checkChar = fileIn.get();
							if (checkChar == '-')
							{
								checkChar = fileIn.get();
								if (checkChar == 'b')
								{
									checkChar = fileIn.get();
									if (checkChar == 'm')
									{
										fileIn.get();
										fileIn.get();
										fileIn.get();

										wstring filename;
										bool ended = false;
										while (!ended)
										{
											// Store the filename
											checkChar = fileIn.get();
											filename += checkChar;
											if (checkChar == '.')
											{
												for (int i = 0; i < 3; i++)
												{
													filename += fileIn.get();

												}
												ended = true;
											}
										}

										// Create the normal map and store it to the model
										ID3D11ShaderResourceView* tempNormal;
										model->GetTextureNameVector().push_back(filename.c_str());
										model->LoadNormalMap(device, filename.c_str());
										model->GetMaterial()[0].hasNormalMap = true;
									}
								}
							}
						}
					}
				}
				break;

				// Texture map
			case 'm':
				checkChar = fileIn.get();
				if (checkChar == 'a')
				{
					checkChar = fileIn.get();
					if (checkChar == 'p')
					{
						checkChar = fileIn.get();
						if (checkChar == '_')
						{
							// Diffuse map = map_kd
							checkChar = fileIn.get();
							if (checkChar == 'K')
							{
								checkChar = fileIn.get();
								if (checkChar == 'd')
								{
									wstring fileNamePath;
									fileIn.get(); // Removes the whitespace between map_kd and file

									// Get the file path by reading char by char
									bool filePathEnded = false;
									while (filePathEnded == false)
									{
										checkChar = fileIn.get();
										fileNamePath += checkChar;
										if (checkChar == '.')
										{
											for (int i = 0; i < 3; i++)
											{
												fileNamePath += fileIn.get();
											}
											filePathEnded = true;
										}
									}
									// Check if this texture has already been loaded
									bool alreadyLoaded = false;
									for (int i = 0; (unsigned)i < model->GetTextureNameVector().size(); i++)
									{
										MessageBox(0, L"Error", L"Error", MB_OK);
										if (fileNamePath == model->GetTextureNameVector()[i])
										{
											alreadyLoaded = true;
											model->GetMaterial()[materialCount - (long long)1].textureArrayIndex = i;
											model->GetMaterial()[materialCount - (long long)1].hasTexture = true;
										}
									}

									// If the file hasnt been loaded before we can load it now
									if (alreadyLoaded == false)
									{
										ID3D11ShaderResourceView* tempMeshSRV;
										this->hr = DirectX::CreateWICTextureFromFile(device, fileNamePath.c_str(), nullptr, &tempMeshSRV);
										assert(SUCCEEDED(hr));
										if (SUCCEEDED(this->hr))
										{
											model->GetTextureNameVector().push_back(fileNamePath.c_str());
											model->GetMaterial()[materialCount - (long long)1].textureArrayIndex = 1; // TESTA DETTA
											//model->SetTexture(tempMeshSRV);// meshShaderResourceView.push_back(tempMeshSRV);
											model->LoadTextureObj(tempMeshSRV);
											model->GetMaterial()[materialCount - (long long)1].hasTexture = true;
										}
									}
								}
							}
							// alpha map = map_d
							else if (checkChar == 'd')
							{
								model->GetMaterial()[materialCount - (long long)1].isTransparent = true;
							}
						}
					}
				}
				break;
				// newmtl - Declares a new material
			case 'n':
				checkChar = fileIn.get();
				if (checkChar == 'e')
				{
					checkChar = fileIn.get();
					if (checkChar == 'w')
					{
						checkChar = fileIn.get();
						if (checkChar == 'm')
						{
							checkChar = fileIn.get();
							if (checkChar == 't')
							{
								checkChar = fileIn.get();
								if (checkChar == 'l')
								{
									checkChar = fileIn.get();
									if (checkChar == ' ')
									{
										// New material, set default values
										// Pushes it back to the model
										// This will be done first when entering the MTL file
										SurfaceMaterial tempMat;
										model->GetMaterial().push_back(tempMat);
										fileIn >> model->GetMaterial()[materialCount].materialName;
										model->GetMaterial()[materialCount].isTransparent = false;
										model->GetMaterial()[materialCount].hasTexture = false;
										model->GetMaterial()[materialCount].canMove = false; //MOVING TEXTURE
										model->GetMaterial()[materialCount].textureArrayIndex = 0;
										model->GetMaterial()[materialCount].translation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
										materialCount++;
										setDiffuse = false;

									}
								}
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// set fullscreen to false incase its needed, include swapchain in parameter
		wstring message = L"Could not open: ";
		message += meshMatLib;

		MessageBox(0, message.c_str(), L"Error", MB_OK);

		return false;
	}

	// Set the subsets material to the index value of its material in the material array
	// Subsets will only be used if a model contains of more than 1 mesh
	for (int i = 0; i < model->GetSubsetCount(); i++)
	{
		bool hasMat = false;
		for (int j = 0; (unsigned)j < model->GetMaterial().size(); j++)
		{
			if (meshMaterials[i] == model->GetMaterial()[j].materialName)
			{
				model->GetSubsetMaterialVector().push_back(j);
				hasMat = true;
			}
		}
		if (hasMat == false)
		{
			model->GetSubsetMaterialVector().push_back(0); // Use the first material
		}
	}

	Vertex tempVertex;
	// Create the vertices
	for (int j = 0; j < totalVertices; j++)
	{
		tempVertex.pos = vertexPos[vertexPosIndex[j]];
		tempVertex.texCoord = vertexTexCoord[vertexTexCoordIndex[j]];
		tempVertex.normal = vertexNorm[vertexNormIndex[j]];

		model->GetVertices().push_back(tempVertex);

		//Copy just the vertex positions to the vector
		model->GetVerticesArray().push_back(tempVertex.pos);	// For picking
	}

	// Compute the normals
	// This will only be done if we pass true when we load the model
	// For example a model which doesnt contain precalculated normals 
	if (computeNormals == true)
	{
		vector<XMFLOAT3> tempNormal;

		XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

		float vecX, vecY, vecZ;

		XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		// Compute face normals for every vertex
		for (int i = 0; i < meshTriangles; i++)
		{

			vecX = model->GetVertices()[model->GetIndices()[(i * (long long)3)]].pos.x - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.x;
			vecY = model->GetVertices()[model->GetIndices()[(i * (long long)3)]].pos.y - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.y;
			vecZ = model->GetVertices()[model->GetIndices()[(i * (long long)3)]].pos.z - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.z;
			edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f); // Creates the first edge

			vecX = model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.x - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 1]].pos.x;
			vecY = model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.y - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 1]].pos.y;
			vecZ = model->GetVertices()[model->GetIndices()[(i * (long long)3) + 2]].pos.z - model->GetVertices()[model->GetIndices()[(i * (long long)3) + 1]].pos.z;
			edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);

			// Create normal with cross product
			XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
			tempNormal.push_back(unnormalized);
		}

		// Compute face normals (Normal averaging)
		XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		int faceUsing = 0;
		float tX, tY, tZ;

		// Go through each vertex
		for (int i = 0; i < totalVertices; i++)
		{
			// Check which triangle use this vertex
			for (int j = 0; j < meshTriangles; j++)
			{
				if (model->GetIndices()[j * (long long)3] == i || model->GetIndices()[(j * (long long)3) + 1] == i || model->GetIndices()[(j * (long long)3) + 2] == i)
				{
					tX = XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = XMVectorSet(tX, tY, tZ, 0.0f);

					faceUsing++;
				}
			}

			// Get the actual normal by dividing the sum by the number of faces sharing the vertex
			normalSum = normalSum / (float)faceUsing;

			// Normalize the normal vector
			normalSum = XMVector3Normalize(normalSum);

			// Store the normal in the current vertex
			model->GetVertices()[i].normal.x = XMVectorGetX(normalSum);
			model->GetVertices()[i].normal.y = XMVectorGetY(normalSum);
			model->GetVertices()[i].normal.z = XMVectorGetZ(normalSum);

			// Clear the variables for next vertex
			normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			faceUsing = 0;

		}
	}

	model->SetVertexCount((int)model->GetVertices().size());
	model->SetIndexCount((int)model->GetIndices().size());

	// Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &model->GetIndices()[0];
	this->hr = device->CreateBuffer(&indexBufferDesc, &iinitData, &indexBuffer);
	assert(SUCCEEDED(this->hr));

	// Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &model->GetVertices()[0];
	this->hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);
	assert(SUCCEEDED(this->hr));

	model->SetIndexBuffer(indexBuffer);
	model->SetVertexBuffer(vertexBuffer);

	return true;
}
