#pragma once
#include <DirectXMath.h>

class Light
{
public:
	Light();
	Light(const Light&);
	~Light();

	void SetAmbientColor(float r, float g, float b, float a);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetLightPosition(float x, float y, float z);
	void SetLightRange(float range);
	void SetLightAttentuation(float a1, float a2, float a3);
		
	DirectX::XMFLOAT4 GetAmbientColor();
	DirectX::XMFLOAT4 GetDiffuseColor();
	DirectX::XMFLOAT4 GetSpecularColor();
	DirectX::XMFLOAT3 GetLightPosition();
	float GetLightRange();
	DirectX::XMFLOAT3 GetLightAttenuation();

private:
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 specularColor;
	DirectX::XMFLOAT3 position;
	float lightRange;

	DirectX::XMFLOAT3 attenuation;
};