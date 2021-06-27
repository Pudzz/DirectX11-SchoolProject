#include "Shader.h"

Shader::Shader(ID3D11Device* device)
{
	this->hr = 0;
	this->vertexShader = 0;
	this->pixelShader = 0;
	this->geometryShader = 0;
	this->inputLayout = 0;

	this->dx11 = device;
	this->bufferPerObject = 0;
	this->cameraBuffer = 0;
	this->lightBuffer = 0;
	this->materialBuffer = 0;
	this->bufferPerObjectParticles = 0;

	this->VSBlob = 0;
	this->PSBlob = 0;
	this->GSBlob = 0;
	this->ErrorBlob = 0;

	this->cubeMap = 0;
	this->normalMapSRV = 0;
	this->texture = 0;

	ZeroMemory(&cbPerCamera, sizeof(cBufferCamera));
	ZeroMemory(&cbPerLight, sizeof(cBufferLight));
	ZeroMemory(&cbPerMaterial, sizeof(cBufferMaterial));
	ZeroMemory(&cbPerObject, sizeof(cBufferPerObject));
	ZeroMemory(&cbPerObjectParticles, sizeof(bufferPerObjectParticles));
}

Shader::Shader(const Shader& other)
{
	hr = other.hr;
	vertexShader = other.vertexShader;
	pixelShader = other.pixelShader;
	geometryShader = other.geometryShader;
	inputLayout = other.inputLayout;

	dx11 = other.dx11;
	bufferPerObject = other.bufferPerObject;
	cameraBuffer = other.cameraBuffer;
	lightBuffer = other.lightBuffer;
	materialBuffer = other.materialBuffer;
	bufferPerObjectParticles = other.bufferPerObjectParticles;

	VSBlob = other.VSBlob;
	PSBlob = other.PSBlob;
	GSBlob = other.GSBlob;
	ErrorBlob = other.ErrorBlob;

	cubeMap = other.cubeMap;
	normalMapSRV = other.normalMapSRV;
	texture = other.texture;

	cbPerCamera = other.cbPerCamera;
	cbPerLight = other.cbPerLight;
	cbPerMaterial = other.cbPerMaterial;
	cbPerObject = other.cbPerObject;
	cbPerObjectParticles = other.cbPerObjectParticles;
}

Shader::~Shader()
{
}

void Shader::Shutdown()
{
	ShutdownShader();
}

bool Shader::Render(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, Camera* camera, Light* light, ID3D11SamplerState* sampler)	// CAM AND LIGHT
{
	/*
		Set cbuffer parameters and render object. 
	*/

	bool result;	
	result = SetCBuffers(context, model, view, projection, camera, light);
	if (!result) {
		return false;
	}

	RenderShader(context, model->GetIndexCount(), sampler);
	return true;
}

bool Shader::RenderWithCubemap(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* cubemap, Camera* camera, Light* light, ID3D11SamplerState* sampler)
{
	/*
		Set cbuffer parameters and render object with cubemap texture.
	*/

	bool result;
	result = SetCBuffersWithCubemap(context, model, view, projection, cubemap, camera, light);
	if (!result) {
		return false;
	}

	RenderShader(context, model->GetIndexCount(), sampler);
	return true;		
}

bool Shader::RenderParticles(ID3D11DeviceContext* context, int indexCount, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* texture, ID3D11SamplerState* sampler)
{
	/*
		Set cbuffer parameters and render object with cubemap texture.
	*/

	bool result;
	result = SetCBufferParticles(context, world, view, projection, texture);
	if (!result) {
		return false;
	}

	RenderShader(context, indexCount, sampler);
	return true;
}

bool Shader::InitializeParticleShaders(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryPS)
{
	/*
		COMPILE VERTEX AND PIXEL SHADERS, THEN CREATE THEM
	*/

	hr = D3DCompileFromFile(vsFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryVS, "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &VSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (VSBlob) {
			ReleasePtr(VSBlob);
		}
		assert(false);

		return false;
	}

	hr = D3DCompileFromFile(psFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPS, "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &PSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (PSBlob) {
			ReleasePtr(PSBlob);
		}
		assert(false);

		return false;
	}

	// Create Vertexshader
	hr = device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Create pixelshader
	hr = device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(hr))
	{
		return false;
	}

	/*
		CREATE INPUT LAYOUT
	*/

	D3D11_INPUT_ELEMENT_DESC INPUT_LAYOUT_V_UV_C[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	 D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = device->CreateInputLayout(INPUT_LAYOUT_V_UV_C, ARRAYSIZE(INPUT_LAYOUT_V_UV_C), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &inputLayout);
	if (FAILED(hr))
	{
		return false;
	}

	ReleasePtr(VSBlob);
	ReleasePtr(PSBlob);



	/*
		CONSTANT BUFFER , CBPEROBJECT/Camerabuffer/Lightbuffer
	*/

	D3D11_BUFFER_DESC cBufferDescription;

	// Setup the description of the bufferPerObject constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(D3D11_BUFFER_DESC));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferPerObjectParticles) + (16 - (sizeof(cBufferPerObjectParticles) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &bufferPerObjectParticles);
	if (FAILED(hr))
	{
		return false;
	}

	// Setup the description of the camera constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferCamera) + (16 - (sizeof(cBufferCamera) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &cameraBuffer);
	if (FAILED(hr))
	{
		return false;
	}
		

	return true;
}

bool Shader::InitializeShaders(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryPS)
{
	
	/*
		COMPILE VERTEX AND PIXEL SHADERS, THEN CREATE THEM
	*/

	hr = D3DCompileFromFile(vsFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryVS, "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &VSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (VSBlob) {
			ReleasePtr(VSBlob);
		}
		assert(false);

		return false;
	}
		
	hr = D3DCompileFromFile(psFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPS, "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &PSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (PSBlob) {
			ReleasePtr(PSBlob);
		}
		assert(false);

		return false;
	}

	// Create Vertexshader
	hr = device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Create pixelshader
	hr = device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(hr))
	{
		return false;
	}



	/*
		CONSTANT BUFFER , CBPEROBJECT/Camerabuffer/Lightbuffer
	*/

	D3D11_BUFFER_DESC cBufferDescription;

	// Setup the description of the bufferPerObject constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(D3D11_BUFFER_DESC));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferPerObject) + (16 - (sizeof(cBufferPerObject) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &bufferPerObject);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the camera constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferCamera) + (16 - (sizeof(cBufferCamera) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &cameraBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the light constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferLight) + (16 - (sizeof(cBufferLight) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &lightBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the light constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferMaterial) + (16 - (sizeof(cBufferMaterial) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &materialBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Shader::InitializeShaderWithGeometryShader(ID3D11Device* device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR gsFilename, LPCWSTR psFilename, LPCSTR entryVS, LPCSTR entryGS, LPCSTR entryPS)
{
	/*
		COMPILE VERTEX AND PIXEL SHADERS, THEN CREATE THEM
	*/

	hr = D3DCompileFromFile(vsFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryVS, "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &VSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (VSBlob) {
			ReleasePtr(VSBlob);
		}
		assert(false);

		return false;
	}

	hr = D3DCompileFromFile(gsFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryGS, "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &GSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, gsFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (GSBlob) {
			ReleasePtr(GSBlob);
		}
		assert(false);

		return false;
	}

	hr = D3DCompileFromFile(psFilename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPS, "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, NULL, &PSBlob, &ErrorBlob);
	if (FAILED(hr)) {
		if (ErrorBlob) {
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			ReleasePtr(ErrorBlob);
		}

		if (PSBlob) {
			ReleasePtr(PSBlob);
		}
		assert(false);

		return false;
	}

	hr = device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreateGeometryShader(GSBlob->GetBufferPointer(), GSBlob->GetBufferSize(), nullptr, &this->geometryShader);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(hr))
	{
		return false;
	}


	/*
		CONSTANT BUFFER , CBPEROBJECT/Camerabuffer/Lightbuffer/Materialbuffer
	*/

	D3D11_BUFFER_DESC cBufferDescription;

	// Setup the description of the bufferPerObject constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(D3D11_BUFFER_DESC));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferPerObject) + (16 - (sizeof(cBufferPerObject) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &bufferPerObject);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the camera constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferCamera) + (16 - (sizeof(cBufferCamera) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &cameraBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the light constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferLight) + (16 - (sizeof(cBufferLight) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &lightBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	// Setup the description of the material constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(cBufferDescription));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferMaterial) + (16 - (sizeof(cBufferMaterial) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	hr = device->CreateBuffer(&cBufferDescription, NULL, &materialBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Shader::CreateDefaultInputLayout(ID3D11Device* device)
{
	/*
		CREATE INPUT LAYOUT
	*/

	D3D11_INPUT_ELEMENT_DESC INPUT_LAYOUT_V_UV_N_T[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	 D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEWEIGHTS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				
	};

	hr = device->CreateInputLayout(INPUT_LAYOUT_V_UV_N_T, ARRAYSIZE(INPUT_LAYOUT_V_UV_N_T), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &inputLayout);
	if (FAILED(hr))
	{
		return false;
	}

	ReleasePtr(VSBlob);
	ReleasePtr(PSBlob);

	return true;
}

bool Shader::CreateSkyboxInputLayout(ID3D11Device* device, ID3D11DeviceContext* context)
{
	/* Input layout for skyVertex */
	D3D11_INPUT_ELEMENT_DESC skyinputLayout[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = device->CreateInputLayout(skyinputLayout, ARRAYSIZE(skyinputLayout), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &inputLayout);
	if (FAILED(hr))
	{
		return false;
	}

	ReleasePtr(VSBlob);
	ReleasePtr(PSBlob);

	hr = CreateDDSTextureFromFileEx(device, context, L"Textures/skymap.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE, false, nullptr, &cubeMap); // change false to SMViewDesc
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}	

	return true;
}

void Shader::ShutdownShader()
{
	if (bufferPerObjectParticles)
	{
		bufferPerObjectParticles->Release();
		bufferPerObjectParticles = 0;
	}

	// Release the constant buffers.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	if (bufferPerObject)
	{
		bufferPerObject->Release();
		bufferPerObject = 0;
	}

	if (materialBuffer)
	{
		materialBuffer->Release();
		materialBuffer = 0;
	}

	// Release the layout.
	if (inputLayout)
	{
		inputLayout->Release();
		inputLayout = 0;
	}

	// Release the pixel shader.
	if (pixelShader)
	{
		pixelShader->Release();
		pixelShader = 0;
	}

	// Release the vertex shader.
	if (vertexShader)
	{
		vertexShader->Release();
		vertexShader = 0;
	}

	// Release the geometry shader.
	if (geometryShader)
	{
		geometryShader->Release();
		geometryShader = 0;
	}

	if (cubeMap) {
		cubeMap->Release();
		cubeMap = 0;
	}

	if (ErrorBlob)
	{
		ReleasePtr(ErrorBlob);
	}

	if (VSBlob)
	{
		ReleasePtr(VSBlob);
	}

	if (PSBlob)
	{
		ReleasePtr(PSBlob);
	}

	if (GSBlob)
	{
		ReleasePtr(GSBlob);
	}

}

bool Shader::SetCBuffers(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, Camera* camera, Light* light)
{	
	DirectX::XMMATRIX worldViewProjection;
	worldViewProjection = model->GetWorldMatrix() * view * projection;

	cbPerObject.worldViewProj = DirectX::XMMatrixTranspose(worldViewProjection);
	cbPerObject.world = DirectX::XMMatrixTranspose(model->GetWorldMatrix());
	cbPerObject.InverseWorld = DirectX::XMMatrixInverse(nullptr, model->GetWorldMatrix());

	if (model->GetSkeleton())
	{
		if (model->GetSkeleton()->GetBoneAmount() > 0)
		{
			for (unsigned int i = 0; i < model->GetSkeleton()->GetBoneAmount(); i++)
			{
				int keyframe = model->GetSkeleton()->GetCurrentKeyframe();
				cbPerObject.boneTransforms[i] = model->GetSkeleton()->GetCurrentAnimation()->GetBonesVector()[i].GetFinalTransformationMatrix(keyframe);
			}
		}
	}

	context->UpdateSubresource(bufferPerObject, 0, nullptr, &cbPerObject, 0, 0);
	context->VSSetConstantBuffers(0, 1, &bufferPerObject);

	// Set shader texture resource in the pixel shader.	
	if (model->GetMaterial()[0].hasTexture) {
		texture = model->GetTexture();
		context->PSSetShaderResources(0, 1, &texture);
	}

	if (model->GetMaterial()[0].hasNormalMap) {
		normalMapSRV = model->GetNormalMap();
		context->PSSetShaderResources(2, 1, &normalMapSRV);
	}


	/*	
		Set Camera buffer	// To Vertexshader
	*/
	cbPerCamera.cameraPosition = camera->GetPosition();

	context->UpdateSubresource(cameraBuffer, 0, nullptr, &cbPerCamera, 0, 0);
	context->VSSetConstantBuffers(1, 1, &cameraBuffer);
	context->GSSetConstantBuffers(0, 1, &cameraBuffer);

	/*
		Set Light buffer	// To Pixelshader
	*/
	cbPerLight.ambientLightColor = light->GetAmbientColor();
	cbPerLight.diffuseLightColor = light->GetDiffuseColor();
	cbPerLight.specularLightColor = light->GetSpecularColor();
	cbPerLight.lightPosition = light->GetLightPosition();
	cbPerLight.lightRange = light->GetLightRange();
	cbPerLight.lightAttenuation = light->GetLightAttenuation();

	context->UpdateSubresource(lightBuffer, 0, nullptr, &cbPerLight, 0, 0);
	context->PSSetConstantBuffers(0, 1, &lightBuffer);


	/*
		MATERIALEEE			// To Pixelshader
	*/
	cbPerMaterial.ambientColor = model->GetMaterial()[0].ambientColor;
	cbPerMaterial.diffuseColor = model->GetMaterial()[0].diffuseColor;
	cbPerMaterial.specularColor = model->GetMaterial()[0].specularColor;
	cbPerMaterial.reflectionColor = model->GetMaterial()[0].reflectionColor;
	cbPerMaterial.shine = model->GetMaterial()[0].shine;
	cbPerMaterial.hasTexture = model->GetMaterial()[0].hasTexture;
	cbPerMaterial.hasReflection = model->GetMaterial()[0].hasReflection;
	cbPerMaterial.isTerrain = model->GetMaterial()[0].isTerrain;
	cbPerMaterial.canMove = model->GetMaterial()[0].canMove;
	cbPerMaterial.translation = model->GetMaterial()[0].translation;
	cbPerMaterial.hasNormMap = model->GetMaterial()[0].hasNormalMap;
	cbPerMaterial.isObj = model->GetMaterial()[0].isObj;

	context->UpdateSubresource(materialBuffer, 0, nullptr, &cbPerMaterial, 0, 0);
	context->PSSetConstantBuffers(1, 1, &materialBuffer);
	context->GSSetConstantBuffers(1, 1, &materialBuffer);

	return true;
}

bool Shader::SetCBuffersWithCubemap(ID3D11DeviceContext* context, Models* model, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* cubemap, Camera* camera, Light* light)
{
	int vertexBuffernumber = 0;
	int pixelBuffernumber = 0;

	DirectX::XMMATRIX worldViewProjection;
	worldViewProjection = model->GetWorldMatrix() * view * projection;

	cbPerObject.worldViewProj = DirectX::XMMatrixTranspose(worldViewProjection);
	cbPerObject.world = DirectX::XMMatrixTranspose(model->GetWorldMatrix());
	cbPerObject.InverseWorld = DirectX::XMMatrixInverse(nullptr, model->GetWorldMatrix());

	context->UpdateSubresource(bufferPerObject, 0, nullptr, &cbPerObject, 0, 0);
	context->VSSetConstantBuffers(vertexBuffernumber, 1, &bufferPerObject);
	vertexBuffernumber++;

	// Set shader texture resource in the pixel shader.	
	if (model->GetMaterial()[0].hasTexture) {
		texture = model->GetTexture();
		context->PSSetShaderResources(0, 1, &texture);
	}

	if (model->GetMaterial()[0].hasReflection) {
		context->PSSetShaderResources(1, 1, &cubemap);
	}

	if (model->GetMaterial()[0].hasNormalMap) {
		normalMapSRV = model->GetNormalMap();
		context->PSSetShaderResources(2, 1, &normalMapSRV);
	}


	/*
		Set Camera buffer	// To Vertexshader
	*/
	cbPerCamera.cameraPosition = camera->GetPosition();
	context->UpdateSubresource(cameraBuffer, 0, nullptr, &cbPerCamera, 0, 0);
	context->VSSetConstantBuffers(vertexBuffernumber, 1, &cameraBuffer);
	context->GSSetConstantBuffers(0, 1, &cameraBuffer);
	vertexBuffernumber++;


	/*
		Set Light buffer	// To Pixelshader
	*/
	cbPerLight.ambientLightColor = light->GetAmbientColor();
	cbPerLight.diffuseLightColor = light->GetDiffuseColor();
	cbPerLight.specularLightColor = light->GetSpecularColor();
	cbPerLight.lightPosition = light->GetLightPosition();
	cbPerLight.lightRange = light->GetLightRange();
	cbPerLight.lightAttenuation = light->GetLightAttenuation();

	context->UpdateSubresource(lightBuffer, 0, nullptr, &cbPerLight, 0, 0);
	context->PSSetConstantBuffers(pixelBuffernumber, 1, &lightBuffer);
	pixelBuffernumber++;


	/*
		MATERIALEEE			// To Pixelshader
	*/
	cbPerMaterial.ambientColor = model->GetMaterial()[0].ambientColor;
	cbPerMaterial.diffuseColor = model->GetMaterial()[0].diffuseColor;
	cbPerMaterial.specularColor = model->GetMaterial()[0].specularColor;
	cbPerMaterial.reflectionColor = model->GetMaterial()[0].reflectionColor;
	cbPerMaterial.shine = model->GetMaterial()[0].shine;
	cbPerMaterial.hasTexture = model->GetMaterial()[0].hasTexture;
	cbPerMaterial.hasReflection = model->GetMaterial()[0].hasReflection;
	cbPerMaterial.canMove = model->GetMaterial()[0].canMove;
	cbPerMaterial.translation = model->GetMaterial()[0].translation;
	cbPerMaterial.hasNormMap = model->GetMaterial()[0].hasNormalMap;
	cbPerMaterial.isObj = model->GetMaterial()[0].isObj;

	context->UpdateSubresource(materialBuffer, 0, nullptr, &cbPerMaterial, 0, 0);
	context->PSSetConstantBuffers(pixelBuffernumber, 1, &materialBuffer);
	context->GSSetConstantBuffers(1, 1, &materialBuffer);
	pixelBuffernumber++;

	return true;
}

bool Shader::SetCBufferParticles(ID3D11DeviceContext* context, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, ID3D11ShaderResourceView* texture)
{
	XMMATRIX WVP = world * view * projection;
	cbPerObjectParticles.worldViewProj = DirectX::XMMatrixTranspose(WVP);
	cbPerObjectParticles.world = DirectX::XMMatrixTranspose(world);
		
	context->UpdateSubresource(bufferPerObjectParticles, 0, nullptr, &cbPerObjectParticles, 0, 0);
	context->VSSetConstantBuffers(0, 1, &bufferPerObjectParticles);
	
	// Set shader texture resource in the pixel shader.
	context->PSSetShaderResources(0, 1, &texture);

	return true;
}

void Shader::RenderShader(ID3D11DeviceContext* context, int indexcount, ID3D11SamplerState* sampler)
{
	// sets the vertex shader and layout
	context->IASetInputLayout(inputLayout);

	// Sets pixel and vertex shader
	context->VSSetShader(vertexShader, 0, 0);	
	context->GSSetShader(geometryShader, 0, 0);
	context->PSSetShader(pixelShader, 0, 0);

	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &sampler);

	context->DrawIndexed(indexcount, 0, 0);
}