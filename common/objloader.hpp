// objloader.hpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
 Header for a simple OBJ loader with material support.
*/

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Very simple OBJ loader with material support.
bool loadOBJWithMaterials(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec3>& out_normals,
	std::vector<unsigned int>& out_indices,
	std::vector<unsigned int>& out_triangleMaterialIDs,
	std::vector<std::string>& out_materialNames);

#endif
