/*
	plus/hexadecimal.hh
	-------------------
*/

#ifndef PLUS_HEXADECIMAL_HH
#define PLUS_HEXADECIMAL_HH

// plus
#include "plus/string.hh"
#include "plus/var_string_fwd.hh"


namespace plus
{
	
	typedef unsigned align_t;
	
	unsigned decode_32_bit_hex( const string& s );
	
	void encode_8_bit_hex( var_string& out, unsigned char x );
	
	void encode_16_bit_hex( var_string& out, unsigned short x );
	
	void encode_32_bit_hex( var_string& out, unsigned x );
	
	string encode_8_bit_hex( unsigned char x );
	
	string encode_16_bit_hex( unsigned short x );
	
	string encode_32_bit_hex( unsigned x );
	
	string unhex( const char* data, unsigned long size, align_t align );
	
	template < class String >
	inline
	string unhex( const String& s, align_t align = 1 )
	{
		return unhex( get_string_data( s ), get_string_size( s ), align );
	}
	
	string hex_lower( const void* in, unsigned long n );
	string hex_upper( const void* in, unsigned long n );
	
}

#endif
