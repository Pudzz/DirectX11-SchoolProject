#pragma once
#include <vector>
#include <fstream>
#include <istream>
#include <sstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <assert.h>
#include <WICTextureLoader.h>
#include "Models.h"
#include <string>

using namespace DirectX;
using namespace std;

class objReader
{
private:
	HRESULT hr;
public:
	objReader();
	~objReader();

	bool loadObj(Models* model, ID3D11Device* device, wstring fileName, bool isRightHanded, bool computeNormals);
};

