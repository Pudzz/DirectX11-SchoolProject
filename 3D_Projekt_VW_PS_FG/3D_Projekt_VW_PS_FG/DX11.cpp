#include "DX11.h"

DX11::DX11()
{    
    ZeroMemory(&hwnd, sizeof(HWND));  
    this->swapchain = 0;
    this->device = 0;
    this->context = 0;

   /* this->pRasterizerState_CullBack = 0;
    this->pRasterizerState_CullFront = 0;
    this->pRasterizerState_CullNone = 0;*/

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    this->renderTargetView = 0;
    this->depthState_lessEqual = 0;
    this->depthStencilView = 0;

    this->anisotropic = 0;
    this->minmagmipLin = 0;

    this->alphaEnableBlendingState = 0;
    this->alphaDisableBlendingState = 0;

    this->hr = 0;

    this->projectionMatrix = DirectX::XMMatrixIdentity();
    this->worldMatrix = DirectX::XMMatrixIdentity();    
}

DX11::DX11(const DX11& other)
{
    this->hwnd = other.hwnd;
    this->swapchain = other.swapchain;
    this->device = other.device;
    this->context = other.context;

    //this->pRasterizerState_CullBack = other.pRasterizerState_CullBack;
    //this->pRasterizerState_CullFront = other.pRasterizerState_CullFront;
    //this->pRasterizerState_CullNone = other.pRasterizerState_CullNone;

    this->viewport = other.viewport;
    this->renderTargetView = other.renderTargetView;
    this->depthState_lessEqual = other.depthState_lessEqual;
    this->depthStencilView = other.depthStencilView;

    this->anisotropic = other.anisotropic;
    this->minmagmipLin = other.minmagmipLin;

    this->alphaEnableBlendingState = other.alphaEnableBlendingState;
    this->alphaDisableBlendingState = other.alphaDisableBlendingState;

    this->hr = other.hr;

    this->projectionMatrix = other.projectionMatrix;
    this->worldMatrix = other.worldMatrix;
}

DX11::~DX11()
{
}

bool DX11::Initialize(int screenWidth, int screenHeight, HWND hwnd, float screenDepth, float screenNear)
{

    /*
        Resize window screenwidth and height to clientsize.
        Create a swap chain description.
        Create swap chain.
    */

    this->hwnd = hwnd;
    RECT windowRect = { 0,0, screenWidth, screenHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    DXGI_SWAP_CHAIN_DESC swapChainDescription;
    ZeroMemory(&swapChainDescription, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDescription.BufferCount                        = 1;
    swapChainDescription.BufferDesc.Width                   = windowRect.right - windowRect.left;
    swapChainDescription.BufferDesc.Height                  = windowRect.bottom - windowRect.top;
    swapChainDescription.BufferDesc.RefreshRate.Numerator   = 60;
    swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDescription.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDescription.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDescription.SampleDesc.Count   = 1u;
    swapChainDescription.SampleDesc.Quality = 0;
    swapChainDescription.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    swapChainDescription.OutputWindow       = hwnd;
    swapChainDescription.Windowed           = true;
    swapChainDescription.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDescription.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;   // Allow mode switch so we can use ResizeTarget ( From windowed -> fullscreen ) 

    UINT swapflags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef  _DEBUG
    swapflags |= D3D11_CREATE_DEVICE_DEBUG; // Is set to enable the debug layer
#endif

    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };
    hr = D3D11CreateDeviceAndSwapChain( nullptr,
                                        D3D_DRIVER_TYPE_HARDWARE,
                                        nullptr,
                                        swapflags,
                                        featureLevel,
                                        1,
                                        D3D11_SDK_VERSION,
                                        &swapChainDescription,
                                        &swapchain,
                                        &device,
                                        nullptr,
                                        &context);
    assert(SUCCEEDED(hr));


    /*
        Get backbuffer from swapchain.
        Create rendertargetview with the backbuffer.
    */

    ID3D11Texture2D* backbufferPtr;
    hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbufferPtr);
    assert(SUCCEEDED(hr));
  
    hr = device->CreateRenderTargetView(backbufferPtr, nullptr, &renderTargetView);
    assert(SUCCEEDED(hr));

    backbufferPtr->Release();
    backbufferPtr = 0;


    /*
        Create a texture2d description for z-buffer.
        Create texture based on description.
    */

    ID3D11Texture2D* depthStencilBuffer;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    depthBufferDesc.Width               = windowRect.right - windowRect.left;
    depthBufferDesc.Height              = windowRect.bottom - windowRect.top;
    depthBufferDesc.MipLevels           = 1;
    depthBufferDesc.ArraySize           = 1;
    depthBufferDesc.SampleDesc.Count    = 1;
    depthBufferDesc.SampleDesc.Quality  = 0;
    depthBufferDesc.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags      = 0;
    depthBufferDesc.MiscFlags           = 0;

    hr = device->CreateTexture2D(&depthBufferDesc, 0, &depthStencilBuffer);
    assert(SUCCEEDED(hr));


    /*
        Create depth and stencil state based on description. 
        Set deptchstencil state to output merger. 
    */

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    
    depthStencilDesc.DepthEnable        = true;
    depthStencilDesc.DepthWriteMask     = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc          = D3D11_COMPARISON_LESS_EQUAL;   // LESS OR LESS EQUAL

    // Set up the description of the stencil state.
    depthStencilDesc.StencilEnable      = false;
    depthStencilDesc.StencilReadMask    = 0xFF;
    depthStencilDesc.StencilWriteMask   = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp        = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc          = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp         = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp    = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilPassOp         = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc           = D3D11_COMPARISON_ALWAYS;
        
    hr = device->CreateDepthStencilState(&depthStencilDesc, &depthState_lessEqual);
    assert(SUCCEEDED(hr));
    
    context->OMSetDepthStencilState(depthState_lessEqual, 1);


    /*
        Create the depth stencil view with texture and description.
        Bind it to the rendertargetview. 
        Release depthtexture ptr. 
    */

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;    
	
    hr = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
    assert(SUCCEEDED(hr));    

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    depthStencilBuffer->Release();
    depthStencilBuffer = 0;

    /*
        Create description for meshes culling and filling mode.  (back, front, none. | Solid or wireframe )
        *Rasterizer state is set to backculling as standard.*
        
        ** Rasterizer state IT OFF RIGHT NOW. **
    */

   /* D3D11_RASTERIZER_DESC rasterizerStateDesc;
    ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));

    rasterizerStateDesc.AntialiasedLineEnable   = false;
    rasterizerStateDesc.MultisampleEnable       = false;
    rasterizerStateDesc.ScissorEnable           = false;
    rasterizerStateDesc.DepthClipEnable         = true;

    rasterizerStateDesc.CullMode                = D3D11_CULL_BACK;
    rasterizerStateDesc.FillMode                = D3D11_FILL_SOLID;
    rasterizerStateDesc.FrontCounterClockwise   = false;

    rasterizerStateDesc.DepthBias               = 0;
    rasterizerStateDesc.DepthBiasClamp          = 0.0f;
    rasterizerStateDesc.SlopeScaledDepthBias    = 0.0f;*/

    //// Cull Back
    //hr = device->CreateRasterizerState(&rasterizerStateDesc, &pRasterizerState_CullBack);
    //if (FAILED(hr)) {
    //    MessageBox(0, L"Failed to 'CreateRasterizerState'- CullBack.", L"Graphics scene Initialization Message", MB_ICONERROR);
    //}

    //// Cull Front 
    //rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
    //rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
    //hr = device->CreateRasterizerState(&rasterizerStateDesc, &pRasterizerState_CullFront);
    //if (FAILED(hr)) {
    //    MessageBox(0, L"Failed to 'CreateRasterizerState'. - CullFront", L"Graphics scene Initialization Message", MB_ICONERROR);
    //}

    //// Cull None
    //rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
    //rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
    //hr = device->CreateRasterizerState(&rasterizerStateDesc, &pRasterizerState_CullNone);
    //if (FAILED(hr)) {
    //    MessageBox(0, L"Failed to 'CreateRasterizerState'. - CullNone", L"Graphics scene Initialization Message", MB_ICONERROR);
    //}

    //context->RSSetState(pRasterizerState_CullBack);


    /*
        VIEWPORT with clientsizes.
        Is set to main viewport with devicecontext.
    */
    
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX   = 0.0f;
    viewport.TopLeftY   = 0.0f;
    viewport.Width      = (float)windowRect.right - windowRect.left;
    viewport.Height     = (float)windowRect.bottom - windowRect.top;
    viewport.MinDepth   = 0.0f;
    viewport.MaxDepth   = 1.0f;

    context->RSSetViewports(1, &viewport);


    /*
        Set projection matrix settings.
        FoV, aspectradio based on client sizes, near and far depth. 

        Set worldmatrix to identity matrix for later use. 
    */

    float clientWidth = (float)windowRect.right - windowRect.left;
    float clientheight = (float)windowRect.bottom - windowRect.top;

    float fovInDegrees = 90.0f;  
    float fovInRadians = (fovInDegrees / 360.0f) * 3.14f;
    float aspectRatio = (float)clientWidth / (float)clientheight;
    float nearZ = screenNear;
    float farZ = screenDepth;

    projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovInRadians, aspectRatio, nearZ, farZ);
    cubeProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
    worldMatrix = DirectX::XMMatrixIdentity();


    /* 
        Create two blendstate pointers to switch between.
        So we can blend textures such as pngs. Kind of transparency.. 
    */

    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
    blendStateDesc.RenderTarget[0].BlendEnable = true;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

    hr = device->CreateBlendState(&blendStateDesc, &alphaEnableBlendingState);
    if(FAILED(hr))
        return false;

    blendStateDesc.RenderTarget[0].BlendEnable = false;

    hr = device->CreateBlendState(&blendStateDesc, &alphaDisableBlendingState);
    if (FAILED(hr))
        return false;


    /*
        Create a texture sampler state description. ( how textures should be sampled through a mesh. )
        We create 2 different, minmagmip-linear and anisotropic. ( Anisotropic for skybox atm (no difference) )
    */
    
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias      = 0.0f;
    samplerDesc.MaxAnisotropy   = 1;
    samplerDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0]  = 0;
    samplerDesc.BorderColor[1]  = 0;
    samplerDesc.BorderColor[2]  = 0;
    samplerDesc.BorderColor[3]  = 0;
    samplerDesc.MinLOD          = 0;
    samplerDesc.MaxLOD          = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&samplerDesc, &minmagmipLin);
    if (FAILED(hr))
    {
        return false;
    }    

    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter          = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias      = 0.0f;
    samplerDesc.MaxAnisotropy   = 4;
    samplerDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0]  = 0;
    samplerDesc.BorderColor[1]  = 0;
    samplerDesc.BorderColor[2]  = 0;
    samplerDesc.BorderColor[3]  = 0;
    samplerDesc.MinLOD          = FLT_MIN;
    samplerDesc.MaxLOD          = FLT_MAX;

    hr = device->CreateSamplerState(&samplerDesc, &anisotropic);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void DX11::Shutdown()
{
    ReleasePtr(anisotropic);
    ReleasePtr(minmagmipLin);
    ReleasePtr(device);
    ReleasePtr(context);
    ReleasePtr(swapchain);
    /*ReleasePtr(pRasterizerState_CullBack);
    ReleasePtr(pRasterizerState_CullFront);
    ReleasePtr(pRasterizerState_CullNone);*/
    ReleasePtr(renderTargetView);
    ReleasePtr(depthState_lessEqual);
    ReleasePtr(depthStencilView);
    ReleasePtr(alphaEnableBlendingState);
    ReleasePtr(alphaDisableBlendingState);
}

void DX11::BeginScene(float red, float green, float blue, float alpha)
{
    float color[4] = { red, green, blue, alpha };

    // Clear the back buffer and depth buffer
    context->ClearRenderTargetView(renderTargetView, color);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DX11::EndScene()
{
    swapchain->Present(1, 0);
}

void DX11::GetProjectionMatrix(DirectX::XMMATRIX& projMatrix)
{
    projMatrix = this->projectionMatrix;
}

void DX11::GetCubemapProjectionMatrix(DirectX::XMMATRIX& cubeProj)
{
    cubeProj = this->cubeProjectionMatrix;
}

void DX11::GetWorldMatrix(DirectX::XMMATRIX& world)
{
    world = this->worldMatrix;
}

void DX11::EnableAlphaBlending()
{
    float blendfact[4] = { 0.0f,0.0f,0.0f,0.0f };
    context->OMSetBlendState(alphaEnableBlendingState, blendfact, 0xffffffff);
}

void DX11::DisableAlphaBlending()
{
    float blendfact[4] = { 0.0f,0.0f,0.0f,0.0f };
    context->OMSetBlendState(alphaDisableBlendingState, blendfact, 0xffffffff);
}
