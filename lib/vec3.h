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
	float val[3] = {};

	vec3()	{}

	vec3(float a, float b, float c)
	{
		val[0] = a ;
		val[1] = b ;
		val[2] = c ;
	}

	vec3 operator/(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] /= c ;
		new_val[1] /= c ;
		new_val[2] /= c ;
		return new_val ;
	}

	vec3 operator*(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] *= c ;
		new_val[1] *= c ;
		new_val[2] *= c ;
		return new_val ;
	}

	vec3 operator+(const float& c) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] += c ;
		new_val[1] += c ;
		new_val[2] += c ;
		return new_val ;
	}

	vec3 operator+(const vec3& v) const
	{
		vec3 new_val(val[0],val[1],val[2]) ;
		new_val[0] += v[0] ;
		new_val[1] += v[1] ;
		new_val[2] += v[2] ;
		return new_val ;
	}

    static vec3 lerp(const vec3& a, const vec3& b, const float alpha)
    {
        vec3 res ;

        for (int c=0; c < 3; ++c)
        {
            res[c] = (1 - alpha) * a[c] + alpha * b[c];
        }

        return res ;
    }


	const float& operator[](const int& i) const
	{
		return val[i] ;
	}

	float& operator[](const int& i)
	{
		return val[i] ;
	}
};

#endif // VEC3_H
