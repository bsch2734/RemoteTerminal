#pragma once
#include <utility>
#include <functional>

struct coord {
	constexpr coord(std::pair<int, int> coordPair)
		: d(coordPair.first), o(coordPair.second) {
	}

	bool isUnspecified() const noexcept;

	bool operator==(const coord& other) const noexcept;

	coord operator+(const coord& other);

	coord& operator+=(const coord& other);

	coord operator-(coord const& other) const;

	bool operator<(const coord& other) const;

	coord rotate(int turns) const;

	coord& rotateSelf(int turns);

	coord applyTransform(coord translation, int rotation) const;

	coord applyInverseTransform(coord translation, int rotation) const;

	int d, o;

	static const coord unspecified;
};


template<>
struct std::hash<coord> {
	std::size_t operator()(const coord& c) const noexcept{
		return std::hash<int>()(c.d) ^ (std::hash<int>()(c.o) << 1);
	}
};