#pragma once
#include "DX11.h"
#include "Camera.h"
#include "Models.h"
#include "Shader.h"
#include "Light.h"
#include "objReader.h"
#include "FbxLoader.h"
#include "Terrain.h"
#include "ParticleSystem.h"

const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
	Graphics(Inputs* input);
	Graphics(const Graphics& other);
	~Graphics();

	bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();
	bool RenderFrame(float deltaTime);

	void Update(float deltaTime);

	/*  CUBEMAPPING */
	bool InitializeCubeMapViews(ID3D11Device* device);
	bool BuildCubeFaceCameras(float x, float y, float z);
	bool RenderCubemapping();	

	/* PICKING */
	void ScreenSpaceToWorldSpace(float mouseX, float mouseY, XMVECTOR& pickRayInWorldSpacePos, XMVECTOR& pickRayInWorldSpaceDir);
	float Pick(XMVECTOR pickRayInWorldSpacePos, XMVECTOR pickRayInWorldSpaceDir, std::vector<XMFLOAT3>& vertPosArray, std::vector<DWORD>& indexPosArray, XMMATRIX worldSpace);
	bool CheckIfModelPointOnTriangle(XMVECTOR& triV1, XMVECTOR& triV2, XMVECTOR& triV3, XMVECTOR& point);
	void IntersectionTest(Inputs* input);

private:
	bool Render();

private:
	DX11* dx11;
	Camera* camera;
	Camera* cubeCameras;

	/* Models rotating or calc some stuff*/
	Models* model;
	Models* skybox;
	
	XMFLOAT3 cubicTrans = { 40.0f, 15.0f, 25.0f };
	Models* cubemapSphere;
	Models* cubicCube;
	Models* pyramidCube;
	Models* donutCube;

	Models* water;

	Models* billboardPlane;
	Models* falloutQuadBackfaceTest;
	Models* fbxModel;
	Models* pepTalk;
	/*			 - - - - - - 			*/

	Shader* shader;
	Shader* skyboxShader;
	Shader* particleShader;	

	Light* light;		
	ParticleSystem* particles;	 

	/* Input devices */
	Inputs* input;
	IDirectInput8* directInput;
	IDirectInputDevice8* keyboard;
	IDirectInputDevice8* mouse;

	std::vector<Models*> pickableModels;
	std::vector<Models*> allModels;
	Assimp::Importer importer;
	const aiScene* scene;
	Terrain* terrain;

	/* CUBEMAPPING */
	const int CubeMapSize = 512;
	HRESULT hr;
	ID3D11RenderTargetView* cubeRtv[6];
	ID3D11ShaderResourceView* cubeSrv;
	ID3D11DepthStencilView* cubeDsv;
	D3D11_VIEWPORT cubeViewport;

	float rotSky = 0.0f;
	float rotQuad = 0.0f;
	float rotDonut = 0.0f;
	float rotCube = 0.0f;
	float rotPyramid = 0.0f;
	float rotPep = 0.0f;

	/* FOR PICKING */
	bool rayShot = false;
	int ScreenWidth, ScreenHeight;
	
	SurfaceMaterial terrainMat;
	SurfaceMaterial modelmat;

};