#pragma once
#define WIN32_LEAN_AND_MEAN
#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <assert.h>

#include "Graphics.h"
#include "Input.h"
#include "Timer.h"

#define DEFAULT_SCREEN_WIDTH 1280	
#define DEFAULT_SCREEN_HEIGHT 720

class Appsys {
public:
	Appsys(HINSTANCE hInstance);
	Appsys(const Appsys& other);
	~Appsys();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	bool Initialize();
	void Shutdown();
	void Run();		

private:
	bool UpdateOnFrame();
	void InitializeWindows(int& width, int& height);
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	const wchar_t* projectTitel = L"D3D11 Project";
	int screenWidth, screenHeight;
	HINSTANCE hInstance;
	HWND hwnd;	
	HRESULT hr;

	Graphics* graphics;
	Inputs* input;

	Timer fpsTimer;

	int frameCount = 0;
	double frameTime = 0;
};

static Appsys* appHandle;