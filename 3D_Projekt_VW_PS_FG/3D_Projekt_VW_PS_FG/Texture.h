#pragma once
#include "DX11.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

#include <string>

class Texture
{
public:
	Texture();
	Texture(const Texture&);
	~Texture();

	bool InitializeSkyboxTexture(ID3D11Device*, ID3D11DeviceContext* context, LPCWSTR textureFilename);
	bool Initialize(ID3D11Device*, LPCWSTR);

	void Shutdown();
	bool CreateDDSTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCWSTR textureFilename);
	ID3D11ShaderResourceView* GetTexture() { return this->texture; };
	void SetTexture(ID3D11ShaderResourceView* resource) { this->texture = resource; }
	void SetName(std::string name) { this->name = name; }
	std::string GetName() { return this->name; }

private:
	HRESULT hr;
	ID3D11ShaderResourceView* texture;
	std::string name;
};