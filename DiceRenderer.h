// DiceRenderer.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
 This file contains the declaration of the DiceRenderer class, which is responsible for rendering 3D dice
 using SFML and OpenGL. It includes functionality for loading 3D models, computing die orientations based on values,
 and rendering animated dice rolls with lighting effects.
*/

#ifndef ZILCH_DICE_RENDERER_H
#define ZILCH_DICE_RENDERER_H

#include <array>
#include <chrono>
#include <vector>

#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>

/**
 * @class DiceRenderer DiceRenderer.h "DiceRenderer.h"
 *
 * @brief Renders 3D dice with animations and lighting using SFML.
 */
class DiceRenderer {
public:
	void startRoll(const std::vector<uint16_t>& diceValues) const;
	void updateValues(const std::vector<uint16_t>& diceValues) const;
	void draw(sf::RenderTarget& target, const sf::Font* labelFont) const;
	bool isAnimating() const { return animating; }

private:
	// Represents a 2D triangle with depth information for rendering.
	struct Triangle2D {
		sf::Vertex v0;
		sf::Vertex v1;
		sf::Vertex v2;
		float depth{};   // larger = closer to camera
	};

	// Ensures that the necessary resources for rendering dice are loaded.
	void ensureResourcesLoaded() const;
	sf::Vector3f orientationForValue(uint16_t value) const;

	// Adds triangles for a specific material to the output list.
	void addTrianglesForMaterial(
		const sf::Vector3f& rotation,
		const sf::Vector2f& offset,
		const std::vector<unsigned int>& indices,
		const sf::Color& baseColor,
		std::vector<Triangle2D>& outTris,
		float minFacingCos = 0.f
	) const;

	// Geometry data
	mutable std::vector<glm::vec3> vertices;
	mutable std::vector<glm::vec3> normals;
	mutable std::vector<unsigned int> bodyIndices; // “Material” (die body)
	mutable std::vector<unsigned int> pipIndices;  // “Material.001” (pips)
	mutable std::array<std::vector<unsigned int>, 6> pipFaceIndices; // Pips grouped by model-space cube face

	// State data
	mutable bool resourcesLoaded = false;
	mutable bool orientationsComputed = false;
	mutable std::array<sf::Vector3f,6> valueOrientations{};
	mutable std::vector<uint16_t> currentValues;
	mutable std::vector<float> baseAngles;
	mutable bool animating = false;
	mutable std::chrono::steady_clock::time_point rollStart;
};

#endif // ZILCH_DICE_RENDERER_H
