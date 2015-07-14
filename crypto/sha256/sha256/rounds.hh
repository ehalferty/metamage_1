/*
	rounds.hh
	---------
*/

#ifndef SHA256_ROUNDS_HH
#define SHA256_ROUNDS_HH

// sha256
#include "sha256/state.hh"


namespace crypto
{
	
	void sha256_extend_block( u32* block );
	
	void sha256_rounds( sha256_hash& digest, u32* const block );
	
}

#endif
