#pragma once
#include "Mesh.h"
#include <string>

struct aiNode;
struct aiScene;
struct aiMesh;

class Model
{
public:
	Model(std::string path);

	void Draw();

	const std::vector<Mesh>& GetMeshes() const
	{
		return meshes;
	}

	std::string GetFileName() const
	{
		return fileName;
	}

	std::string GetDirectory() const
	{
		return directory;
	}

private:
	void LoadFrom(std::string& path);

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Mesh> meshes;

	std::string directory;
	std::string fileName;
};

