#ifndef MODEL_HH
#define MODEL_HH

#include <SFML/Graphics/Image.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>

class Model
{
	private:
		GLuint vbo=0;
		GLuint vao=0;
		GLuint texture;
		int verticesCount;

	public:
		Model(const std::string& objPath, const std::string& texturePath);
		void draw() const;
		~Model();

	private:
		void loadObj(const std::string& path);
		void setupBuffers(std::vector<float> vertices);
		void loadTexture(const std::string& path);
};

Model::Model(const std::string& objPath, const std::string& texturePath)
{
	loadObj(objPath);
	loadTexture(texturePath);
}

void Model::loadObj(const std::string& path)
{
	// opening file
	std::ifstream file(path);
	if(!file.is_open())
	{
		std::cerr<<"Failure: could not open "<<path<<"."<<std::endl;
		exit(1);
	}

	// info to recover from parsing
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> textCoords;
	std::vector<float> vertices;

	// parsing lines
	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream stream(line);
		std::string prefix;
		stream>>prefix;

		// positions
		if(prefix=="v")
		{
			float x, y, z;
			stream>>x>>y>>z;
			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
		}

		// normals
		else if(prefix=="vn")
		{
			float nx, ny, nz;
			stream>>nx>>ny>>nz;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);
		}

		// texture coordinates
		else if(prefix=="vt")
		{
			float u, v;
			stream>>u>>v;
			textCoords.push_back(u);
			textCoords.push_back(v);
		}

		// combine vertices info
		else if(prefix=="f")
		{
			int v1, vt1, vn1;
			int v2, vt2, vn2;
			int v3, vt3, vn3;
			char slash;
			stream	>>v1>>slash>>vt1>>slash>>vn1
					>>v2>>slash>>vt2>>slash>>vn2
					>>v3>>slash>>vt3>>slash>>vn3;

			if(stream.fail())
			{
				std::cerr<<"Failure: parsing failed on line "<<line<< std::endl;
				exit(1);
			}
		
			int v[]={(v1-1)*3, (v2-1)*3, (v3-1)*3};
			int vn[]={(vn1-1)*3, (vn2-1)*3, (vn3-1)*3};
			int vt[]={(vt1-1)*2, (vt2-1)*2, (vt3-1)*2};
			
			for(int i=0; i<3; i++)
			{
				vertices.push_back(positions[v[i]]);
				vertices.push_back(positions[v[i]+1]);
				vertices.push_back(positions[v[i]+2]);

				vertices.push_back(normals[vn[i]]);
				vertices.push_back(normals[vn[i]+1]);
				vertices.push_back(normals[vn[i]+2]);

				vertices.push_back(textCoords[vt[i]]);
				vertices.push_back(1.0f-textCoords[vt[i]+1]);
			}
		}
	}
	// save vertex quantity
	verticesCount=vertices.size()/8;
	setupBuffers(vertices);
}

void Model::setupBuffers(std::vector<float> vertices)
{
	//VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	//VBO
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
	int AttribSize=8*sizeof(float);

	// positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, AttribSize, NULL);
	glEnableVertexAttribArray (0);
	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, AttribSize, (void*)(3*sizeof(float)));
	glEnableVertexAttribArray (1);
	// texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, AttribSize, (void*)(6*sizeof(float)));
	glEnableVertexAttribArray (2);
}

void Model::loadTexture(const std::string& path)
{
	sf::Image image;
	if(!image.loadFromFile(path))
	{
		std::cerr<<"Failure: could not load texture image"<<path<<"."<<std::endl;
		exit(1);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	sf::Vector2u size=image.getSize();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Model::draw() const
{
	// apply texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	// draw triangles
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, verticesCount);

	// clean
	glBindVertexArray(0);
}

Model::~Model()
{
	if(vao) glDeleteVertexArrays(1, &vao);
	if(vbo) glDeleteBuffers(1, &vbo);
	if(texture) glDeleteTextures(1, &texture);
}

#endif
