#pragma once
#include "DX11.h"
#include "Models.h"
#include "Light.h"
#include "Camera.h"

class Shader{
private:
	/* Patricle system cbuffer */
	__declspec(align(16))
	struct cBufferPerObjectParticles
	{
		cBufferPerObjectParticles() { ZeroMemory(this, sizeof(this)); }
		DirectX::XMMATRIX worldViewProj;
		DirectX::XMMATRIX world;
	};

	__declspec(align(16))
	struct cBufferPerObject
	{
		cBufferPerObject() { ZeroMemory(this, sizeof(this)); }

		DirectX::XMMATRIX worldViewProj;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX InverseWorld;
		DirectX::XMMATRIX boneTransforms[50];
	};

	__declspec(align(16))
	struct cBufferCamera {
		
		DirectX::XMFLOAT3 cameraPosition;
		float padding;
	};

	__declspec(align(16))
	struct cBufferLight
	{
		DirectX::XMFLOAT4 ambientLightColor;
		DirectX::XMFLOAT4 diffuseLightColor;
		DirectX::XMFLOAT4 specularLightColor;
		DirectX::XMFLOAT3 lightPosition;
		float lightRange;

		DirectX::XMFLOAT3 lightAttenuation;
		float padding;
	};

	__declspec(align(16))
	struct cBufferMaterial
	{
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 specularColor;
		DirectX::XMFLOAT4 reflectionColor;

		float shine;
		int hasTexture;
		int hasReflection;
		int isTerrain;		
		
		int canMove;
		DirectX::XMFLOAT3 padding2;

		DirectX::XMFLOAT4 translation;

		int hasNormMap;
		DirectX::XMFLOAT3 padding3;

		int isObj;
		DirectX::XMFLOAT3 padding4;
	};

public:
	Shader(ID3D11Device* device);
	Shader(const Shader&);
	~Shader();

	bool InitializeParticleShaders(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryPS);
	bool InitializeShaders(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryPS);
	bool InitializeShaderWithGeometryShader(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR gsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryGS, LPCSTR entryPS);

	bool CreateDefaultInputLayout(ID3D11Device* device);
	bool CreateSkyboxInputLayout(ID3D11Device* device, ID3D11DeviceContext* context);
	void Shutdown();

	bool Render(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, Camera* camera, Light* light, ID3D11SamplerState* sampler);
	bool RenderWithCubemap(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* cubemap, Camera* camera, Light* light, ID3D11SamplerState* sampler);
	bool RenderParticles(ID3D11DeviceContext* context, int indexCount, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* texture, ID3D11SamplerState* sampler);

private:	
	void ShutdownShader();

	bool SetCBuffers(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, Camera* camera, Light* light);
	bool SetCBuffersWithCubemap(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* cubemap, Camera* camera, Light* light);
	bool SetCBufferParticles(ID3D11DeviceContext* context, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* texture);
	
	void RenderShader(ID3D11DeviceContext*, int, ID3D11SamplerState* sampler);

private:
	HRESULT hr;
	ID3D11VertexShader* vertexShader;
	ID3D11GeometryShader* geometryShader; 
	ID3D11PixelShader* pixelShader;

	ID3D11InputLayout* inputLayout;		

	ID3DBlob* ErrorBlob;
	ID3DBlob* VSBlob;
	ID3DBlob* GSBlob; 
	ID3DBlob* PSBlob;

	cBufferPerObjectParticles cbPerObjectParticles;	// Particles
	cBufferPerObject cbPerObject;
	cBufferCamera cbPerCamera;
	cBufferLight cbPerLight;
	cBufferMaterial cbPerMaterial;

	ID3D11Buffer* bufferPerObjectParticles;	// Particles
	ID3D11Buffer* bufferPerObject;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* materialBuffer;

	ID3D11ShaderResourceView* cubeMap;
	ID3D11ShaderResourceView* normalMapSRV;
	ID3D11ShaderResourceView* texture;

	ID3D11Device* dx11;
};