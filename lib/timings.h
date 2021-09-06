#ifndef TIMINGS_H
#define TIMINGS_H

/**
 * @brief The Timings class stores runtime statistics
 */
class Timings
{
public:
	double min ; /*!< minimal measured runtime */
	double mean ; /*!< mean measured runtime */
	double median ; /*!< median measured runtime */
	double max ; /*!< maximal measured runtime */

	/**
	 * @brief Timings constructor that initializes all values to 0
	 */
	Timings():
		min(0), mean(0), median(0), max(0)
	{}

	/**
	 * @brief operator += adds timings to the current values
	 * @param rhs the timings to add
	 * @return Reference to the current instance.
	 */
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
