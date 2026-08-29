#ifndef MODEL_COLLECTION_HH
#define MODEL_COLLECTION_HH

#include "model.hh"

#include <cstdlib>
#include <string>
#include <unordered_map>

class ModelCollection
{
	private:
	std::unordered_map<std::string, std::unique_ptr<Model>> models;

	public:
		ModelCollection()=default;
		Model& load(const std::string& name, const std::string& objPath, const std::string& texturePath)
		{
			std::unique_ptr<Model> model=std::make_unique<Model>(objPath, texturePath);
			models[name]=std::move(model);
			return *models[name];
		}
		Model& get(const std::string& name)
		{
			return *models.at(name);
		}
};

#endif
