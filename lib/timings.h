#ifndef TIMINGS_H
#define TIMINGS_H

#include <algorithm>
#include <numeric>

/**
 * @brief The Timing_stats class stores runtime statistics
 */
class Timing_stats
{
public:
	double min ; /*!< minimal measured runtime */
	double mean ; /*!< mean measured runtime */
	double median ; /*!< median measured runtime */
	double max ; /*!< maximal measured runtime */

	/**
	 * @brief Timing_stats constructor that initializes all values to 0
	 */
	Timing_stats():
		min(0), mean(0), median(0), max(0)
	{}

	/**
	 * @brief operator += adds timings to the current values
	 * @param rhs the timings to add
	 * @return Reference to the current instance.
	 */
	Timing_stats& operator+=(const Timing_stats& rhs)
	{
		min += rhs.min ;
		mean += rhs.mean ;
		median += rhs.median ;
		max += rhs.max ;
		return *this ;
	}

	static void compute_stats(std::vector<double>& in_times, Timing_stats& out_stats)
	{
		std::sort(in_times.begin(), in_times.end()) ;

		out_stats.median = in_times[in_times.size() / 2] ;
		out_stats.min = in_times.front() ;
		out_stats.max = in_times.back() ;
		out_stats.mean = std::accumulate(in_times.begin(), in_times.end(), 0.0) / in_times.size() ;
	}

	friend std::ostream& operator<< (std::ostream& stream, const Timing_stats& stats)
	{
		stream << std::fixed << stats.min		 << "\t/\t" ;
		stream << std::fixed << stats.mean	 << "\t/\t" ;
		stream << std::fixed << stats.median	 << "\t/\t" ;
		stream << std::fixed << stats.max					;
		return stream ;
	}

};

#endif // TIMINGS_H
