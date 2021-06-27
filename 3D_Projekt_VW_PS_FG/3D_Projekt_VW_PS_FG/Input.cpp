#include "Input.h"

Inputs::Inputs(HWND hwnd)
{
	this->hwnd = hwnd;
	m_directInput = 0;
	m_keyboard = 0;
	m_mouse = 0;

	for (int i = 0; i < 256; i++)
		m_keyboardState[i] = 0;
	ZeroMemory(&m_mouseState, sizeof(DIMOUSESTATE));
}

Inputs::Inputs(const Inputs& other)
{
	this->hwnd = other.hwnd;
	m_directInput = other.m_directInput;
	m_keyboard = other.m_keyboard;
	m_mouse = other.m_mouse;

	for (int i = 0; i < 256; i++)
		m_keyboardState[i] = 0;
	ZeroMemory(&m_mouseState, sizeof(DIMOUSESTATE));
}

Inputs::~Inputs()
{
}

bool Inputs::Initialize(HINSTANCE hinstance, HWND hwnd)
{
	HRESULT result;
	
	/*
		1. Initialize direct input 
		2. Create keyboard and mouse devices with directInput.
	*/

	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))	{
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))	{
		return false;
	}


	/*
		1. Set data format for keyboard and mouse, default formats.						(There are 5 different, 1 for keyboard, 2 for mouse, 2 for joystick)

		2. Establish the cooperative level for this instance of the device.
		The cooperative level determines how this instance of the device interacts with other instances of the device and the rest of the system.
		(Cooperative levels that they not gonna share with other programs)

		3. Acquire mouse and keyboard. (Basiclly makes them accecible to the input device)
	*/

	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	result = m_keyboard->Acquire();
	if (FAILED(result))
	{
		return false;
	}
	
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_BACKGROUND);
	if (FAILED(result))	{
		return false;
	}

	result = m_mouse->Acquire();
	if (FAILED(result)) {
		return false;
	}
	
	return true;
}

void Inputs::Shutdown()
{
	/*
		1. Release mouse and keyboard
		2. Release input device for mouse and keyboard
	*/

	if (m_mouse) {
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	if (m_keyboard)	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	if (m_directInput) {
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
}