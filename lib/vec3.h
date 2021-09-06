#ifndef VEC3_H
#define VEC3_H

/**
 * @brief The vec3 class is a utility class for XYZ coordinates.
 */
class vec3
{
	vec3 friend operator*(const float& c, const vec3& v)
	{
		return v * c ;
	}

	vec3 friend operator+(const float& c, const vec3& v)
	{
		return v + c ;
	}

private:
	float val[3] = {}; /*!< Stores the value of the coordinate */

public:
	vec3()	{}

	/**
	 * @brief vec3 constructor from xyz values
	 * @param x first ordinate
	 * @param y second ordinate
	 * @param z third ordinate
	 */
	vec3(float x, float y, float z)
	{
		val[0] = x ;
		val[1] = y ;
		val[2] = z ;
	}

	/**
	 * @brief operator / divides the current coordinate by a constant
	 * @param c a scalar
	 * @return the coordinate that is the result of the operation
	 */
	vec3 operator/(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] /= c ;
		new_val[1] /= c ;
		new_val[2] /= c ;
		return new_val ;
	}

	/**
	 * @brief operator * multiplies the current coordinate by a constant
	 * @param c a scalar
	 * @return the coordinate that is the result of the operation
	 */
	vec3 operator*(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] *= c ;
		new_val[1] *= c ;
		new_val[2] *= c ;
		return new_val ;
	}

	/**
	 * @brief operator + adds a constant to the current coordinate
	 * @param c a scalar
	 * @return the coordinate that is the result of the operation
	 */
	vec3 operator+(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] += c ;
		new_val[1] += c ;
		new_val[2] += c ;
		return new_val ;
	}

	/**
	 * @brief operator + sums two coordinates
	 * @param v the coordinate to sum to the current one
	 * @return the coordinate that is the result of the operation
	 */
	vec3 operator+(const vec3& v) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] += v[0] ;
		new_val[1] += v[1] ;
		new_val[2] += v[2] ;
		return new_val ;
	}

	/**
	 * @brief operator - subtracts a coordinate from the current one
	 * @param v the coordinate to subtract to the current one
	 * @return the coordinate that is the result of the operation
	 */
	vec3 operator-(const vec3& v) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] -= v[0] ;
		new_val[1] -= v[1] ;
		new_val[2] -= v[2] ;
		return new_val ;
	}

	/**
	 * @brief operator [] is an accessor for each element of the coordinate
	 * @param i index to access
	 * @return const reference to the scalar
	 */
	const float& operator[](const int& i) const
	{
		return val[i] ;
	}

	/**
	 * @brief operator [] is an accessor for each element of the coordinate
	 * @param i index to access
	 * @return non-const reference to the scalar
	 */
	float& operator[](const int& i)
	{
		return val[i] ;
	}
};

#endif // VEC3_H
