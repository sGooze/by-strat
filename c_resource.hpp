// c_resource.hpp
//	PURPOSE: Resource management system

#pragma once

#include <string>
#include <unordered_map>

// Used for resources loaded from external C libraries
class ExtResource {
public:
	virtual void Free() = 0;
};

template <class TY>
class Manager {
protected:
	std::unordered_map<std::string, TY> loaded;
	std::string path_start;
	Manager(const std::string& prefix) : path_start(prefix) {}
public:
	void Load(const std::string& path) {
		if (loaded.count(path) != 0) {
			//std::cout << path << ": already precached\n";
			return;
		}
		loaded.emplace(std::piecewise_construct, std::forward_as_tuple(path), std::forward_as_tuple(path_start + path));
	}
	TY& Get(const std::string& name) {
		if (loaded.count(name) == 0)
			// TODO: Return a placeholder if the requested resource is missing
			return loaded.at("NULL");
		return loaded.at(name);
	}
	virtual ~Manager() {}
};