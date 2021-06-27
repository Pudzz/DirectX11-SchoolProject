#pragma once
#include <vector>
#include <map>
#include <string>
#include <DirectXMath.h>
#include "Joint.h"
#include "Animation.h"

class Skeleton
{
public:
	Skeleton();
	~Skeleton();

	// A map containing names of the skeletons bones and corresponding indices
	// Is used when the skeletons hierarchy is calculated during reading
	std::map<std::string, int> boneMap;

	// Vector which can hold several animations
	std::vector<Animation*> animations;

	// Get the number of bones in the this skeleton
	unsigned int GetBoneAmount();

	// Get the global mesh inverse matrix
	DirectX::XMMATRIX GetGlobalMeshInverseMatrix();

	// Get the bone at a given index as a reference
	Joint& GetBone(unsigned int boneIndex);

	// Get the current keyframe
	int GetCurrentKeyframe();

	// Get the current animation
	Animation* GetCurrentAnimation();

	// Set global mesh inverse transform
	void SetGlobalMeshInverseTransformMatrix(DirectX::XMMATRIX matrix);

	// Add +1 to the amount of bones in this skeleton
	void AddBoneAmount();

	// Pushes a new bone to the bone vector
	void AddNewBone(Joint newBone);

	// Changes keyframe during the update phase
	void AddKeyframe();

	// Set which animation is the current, can be used if we load several animations
	void SetCurrentAnimation(Animation* anim);

	
private:
	// The skeletons bones
	std::vector<Joint> bones;
	unsigned int numberOfBones;
	
	// The global mesh inverse transform is a matrix which corresponds to the whole model
	// If the model is made in Maya, this matrix will almost certainly always be an identity matrix
	DirectX::XMMATRIX globalMeshInverseTransform;

	// The current animation
	Animation* currentAnimation;

	// Current keyframe in the current animation
	int Currentkeyframe;
};