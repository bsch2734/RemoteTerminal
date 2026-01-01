#include "coord.h"
#include <climits>

coord::coord(std::pair<int, int> coordPair)
		: d(coordPair.first), o(coordPair.second) {
	}

bool coord::isUnspecified() const noexcept {
	return *this == coord::unspecified;
}

bool coord::operator==(const coord& other) const noexcept {
	return d == other.d && o == other.o;
}

coord coord::operator+(const coord& other) {
	if (other.isUnspecified() || this->isUnspecified())
		return unspecified;
	return coord({ d + other.d, o + other.o });
}

coord& coord::operator+=(const coord& other) {
	if (!other.isUnspecified() && !this->isUnspecified()) {
		d += other.d;
		o += other.o;
	}
	return *this;
}

coord coord::operator-(coord const& other) const {
	return { {d-other.d, o-other.o} };
}

bool coord::operator<(const coord& other) const {
	if (d != other.d)
		return d < other.d;
	return o < other.o;
}

coord coord::rotate(int turns) const{
	coord answer(*this);

	//apply mod (works with negatives)
	turns %= 4;
	turns += 4;
	turns %= 4;
	switch (turns) {
		case 0: {
			break;
		}
		case 1: {
			answer.d = -o;
			answer.o = d;
			break;
		}
		case 2: {
			answer.d *= -1;
			answer.o *= -1;
			break;
		}
		case 3: {
			answer.d = o;
			answer.o = -d;
			break;
		}
	}
	return answer;
}

coord& coord::rotateSelf(int turns) {
	coord rotated = rotate(turns);
	d = rotated.d;
	o = rotated.o;
	return *this;
}

coord coord::applyTransform(coord translation, int rotation) const{
	return rotate(rotation) + translation;
}

coord coord::applyInverseTransform(coord translation, int rotation) const{
	return (*this - translation).rotate(-rotation);
}


const coord coord::unspecified{ {INT_MAX, INT_MAX} };