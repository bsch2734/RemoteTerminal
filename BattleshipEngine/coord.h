#pragma once
#include <utility>

struct coord {
	coord(std::pair<int, int> coordPair);

	bool isUnspecified() const noexcept;

	bool operator==(const coord& other) const noexcept;

	coord operator+(const coord& other);

	coord& operator+=(const coord& other);

	coord operator-(coord const& other) const;

	bool operator<(const coord& other) const;

	coord rotate(int turns);

	coord& rotateSelf(int turns);

	coord applyTransform(coord translation, int rotation);

	coord applyInverseTransform(coord translation, int rotation);

	int d, o;

	static const coord unspecified;
};


template<>
struct std::hash<coord> {
	std::size_t operator()(coord c) const {
		return std::hash<int>()(c.d) ^ (std::hash<int>()(c.o) << 1);
	}
};