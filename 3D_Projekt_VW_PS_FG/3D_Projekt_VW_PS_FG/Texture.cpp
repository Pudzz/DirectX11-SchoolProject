#include "Texture.h"

Texture::Texture()
{
	this->hr = 0;
	this->texture = 0;
}

Texture::Texture(const Texture& other)
{
	this->hr = other.hr;
	this->texture = other.texture;
}

Texture::~Texture()
{
}

bool Texture::InitializeSkyboxTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCWSTR textureFilename)
{
	//Tell D3D We have a cube texture, which is an array of 2D textures
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = 1;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = CreateDDSTextureFromFileEx(device, context, textureFilename, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE, false, nullptr, &texture); // change false to SMViewDesc
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}
	return true;
}

bool Texture::Initialize(ID3D11Device* device, LPCWSTR textureFilepath)
{	
	hr = DirectX::CreateWICTextureFromFile(device, textureFilepath, nullptr, &texture);
	if (FAILED(hr))
		return false;

	return true;
}

void Texture::Shutdown()
{
	if (texture)
	{		
		texture->Release();
		texture = 0;
	}
}

bool Texture::CreateDDSTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCWSTR textureFilename)
{
	//Tell D3D We have a cube texture, which is an array of 2D textures
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = 1;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = CreateDDSTextureFromFileEx(device, context, textureFilename, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE, false, nullptr, &texture); // change false to SMViewDesc
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}

	return true;
}
