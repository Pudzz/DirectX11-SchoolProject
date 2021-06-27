#include "Joint.h"

Joint::Joint()
{
}

Joint::~Joint()
{
}

void Joint::SetInverseBindPoseMatrix(DirectX::XMMATRIX matrix)
{
	this->inverseBindPoseMatrix = matrix;
}

void Joint::SetFinalTransformMatrix(DirectX::XMMATRIX matrix)
{
	this->finalTransformationMatrices.push_back(matrix);
}

void Joint::SetName(std::string name)
{
	this->name = name;
}

DirectX::XMMATRIX Joint::GetInverseBindPoseMatrix()
{
	return this->inverseBindPoseMatrix;
}

DirectX::XMMATRIX Joint::GetFinalTransformationMatrix(unsigned int keyframeIndex)
{
	return this->finalTransformationMatrices[keyframeIndex];
}

std::vector<DirectX::XMMATRIX>& Joint::GetFinalTransformVector()
{
	return this->finalTransformationMatrices;
}

std::string Joint::GetName()
{
	return this->name;
}
