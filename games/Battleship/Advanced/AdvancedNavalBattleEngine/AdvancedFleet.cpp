#inlude "AdvancedFleet.h"

bool AdvancedFleet::wouldBeHit(const coord& c) {
	auto m = getHitmap();
	auto r = m.find(c);
	if (r == m.end())
		return false;
	return true;
}
