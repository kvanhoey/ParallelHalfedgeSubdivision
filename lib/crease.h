#ifndef __CREASE__
#define __CREASE__

/**
 * @brief The Crease struct stores the attributes of a crease
 */
struct Crease
{
	float Sharpness ; /*!< Crease sharpness value */
	int Next ; /*!< Index of the next crease */
	int Prev ; /*!< Index of the previous crease */
};

#endif
