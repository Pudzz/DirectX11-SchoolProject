#include "Light.h"

Light::Light()
{
}

Light::Light(const Light&)
{
}

Light::~Light()
{
}

void Light::SetAmbientColor(float r, float g, float b, float a)
{
	ambientColor = DirectX::XMFLOAT4(r, g, b, a);
}

void Light::SetDiffuseColor(float r, float g, float b, float a)
{
	diffuseColor = DirectX::XMFLOAT4(r, g, b, a);
}

void Light::SetSpecularColor(float r, float g, float b, float a)
{
	specularColor = DirectX::XMFLOAT4(r, g, b, a);
}

void Light::SetLightPosition(float x, float y, float z)
{
	position = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetLightRange(float range)
{
	lightRange = range;
}

void Light::SetLightAttentuation(float a1, float a2, float a3)
{
	attenuation = DirectX::XMFLOAT3(a1, a2, a3);
}

DirectX::XMFLOAT4 Light::GetAmbientColor()
{
	return this->ambientColor;
}

DirectX::XMFLOAT4 Light::GetDiffuseColor()
{
	return this->diffuseColor;
}

DirectX::XMFLOAT4 Light::GetSpecularColor()
{
	return this->specularColor;
}

DirectX::XMFLOAT3 Light::GetLightPosition()
{
	return this->position;
}

float Light::GetLightRange()
{
	return this->lightRange;
}

DirectX::XMFLOAT3 Light::GetLightAttenuation()
{
	return this->attenuation;
}
