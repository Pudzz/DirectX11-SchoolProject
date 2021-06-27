#include "Graphics.h"

Graphics::Graphics(Inputs* input) {		

	this->input = input;
	this->directInput = input->GetDirectInput8();
	this->keyboard = input->GetDirectInput8Keyboard();
	this->mouse = input->GetDirectInput8Mouse();

	this->ScreenHeight = 0;
	this->ScreenWidth = 0;
	this->dx11 = 0;
	this->hr = 0;
	this->camera = 0;
	this->model = 0;
	this->fbxModel = 0;
	this->shader = 0;	
	this->skyboxShader = 0;
	this->particleShader = 0;
	this->terrain = 0;
	this->skybox = 0;
	this->particles = 0;
	this->billboardPlane = 0;	
	this->falloutQuadBackfaceTest = 0;

	this->light = 0;
	this->scene = 0;

	ZeroMemory(&cubeViewport, sizeof(D3D11_VIEWPORT));
	this->cubeDsv = 0;
	this->cubeSrv = 0;
	this->cubeCameras = 0;
	this->cubemapSphere = 0;
	for (int i = 0; i < 6; i++)
		this->cubeRtv[i] = 0;	
	
	this->pepTalk = 0;
	this->cubicCube = 0;
	this->donutCube = 0;
	this->pyramidCube = 0;

	this->water = 0;
}

Graphics::Graphics(const Graphics& other) {

	this->input = other.input;
	this->directInput = other.directInput;
	this->keyboard = other.keyboard;
	this->mouse = other.mouse;

	this->ScreenHeight = other.ScreenHeight;
	this->ScreenWidth = other.ScreenWidth;
	this->dx11 = other.dx11;
	this->hr = 0;
	this->camera = other.camera;
	this->model = other.model;
	this->fbxModel = other.fbxModel;
	this->shader = other.shader;
	this->terrain = other.terrain;
	this->skyboxShader = other.skyboxShader;
	this->particleShader = other.particleShader;
	this->skybox = other.skybox;
	this->particles = other.particles;
	this->billboardPlane = other.billboardPlane;
	this->falloutQuadBackfaceTest = other.falloutQuadBackfaceTest;

	this->light = other.light;
	this->scene = other.scene;

	this->cubeViewport = other.cubeViewport;
	this->cubeDsv = other.cubeDsv;
	this->cubeSrv = other.cubeSrv;
	this->cubeCameras = other.cubeCameras;
	this->cubemapSphere = other.cubemapSphere;
	for (int i = 0; i < 6; i++)
		this->cubeRtv[i] = other.cubeRtv[i];	
		
	this->pepTalk = other.pepTalk;
	this->cubicCube = other.cubicCube;
	this->donutCube = other.donutCube;
	this->pyramidCube = other.pyramidCube;

	this->water = other.water;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	objReader newReader;


	/*
		Initialize DX11 and set screenwidth and height for later use in picking.
	*/

	dx11 = new DX11;
	if (!dx11)
		return false;

	result = dx11->Initialize(screenWidth, screenHeight, hwnd, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
		return false;

	this->ScreenWidth = screenWidth;
	this->ScreenHeight = screenHeight;


	/*
		Create a new main camera.
		Startposition for camera and give camera inputdevice settings.
	*/

	camera = new Camera(hwnd);
	if (!camera)
		return false;

	camera->SetPosition(10.0f, 100.0f, 10.0f);
	camera->GetInputs(input);


	/*
		Create light to give to cbuffer.
	*/

	light = new Light;
	// Initialize the light object.
	light->SetAmbientColor(1.0f, 1.0f, 1.0f, 1.0f);
	light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	light->SetLightPosition(-5.0f, 30.0f, 30.0f);
	light->SetLightAttentuation(1.0f, 0.02f, 0.0f);
	light->SetLightRange(2000.0f);


	/*
		Init different shaders
	*/

	/* Default shader */
	shader = new Shader(dx11->GetDevice());
	result = shader->InitializeShaderWithGeometryShader(dx11->GetDevice(), hwnd, L"Shaders/VSDefault.hlsl", L"Shaders/GSDefault.hlsl", L"Shaders/PSDefault.hlsl", "VSMain", "GSMain", "PSMain");
	if (!result)
		return false;
	result = shader->CreateDefaultInputLayout(dx11->GetDevice());
	if (!result)
		return false;


	/* Skybox shader */
	skyboxShader = new Shader(dx11->GetDevice());
	result = skyboxShader->InitializeShaders(dx11->GetDevice(), hwnd, L"Shaders/SkyVS.hlsl", L"Shaders/SkyPS.hlsl", "SkyVSMain", "SkyPSMain");
	if (!result)
		return false;
	result = skyboxShader->CreateSkyboxInputLayout(dx11->GetDevice(), dx11->GetContext());
	if (!result)
		return false;


	/* Particles shader */
	particleShader = new Shader(dx11->GetDevice());
	result = particleShader->InitializeParticleShaders(dx11->GetDevice(), hwnd, L"Shaders/ParticlesVS.hlsl", L"Shaders/ParticlesPS.hlsl", "VSMain", "PSMain");
	if (!result)
		return false;


	/*	Index 0
		FBX MODEL. Glasse model. Has an animation.
		FBXLoader loads the fbx model.
		Importer loads the animation in fbx.
	*/

	fbxModel = FbxLoader::loadFbxModel("FbxModels/Glasse_Walk_NO_NORMAL.fbx", shader, dx11->GetDevice());
	fbxModel->SetWorldMatrix(DirectX::XMMatrixScaling(1.75f,1.75f,1.75f) * DirectX::XMMatrixTranslation(60.0f, 10.6f, 77.0f));

	scene = importer.ReadFile("FbxModels/Glasse_Walk_Cycle.fbx", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);
	FbxLoader::saveAnimationData(scene, fbxModel->GetSkeleton(), "walk");
	fbxModel->GetSkeleton()->SetCurrentAnimation(fbxModel->GetSkeleton()->animations[0]);
	fbxModel->GetMaterial()[0].specularColor.w = 100.0f;
	allModels.push_back(fbxModel);


	/*
		Dynamic cube mapping
		Create a 512x512 texture.
		Initialize the 6 render target views with the texture.
		Create a depth texture for the cubemapping render pipeline.
		Build the texture based on the 6 cameras directions.
	*/

	cubeCameras = new Camera[6];
	result = InitializeCubeMapViews(dx11->GetDevice());
	if (!result)
		return false;


	result = BuildCubeFaceCameras(cubicTrans.x, cubicTrans.y, cubicTrans.z);
	if (!result)
		return false;


	/*
		Centersphere for environment mapping.
		OWN MODEL OUTSIDE allModels-vector.
		This is the sphere that has the cubemapping texture.

		+ 3 Models to demonstrate cubic environment mapping.
	*/

	cubemapSphere = new Models;
	newReader.loadObj(cubemapSphere, dx11->GetDevice(), L"asdf.obj", true, false);
	cubemapSphere->GetTextureStruct()->SetName("boll");
	cubemapSphere->SetWorldMatrix(XMMATRIX(XMMatrixScaling(1, 1, 1)) * XMMATRIX(XMMatrixTranslation(cubicTrans.x, cubicTrans.y, cubicTrans.z)));
	cubemapSphere->GetMaterial()[0].specularColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	cubemapSphere->GetMaterial()[0].ambientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
	cubemapSphere->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.2f);
	cubemapSphere->GetMaterial()[0].reflectionColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cubemapSphere->GetMaterial()[0].hasTexture = false;
	cubemapSphere->GetMaterial()[0].hasReflection = true;
	cubemapSphere->GetMaterial()[0].isObj = true;

	cubicCube = new Models;
	newReader.loadObj(cubicCube, dx11->GetDevice(), L"cubecube.obj", true, false);
	cubicCube->GetMaterial()[0].hasTexture = false;
	cubicCube->GetMaterial()[0].hasReflection = false;
	cubicCube->GetMaterial()[0].isObj = true;
	allModels.push_back(cubicCube);

	pyramidCube = new Models;
	newReader.loadObj(pyramidCube, dx11->GetDevice(), L"pyramidCube.obj", true, false);
	pyramidCube->GetMaterial()[0].hasTexture = false;
	pyramidCube->GetMaterial()[0].hasReflection = false;
	pyramidCube->GetMaterial()[0].isObj = true;
	allModels.push_back(pyramidCube);

	donutCube = new Models;
	newReader.loadObj(donutCube, dx11->GetDevice(), L"donutCube.obj", true, false);
	donutCube->GetMaterial()[0].hasTexture = false;
	donutCube->GetMaterial()[0].hasReflection = false;
	donutCube->GetMaterial()[0].isObj = true;
	allModels.push_back(donutCube);


	/*
		Create a new terrain.

		Create terrain based on a texture(heightmap).
		Give camera access to terrain. ( So we can calc y-axis to walk on terrain )
	*/

	terrain = new Terrain;
	terrain->CreateTerrain("Textures/height100.png", dx11->GetDevice(), hwnd);
	terrain->GetMesh()->LoadTexture(dx11->GetDevice(), L"Textures/diffuse.png");
	/* Terrain material */
	terrainMat.ambientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	terrainMat.diffuseColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	terrainMat.shine = 0.0f;
	terrainMat.specularColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, terrainMat.shine);
	terrainMat.hasTexture = true;
	terrainMat.isTerrain = true;
	terrain->GetMesh()->GetMaterial().push_back(terrainMat);

	allModels.push_back(terrain->GetMesh());
	camera->terrain = terrain;


	/*
		Water.
		Faked water.
		Translation on u-coordinate.
	*/

	water = FbxLoader::loadFbxModel("FbxModels/water.fbx", shader, dx11->GetDevice());
	water->SetWorldMatrix(DirectX::XMMatrixTranslation(50, 5.0f, 65.0f));
	water->GetMaterial()[0].canMove = true;
	water->GetMaterial()[0].specularColor = XMFLOAT4{ 0.5,0.5,0.5, 16.0f };
	allModels.push_back(water);


	/*
		2 Spheres with brick wall texture and tangent space normal map
	*/

	Models* brickSphere;
	brickSphere = FbxLoader::loadFbxModel("FbxModels/brick_normal.fbx", shader, dx11->GetDevice());
	brickSphere->SetWorldMatrix(DirectX::XMMatrixTranslation(25.0f, 11.0f, 40.0f));
	allModels.push_back(brickSphere);

	/* No normalmap */
	Models* brickSphere1;
	brickSphere1 = FbxLoader::loadFbxModel("FbxModels/brick_normal.fbx", shader, dx11->GetDevice());
	brickSphere1->SetWorldMatrix(DirectX::XMMatrixTranslation(20.0f, 11.0f, 40.0f));
	brickSphere1->GetMaterial()[0].hasNormalMap = false;
	allModels.push_back(brickSphere1);

	
	/*
		2 Spheres with brick wall texture and a tangent space normal map
	*/

	Models* testSphere = new Models;
	newReader.loadObj(testSphere, dx11->GetDevice(), L"brick_bump.obj", true, false);
	testSphere->SetWorldMatrix(DirectX::XMMatrixTranslation(13.0f, 11.0f, 40.0f));
	testSphere->GetMaterial()[0].isObj = true;
	testSphere->GetMaterial()[0].diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	allModels.push_back(testSphere);

	/* No normalmap */
	Models* testSphere1 = new Models;
	newReader.loadObj(testSphere1, dx11->GetDevice(), L"brick_bump.obj", true, false);
	testSphere1->SetWorldMatrix(DirectX::XMMatrixTranslation(8.0f, 11.0f, 40.0f));
	testSphere1->GetMaterial()[0].isObj = true;
	testSphere1->GetMaterial()[0].diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	testSphere1->GetMaterial()[0].hasNormalMap = false;
	allModels.push_back(testSphere1);


	/*
		SKYBOX, OWN MODEL OUTSIDE allModels-vector. Has an own shader.
		Obj model.
		Initialized with dds texture as texturecube.
	*/

	skybox = new Models;
	newReader.loadObj(skybox, dx11->GetDevice(), L"skysphere.obj", true, false);
	skybox->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	skybox->GetMaterial()[0].hasTexture = true;

	result = skybox->InitializeSkybox(dx11->GetDevice(), dx11->GetContext(), L"Textures/skymap.dds");
	if (!result)
		return false;

	skybox->GetTextureStruct()->SetName("Skybox");
	skybox->SetWorldMatrix(XMMATRIX(XMMatrixScaling(75.0f, 75.0f, 75.0f) * XMMatrixTranslation(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z)));


	/*
		MODEL STUFF.
		Own created cube.
		Textured with edea texture.
		Has a normalmap. - UV-moving as faked water effect.
	*/

	model = new Models;
	result = model->InitializeCube(dx11->GetDevice(), L"Textures/Edea_Kramer.jpg");
	if (!result)
		return false;

	result = model->LoadNormalMap(dx11->GetDevice(), L"Textures/water_normal.jpg");
	if (!result)
		return false;

	model->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(75.0f, 9.5f, 20.0f)));
	model->GetTextureStruct()->SetName("Edea");

	/* Model Material */
	modelmat.ambientColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	modelmat.diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	modelmat.shine = 32.0f;
	modelmat.specularColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, modelmat.shine);
	modelmat.hasTexture = true;
	modelmat.hasNormalMap = true;
	modelmat.canMove = true;
	model->GetMaterial().push_back(modelmat);
	allModels.push_back(model);


	/*
		Peptalk to the guys.
	*/

	pepTalk = new Models;
	newReader.loadObj(pepTalk, dx11->GetDevice(), L"pepTalk.obj", true, false);
	pepTalk->SetVertexCount((int)pepTalk->GetVertices().size());
	pepTalk->SetIndexCount((int)pepTalk->GetIndices().size());
	pepTalk->GetMaterial()[0].isObj = true;
	allModels.push_back(pepTalk);


	/*
		OBJ Model. Textured with anger texture.
		isObj = true | for backface culling with geometry shader.
	*/

	Models* cube = new Models;
	newReader.loadObj(cube, dx11->GetDevice(), L"testingCube.obj", true, false);
	cube->SetVertexCount((int)cube->GetVertices().size());
	cube->SetIndexCount((int)cube->GetIndices().size());
	cube->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(80.0f, 10.0f, 20.0f)));
	cube->GetTextureStruct()->SetName("Anger");
	cube->GetMaterial()[0].ambientColor = DirectX::XMFLOAT4(0.5f, 0.8f, 1.0f, 1.0f);
	cube->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cube->GetMaterial()[0].isObj = true;
	allModels.push_back(cube);


	/*
		3 Picking spheres. 
		Stored in an own pickableModels vector. 
	*/

	Models* pickingSphere = new Models();
	if (!pickingSphere)
		return false;
	newReader.loadObj(pickingSphere, dx11->GetDevice(), L"sphere.obj", true, false);
	pickingSphere->SetVertexCount((int)pickingSphere->GetVertices().size());
	pickingSphere->SetIndexCount((int)pickingSphere->GetIndices().size());
	pickingSphere->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(10.0f, 10.0f, 95.0f)));
	pickingSphere->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pickingSphere->GetMaterial()[0].hasTexture = false;
	pickingSphere->GetMaterial()[0].isObj = true;
	allModels.push_back(pickingSphere);
	pickableModels.push_back(pickingSphere);

	Models* pickingSphere1 = new Models();
	if (!pickingSphere1)
		return false;
	newReader.loadObj(pickingSphere1, dx11->GetDevice(), L"sphere.obj", true, false);
	pickingSphere1->SetVertexCount((int)pickingSphere1->GetVertices().size());
	pickingSphere1->SetIndexCount((int)pickingSphere1->GetIndices().size());
	pickingSphere1->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(5.0f, 10.0f, 90.0f)));
	pickingSphere1->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pickingSphere1->GetMaterial()[0].hasTexture = false;
	pickingSphere1->GetMaterial()[0].isObj = true;
	allModels.push_back(pickingSphere1);
	pickableModels.push_back(pickingSphere1);

	Models* pickingSphere2 = new Models();
	if (!pickingSphere2)
		return false;
	newReader.loadObj(pickingSphere2, dx11->GetDevice(), L"sphere.obj", true, false);
	pickingSphere2->SetVertexCount((int)pickingSphere2->GetVertices().size());
	pickingSphere2->SetIndexCount((int)pickingSphere2->GetIndices().size());
	pickingSphere2->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(15.0f, 10.0f, 95.0f)));
	pickingSphere2->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pickingSphere2->GetMaterial()[0].hasTexture = false;
	pickingSphere2->GetMaterial()[0].isObj = true;
	allModels.push_back(pickingSphere2);
	pickableModels.push_back(pickingSphere2);


	/* 
		Palm for picking test
	*/

	Models* palmPicking = new Models();
	if (!palmPicking)
		return false;
	newReader.loadObj(palmPicking, dx11->GetDevice(), L"palm.obj", true, false);
	palmPicking->SetVertexCount((int)palmPicking->GetVertices().size());
	palmPicking->SetIndexCount((int)palmPicking->GetIndices().size());
	palmPicking->SetWorldMatrix(XMMatrixScaling(0.3f,0.3f,0.3f) * XMMATRIX(XMMatrixTranslation(20.0f, 8.0f, 95.0f)));
	palmPicking->GetTextureStruct()->SetName("pickingpalm");
	palmPicking->GetMaterial()[0].specularColor = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	palmPicking->GetMaterial()[0].ambientColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	palmPicking->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	palmPicking->GetMaterial()[0].reflectionColor = DirectX::XMFLOAT4(0.8f, 0.2f, 0.8f, 0.2f);
	palmPicking->GetMaterial()[0].hasTexture = true;
	palmPicking->GetMaterial()[0].hasReflection = false;
	palmPicking->GetMaterial()[0].isObj = true;
	allModels.push_back(palmPicking);
	pickableModels.push_back(palmPicking);


	/* 
		Quad backface culling test 
	*/

	falloutQuadBackfaceTest = new Models();
	if (!falloutQuadBackfaceTest)
		return false;

	newReader.loadObj(falloutQuadBackfaceTest, dx11->GetDevice(), L"falloutQuad.obj", true, false);
	falloutQuadBackfaceTest->GetMaterial()[0].specularColor = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	falloutQuadBackfaceTest->GetMaterial()[0].ambientColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	falloutQuadBackfaceTest->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	falloutQuadBackfaceTest->GetMaterial()[0].reflectionColor = DirectX::XMFLOAT4(0.8f, 0.2f, 0.8f, 0.2f);
	falloutQuadBackfaceTest->GetMaterial()[0].hasTexture = true;
	falloutQuadBackfaceTest->GetMaterial()[0].isObj = true;
	allModels.push_back(falloutQuadBackfaceTest);


	/*
		Billboard plane test. 
		Demonstrate backface culling using a geometry shader. 
	*/

	billboardPlane = new Models();
	if (!billboardPlane)
		return false;
	newReader.loadObj(billboardPlane, dx11->GetDevice(), L"billboard_plane.obj", true, false);
	billboardPlane->SetWorldMatrix(XMMATRIX(XMMatrixTranslation(-2.0f, 2.0f, 5.0f)));
	billboardPlane->GetTextureStruct()->SetName("Star");
	billboardPlane->GetMaterial()[0].ambientColor = DirectX::XMFLOAT4(0.5f, 0.8f, 1.0f, 1.0f);
	billboardPlane->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	billboardPlane->GetMaterial()[0].hasTexture = true;
	billboardPlane->GetMaterial()[0].isObj = true;


	/*
		Particles.
		Creating a bunch of particles. Max is 100 on the scene at the same time. 	
		Buffers are used dynamicly. 
	*/

	particles = new ParticleSystem();
	if (!particles)
		return false;

	result = particles->Initialize(dx11->GetDevice(), L"Textures/dropdrop.png");
	if (!result)
		return false;
	

	return true;
}

void Graphics::Shutdown()
{	
	if (particleShader) {
		particleShader->Shutdown();
		delete particleShader;
		particleShader = 0;
	}

	if (particles) {
		particles->Shutdown();
		delete particles;
		particles = 0;
	}

	if (input) {
		input->Shutdown();
		delete input;
		input = 0;
	}

	if (camera)	{
		delete camera;
		camera = 0;
	}

	if (dx11) {
		dx11->Shutdown();
		delete dx11;
		dx11 = 0;
	}

	if (terrain)
	{
		delete terrain;
		terrain = 0;
	}
		
	if (allModels.size() > 0) {
		for (int i = 0; i < allModels.size(); i++)
		{
			allModels[i]->Shutdown();
			delete allModels[i];
			allModels[i] = 0;
		}
		allModels.clear();
	}

	if (skyboxShader) 
	{
		skyboxShader->Shutdown();
		delete skyboxShader;
		skyboxShader = 0;
	}

	if (skybox)
	{
		skybox->Shutdown();
		delete skybox;
		skybox = 0;
	}

	if (shader) {
		shader->Shutdown();
		delete shader;
		shader = 0;
	}

	if (cubemapSphere)
	{
		cubemapSphere->Shutdown();
		delete cubemapSphere;
		cubemapSphere = 0;
	}

	if (cubeCameras)
	{
		delete[] cubeCameras;
		cubeCameras = 0;
	}

	if (light)
	{
		delete light;
		light = 0;
	}

	for (int i = 0; i < 6; i++) 
	{
		if (cubeRtv[i]) 
		{
			cubeRtv[i]->Release();
			cubeRtv[i] = 0;
		}
	}

	if (cubeSrv) 
	{
		cubeSrv->Release();
		cubeSrv = 0;
	}

	if (cubeDsv) 
	{
		cubeDsv->Release();
		cubeDsv = 0;
	}
	
	if (billboardPlane)
	{
		billboardPlane->Shutdown();
		delete billboardPlane;
		billboardPlane = 0;
	}
}

bool Graphics::RenderFrame(float deltaTime)
{
	bool result;

	// Update camera position	
	camera->DetectInputs(deltaTime);
	
	IntersectionTest(input);
	
	/* Update models suff */
	Update(deltaTime);

	// Update cubemapping cameras 
	result = RenderCubemapping();
	if (!result)
		return false;	

	// Render frame
	result = Render();
	if (!result)
		return false;

	return true;
}

bool Graphics::InitializeCubeMapViews(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width				= CubeMapSize;
	texDesc.Height				= CubeMapSize;
	texDesc.MipLevels			= 0;
	texDesc.ArraySize			= 6;
	texDesc.SampleDesc.Count	= 1;
	texDesc.SampleDesc.Quality	= 0;
	texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage				= D3D11_USAGE_DEFAULT;
	texDesc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags		= 0;
	texDesc.MiscFlags			= D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* cubeTex = 0;
	hr = device->CreateTexture2D(&texDesc, nullptr, &cubeTex);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to CreateTexture2D- Cubemapping", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}

	// Create a render target view to each cube map face 
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format						= texDesc.Format;
	RTVDesc.ViewDimension				= D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	RTVDesc.Texture2DArray.ArraySize	= 1;
	RTVDesc.Texture2DArray.MipSlice		= 0;

	for (int i = 0; i < 6; i++) {
		RTVDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateRenderTargetView(cubeTex, &RTVDesc, &cubeRtv[i]);
		assert(SUCCEEDED(hr));
		if (FAILED(hr)) {
			MessageBox(0, L"Failed to create a rendertarget - Cubemapping", L"Graphics scene Initialization Message", MB_ICONERROR);
			return false;
		}
	}

	// CREATE A SHADER RESOURCE VIEW to the cube map 
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format						= texDesc.Format;
	SRVDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MostDetailedMip = 0;
	SRVDesc.TextureCube.MipLevels		= -1;

	hr = device->CreateShaderResourceView(cubeTex, &SRVDesc, &cubeSrv);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to create a shaderresourceview - Cubemapping", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}

	// Release texture2d, no use anymore
	ReleasePtr(cubeTex);


	// Building depth buffer and viewports   
	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthTexDesc.Width					= CubeMapSize;
	depthTexDesc.Height					= CubeMapSize;
	depthTexDesc.MipLevels				= 1;
	depthTexDesc.ArraySize				= 1;
	depthTexDesc.SampleDesc.Count		= 1;
	depthTexDesc.SampleDesc.Quality		= 0;
	depthTexDesc.Format					= DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage					= D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags			= 0;
	depthTexDesc.MiscFlags				= 0;

	ID3D11Texture2D* depthTex = 0;
	hr = device->CreateTexture2D(&depthTexDesc, 0, &depthTex);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to create a texture2d - depthstencilview - Cubemapping", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}

	// Create a depth stencil view for the entire buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	ZeroMemory(&DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	DSVDesc.Format = depthTexDesc.Format;
	DSVDesc.Flags = 0;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthTex, &DSVDesc, &cubeDsv);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		MessageBox(0, L"Failed to CreateDepthStencilView - Cubemapping", L"Graphics scene Initialization Message", MB_ICONERROR);
		return false;
	}

	ReleasePtr(depthTex);

	ZeroMemory(&cubeViewport, sizeof(D3D11_VIEWPORT));
	cubeViewport.TopLeftX = 0.0f; 
	cubeViewport.TopLeftY = 0.0f;
	cubeViewport.Width = (float)CubeMapSize;
	cubeViewport.Height = (float)CubeMapSize;
	cubeViewport.MinDepth = 0.0f;
	cubeViewport.MaxDepth = 1.0f;
		
	return true;
}

bool Graphics::BuildCubeFaceCameras(float x, float y, float z)
{
	// Generate the cube map about the given position
	XMFLOAT3 center(x, y, z);

	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X 
		XMFLOAT3(x, y + 1.0f, z), // +Y 
		XMFLOAT3(x, y - 1.0f, z), // -Y 
		XMFLOAT3(x, y, z + 1.0f), // +Z 
		XMFLOAT3(x, y, z - 1.0f)  // -Z
	};
		
	/*
		Use world up vector (0,1,0) for all directions except +Y/-Y.
		In these cases, we are looking down +Y or -Y.
	*/
	
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X 
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X 
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y 
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y 
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +Z 
		XMFLOAT3(0.0f, 1.0f, 0.0f)   // -Z
	};

	// Use sphere worldspace and cubemap view and projection
	for (int i = 0; i < 6; ++i) {
		cubeCameras[i].SetPosition(center.x, center.y, center.z);
		cubeCameras[i].SetTargetPosition(targets[i].x, targets[i].y, targets[i].z);
		cubeCameras[i].SetUpvector(ups[i].x, ups[i].y, ups[i].z);
		cubeCameras[i].SetViewMatrix(XMVECTOR(XMVectorSet(center.x, center.y, center.z, 0.0f)), XMVECTOR(XMVectorSet(targets[i].x, targets[i].y, targets[i].z, 0.0f)), XMVECTOR(XMVectorSet(ups[i].x, ups[i].y, ups[i].z, 0.0f)));
	}

	return true;
}

bool Graphics::RenderCubemapping()
{	
	bool result;
			
	/* Make cubemap srv slot empty */
	ID3D11ShaderResourceView* const nullsrv[1] = { NULL };
	dx11->GetContext()->PSSetShaderResources(1, 1, nullsrv);

	dx11->GetContext()->RSSetViewports(1, &cubeViewport);
	float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		
	for (int i = 0; i < 6; i++) {
		
		dx11->GetContext()->ClearRenderTargetView(cubeRtv[i], black);
		dx11->GetContext()->ClearDepthStencilView(cubeDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		/* Bind cube map face as render target. */
		dx11->GetContext()->OMSetRenderTargets(1, &cubeRtv[i], cubeDsv);

		/* Draw scene without sphere */
		XMMATRIX views;
		cubeCameras[i].GetViewMatrix(views);	
		XMMATRIX cubeProj;
		dx11->GetCubemapProjectionMatrix(cubeProj);

		/* Models here with default shader*/	
		for (int k = 0; k < allModels.size(); k++) {
			allModels[k]->Render(dx11->GetContext());
			result = shader->Render(dx11->GetContext(), allModels[k], views, cubeProj, &cubeCameras[i], light, dx11->GetMinMagMipSampler());
			if (!result)
				return false;
		}
					
		
		/* Skybox render alone with skybox shader */
		skybox->Render(dx11->GetContext());
		result = skyboxShader->Render(dx11->GetContext(), skybox, views, cubeProj, &cubeCameras[i], light, dx11->GetMinMagMipSampler());
		if (!result)
			return false;
			

		dx11->EnableAlphaBlending();

		/* Render billboardplane for cubemapping */
		billboardPlane->Render(dx11->GetContext());
		result = shader->Render(dx11->GetContext(), billboardPlane, views, cubeProj, &cubeCameras[i], light, dx11->GetMinMagMipSampler());
		if (!result)
			return false;

		/* Render particle system */
		particles->Render(dx11->GetContext());
		result = particleShader->RenderParticles(dx11->GetContext(), particles->GetIndexCount(), particles->GetWorldMatrix(), views, cubeProj, particles->GetTexture(), dx11->GetMinMagMipSampler());
		if (!result)
			return false;
		
		dx11->DisableAlphaBlending();		
	}


	/* Restore old viewport and render targets. */
	dx11->GetContext()->RSSetViewports(1, &dx11->GetViewport());
	dx11->GetContext()->OMSetRenderTargets(1, &dx11->GetRenderTarget(), dx11->GetDepthStencilView());

	/* Create scene to the srv to use as a cubemap-texture */
	dx11->GetContext()->GenerateMips(cubeSrv);

	return true;
}

void Graphics::Update(float deltaTime)
{
	/* Skeleton mesh */
	allModels[0]->GetSkeleton()->AddKeyframe();
	

	/* 
		CUBE ENVIRONMENT MAPPING WORLD SETTING 
		3 Object for demonstation. 
	*/	

	rotPyramid += 0.6f * (float)deltaTime;
	if (rotPyramid > 6.28f)
		rotPyramid = 0.0f;

	XMVECTOR rotPyramidAxis = DirectX::XMVectorSet(rotPyramid, 0.0f, 0.0f, 0.0f);
	XMMATRIX rotPyramidRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(rotPyramidAxis);
	pyramidCube->SetWorldMatrix(rotPyramidRotation * XMMATRIX(XMMatrixTranslation(cubicTrans.x + 3.0f, cubicTrans.y, cubicTrans.z)));

	rotDonut += 0.7f * (float)deltaTime;
	if (rotDonut > 6.28f)
		rotDonut = 0.0f;

	XMVECTOR rotDonutAxis = DirectX::XMVectorSet(rotDonut, rotDonut, 0.0f, 0.0f);
	XMMATRIX rotDonutRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(rotDonutAxis);
	donutCube->SetWorldMatrix(rotDonutRotation * XMMATRIX(XMMatrixTranslation(cubicTrans.x, cubicTrans.y, cubicTrans.z - 5.0f)));

	rotCube += 0.5f * (float)deltaTime;
	if (rotCube > 6.28f)
		rotCube = 0.0f;

	XMVECTOR rotCubeAxis = DirectX::XMVectorSet(rotCube, rotCube, rotCube, 0.0f);
	XMMATRIX rotCubeRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(rotCubeAxis);
	cubicCube->SetWorldMatrix(rotCubeRotation * XMMATRIX(XMMatrixTranslation(cubicTrans.x - 4.0f, cubicTrans.y, cubicTrans.z)));


	/* 
		Skybox follow cam 
	*/

	rotSky += 0.02f * (float)deltaTime;
	if (rotSky > 6.28f)
		rotSky = 0.0f;
	
	XMVECTOR skyrotaxis = DirectX::XMVectorSet(0.0f, rotSky, 0.0f, 0.0f);
	XMMATRIX skycubeRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(skyrotaxis);
	XMMATRIX skyboxScale = XMMatrixScaling(75.0f, 75.0f, 75.0f);
	skybox->SetWorldMatrix(skyboxScale * skycubeRotation * XMMATRIX(XMMatrixTranslation(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z)));

	
	/*
		MOVING TEXTURE.
		Fake water demonstration.
	*/

	if (modelmat.canMove == true) {
		model->GetMaterial()[0].translation.y += (float)deltaTime * 0.3f;
	}

	if (water->GetMaterial()[0].canMove) {
		water->GetMaterial()[0].translation.y += 0.02f * (float)deltaTime;
	}
		

	/*
		Rot quad backfaceculling test.
	*/

	rotQuad += 0.4f * (float)deltaTime;
	if (rotQuad > 6.28f)
		rotQuad = 0.0f;

	XMVECTOR quadrotaxis = DirectX::XMVectorSet(0.0f, rotQuad, 0.0f, 0.0f);
	XMMATRIX quadRot = DirectX::XMMatrixRotationRollPitchYawFromVector(quadrotaxis);
	XMMATRIX quadtranslation = DirectX::XMMatrixTranslation(80.0f, 10.0f, 15.0f);
	falloutQuadBackfaceTest->SetWorldMatrix(quadRot * quadtranslation);


	/*
		Billboard test on a plane with transparent texture.
	*/

	XMFLOAT3 cameraPosition, modelPosTest;
	cameraPosition = camera->GetPosition();		
	
	/* Set billboardplane position */
	modelPosTest.x = 80.0f;
	modelPosTest.y = 10.0f;
	modelPosTest.z = 10.0f;

	/* Set angle for plane, "look at cameraposition all the time" */
	double angle = atan2(modelPosTest.x - cameraPosition.x, modelPosTest.z - cameraPosition.z) * (180.0 / XM_PI);
	float rotation = (float)angle * 0.0174532925f;

	XMMATRIX billboardWorld = XMMatrixIdentity();
	XMMATRIX billboardRotationY = XMMatrixRotationY(rotation);

	XMMATRIX billboardTranslation = XMMatrixTranslation(modelPosTest.x, modelPosTest.y, modelPosTest.z);
	billboardWorld = billboardRotationY * billboardTranslation;

	billboardPlane->SetWorldMatrix(billboardWorld);


	/* 
		Particles 
	*/

	XMFLOAT3 particlesPosition;

	/* Set billboardplane position */
	particlesPosition.x = 40.0f;
	particlesPosition.y = 40.0f;
	particlesPosition.z = 55.0f;

	/* Set angle for particles, "look at cameraposition all the time" */
	/* Defines the angle in radians between two vectors * 1 degrees to give our xmatrixrotationy an degree value*/
	double anglepart = atan2(particlesPosition.x - cameraPosition.x, particlesPosition.z - cameraPosition.z) * (180.0 / XM_PI);
	float rotationPart = (float)anglepart * 0.0174532925f;

	XMMATRIX worldParticles = XMMatrixIdentity();
	XMMATRIX particlesRotationY = XMMatrixRotationY(rotationPart);
	XMMATRIX particlesTranslation = XMMatrixTranslation(particlesPosition.x, particlesPosition.y, particlesPosition.z);
	worldParticles = particlesRotationY * particlesTranslation;
	particles->SetWorldMatrix(worldParticles);

	particles->Update(deltaTime, dx11->GetContext());

	/*
		Rot peptalk model
	*/

	rotPep += 0.7f * (float)deltaTime;
	if (rotPep > 6.28f)
		rotPep = 0.0f;

	XMVECTOR peprotaxis = DirectX::XMVectorSet(0.0f, rotPep, 0.0f, 0.0f);
	XMMATRIX peptalkeRotation = DirectX::XMMatrixRotationRollPitchYawFromVector(peprotaxis);
	XMMATRIX peptalkTranslation = XMMatrixTranslation(10.0f, 100.0f, 200.0f);
	pepTalk->SetWorldMatrix(peptalkeRotation * peptalkTranslation);
}

void Graphics::ScreenSpaceToWorldSpace(float mouseX, float mouseY, XMVECTOR& pickRayInWorldSpacePos, XMVECTOR& pickRayInWorldSpaceDir)
{
	/*
		Make our picked xy mouse position to world space coordinates.

		1. Transform 2D pick position on screen space to a 3D ray in View Space by projection matrix.
		2. Transform 3D Ray from View space to 3D ray in World space.
		3. Save position and direction vectos for later use. 
	*/
		
	XMMATRIX camProjection;
	dx11->GetProjectionMatrix(camProjection);
	float viewX = (((2.0f * mouseX) / ScreenWidth) - 1) / camProjection.r[0].m128_f32[0];	   // (0, 0);
	float viewY = -(((2.0f * mouseY) / ScreenHeight) - 1) / camProjection.r[1].m128_f32[1];    // (1, 1);

	// View space's Z direction ranges from 0 to 1, so we set 1 since the ray goes "into" the screen
	XMVECTOR rayInViewspaceDirection = XMVectorSet(viewX, viewY, 1.0f, 0.0f);
	
	// Transform 3D Ray from View space to 3D ray in World space
	XMMATRIX camView;
	camera->GetViewMatrix(camView);

	XMVECTOR matInvDeter;	//We don't use this, but the xmmatrix inverse function requires the first parameter to not be null
	XMMATRIX makeRayToWorldspace = XMMatrixInverse(&matInvDeter, camView);	// Inverse of View Space matrix is going to get us the world space matrix
	
	// Position is at point (0,0,0) because we are in view space at this time.
	XMVECTOR rayInViewspacePosition = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	pickRayInWorldSpacePos = XMVector3TransformCoord(rayInViewspacePosition, makeRayToWorldspace);
	pickRayInWorldSpaceDir = XMVector3TransformNormal(rayInViewspaceDirection, makeRayToWorldspace);
}

float Graphics::Pick(XMVECTOR pickRayInWorldSpacePos, XMVECTOR pickRayInWorldSpaceDir, std::vector<XMFLOAT3>& vertPosArray, std::vector<DWORD>& indexPosArray, XMMATRIX worldSpace)
{
	/*
		Go from worldspace picking ray to local space picking to see if we hit the hitable model.

		1. Loop through each triangle in the object.
		2. Get the triangle as floats (because of our struct) and create 3 vectors. One for each side of the triangle. 
		3. Transform the vertices to worldspace. 

		4. Find the normal using two edges from the triangle and get the crossproduct of them. (U get the face normal of the triangle)
		5. Use one point from the triangle to get the planeequation with facenormals. 
	*/

	// Loop through each triangle in the object
	for (int i = 0; i < indexPosArray.size() / 3; i++)
	{				
		XMFLOAT3 triangleV1 = vertPosArray[indexPosArray[((i * (long long)3) + 0)]];
		XMFLOAT3 triangleV2 = vertPosArray[indexPosArray[((i * (long long)3) + 1)]];
		XMFLOAT3 triangleV3 = vertPosArray[indexPosArray[((i * (long long)3) + 2)]];

		XMVECTOR tri1V1 = XMVectorSet(triangleV1.x, triangleV1.y, triangleV1.z, 0.0f);
		XMVECTOR tri1V2 = XMVectorSet(triangleV2.x, triangleV2.y, triangleV2.z, 0.0f);
		XMVECTOR tri1V3 = XMVectorSet(triangleV3.x, triangleV3.y, triangleV3.z, 0.0f);

		// Transform the vertices to world space
		tri1V1 = XMVector3TransformCoord(tri1V1, worldSpace);
		tri1V2 = XMVector3TransformCoord(tri1V2, worldSpace);
		tri1V3 = XMVector3TransformCoord(tri1V3, worldSpace);

		// Find the normal using U, V coordinates (two edges)		
		XMVECTOR U = tri1V2 - tri1V1;
		XMVECTOR V = tri1V3 - tri1V1;

		// Compute face normal by crossing U, V
		XMVECTOR faceNormal = XMVector3Cross(U, V);
		faceNormal = XMVector3Normalize(faceNormal);

		// Calculate a point on the triangle for the plane equation
		XMVECTOR pointOnTheTriangle = tri1V1;


		/*
			Get plane equation ("Ax + By + Cz + D = 0") Variables
			To get "D", move the equation to the other side of the "=".

			xyz is the point of triangle (one point that completes the equation)
			ABC is the normal for the plane. 
		*/		

		float A = XMVectorGetX(faceNormal);
		float B = XMVectorGetY(faceNormal);
		float C = XMVectorGetZ(faceNormal);
		float D = (-A * XMVectorGetX(pointOnTheTriangle) - B * XMVectorGetY(pointOnTheTriangle) - C * XMVectorGetZ(pointOnTheTriangle));


		/*
			Now we find where (on the ray) the ray intersects with the triangles plane
			
			t = -(A*x2 + B*y2 + C*z2 + D) / (A*(x1-x2) + B*(y1-y2) + C*(z1-z2))

			ep1 = (A*x2 + B*y2 + C*z2 + D)	// ray originposition in worldspace
			ep2 = (A*(x1-x2) + B*(y1-y2) + C*(z1-z2) // We get the direction from ScreenSpaceToWorldSpace function

			We are breaking this equation down to two floats.
			Then making the equation after that to get the distance. 
		*/
		
		// The distance from ray origin to the picked model point is t. 
		float ep1 = (XMVectorGetX(pickRayInWorldSpacePos) * A) + (XMVectorGetY(pickRayInWorldSpacePos) * B) + (XMVectorGetZ(pickRayInWorldSpacePos) * C);
		float ep2 = (XMVectorGetX(pickRayInWorldSpaceDir) * A) + (XMVectorGetY(pickRayInWorldSpaceDir) * B) + (XMVectorGetZ(pickRayInWorldSpaceDir) * C);

		// Make sure there are no divide-by-zeros
		float t = 0.0f;
		if (ep2 != 0.0f)
			t = -(ep1 + D) / (ep2);

		// Make sure we don't pick objects behind the camera
		if (t > 0.0f)    
		{
			// Get the point on the plane
			float planeIntersectX = XMVectorGetX(pickRayInWorldSpacePos) + XMVectorGetX(pickRayInWorldSpaceDir) * t;
			float planeIntersectY = XMVectorGetY(pickRayInWorldSpacePos) + XMVectorGetY(pickRayInWorldSpaceDir) * t;
			float planeIntersectZ = XMVectorGetZ(pickRayInWorldSpacePos) + XMVectorGetZ(pickRayInWorldSpaceDir) * t;

			XMVECTOR pointInPlane = XMVectorSet(planeIntersectX, planeIntersectY, planeIntersectZ, 0.0f);

			// Call function to check if point is in the triangle
			if (CheckIfModelPointOnTriangle(tri1V1, tri1V2, tri1V3, pointInPlane))
			{
				// Return the distance to the hit, so we can check all the pickable objects in our scene
				return t / 2.0f;
			}
		}
	}

	// If an object was not picked
	return FLT_MAX;
}

bool Graphics::CheckIfModelPointOnTriangle(XMVECTOR& triV1, XMVECTOR& triV2, XMVECTOR& triV3, XMVECTOR& point)
{
	/*
		To find out if the point is inside the triangle, we will check to see if the point
		is on the correct side of each of the triangles edges.
	*/	

	XMVECTOR cp1 = XMVector3Cross((triV3 - triV2), (point - triV2));
	XMVECTOR cp2 = XMVector3Cross((triV3 - triV2), (triV1 - triV2));
	if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0) { 

		cp1 = XMVector3Cross((triV3 - triV1), (point - triV1));
		cp2 = XMVector3Cross((triV3 - triV1), (triV2 - triV1));
		if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0) {

			cp1 = XMVector3Cross((triV2 - triV1), (point - triV1));
			cp2 = XMVector3Cross((triV2 - triV1), (triV3 - triV1));
			if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0){
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	return false;
}

void Graphics::IntersectionTest(Inputs* input)
{
	/*
		Get mouse and keyboard inputs.
		Check if left mousebutton is pressed. -> Sending out one ray.
		Get mouseposition from screen to client coordinates.
		Calculates mouseposition from screen to world space coordinates.
		Checks if the worldspace coordinate from mouse picking is a coordinate of the picked models worldspace coordinates. 
	*/

	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];
		
	input->GetDirectInput8Mouse()->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	input->GetDirectInput8Keyboard()->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	
	if (mouseCurrState.rgbButtons[0]) {

		if (rayShot == false)
		{
			POINT mousePos;			
			GetCursorPos(&mousePos);
			ScreenToClient(dx11->GetHwnd(), &mousePos);

			int mousex = mousePos.x;
			int mousey = mousePos.y;

			float tempDist = -1;
			float closestDist = FLT_MAX;
			int modelIndex = 0;

			/* Go from 2d screenspace to worldspace coords */
			/* Check the coords on every pickable model and we if we hit something */
			XMVECTOR rayWorldspacePosition, rayWorldspaceDirection;
			ScreenSpaceToWorldSpace((float)mousex, (float)mousey, rayWorldspacePosition, rayWorldspaceDirection);
			for (int i = 0; i < pickableModels.size(); i++) {
				tempDist = Pick(rayWorldspacePosition, rayWorldspaceDirection, pickableModels[i]->GetVerticesArray(), pickableModels[i]->GetIndices(), pickableModels[i]->GetWorldMatrix());
				
				if (tempDist < closestDist) {
					closestDist = tempDist;	
					modelIndex = i;
				}											
			}	

			/* If the closest distance is less that 15.0f, we change color */
			if (closestDist < 15.0f) {
				if(modelIndex == 3)
					pickableModels[modelIndex]->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
				else
					pickableModels[modelIndex]->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
			}

			rayShot = true;
		}			
	}

	if (!mouseCurrState.rgbButtons[0]) {
		rayShot = false;
	}
			
	/* Reset rendering for picking sphere */
	if (keyboardState[DIK_R] & 0x80) {
		for(int i = 0; i < pickableModels.size(); i++)
			pickableModels[i]->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		/* palm */
		pickableModels[3]->GetMaterial()[0].diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);		
	}
}

bool Graphics::Render()
{
	// RENDER STUFF HERE
	bool result;
	DirectX::XMMATRIX view, projection;

	dx11->BeginScene(0.0f, 0.8f, 0.2f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	camera->GetViewMatrix(view);
	dx11->GetProjectionMatrix(projection);		
		

	/* Rest of the models here with default shader*/
	for (int i = 0; i < allModels.size(); i++) {
		allModels[i]->Render(dx11->GetContext());
		result = shader->Render(dx11->GetContext(), allModels[i], view, projection, camera, light, dx11->GetMinMagMipSampler());
		if (!result)
			return false;
	}


	/* Render sphere */
	cubemapSphere->Render(dx11->GetContext());
	result = shader->RenderWithCubemap(dx11->GetContext(), cubemapSphere, view, projection, cubeSrv, camera, light, dx11->GetAnisotropicSampler());
	if (!result)
		return false;
	

	/*Skybox render alone with skybox shader*/
	skybox->Render(dx11->GetContext());
	result = skyboxShader->Render(dx11->GetContext(), skybox, view, projection, camera, light, dx11->GetMinMagMipSampler());
	if (!result)
		return false;
		

	dx11->EnableAlphaBlending();

	/* Render billboardPlane */
	billboardPlane->Render(dx11->GetContext());
	result = shader->Render(dx11->GetContext(), billboardPlane, view, projection, camera, light, dx11->GetAnisotropicSampler());
	if (!result)
		return false;		
	
	/* Render particle system */
	particles->Render(dx11->GetContext());
	result = particleShader->RenderParticles(dx11->GetContext(), particles->GetIndexCount(), particles->GetWorldMatrix(), view, projection, particles->GetTexture(), dx11->GetMinMagMipSampler());
	if (!result)
		return false;

	dx11->DisableAlphaBlending();

	dx11->EndScene();
	return true;
}
