/*
 * Events.h
 *
 *  Created on: Feb 17, 2017
 *      Author: vadim
 */

#include <vector>
#include <map>

using std::vector;
using std::map;

#pragma once

struct Event {
public:
	explicit Event(double _time, int _executor = -1) :
		scheduled_time(_time),
		creation_time(_time),
		executor(_executor),
		id(0)
	{
		id = ID;
		ID += 1;
	}

	double scheduled_time;
	double creation_time;
	int executor;
	int id;
	vector<int> routing;

	virtual ~Event() {}
private:
	static int ID;
};

struct SendEvent : Event {
	SendEvent(double _time, int _src, int _dst, int _size) :
		Event(_time),
		src(_src),
		dst(_dst),
		size(_size)
	{
		executor = _src;
	}
	int src;
	int dst;
	int size;
};

struct TransitEvent : Event {
	TransitEvent(double _time, int _src, int _dst, int _size) :
		Event(_time),
		src(_src),
		dst(_dst),
		size(_size)
	{

	}
	int src;
	int dst;
	int size;
};

struct QualityRefreshEvent : Event {
	QualityRefreshEvent(double _time, int _executor) :
		Event(_time, _executor)
	{}
};

struct TradingQualityEvent : Event {
	TradingQualityEvent(double _time, int _executor) :
		Event(_time, _executor)
	{}
};

struct QualityAnnounceEvent : Event {
	QualityAnnounceEvent(double _time, int _executor) :
		Event(_time, _executor)
	{}
	map<int, double> local_delays;
};
