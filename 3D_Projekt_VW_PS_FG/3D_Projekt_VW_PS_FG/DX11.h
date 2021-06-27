#pragma once

/*
	LINKINGS
*/

#pragma comment(lib, "gdi32")
#pragma comment(lib, "d3d11") 
#pragma comment( lib, "dxgi" )   
#pragma comment(lib, "d3dcompiler.lib")

/* 
	Win and DX Headers 
*/

#include <Windows.h>
#include <d3d11.h>          
#include <dxgi.h>          
#include <d3dcompiler.h>  

/* 
	Useful headers for textures, math and debug ..
*/

#include <assert.h>
#include <DirectXMath.h>
using namespace DirectX;

class DX11
{
public:
	DX11();
	DX11(const DX11&);
	~DX11();

	bool Initialize(int screenWidth, int screenHeight, HWND hwnd, float screenDepth, float screenNear);
	void Shutdown();

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

	HWND& GetHwnd() { return this->hwnd; }
	ID3D11Device* GetDevice() const { return this->device; }
	ID3D11DeviceContext* GetContext() const { return this->context; }

	/*ID3D11RasterizerState* GetRasterizerStateCullBack() { return this->pRasterizerState_CullBack; }
	ID3D11RasterizerState* GetRasterizerStateCullNone() { return this->pRasterizerState_CullNone; }
	ID3D11RasterizerState* GetRasterizerStateCullFront() { return this->pRasterizerState_CullFront; }*/

	D3D11_VIEWPORT& GetViewport() { return this->viewport; }
	ID3D11RenderTargetView*& GetRenderTarget() { return this->renderTargetView; }
	ID3D11DepthStencilView*& GetDepthStencilView() { return this->depthStencilView; }
	ID3D11DepthStencilState* GetDepthStencilState() { return this->depthState_lessEqual; }

	ID3D11SamplerState* GetMinMagMipSampler() { return this->minmagmipLin; }
	ID3D11SamplerState* GetAnisotropicSampler() { return this->anisotropic; }

	void GetProjectionMatrix(DirectX::XMMATRIX&);
	void GetCubemapProjectionMatrix(DirectX::XMMATRIX&);
	void GetWorldMatrix(DirectX::XMMATRIX&);

	void EnableAlphaBlending();
	void DisableAlphaBlending();

private:
	HWND hwnd;
	IDXGISwapChain* swapchain;
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	/*ID3D11RasterizerState* pRasterizerState_CullBack;
	ID3D11RasterizerState* pRasterizerState_CullFront;
	ID3D11RasterizerState* pRasterizerState_CullNone;*/

	D3D11_VIEWPORT viewport;
	ID3D11RenderTargetView* renderTargetView;	
	ID3D11DepthStencilState* depthState_lessEqual;
	ID3D11DepthStencilView* depthStencilView;

	ID3D11SamplerState* minmagmipLin;
	ID3D11SamplerState* anisotropic;	

	ID3D11BlendState* alphaEnableBlendingState;
	ID3D11BlendState* alphaDisableBlendingState;

	HRESULT hr;

	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX cubeProjectionMatrix;
};

/* Template to release pointers */
template<typename R>
inline void ReleasePtr(R& ptr)
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
};