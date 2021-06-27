#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include "Models.h"
#include <DirectXMath.h>
#include "Shader.h"

namespace FbxLoader
{
	inline void loadModel(const aiScene* scene, ID3D11Device* device, Models* model)
	{


		std::vector<Vertex> vertices;
		std::vector<DWORD> indices;
		int vertexAmount = 0;

		aiMesh* mesh = scene->mMeshes[0];
		vertexAmount = mesh->mNumVertices;

		//Vertex information
		// For every vertex
		for (int i = 0; i < vertexAmount; i++)
		{
			Vertex tempVertex;
			tempVertex.pos.x = mesh->mVertices[i].x; tempVertex.pos.y = mesh->mVertices[i].y; tempVertex.pos.z = mesh->mVertices[i].z;
			tempVertex.normal.x = mesh->mNormals[i].x; tempVertex.normal.y = mesh->mNormals[i].y; tempVertex.normal.z = mesh->mNormals[i].z;
			tempVertex.texCoord.x = mesh->mTextureCoords[0][i].x; tempVertex.texCoord.y = mesh->mTextureCoords[0][i].y;

			tempVertex.tangent.x = mesh->mTangents[i].x; tempVertex.tangent.y = mesh->mTangents[i].y; tempVertex.tangent.z = mesh->mTangents[i].z;
			

			// Save the vertex
			vertices.push_back(tempVertex);
		}

		// For every face, save its indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			indices.push_back(mesh->mFaces[i].mIndices[0]);
			indices.push_back(mesh->mFaces[i].mIndices[1]);
			indices.push_back(mesh->mFaces[i].mIndices[2]);
		}

		Skeleton* tempSkeleton = nullptr;
		// If the mesh in the scene has more than 0 bones
		// We will then contstruct a skeleton
		if (mesh->mNumBones > 0)
		{
			tempSkeleton = new Skeleton();

			/*
			The reason we want to use GlobalMeshInverse and InverseBindpose (later in the code)
			is to bring us from the skeletons local space to each bones local space.
			*/
			// Collect the global mesh inverse transform from the scene
			tempSkeleton->SetGlobalMeshInverseTransformMatrix(DirectX::XMMATRIX(&scene->mRootNode->mTransformation.Inverse().a1));

			// For each bone
			for (unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				int boneIndex = 0;

				// Get the bones name
				std::string boneName = mesh->mBones[i]->mName.data;

				// Check if the bone is already in our map, if not, we create a new joint and saves it, together with an index
				if (tempSkeleton->boneMap.find(boneName) == tempSkeleton->boneMap.end())
				{
					boneIndex = tempSkeleton->GetBoneAmount();
					tempSkeleton->AddBoneAmount();
					tempSkeleton->boneMap[boneName] = boneIndex;

					Joint joint;

					// Sets the inverse bind pose matrix for each bone
					joint.SetInverseBindPoseMatrix(DirectX::XMMATRIX(&mesh->mBones[i]->mOffsetMatrix.a1));
					joint.SetFinalTransformMatrix(DirectX::XMMatrixIdentity());
					joint.SetName(boneName);

					// Pushes back the bone 
					tempSkeleton->AddNewBone(joint);
				}
				else
				{
					boneIndex = tempSkeleton->boneMap[boneName];
				}

				// For each bones amount of weights, which is how many vertices each bone is affecting
				for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
				{
					// Collect which vertices the bone affects
					int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
					// Saves the weight
					float weight = mesh->mBones[i]->mWeights[j].mWeight;

					// Checks if we already hasnt save a weight for this specifik vertex
					// Since we only supports one weight per vertex at the moment
					if (vertices[vertexId].weight == 0.0f)
					{
						vertices[vertexId].ID = boneIndex;
						vertices[vertexId].weight = weight;
					}
				}
			}
		}

		// Initialize the model
		model->InitializeFromFbx(vertices, indices, tempSkeleton, device);
	}

	inline SurfaceMaterial loadFbxMaterial(const aiScene* scene)
	{
		// Collects the materials on the scene
		aiMaterial** materialArray = scene->mMaterials;
		aiColor4D assimpSpecular, assimpDiffuse, assimpAmbient;
		float assimpShine;

		// Since we load one model at the time, we collect the first materials attributes
		aiGetMaterialColor(materialArray[0], AI_MATKEY_COLOR_SPECULAR, &assimpSpecular);
		aiGetMaterialColor(materialArray[0], AI_MATKEY_COLOR_DIFFUSE, &assimpDiffuse);
		aiGetMaterialColor(materialArray[0], AI_MATKEY_COLOR_AMBIENT, &assimpAmbient);
		aiGetMaterialFloat(materialArray[0], AI_MATKEY_SHININESS, &assimpShine);

		// Sets the materials attributes to our own structure
		SurfaceMaterial tempMaterial;
		tempMaterial.diffuseColor.x = assimpDiffuse.r; tempMaterial.diffuseColor.y = assimpDiffuse.g;
		tempMaterial.diffuseColor.z = assimpDiffuse.b; tempMaterial.diffuseColor.w = assimpDiffuse.a;

		tempMaterial.ambientColor.x = assimpAmbient.r; tempMaterial.ambientColor.y = assimpAmbient.g;
		tempMaterial.ambientColor.z = assimpAmbient.b; tempMaterial.ambientColor.w = assimpAmbient.a;
		
		tempMaterial.specularColor.x = assimpSpecular.r; tempMaterial.specularColor.y = assimpSpecular.g;
		tempMaterial.specularColor.z = assimpSpecular.b; tempMaterial.specularColor.w = assimpSpecular.a;
		tempMaterial.specularColor.w = assimpShine;

		tempMaterial.shine = assimpShine;
		return tempMaterial;
	}

	inline Texture* loadFbxTexture(ID3D11Device* device, aiString filepath)
	{
		Texture* tempTexture = new Texture;

		// Path to our textures
		std::string path = "Textures/";

		// Saves the filepath as a regular string instead of AiString
		std::string name = filepath.C_Str();

		// Finds the last position of "/" so that we can crop the string
		std::size_t position = name.find_last_of("/\\");

		// Puts "Textures/" and the name of the texture together
		path += name.substr(position + 1);

		// Convert to wstring so it can be used with wictextureloader
		std::wstring convertPath(path.begin(), path.end());
		
		// Loads the texture
		ID3D11ShaderResourceView* tempSrv;
		HRESULT hr = DirectX::CreateWICTextureFromFile(device, convertPath.c_str(), nullptr, &tempSrv);
		assert(SUCCEEDED(hr));

		tempTexture->SetTexture(tempSrv);
		tempTexture->SetName(name.substr(position + 1));

		return tempTexture;
	}

	inline const aiNodeAnim* checkNodeAnim(const aiAnimation* animation, const std::string nodeName)
	{
		// For every channel/curve in the current animation
		for (unsigned int i = 0; i < animation->mNumChannels; i++)
		{
			// Checks if the node responding to this channel, is the same node we are on in the main loop
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == nodeName)
			{
				// returns the node
				return nodeAnim;
			}
		}
		// If the node is not in this animation, return null
		return NULL;
	}

	inline void getPositionAtKeyframe(aiVector3D& position, unsigned int keyframe, const aiNodeAnim* nodeAnim)
	{
		// Check if there are more than 1 key of translation, otherwise return the translation at keyframe 0
		if (nodeAnim->mNumPositionKeys == 1)
		{
			position = nodeAnim->mPositionKeys[0].mValue;
			return;
		}

		// Get the position at the current keyframe
		const aiVector3D& pos = nodeAnim->mPositionKeys[keyframe].mValue;
		position = pos;

	}

	inline void getRotationAtKeyframe(aiQuaternion& rotation, unsigned int keyframe, const aiNodeAnim* nodeAnim)
	{
		// Check if there are more than 1 key of rotation, otherwise return the rotation at keyframe 0
		if (nodeAnim->mNumRotationKeys == 1)
		{
			rotation = nodeAnim->mRotationKeys[0].mValue;
			return;
		}

		// Get the rotation at the current keyframe and normalize it
		aiQuaternion& rot = nodeAnim->mRotationKeys[keyframe].mValue;
		rot.Normalize();
		rotation = rot;
	}

	inline void getScaleAtKeyframe(aiVector3D& scale, unsigned int keyframe, const aiNodeAnim* nodeAnim)
	{
		// Check if there are more than 1 key of scaling, otherwise return the scaling at keyframe 0
		if (nodeAnim->mNumScalingKeys == 1)
		{
			scale = nodeAnim->mScalingKeys[0].mValue;
			return;
		}

		// Get the scaling at the current keyframe
		aiVector3D& scaling = nodeAnim->mScalingKeys[keyframe].mValue;
		scale = scaling;
	}

	inline void ReadSceneHierarchy(unsigned int keyframe, const aiScene* scene, aiNode* node, DirectX::XMMATRIX parentTransform, Animation* animation, Skeleton* skeleton)
	{
		// Name of the current node
		std::string nodeName = node->mName.data;

		// First animation in the scene
		const aiAnimation* assimpAnimation = scene->mAnimations[0];

		// The nodes transformation relative to its parent
		// If this node is affected in the animation, it will be overwritten later
		DirectX::XMMATRIX nodeTransformation = DirectX::XMMATRIX(&node->mTransformation.a1);

		// Check if the node is affected by the animation
		const aiNodeAnim* nodeAnim = checkNodeAnim(assimpAnimation, nodeName);

		// If its not null, the nodes transformation will be overwritten here
		if (nodeAnim)
		{
			aiVector3D scaling, translation;
			aiQuaternion rotation;

			// Get the position, rotation and scaling at the current keyframe
			getPositionAtKeyframe(translation, keyframe, nodeAnim);
			getRotationAtKeyframe(rotation, keyframe, nodeAnim);
			getScaleAtKeyframe(scaling, keyframe, nodeAnim);

			// Convert the data from Assimps structure to DirectX XMVector
			DirectX::XMVECTOR scale = DirectX::XMVectorSet(scaling.x, scaling.y, scaling.z, 1.0f);
			DirectX::XMVECTOR trans = DirectX::XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
			DirectX::XMVECTOR rotate = DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, 1.0f);
			DirectX::XMVECTOR origin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			// Create a transformation matrix and transpose it, for multiplication purposes
			nodeTransformation = DirectX::XMMatrixAffineTransformation(scale, origin, rotate, trans);
			nodeTransformation = DirectX::XMMatrixTranspose(nodeTransformation);
		}

		// Multiply the transformation with its parent
		DirectX::XMMATRIX globalTransform = parentTransform * nodeTransformation;

		// If the current node is also a bone in the skeleton
		if (skeleton->boneMap.find(nodeName) != skeleton->boneMap.end())
		{
			unsigned int boneIndex = skeleton->boneMap[nodeName];

			// Save these to the animation struct, just for future reference
			//animation->GetBoneAtIndex(boneIndex).SetGlobalTransform(globalTransform);
			animation->GetBoneAtIndex(boneIndex).SetInverseBindPoseMatrix(skeleton->GetBone(boneIndex).GetInverseBindPoseMatrix());

			// Combine the transformation with the skeletons globalmeshInverse and the bones inverse bind pose
			DirectX::XMMATRIX final = skeleton->GetGlobalMeshInverseMatrix() * globalTransform * skeleton->GetBone(boneIndex).GetInverseBindPoseMatrix();

			// Save the transformation to the corresponding bone in the animation class
			animation->GetBoneAtIndex(boneIndex).SetFinalTransformMatrix(final);
		}

		// Repeat the function recursively for each child
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ReadSceneHierarchy(keyframe, scene, node->mChildren[i], globalTransform, animation, skeleton);
		}

	}

	inline void saveAnimationData(const aiScene* scene, Skeleton* skeleton, std::string animName)
	{
		DirectX::XMMATRIX identityMatrix = DirectX::XMMatrixIdentity();

		// If there is no animations in this scene
		if (!scene->mAnimations[0])
		{
			// Do whatever
		}

		else
		{
			// Length of the animation in ticks/frames
			float animationLength = (float)scene->mAnimations[0]->mDuration;

			// Store the needed animation data to our own animation class
			// Amount of bones is stored from the skeleton passed in the function
			Animation* newAnimation = new Animation();
			newAnimation->SetAnimationLength((unsigned int)animationLength);
			newAnimation->SetNumberOfBones(skeleton->GetBoneAmount());
			newAnimation->GetBonesVector().resize(newAnimation->GetBoneAmount());
			newAnimation->SetAnimationName(animName);

			// Read the scenes hierarchy once, for every keyframe
			for (unsigned int i = 0; i < animationLength; i++)
			{
				ReadSceneHierarchy(i, scene, scene->mRootNode, identityMatrix, newAnimation, skeleton);
			}

			// Saves the animation to the skeleton
			skeleton->animations.push_back(newAnimation);
		}

	}

	inline Models* loadFbxModel(const char* file, Shader* shader, ID3D11Device* device)
	{
		Assimp::Importer importer;

		// Read the fbx file
		const aiScene* scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
		Models* newModel = new Models;

		if (scene)
		{
			Texture* normalMap = nullptr;
			Texture* albedo = nullptr;
			aiString filepath;

			// Load the model
			loadModel(scene, device, newModel);

			// Load the materials attributes
			SurfaceMaterial material = loadFbxMaterial(scene);
			newModel->GetMaterial().push_back(material);

			// Load a texture
			if (scene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &filepath) == AI_SUCCESS)
			{
				albedo = loadFbxTexture(device, filepath);
				newModel->LoadFbxTexture(albedo);
				newModel->GetMaterial()[0].hasTexture = true;
			}

			// Load a normal map
			if (scene->mMaterials[0]->GetTexture(aiTextureType_NORMALS, 0, &filepath) == AI_SUCCESS)
			{
				normalMap = loadFbxTexture(device, filepath);
				newModel->LoadNormalMapFbx(normalMap);
				newModel->GetMaterial()[0].hasNormalMap = true;
			}
			
		}

		// Return the complete model
		return newModel;
	}
}