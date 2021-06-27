#pragma once
#include <DirectXMath.h>
#include <string>
#include <vector>

class Joint
{
public:
	Joint();
	~Joint();

	// Set the inverse bind pose matrix for this joint
	void SetInverseBindPoseMatrix(DirectX::XMMATRIX matrix);

	// Pushes a new final tranformation matrix to the bone, each one corresponding a keyframe in an animation
	void SetFinalTransformMatrix(DirectX::XMMATRIX matrix);

	// Set the bones name
	void SetName(std::string name);

	// Get the inverse bind pose matrix
	DirectX::XMMATRIX GetInverseBindPoseMatrix();

	// Get the final transformation matrix at the given keyframe index
	DirectX::XMMATRIX GetFinalTransformationMatrix(unsigned int keyframeIndex);

	// Get the vector than contains all the final transformations, as a reference
	std::vector<DirectX::XMMATRIX>& GetFinalTransformVector();

	// Get the name of the bone
	std::string GetName();

private:
	// Bone name
	std::string name;

	// Inverse bind pose matrix
	DirectX::XMMATRIX inverseBindPoseMatrix;

	// Vector with final transformation matrices
	std::vector<DirectX::XMMATRIX> finalTransformationMatrices;
};