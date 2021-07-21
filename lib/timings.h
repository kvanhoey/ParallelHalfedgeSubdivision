#ifndef TIMINGS_H
#define TIMINGS_H

class Timings
{
public:
	double min ;
	double mean ;
	double median ;
	double max ;

	Timings():
		min(0), mean(0), median(0), max(0)
	{}

	Timings& operator+=(const Timings& rhs)
	{
		min += rhs.min ;
		mean += rhs.mean ;
		median += rhs.median ;
		max += rhs.max ;
		return *this ;
	}

};

#endif // TIMINGS_H
