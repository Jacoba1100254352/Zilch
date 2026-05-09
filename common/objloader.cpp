// objloader.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of a simple OBJ loader with material support.
*/

#include "objloader.hpp"

#include <cstdio>
#include <cstring>

// Very simple OBJ loader with material support.
bool loadOBJWithMaterials(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec3>& out_normals,
	std::vector<unsigned int>& out_indices,
	std::vector<unsigned int>& out_triangleMaterialIDs,
	std::vector<std::string>& out_materialNames
)
{
	// Inform user
	printf("Loading OBJ (with materials) file %s...\n", path);

	// Temporary storage
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec3> temp_normals;

	// Indices
	std::vector<unsigned int> vertexIndices;
	std::vector<unsigned int> normalIndices;
	std::vector<unsigned int> faceMaterialIDs; // one per triangle

	// Open the file
	FILE* file = fopen(path, "r");
	if (file == nullptr) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		return false;
	}

	// Current material tracking
	std::string currentMaterialName;
	unsigned int currentMaterialID = 0;
	bool hasCurrentMaterial = false;

	// Helper to get material ID, adding new materials as needed
	auto getMaterialID = [&](const std::string& name) -> unsigned int {
		for (std::size_t i = 0; i < out_materialNames.size(); ++i) {
			if (out_materialNames[i] == name) {
				return static_cast<unsigned int>(i);
			}
		}
		out_materialNames.push_back(name);
		return static_cast<unsigned int>(out_materialNames.size() - 1);
	};

	// Read file line by line
	while (true) {
		char lineHeader[128];
		if (int res = fscanf(file, "%127s", lineHeader); res == EOF) {
			break;
		}

		// Parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		} else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		} else if (strcmp(lineHeader, "usemtl") == 0) {
			char matName[128];
			// read material name until EOL or whitespace
			fscanf(file, "%127s", matName);
			currentMaterialName = matName;
			currentMaterialID = getMaterialID(currentMaterialName);
			hasCurrentMaterial = true;

			// consume rest of the line if any
			char buffer[1000];
			fgets(buffer, sizeof(buffer), file);
		} else if (strcmp(lineHeader, "f") == 0) {
			// Read face with format: vertex/uv/normal
			unsigned int vIdx[3], vtIdx[3], nIdx[3];
			int matches = fscanf(
				file,
				"%u/%u/%u %u/%u/%u %u/%u/%u\n",
				&vIdx[0], &vtIdx[0], &nIdx[0],
				&vIdx[1], &vtIdx[1], &nIdx[1],
				&vIdx[2], &vtIdx[2], &nIdx[2]
			);

			// Validate face format
			if (matches != 9) {
				printf("File can't be read by this parser. Check OBJ export options.\n");
				fclose(file);
				return false;
			}

			// Store vertex and normal indices
			vertexIndices.push_back(vIdx[0]);
			vertexIndices.push_back(vIdx[1]);
			vertexIndices.push_back(vIdx[2]);

			// Texture coordinates not used in this loader, so skip vtIdx
			normalIndices.push_back(nIdx[0]);
			normalIndices.push_back(nIdx[1]);
			normalIndices.push_back(nIdx[2]);

			// Assign material ID for this face
			if (!hasCurrentMaterial) {
				currentMaterialName = "default";
				currentMaterialID = getMaterialID(currentMaterialName);
				hasCurrentMaterial = true;
			}
			faceMaterialIDs.push_back(currentMaterialID);
		} else {
			// skip rest of line
			char buffer[1000];
			fgets(buffer, sizeof(buffer), file);
		}
	}

	// Done reading file
	fclose(file);

	// Validate presence of data
	if (temp_vertices.empty() || temp_normals.empty() ||
		vertexIndices.empty() || normalIndices.empty()) {
		printf("OBJ is missing vertices/normals/faces.\n");
		return false;
	}

	// Validate index counts
	if (vertexIndices.size() != normalIndices.size()) {
		printf("Vertex/normal index count mismatch.\n");
		return false;
	}

	// Calculate counts
	const std::size_t indexCount = vertexIndices.size();
	const std::size_t triangleCount = indexCount / 3;

	// Validate material IDs count
	if (faceMaterialIDs.size() != triangleCount) {
		printf("faceMaterialIDs does not match triangle count.\n");
		return false;
	}

	// Clear output arrays
	out_vertices.clear();
	out_normals.clear();
	out_indices.clear();
	out_triangleMaterialIDs.clear();

	// Reserve space
	out_vertices.reserve(indexCount);
	out_normals.reserve(indexCount);
	out_indices.reserve(indexCount);
	out_triangleMaterialIDs.reserve(triangleCount);

	// Expand to non-indexed vertex/normal arrays, with 1:1 index mapping
	for (std::size_t i = 0; i < indexCount; ++i) {
		// Get attribute indices
		unsigned int vIndex = vertexIndices[i];
		unsigned int nIndex = normalIndices[i];

		// Get attributes
		const glm::vec3& v = temp_vertices[vIndex - 1];
		const glm::vec3& n = temp_normals[nIndex - 1];

		// Store attributes
		out_vertices.push_back(v);
		out_normals.push_back(n);
		out_indices.push_back(static_cast<unsigned int>(i));
	}

	// Per-triangle material IDs (length = triangleCount)
	for (std::size_t t = 0; t < triangleCount; ++t) {
		out_triangleMaterialIDs.push_back(faceMaterialIDs[t]);
	}

	// Success
	return true;
}
