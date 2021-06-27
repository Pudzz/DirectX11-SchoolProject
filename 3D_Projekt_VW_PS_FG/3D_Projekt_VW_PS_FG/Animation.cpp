#include "Animation.h"

Animation::Animation()
{
    this->boneAmount = 0;
    this->length = 0;
    this->name = "";
}

Animation::~Animation()
{
    bones.clear();
}

unsigned int Animation::GetAnimationLength()
{
    return this->length;
}

unsigned int Animation::GetBoneAmount()
{
    return this->boneAmount;
}

std::string Animation::GetName()
{
    return this->name;
}

std::vector<Joint>& Animation::GetBonesVector()
{
    return this->bones;
}

Joint& Animation::GetBoneAtIndex(unsigned int boneIndex)
{
    return this->bones[boneIndex];
}

void Animation::SetAnimationLength(unsigned int length)
{
    this->length = length;
}

void Animation::SetNumberOfBones(unsigned int number)
{
    this->boneAmount = number;
}

void Animation::SetAnimationName(std::string name)
{
    this->name = name;
}
