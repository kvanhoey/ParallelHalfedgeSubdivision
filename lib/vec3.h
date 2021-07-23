#ifndef VEC3_H
#define VEC3_H


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

public:
	std::array<float,3> val ;

	vec3()	{}

	vec3(float a, float b, float c)
	{
		val = {a, b, c} ;
	}

	vec3 operator/(const float& c) const
	{
		vec3 new_val(val) ;
		new_val[0] /= c ;
		new_val[1] /= c ;
		new_val[2] /= c ;
		return new_val ;
	}

	vec3 operator*(const float& c) const
	{
		vec3 new_val(val) ;
		new_val[0] *= c ;
		new_val[1] *= c ;
		new_val[2] *= c ;
		return new_val ;
	}

	vec3 operator+(const float& c) const
	{
		vec3 new_val(val) ;
		new_val[0] += c ;
		new_val[1] += c ;
		new_val[2] += c ;
		return new_val ;
	}

	vec3 operator+(const vec3& v) const
	{
		vec3 new_val(val) ;
		new_val[0] += v[0] ;
		new_val[1] += v[1] ;
		new_val[2] += v[2] ;
		return new_val ;
	}



	const float& operator[](const int& i) const
	{
		return val[i] ;
	}

	float& operator[](const int& i)
	{
		return val[i] ;
	}

private:
	vec3(const std::array<float,3>& v)
	{
		val = v ;
	}
};

#endif // VEC3_H
