#include "Skeleton.h"

Skeleton::Skeleton()
{
    this->numberOfBones = 0;
    this->globalMeshInverseTransform = DirectX::XMMatrixIdentity();
    this->Currentkeyframe = 0;
    this->currentAnimation = nullptr;
}

Skeleton::~Skeleton()
{
    currentAnimation = nullptr;
    for (int i = 0; i < animations.size(); i++)
    {
        delete animations[i];
        animations[i] = nullptr;
    }
    animations.clear();
}

unsigned int Skeleton::GetBoneAmount()
{
    return this->numberOfBones;
}

DirectX::XMMATRIX Skeleton::GetGlobalMeshInverseMatrix()
{
    return this->globalMeshInverseTransform;
}

Joint& Skeleton::GetBone(unsigned int boneIndex)
{
    return this->bones[boneIndex];
}

int Skeleton::GetCurrentKeyframe()
{
    return this->Currentkeyframe;
}

Animation* Skeleton::GetCurrentAnimation()
{
    return this->currentAnimation;
}

void Skeleton::SetGlobalMeshInverseTransformMatrix(DirectX::XMMATRIX matrix)
{
    this->globalMeshInverseTransform = matrix;
}

void Skeleton::AddBoneAmount()
{
    this->numberOfBones++;
}

void Skeleton::AddNewBone(Joint newBone)
{
    this->bones.push_back(newBone);
}

void Skeleton::AddKeyframe()
{
    // Checks if we are at the end of the animation
    // Then we start over
    if (this->Currentkeyframe == this->currentAnimation->GetAnimationLength() - 1)
    {
        this->Currentkeyframe = 0;
    }
    // Or we go to the next keyframe
    else
    {
        Currentkeyframe++;
    }
}

void Skeleton::SetCurrentAnimation(Animation* anim)
{
    this->currentAnimation = anim;
}
