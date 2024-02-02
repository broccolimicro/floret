#pragma once

template <typename T>
struct choose {
	choose(T &base, int radix) {
		auto j = base.begin();
		idx.reserve(radix);
		for (int i = 0; i < radix; i++) {
			idx.push_back(j++);
		}
	}
	~choose() {
	}

	const T *base;
	vector<T::iterator> idx;

	bool next() {
		int i = (int)idx.size()-1;
		idx[i]++;
		for (i--; i >= 0 and idx[i+1] == base->end(); i--) {
			idx[i]++;
		}
		if (i < 0) {
			return;
		}
		auto j = idx[i];
		for (i++; i > 0 and i < (int)idx.size(); i++) {
			idx[i] = ++j;
		}

		return idx.back() != base->end();
	}

	T::iterator &operator[](int i) {
		return idx[i];
	}

	size_t size() const {
		return idx.size();
	}

	bool done() {
		return idx.back() == base->end();
	}
};

