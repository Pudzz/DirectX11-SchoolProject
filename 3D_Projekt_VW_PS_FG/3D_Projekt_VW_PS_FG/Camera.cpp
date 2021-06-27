#include "Camera.h"
#include <iostream>

Camera::Camera()
{		
	this->camRotationMatrix = XMMatrixIdentity();
	this->viewMatrix = XMMatrixIdentity();

	this->input = 0;
	this->directInput = 0;
	this->mouse = 0;
	this->keyboard = 0;
	this->hwnd = 0;
		
	ZeroMemory(&this->mouseLastState, sizeof(DIMOUSESTATE));	

	this->terrain = 0;
}

Camera::Camera(HWND hwnd)
{
	this->camRotationMatrix = XMMatrixIdentity();
	this->viewMatrix = XMMatrixIdentity();

	this->input = 0;
	this->directInput = 0;
	this->mouse = 0;
	this->keyboard = 0;
	this->hwnd = hwnd;
	
	ZeroMemory(&this->mouseLastState, sizeof(DIMOUSESTATE));

	this->terrain = 0;
}

Camera::Camera(const Camera& other)
{
	this->camRotationMatrix = other.camRotationMatrix;
	this->directInput = other.directInput;
	this->keyboard = other.keyboard;
	this->mouse = other.mouse;
	this->terrain = other.terrain;
	this->hwnd = other.hwnd;

	this->camRotationMatrix = other.camRotationMatrix; 
	this->viewMatrix = other.viewMatrix;

	this->input = other.input;
	this->directInput = other.directInput;
	this->mouse = other.mouse;
	this->keyboard = other.keyboard;
	this->hwnd = other.hwnd;
		
	this->mouseLastState = other.mouseLastState;

	this->terrain = other.terrain;
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{	
	eyePosition.m128_f32[0] = x;
	eyePosition.m128_f32[1] = y;
	eyePosition.m128_f32[2] = z;
}

void Camera::SetTargetPosition(float x, float y, float z)
{
	this->lookAt = XMVectorSet(x, y, z, 1.0f);
}

void Camera::SetUpvector(float x, float y, float z)
{
	this->upVector = XMVectorSet(x, y, z, 0.0f);
}

DirectX::XMFLOAT3 Camera::GetLookAt()
{
	return DirectX::XMFLOAT3(lookAt.m128_f32[0], lookAt.m128_f32[1], lookAt.m128_f32[2]);
}

DirectX::XMFLOAT3 Camera::GetPosition()
{
	return DirectX::XMFLOAT3(eyePosition.m128_f32[0], eyePosition.m128_f32[1], eyePosition.m128_f32[2]);
}

void Camera::SetViewMatrix(XMVECTOR eye, XMVECTOR lookAt, XMVECTOR upVec)
{
	this->viewMatrix = XMMatrixLookAtLH(eye, lookAt, upVec);
}

void Camera::GetViewMatrix(DirectX::XMMATRIX& viewMatrix)
{
	viewMatrix = this->viewMatrix;
}

void Camera::GetInputs(Inputs* input)
{
	this->input = input;
	this->directInput = input->GetDirectInput8();
	this->keyboard = input->GetDirectInput8Keyboard();
	this->mouse = input->GetDirectInput8Mouse();
}

void Camera::DetectInputs(double deltaTime)

{   /*
	DIMOUSESTATE describes the state of a mouse device that has up to four buttons,
	or another device that is being accessed as if it were a mouse device. 
	*/
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	keyboard->Acquire();
	mouse->Acquire();

	mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	
	float speed = (float)(15.0f * deltaTime);

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseLastState.lX * 0.001f;
		camPitch += mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}

	UpdateCamera();
}

void Camera::UpdateCamera()
{	
	// Set standard definition for mainCamera || mainView
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0.0f);
	lookAt = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	lookAt = XMVector3Normalize(lookAt);

	// Set new value based on where and how my mouse is going
	XMMATRIX RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	upVector = XMVector3TransformCoord(upVector, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	// Move cameraposition to left and right based on keyboard and mouse inputs
	float camRightX = XMVectorGetX(camRight) * moveLeftRight;
	float camRightY = XMVectorGetY(camRight) * moveLeftRight;
	float camRightZ = XMVectorGetZ(camRight) * moveLeftRight;
	camRight = XMVectorSet(camRightX, camRightY, camRightZ, 0.0f);
	eyePosition += camRight;

	// Move cameraposition up and down based on keyboard and mouse inputs
	float camForwardX = XMVectorGetX(camForward) * moveBackForward;
	float camForwardY = XMVectorGetY(camForward) * moveBackForward;
	float camForwardZ = XMVectorGetZ(camForward) * moveBackForward;
	camForward = XMVectorSet(camForwardX, camForwardY, camForwardZ, 0.0f);
	eyePosition += camForward;

	// Reset values before next frame
	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	
	/* Sets the cameras Y position above the current triangle on the terrain 
	 If we are not walking on the terrain, Y position will be 0
	*/
	eyePosition.m128_f32[1] = (terrain->GetTriangleHeight(XMVectorGetX(eyePosition), XMVectorGetZ(eyePosition))) + 2.0f;
	
	lookAt = eyePosition + lookAt;

	// Create mainView matrix based of mouse ang keyboard movment
	viewMatrix = XMMatrixLookAtLH(eyePosition, lookAt, upVector);
}
