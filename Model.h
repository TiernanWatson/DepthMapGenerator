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

private:
	void LoadFrom(std::string& path);

	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Mesh> meshes;

	std::string directory;
};

