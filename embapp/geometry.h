#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "utils.h"

struct SCPoint
{
	int16_t x, y;
	SCPoint() = default;
	constexpr inline SCPoint(int16_t x, int16_t y) :
		x(x), y(y) { }
	constexpr inline SCPoint operator + (const SCPoint &o) const
	{
		return SCPoint {
			static_cast<int16_t>(x + o.x), 
			static_cast<int16_t>(y + o.y)
		};
	}
	constexpr inline SCPoint operator - (const SCPoint &o) const
	{
		return SCPoint {
			static_cast<int16_t>(x - o.x), 
			static_cast<int16_t>(y - o.y)
		};
	}
	inline SCPoint& operator -= (const SCPoint &o)
	{
		x -= o.x; y -= o.y; return *this;
	}
};

struct SCSize
{
	int16_t w, h;
	SCSize() = default;
	constexpr inline SCSize(int16_t w, int16_t h) :
		w(w), h(h) { }
};

struct SCRect
{
	SCPoint loc;
	SCSize size;
	SCRect() = default;
	constexpr inline SCRect(int16_t x, int16_t y, int16_t w, int16_t h) :
		loc(x, y), size(w, h) { }
	constexpr inline SCRect(const SCPoint &loc, const SCSize &size) :
		loc(loc), size(size) { }
	constexpr inline int16_t left() const { return loc.x; }
	constexpr inline int16_t top() const { return loc.y; }
	constexpr inline int16_t right() const { return loc.x + size.w; }
	constexpr inline int16_t bottom() const { return loc.y + size.h; }
	constexpr inline bool contains(const SCPoint &pt) const
	{
		return pt.x >= left() && pt.y >= top() &&
			pt.x < right() && pt.y < bottom();
	}
	constexpr inline bool covers(const SCRect &o) const
	{
		return left() <= o.left() && top() <= o.top() &&
			right() >= o.right() && bottom() >= o.bottom();
	}
	constexpr inline bool intersectsWith(const SCRect &o) const
	{
		return left() < o.right() && top() < o.bottom() &&
			right() > o.left() && bottom() > o.top();
	}
	constexpr inline SCRect translated(const SCPoint &pt) const
	{
		return SCRect {loc + pt, size};
	}
	inline SCRect intersectionWith(const SCRect &o) const
	{
		int16_t x1 = SCMax(left(), o.left());
		int16_t y1 = SCMax(top(), o.top());
		int16_t x2 = SCMin(right(), o.right());
		int16_t y2 = SCMin(bottom(), o.bottom());
		return SCRect {x1, y1, 
			static_cast<int16_t>(x2 - x1), 
			static_cast<int16_t>(y2 - y1)
		};
	}
	inline SCRect unionWith(const SCRect &o) const
	{
		int16_t x1 = SCMin(left(), o.left());
		int16_t y1 = SCMin(top(), o.top());
		int16_t x2 = SCMax(right(), o.right());
		int16_t y2 = SCMax(bottom(), o.bottom());
		return SCRect {x1, y1, 
			static_cast<int16_t>(x2 - x1),
			static_cast<int16_t>(y2 - y1)
		};
	}
	constexpr inline bool empty() const
	{
		return size.w <= 0 || size.h <= 0;
	}
	template <class T>
	void print(T&& printable)
	{
		printable.print(size.w);
		printable.print("x"); 
		printable.print(size.h);
		printable.print("+"); 
		printable.print(loc.x);
		printable.print("+"); 
		printable.print(loc.y);
	}
};

#endif
