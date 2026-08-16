#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

class Model
{
	private:
		GLuint vbo;
		GLuint vao;
		GLuint texture;
		int verticesCount;

	public:
		Model(std::string objPath, std::string texturePath)
		{
			loadObj(objPath);
			loadTexture(texturePath);
		}

		~Model()
		{
			if(vao) glDeleteVertexArrays(1, &vao);
			if(vbo) glDeleteBuffers(1, &vbo);
       		if(texture) glDeleteTextures(1, &texture);
		}

		void loadObj(std::string path)
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

		void setupBuffers(std::vector<float> faces)
		{
			//VBO
			vbo=0;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, faces.size()*sizeof(float), faces.data(), GL_STATIC_DRAW);

			//VAO
			vao=0;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
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

		void loadTexture(std::string path)
		{
			sf::Image image;
			if(!image.loadFromFile(path))
			{
				std::cerr<<"Failure: could not load texture image"<<path<<"."<<std::endl;
				exit(1);
			}

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			sf::Vector2u size=image.getSize();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		void draw()
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, verticesCount);

			glBindVertexArray(0);
		}
};
