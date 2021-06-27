#pragma once
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <dinput.h>

class Inputs
{
public:
	Inputs(HWND hWnd);
	Inputs(const Inputs&);
	~Inputs();

	bool Initialize(HINSTANCE hinstance, HWND hwnd);
	void Shutdown();

	IDirectInput8* GetDirectInput8() { return m_directInput; }
	IDirectInputDevice8* GetDirectInput8Keyboard() { return m_keyboard; }
	IDirectInputDevice8* GetDirectInput8Mouse() { return m_mouse; }
	DIMOUSESTATE GetlastState() { return this->m_mouseState; }

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;
	HWND hwnd;
};