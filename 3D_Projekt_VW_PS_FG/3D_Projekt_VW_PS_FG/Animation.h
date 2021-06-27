#pragma once
#include <vector>
#include <string>
#include "Joint.h"

class Animation
{
public:
	Animation();
	~Animation();

	// Get the length of the animation
	unsigned int GetAnimationLength();

	// Get the number of bones affected by this animation
	unsigned int GetBoneAmount();

	// Get the animation name
	std::string GetName();

	// Get the vector which contains all the bones affected by the animation, as reference
	std::vector<Joint>& GetBonesVector();

	// Get the specifik joint at an index, as a reference
	Joint& GetBoneAtIndex(unsigned int boneIndex);

	// Set the animations length
	void SetAnimationLength(unsigned int length);

	// Set the number of bones
	void SetNumberOfBones(unsigned int number);

	// Set the animations name
	void SetAnimationName(std::string name);

private:
	// Animation length
	unsigned int length;

	// bone amount
	unsigned int boneAmount;

	// Animation name
	std::string name;

	// Affected bones
	std::vector<Joint> bones;
};
