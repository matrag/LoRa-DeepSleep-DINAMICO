#include "main.h"

/*Stream cypher function to encrypt a char set with a pseudo random key.
The key seed is used to generate the key with pseudo random function.
@param text array input
@param key cypher key*/
void stream_cipher( char text [], unsigned long keyseed, size_t pckSize) {
	// Use the 'key' to seed the random number generator
	srand( keyseed );
	// XOR each character in the string with the next random number
	for ( size_t k{0}; k < pckSize; ++k ) {
		text[k] ^= (char)rand();
	}
}