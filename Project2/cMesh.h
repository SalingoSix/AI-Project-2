#ifndef _HG_cMesh_
#define _HG_cMesh_

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cShaderProgram.h"

struct sVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct sTexture
{
	unsigned int ID;
	std::string type;
	std::string path;
};

class cMesh
{
public:
	std::vector<sVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<sTexture> textures;

	cMesh(std::vector<sVertex> theVertices, std::vector<unsigned int> theIndices, std::vector<sTexture> theTextures);
	void Draw(cShaderProgram shader);

private:
	unsigned int VAO, VBO, EBO;

	void setupMesh();
};

#endif
