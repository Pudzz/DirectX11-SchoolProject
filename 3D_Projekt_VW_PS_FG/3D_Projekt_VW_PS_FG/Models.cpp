#include "Models.h"

Models::Models()
{
	this->hr = 0;
	this->vertexBuffer = 0;
	this->indexBuffer = 0;

	this->vertexCount = 0;
	this->indexCount = 0;

    this->texture = 0;
    this->world = DirectX::XMMatrixIdentity();
    this->modelName = "";
    this->subsetCount = 0;
}

Models::Models(const Models& other)
{
	this->hr = other.hr;
	this->vertexBuffer = other.vertexBuffer;
	this->indexBuffer = other.indexBuffer;

	this->vertexCount = other.vertexCount;
	this->indexCount = other.indexCount;

    this->texture = other.texture;
    this->world = other.world;
    this->modelName = other.modelName;
    this->subsetCount = other.subsetCount;
}

Models::Models(std::string name)
{
    this->hr = 0;
    this->vertexBuffer = 0;
    this->indexBuffer = 0;

    this->vertexCount = 0;
    this->indexCount = 0;

    this->texture = 0;
    this->world = DirectX::XMMatrixIdentity();
    this->modelName = name;
    this->subsetCount = 0;
}

Models::~Models()
{    
}

bool Models::InitializeSkybox(ID3D11Device* device, ID3D11DeviceContext* context, LPCWSTR textureFilename)
{
    bool result;

    /*
        Create a new dds texture for model.
    */

    texture = new Texture;
    result = texture->InitializeSkyboxTexture(device, context, textureFilename);
    if (!result)
        return false;

    return true;
}

bool Models::InitializeCube(ID3D11Device* device, LPCWSTR textureFilename)
{
	bool result;

    result = CreateCube(device);
    if (!result)
        return false;

    result = LoadTexture(device, textureFilename);
    if (!result)
        return false;

	return true;
}

bool Models::InitWaterQuad(ID3D11Device* device, LPCWSTR textureFilename)
{
    bool result;

    result = CreateQuad(device);
    if (!result)
        return false;

    result = LoadTexture(device, textureFilename);
    if (!result)
        return false;

    return true;
}

void Models::Shutdown()
{
    ReleaseTexture();
    ShutdownBuffers();

    if (skeleton)
    {
        delete skeleton;
        skeleton = 0;
    }
}

void Models::Render(ID3D11DeviceContext* context)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(Vertex);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int Models::GetVertexCount()
{
    return vertexCount;
}

int Models::GetIndexCount()
{
	return indexCount;
}

ID3D11ShaderResourceView* Models::GetTexture()
{    
    return this->texture->GetTexture();    
}

ID3D11ShaderResourceView* Models::GetNormalMap()
{
    return this->normalMap->GetTexture();
}

bool Models::InitializeFromFbx(std::vector<Vertex> vertices, std::vector<DWORD> indices, Skeleton* skeleton, ID3D11Device* device)
{
    this->indices = indices;
    this->vertices = vertices;

    indexCount = (int)indices.size();
    vertexCount = (int)vertices.size();

    this->skeleton = skeleton;

    // Vertexbuffer desc
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    /*
    * A subresource is a single mipmap-level surface.
    */
    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = &vertices[0];

    hr = device->CreateBuffer(&bufferDesc, &resourceData, &vertexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for the new model", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    // Indexbuffer description for new model
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(DWORD) * indexCount;
    bufferDesc.StructureByteStride = sizeof(DWORD);

    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = &indices[0];

    // Create indexbuffer for cube
    hr = device->CreateBuffer(&bufferDesc, &resourceData, &indexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for IndexBuffer_Cube.", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    if (vertexBuffer != 0 && indexBuffer != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Models::ShutdownBuffers()
{
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = 0;
    }

    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = 0;
    }
}

bool Models::CreateCube(ID3D11Device* device)
{
    // Create own vertexbuffer and indices
    Vertex cube[] = {

        // Front Face
        Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f),
        Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f,-1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f),
        Vertex(1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f),
        Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),

        // Back Face
        Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f),
        Vertex(1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f),
        Vertex(1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(-1.0f,  1.0f, 1.0f, 1.0f, 0.0f,-1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 1.0f),

        // Top Face
        Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f,-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f),
        Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,-1.0f, 1.0f,  1.0f, -1.0f, 1.0f, 1.0f),
        Vertex(1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f),

        // Bottom Face
        Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f),
        Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),
        Vertex(1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f, 1.0f),
        Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, 1.0f),

        // Left Face
        Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f,-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, 1.0f),
        Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f,-1.0f,  1.0f,  1.0f, -1.0f, 1.0f, 1.0f),
        Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f,-1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f),
        Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f),

        // Right Face
        Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),
        Vertex(1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f),
        Vertex(1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f, 1.0f),
    };

    DWORD cubeIndices[]{
        // Front Face
        0,  1,  2,
        0,  2,  3,

        // Back Face
        4,  5,  6,
        4,  6,  7,

        // Top Face
        8,  9, 10,
        8, 10, 11,

        // Bottom Face
        12, 13, 14,
        12, 14, 15,

        // Left Face
        16, 17, 18,
        16, 18, 19,

        // Right Face
        20, 21, 22,
        20, 22, 23
    };

    indexCount = ARRAYSIZE(cubeIndices);
    vertexCount = ARRAYSIZE(cube);

    // Vertexbuffer description for cube
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = cube;

    // Create vertexbuffer for cube
    hr = device->CreateBuffer(&bufferDesc, &resourceData, &vertexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for VertexBuffer_Cube.", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    // Indexbuffer description for cube
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(DWORD) * indexCount;
    bufferDesc.StructureByteStride = sizeof(DWORD);

    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = cubeIndices;

    // Create indexbuffer for cube
    hr = device->CreateBuffer(&bufferDesc, &resourceData, &indexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for IndexBuffer_Cube.", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    return true;
}

bool Models::CreateQuad(ID3D11Device* device)
{
    // Create own vertexbuffer and indices
    Vertex cube[] = {

       Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f,-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f),
        Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,-1.0f, 1.0f,  1.0f, -1.0f, 1.0f, 1.0f),
        Vertex(1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f),
                
    };

    DWORD cubeIndices[]{
        // Front Face
        0,  1,  2,
        0,  2,  3,
               
    };

    indexCount = ARRAYSIZE(cubeIndices);
    vertexCount = ARRAYSIZE(cube);

    // Vertexbuffer description for cube
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = cube;

    // Create vertexbuffer for cube
    hr = device->CreateBuffer(&bufferDesc, &resourceData, &vertexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for VertexBuffer_Cube.", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    // Indexbuffer description for cube
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(DWORD) * indexCount;
    bufferDesc.StructureByteStride = sizeof(DWORD);

    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = cubeIndices;

    // Create indexbuffer for cube
    hr = device->CreateBuffer(&bufferDesc, &resourceData, &indexBuffer);
    if (FAILED(hr)) {
        MessageBox(0, L"Failed to 'CreateBuffer' for IndexBuffer_Cube.", L"Graphics scene Initialization Message", MB_ICONERROR);
        return false;
    }

    return true;
}

bool Models::LoadTexture(ID3D11Device* device, LPCWSTR textureFilename)
{
    bool result;

    texture = new Texture;
    if (!texture)
        return false;

    result = texture->Initialize(device, textureFilename);
    if (!result)
        return false;

    return true;
}

bool Models::LoadNormalMap(ID3D11Device* device, LPCWSTR textureFilename)
{
    bool result;

    normalMap = new Texture;
    if (!normalMap)
        return false;

    result = normalMap->Initialize(device, textureFilename);
    if (!result)
        return false;

    return true;
}

void Models::LoadTextureObj(ID3D11ShaderResourceView* resource)
{
    texture = new Texture;
    
    texture->SetTexture(resource);
}

void Models::ReleaseTexture()
{    
    if (texture)
    {
        texture->Shutdown();
        delete texture;
        texture = 0;
    }

    if (cubemapTexture)
    {
        cubemapTexture->Shutdown();
        delete cubemapTexture;
        cubemapTexture = 0;
    }

    if (normalMap)
    {
        normalMap->Shutdown();
        delete normalMap;
        normalMap = 0;
    }
}

std::vector<int>& Models::GetSubsetIndexVector()
{
    return this->subsetIndexStart;
}

std::vector<int>& Models::GetSubsetMaterialVector()
{
    return this->subsetMaterials;
}

int& Models::GetSubsetCount()
{
    return this->subsetCount;
}

std::vector<DWORD>& Models::GetIndices()
{
    return this->indices;
}

std::vector<XMFLOAT3>& Models::GetVerticesArray()
{
    return this->vertPosArray;
}

std::vector<Vertex>& Models::GetVertices()
{
    return this->vertices;
}

std::vector<SurfaceMaterial>& Models::GetMaterial()
{
    return this->materials;
}

std::vector<std::wstring>& Models::GetTextureNameVector()
{
    return this->textureNames;
}

bool Models::SetCubemapTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCWSTR textureFilename)
{
    bool result;

    cubemapTexture = new Texture;
    if (!cubemapTexture)
        return false;

    result = cubemapTexture->CreateDDSTexture(device, context,textureFilename);
    if (!result)
        return false;

    return true;
}

void Models::SetTexture(ID3D11ShaderResourceView* resource)
{
    this->texture->SetTexture(resource);
}

void Models::SetIndexBuffer(ID3D11Buffer* indexBuffer)
{
    this->indexBuffer = indexBuffer;
}

void Models::SetVertexBuffer(ID3D11Buffer* vertexBuffer)
{
    this->vertexBuffer = vertexBuffer;
}
