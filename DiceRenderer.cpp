// DiceRenderer.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the DiceRenderer class, which is responsible for rendering 3D dice
using SFML and OpenGL. It includes functionality for loading 3D models, computing die orientations based on values,
and rendering animated dice rolls with lighting effects.
*/

#include "DiceRenderer.h"
#include "ResourcePath.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <numbers>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <common/objloader.hpp> // Utility for loading .obj files

namespace {
	constexpr float DISPLAY_SCALE = 50.f;
	constexpr float ANIMATION_DURATION = 2.5f;
	constexpr float AMBIENT_TERM = 0.25f;
	constexpr float PIP_MIN_FACING_COS = 0.12f;
	constexpr float SETTLE_TIME = 2.0f;
	constexpr float SPIN_RATE = 4.2f;
	constexpr float SPIN_PHASE_PER_DIE = 0.25f;
	constexpr float ROTATE_X_SPIN_FACTOR = 0.5f;
	constexpr float ROTATE_Z_SPIN_FACTOR = 0.3f;
	constexpr std::size_t DICE_PER_ROW = 3;
	constexpr float DIE_SPACING_X = 160.f;
	constexpr float DIE_SPACING_Y = 140.f;
	constexpr float DIE_GRID_Y_FRACTION = 0.65f; // Push dice below buttons
	constexpr float LABEL_OFFSET_X = -10.f;
	constexpr float LABEL_OFFSET_Y = 60.f;
	constexpr unsigned LABEL_FONT_SIZE = 22;
	constexpr float MODEL_SCALE = 0.01f;
	const sf::Color BODY_COLOR(204, 0, 0);
	const sf::Color PIP_COLOR (242, 242, 242);
	const sf::Vector3f VIEW_DIR {0.f, 0.f, -1.f};

	// Rotate around the Y axis.
	sf::Vector3f rotateY(const sf::Vector3f& v, const float a) {
		const float c = std::cos(a), s = std::sin(a);
		return {v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
	}
	// Rotate around the X axis.
	sf::Vector3f rotateX(const sf::Vector3f& v, const float a) {
		const float c = std::cos(a), s = std::sin(a);
		return {v.x, v.y * c - v.z * s, v.y * s + v.z * c};
	}
	// Rotate around the Z axis.
	sf::Vector3f rotateZ(const sf::Vector3f& v, const float a) {
		const float c = std::cos(a), s = std::sin(a);
		return {v.x * c - v.y * s, v.x * s + v.y * c, v.z};
	}

	// Apply Euler rotations in Y-X-Z order.
	sf::Vector3f applyRotation(const sf::Vector3f& v, const sf::Vector3f& rot) {
		return rotateZ(rotateX(rotateY(v, rot.y), rot.x), rot.z);
	}

	// Normalize a 3D vector safely.
	sf::Vector3f normalize3(sf::Vector3f v) {
		if (const float lenSq = v.x*v.x + v.y*v.y + v.z*v.z; lenSq > 1e-10f) {
			const float inv = 1.0f / std::sqrt(lenSq);
			v.x *= inv; v.y *= inv; v.z *= inv;
		}
		return v;
	}

	// Dot product of two 3D vectors.
	float dot3(const sf::Vector3f& a, const sf::Vector3f& b) {
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}

	// Length of a 3D vector.
	float length3(const sf::Vector3f& v) {
		return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	// Classify a vector to one of six axis-aligned faces.
		std::size_t faceIndexFromVector(const sf::Vector3f& v) {
		const float ax = std::fabs(v.x);
		const float ay = std::fabs(v.y);
		const float az = std::fabs(v.z);
		if (ax >= ay && ax >= az) {
			return v.x >= 0.f ? 0 : 1;	// +X / -X
		}
		if (ay >= ax && ay >= az) {
			return v.y >= 0.f ? 2 : 3;	// +Y / -Y
		}
		return v.z >= 0.f ? 4 : 5;		// +Z / -Z
	}

	// Get axis direction vector from face index.
		sf::Vector3f axisDirFromIndex(const std::size_t idx) {
		switch (idx) {
			case 0: return { 1.f, 0.f, 0.f};
			case 1: return {-1.f, 0.f, 0.f};
			case 2: return { 0.f, 1.f, 0.f};
			case 3: return { 0.f,-1.f, 0.f};
			case 4: return { 0.f, 0.f, 1.f};
			default: return { 0.f, 0.f,-1.f};
		}
	}

	// Compute Euler angles that orient the given direction toward -Z (camera).
	sf::Vector3f eulerForFaceDirection(const sf::Vector3f& dir) {
		const float ax = std::fabs(dir.x);
		const float ay = std::fabs(dir.y);
		const float az = std::fabs(dir.z);
		constexpr float halfPi = static_cast<float>(std::numbers::pi) * 0.5f;
		constexpr float fullPi = static_cast<float>(std::numbers::pi);

		if (ax >= ay && ax >= az) {
			return {0.f, dir.x > 0.f ? halfPi : -halfPi, 0.f};
		}
		if (ay >= ax && ay >= az) {
			return {dir.y > 0.f ? -halfPi : halfPi, 0.f, 0.f};
		}
		// Z-dominant: -Z already faces camera
		return {0.f, dir.z > 0.f ? fullPi : 0.f, 0.f};
	}
} // namespace

/**
 * @brief Ensures that the necessary resources for rendering dice are loaded.
 *
 * @throws std::runtime_error if loading models fails.
 */
void DiceRenderer::ensureResourcesLoaded() const {
	if (resourcesLoaded) return;

	// Load OBJ data and materials.
	std::vector<unsigned int> allIndices;
	std::vector<unsigned int> matIDs;
	std::vector<std::string> matNames;
	const std::string diceModelPath = resolveResourcePath("graphics/dice.obj");

	if (!loadOBJWithMaterials(
			diceModelPath.c_str(),
			vertices,
			normals,
			allIndices,
			matIDs,
			matNames
		)) {
		throw std::runtime_error("Failed to load dice.obj");
		}

	// Scale vertices into world units.
	for (glm::vec3& v : vertices) {
		v *= MODEL_SCALE;
	}

	// Clear existing index lists.
	bodyIndices.clear();
	pipIndices.clear();
	for (auto& faceIndices : pipFaceIndices) {
		faceIndices.clear();
	}

	// Partition triangles by material.
	const std::size_t triCount = matIDs.size();
	for (std::size_t t = 0; t < triCount; ++t) {
		unsigned int i0 = allIndices[3*t + 0];
		unsigned int i1 = allIndices[3*t + 1];
		unsigned int i2 = allIndices[3*t + 2];
		const unsigned int mid = matIDs[t];
		if (const std::string& mname = matNames[mid]; mname == "Material") {
			bodyIndices.push_back(i0);
			bodyIndices.push_back(i1);
			bodyIndices.push_back(i2);
		} else if (mname == "Material.001") {
			pipIndices.push_back(i0);
			pipIndices.push_back(i1);
			pipIndices.push_back(i2);

			const glm::vec3 centroid = (vertices.at(i0) + vertices.at(i1) + vertices.at(i2)) / 3.f;
			auto& faceIndices = pipFaceIndices.at(faceIndexFromVector({centroid.x, centroid.y, centroid.z}));
			faceIndices.push_back(i0);
			faceIndices.push_back(i1);
			faceIndices.push_back(i2);
		}
	}

	// Mark resources as loaded.
	resourcesLoaded = true;
}

/**
 * @brief Determines the orientation for a given die value.
 *
 * @param value The die value (1-6).
 *
 * @return The corresponding orientation as a 3D vector.
 */
sf::Vector3f DiceRenderer::orientationForValue(const uint16_t value) const {
	ensureResourcesLoaded();
	if (!orientationsComputed) {
		// Accumulate normals and areas per face.
		struct FaceAccum {
			sf::Vector3f normalSum{};
			float areaSum = 0.f;
		};

		std::array<FaceAccum,6> faces{};

		// Walk body triangles to accumulate normals. Parallelized when OMP is available.
#ifdef _OPENMP
#pragma omp parallel default(none) shared(faces, vertices, bodyIndices)
		{
			std::array<FaceAccum,6> localFaces{};
#pragma omp for nowait
			// ReSharper disable CppDFAUnreadVariable CppDFAUnusedValue
			for (std::size_t tri = 0; tri < bodyIndices.size() / 3; ++tri) {
				const std::size_t i = tri * 3;
				const glm::vec3& gp0 = vertices.at(bodyIndices[i + 0]);
				const glm::vec3& gp1 = vertices.at(bodyIndices[i + 1]);
				const glm::vec3& gp2 = vertices.at(bodyIndices[i + 2]);

				const sf::Vector3f e1{gp1.x - gp0.x, gp1.y - gp0.y, gp1.z - gp0.z};
				const sf::Vector3f e2{gp2.x - gp0.x, gp2.y - gp0.y, gp2.z - gp0.z};
				sf::Vector3f normal {
					e1.y*e2.z - e1.z*e2.y,
					e1.z*e2.x - e1.x*e2.z,
					e1.x*e2.y - e1.y*e2.x
				};

				// Area is half the length of the cross product.
				const float area = length3(normal) * 0.5f;
				if (area < 1e-7f) {
					continue;
				}

				// Accumulate into local face.
					const std::size_t idx = faceIndexFromVector(normal);
				localFaces.at(idx).normalSum.x += normal.x;
				localFaces.at(idx).normalSum.y += normal.y;
				localFaces.at(idx).normalSum.z += normal.z;
				localFaces.at(idx).areaSum += area;
			}
#pragma omp critical
			{
				// Merge local accumulations into global.
					for (std::size_t j = 0; j < 6; ++j) {
					faces[j].normalSum.x += localFaces[j].normalSum.x;
					faces[j].normalSum.y += localFaces[j].normalSum.y;
					faces[j].normalSum.z += localFaces[j].normalSum.z;
					faces[j].areaSum += localFaces[j].areaSum;
				}
			}
		}
#else
		for (std::size_t tri = 0; tri < bodyIndices.size() / 3; ++tri) {
			const std::size_t i = tri * 3;
			const glm::vec3& gp0 = vertices.at(bodyIndices[i + 0]);
			const glm::vec3& gp1 = vertices.at(bodyIndices[i + 1]);
			const glm::vec3& gp2 = vertices.at(bodyIndices[i + 2]);

			const sf::Vector3f e1{gp1.x - gp0.x, gp1.y - gp0.y, gp1.z - gp0.z};
			const sf::Vector3f e2{gp2.x - gp0.x, gp2.y - gp0.y, gp2.z - gp0.z};
			sf::Vector3f normal {
				e1.y*e2.z - e1.z*e2.y,
				e1.z*e2.x - e1.x*e2.z,
				e1.x*e2.y - e1.y*e2.x
			};

			const float area = length3(normal) * 0.5f;
			if (area < 1e-7f) {
				continue;
			}

			const std::size_t idx = faceIndexFromVector(normal);
			faces.at(idx).normalSum.x += normal.x;
			faces.at(idx).normalSum.y += normal.y;
			faces.at(idx).normalSum.z += normal.z;
			faces.at(idx).areaSum += area;
		}
#endif

		// Compute final face normals.
		std::array<sf::Vector3f,6> faceNormals{};
		for (std::size_t i = 0; i < 6; ++i) {
			if (faces[i].areaSum > 0.f) {
				faceNormals[i] = normalize3(faces[i].normalSum);
			} else {
				faceNormals[i] = axisDirFromIndex(i);
			}
		}

		std::array<std::size_t,6> pipCounts{};
		// Count pips per face. Parallelized when OMP is available.
#ifdef _OPENMP
#pragma omp parallel default(none) shared(pipCounts, vertices, pipIndices)
		{
			std::array<std::size_t,6> localCounts{};
#pragma omp for nowait
			for (std::size_t tri = 0; tri < pipIndices.size() / 3; ++tri) {
				const std::size_t i = tri * 3;
				const glm::vec3& gp0 = vertices.at(pipIndices[i + 0]);
				const glm::vec3& gp1 = vertices.at(pipIndices[i + 1]);
				const glm::vec3& gp2 = vertices.at(pipIndices[i + 2]);
				const sf::Vector3f centroid {
					(gp0.x + gp1.x + gp2.x) / 3.f,
					(gp0.y + gp1.y + gp2.y) / 3.f,
					(gp0.z + gp1.z + gp2.z) / 3.f
				};
				const std::size_t idx = faceIndexFromVector(centroid);
				++localCounts.at(idx);
			}
#pragma omp critical
			{
				for (std::size_t j = 0; j < 6; ++j) pipCounts[j] += localCounts[j];
			}
		}
#else
		for (std::size_t tri = 0; tri < pipIndices.size() / 3; ++tri) {
			const std::size_t i = tri * 3;
			const glm::vec3& gp0 = vertices.at(pipIndices[i + 0]);
			const glm::vec3& gp1 = vertices.at(pipIndices[i + 1]);
			const glm::vec3& gp2 = vertices.at(pipIndices[i + 2]);
			const sf::Vector3f centroid {
				(gp0.x + gp1.x + gp2.x) / 3.f,
				(gp0.y + gp1.y + gp2.y) / 3.f,
				(gp0.z + gp1.z + gp2.z) / 3.f
			};
			const std::size_t idx = faceIndexFromVector(centroid);
			++pipCounts.at(idx);
		}
#endif

		// Determine face order by pip count.
		std::array<std::size_t,6> faceOrder{0,1,2,3,4,5};
		std::ranges::sort(
			faceOrder,
			[&](const std::size_t a, const std::size_t b)->bool {
				if (pipCounts[a] == pipCounts[b]) return a < b;
				return pipCounts[a] < pipCounts[b];
			}
		);

		// Compute orientations for each die value.
		for (std::size_t v = 0; v < 6; ++v) {
			const std::size_t faceIdx = faceOrder[v];
			valueOrientations[v] = eulerForFaceDirection(faceNormals[faceIdx]);
		}

		// Mark orientations as computed.
		orientationsComputed = true;
	}

	// Return orientation for requested value.
	if (value == 0) return valueOrientations[0];
	return valueOrientations[(value-1) % valueOrientations.size()];
}

/**
 * @brief Adds triangles for a specific material with CPU-side lighting and backface culling.
 *
 * @param rotation The rotation to apply in 3D.
 * @param offset The 2D offset for positioning on screen.
 * @param indices The indices of the vertices to add.
 * @param baseColor Base color for this material (before lighting).
 * @param outTris Output vector to store the resulting triangles.
 * @param minFacingCos Minimum cosine of angle to camera to keep the triangle.
 */
void DiceRenderer::addTrianglesForMaterial(
    const sf::Vector3f& rotation,
    const sf::Vector2f& offset,
    const std::vector<unsigned int>& indices,
    const sf::Color& baseColor,
    std::vector<Triangle2D>& outTris,
    const float minFacingCos
) const {
	const sf::Vector3f lightDir = normalize3(VIEW_DIR);

	// Walk triangles and build lit, culled 2D geometry.
	for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
		// Fetch vertex indices.
		unsigned int i0 = indices[i + 0];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

		// Fetch 3D vertex positions.
        const glm::vec3& gp0 = vertices.at(i0);
        const glm::vec3& gp1 = vertices.at(i1);
        const glm::vec3& gp2 = vertices.at(i2);

		// Convert to SFML vectors.
        sf::Vector3f v0{gp0.x, gp0.y, gp0.z};
        sf::Vector3f v1{gp1.x, gp1.y, gp1.z};
        sf::Vector3f v2{gp2.x, gp2.y, gp2.z};

        // Apply current rotation.
        v0 = applyRotation(v0, rotation);
        v1 = applyRotation(v1, rotation);
        v2 = applyRotation(v2, rotation);

        // Compute face normal.
        sf::Vector3f e1{v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        sf::Vector3f e2{v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        sf::Vector3f normal {
            e1.y*e2.z - e1.z*e2.y,
            e1.z*e2.x - e1.x*e2.z,
            e1.x*e2.y - e1.y*e2.x
        };

        // Backface/edge-on culling.
        normal = normalize3(normal);
		if (const float facing = dot3(normal, VIEW_DIR); facing <= minFacingCos) {
            continue; // back-face or edge-on culling
        }
        // Basic Lambert shading.
        float lambert = std::max(0.f, dot3(normal, lightDir));
        float intensity = std::clamp(AMBIENT_TERM + lambert, 0.f, 1.f);

		// Apply shading to base color.
        sf::Color shaded(
            static_cast<sf::Uint8>(static_cast<float>(baseColor.r) * intensity),
            static_cast<sf::Uint8>(static_cast<float>(baseColor.g) * intensity),
            static_cast<sf::Uint8>(static_cast<float>(baseColor.b) * intensity),
            baseColor.a
        );

		// Convert to 2D vertex.
        auto toVertex = [&](const sf::Vector3f& v)->sf::Vertex {
            sf::Vertex vert;
            vert.position = {v.x * DISPLAY_SCALE + offset.x, v.y * DISPLAY_SCALE + offset.y};
            vert.color    = shaded;
            vert.texCoords = {0.f, 0.f};
            return vert;
        };

        // Push triangle into output list.
        Triangle2D tri;
        tri.v0 = toVertex(v0);
        tri.v1 = toVertex(v1);
        tri.v2 = toVertex(v2);
        tri.depth = (v0.z + v1.z + v2.z) / 3.0f;
        outTris.push_back(tri);
    }
}

/**
 * @brief Starts the dice roll animation with the specified die values.
 *
 * @param diceValues A vector of die values to roll.
 */
void DiceRenderer::startRoll(const std::vector<uint16_t>& diceValues) const {
	ensureResourcesLoaded();

	// Stash incoming values and randomize base spin phases.
	currentValues = diceValues;
	baseAngles.clear();
	baseAngles.reserve(currentValues.size());

	// Random angle generator.
	std::mt19937 rng(static_cast<unsigned>(
		std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	std::uniform_real_distribution<float> dist(0.f, 2.f * static_cast<float>(std::numbers::pi));

	// Assign random base angles for each die.
	for (std::size_t i = 0; i < currentValues.size(); ++i) {
		baseAngles.push_back(dist(rng));
	}

	// Start animation timer.
	rollStart = std::chrono::steady_clock::now();
	animating = true;
}

/**
 * @brief Updates the displayed dice values without triggering an animation.
 *
 * @param diceValues A vector of die values to display.
 */
void DiceRenderer::updateValues(const std::vector<uint16_t>& diceValues) const {
	ensureResourcesLoaded();
	// Directly set values without animation.
	currentValues = diceValues;
	baseAngles.clear();
	rollStart = std::chrono::steady_clock::now();
	animating = false;
}

/**
 * @brief Draws the dice onto the specified render target.
 *
 * @param target The render target to draw onto.
 * @param labelFont Optional font for rendering die value labels.
 */
void DiceRenderer::draw(sf::RenderTarget& target, const sf::Font* labelFont) const {
	// Nothing to draw if no dice values.
    if (currentValues.empty()) return;

	// Ensure resources are loaded.
    ensureResourcesLoaded();

	// Compute elapsed time since roll started.
	const auto elapsed = std::chrono::steady_clock::now() - rollStart;
	float seconds = static_cast<float>(
		std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
	) / 1000.f;

	// Clamp animation time.
	if (!animating) {
		seconds = ANIMATION_DURATION;
	} else if (seconds >= ANIMATION_DURATION) {
		seconds = ANIMATION_DURATION;
		animating = false;
	}

	// Compute settle factor (0 to 1).
	const float settle = std::clamp(seconds / SETTLE_TIME, 0.f, 1.f);

	// Compute base offset for centering dice grid.
	const sf::Vector2u tsize = target.getSize();
	constexpr float gridWidth = DIE_SPACING_X * static_cast<float>(DICE_PER_ROW > 0 ? DICE_PER_ROW - 1 : 0);
	const sf::Vector2f baseOffset(
		static_cast<float>(tsize.x) * 0.5f - gridWidth * 0.5f,
		static_cast<float>(tsize.y) * DIE_GRID_Y_FRACTION
	);

	// After computing settle:
	const bool showLabels = (settle >= 1.f);

	// Only create labels if we will show them
	std::vector<sf::Text> labels;
	if (showLabels && labelFont && !labelFont->getInfo().family.empty()) {
		labels.reserve(currentValues.size());
		for (const uint16_t v : currentValues) {
			sf::Text txt(std::to_string(v), *labelFont, LABEL_FONT_SIZE);
			txt.setFillColor(sf::Color::White);
			labels.push_back(txt);
		}
	}

	// The camera faces along -Z, so larger Z values are farther away.
	auto sortBackToFront = [](std::vector<Triangle2D>& triangles)->void {
		std::ranges::sort(
			triangles, [](const Triangle2D& a, const Triangle2D& b)->bool
			{
				return a.depth > b.depth;
			}
		);
	};

	// Reuse triangle buffers across dice to avoid reallocating large model data.
	std::vector<Triangle2D> bodyTris;
	bodyTris.reserve(bodyIndices.size() / 3);
	std::vector<Triangle2D> pipTris;
	pipTris.reserve(pipIndices.size() / 3);

	// Draw each die
	for (std::size_t i = 0; i < currentValues.size(); ++i) {
		// Compute per-die offset in grid.
		const std::size_t row = i / DICE_PER_ROW;
		const std::size_t col = i % DICE_PER_ROW;
		sf::Vector2f offset(
			baseOffset.x + static_cast<float>(col) * DIE_SPACING_X,
			baseOffset.y + static_cast<float>(row) * DIE_SPACING_Y
		);

		// Compute current orientation and spin.
		const sf::Vector3f orient = orientationForValue(currentValues[i]);
		const float spin = baseAngles.empty()
			? 0.f
			: baseAngles[i] + seconds * SPIN_RATE + static_cast<float>(i) * SPIN_PHASE_PER_DIE;

		// Interpolate rotation from spin to final orientation.
		sf::Vector3f rotation {
			std::lerp(spin * ROTATE_X_SPIN_FACTOR, orient.x, settle),
			std::lerp(spin,                        orient.y, settle),
			std::lerp(spin * ROTATE_Z_SPIN_FACTOR, orient.z, settle)
		};

		// Draw the die body first.
		bodyTris.clear();
		addTrianglesForMaterial(rotation, offset, bodyIndices, BODY_COLOR, bodyTris);
		sortBackToFront(bodyTris);
		for (const Triangle2D& t : bodyTris) {
			target.draw(&t.v0, 3, sf::Triangles);
		}

		// The model's recessed pip mesh overlaps the body surface. Draw only pip
		// groups whose cube face points toward the camera, then composite those
		// groups over the body. Per-triangle culling alone admits curved triangles
		// from hidden faces, which produces white fragments around the die edges.
		pipTris.clear();
		for (std::size_t face = 0; face < pipFaceIndices.size(); ++face) {
			const sf::Vector3f faceNormal = applyRotation(axisDirFromIndex(face), rotation);
			if (dot3(faceNormal, VIEW_DIR) <= PIP_MIN_FACING_COS) {
				continue;
			}
			addTrianglesForMaterial(
				rotation,
				offset,
				pipFaceIndices[face],
				PIP_COLOR,
				pipTris,
				PIP_MIN_FACING_COS
			);
		}
		sortBackToFront(pipTris);
		for (const Triangle2D& t : pipTris) {
			target.draw(&t.v0, 3, sf::Triangles);
		}

		// Inside the per-die loop, replace label draw block:
		if (showLabels && !labels.empty()) {
			labels[i].setPosition(offset.x + LABEL_OFFSET_X, offset.y + LABEL_OFFSET_Y);
			target.draw(labels[i]);
		}
	}
}
