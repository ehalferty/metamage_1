/*	======================
 *	FSTree_sys_mac_proc.cc
 *	======================
 */

#include "Genie/FileSystem/FSTree_sys_mac_proc.hh"

// Genie
#include "Genie/FileSystem/FSTree_QueryFile.hh"
#include "Genie/FileSystem/FSTree_sys_mac.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	std::string ProcessSerialNumber_KeyName_Traits::NameFromKey( const Key& psn )
	{
		using BitsAndBytes::EncodeAsHex;
		
		std::string name = EncodeAsHex( psn.lowLongOfPSN );
		
		if ( psn.highLongOfPSN != 0 )
		{
			name = EncodeAsHex( psn.highLongOfPSN ) + "-" + name;
		}
		
		return name;
	}
	
	ProcessSerialNumber_KeyName_Traits::Key ProcessSerialNumber_KeyName_Traits::KeyFromName( const std::string& name )
	{
		ProcessSerialNumber psn = { 0, 0 };
		
		const char* begin = name.c_str();
		const char* end   = name.c_str() + name.size();
		
		if ( const char* hyphen = std::strchr( begin, '-' ) )
		{
			psn.highLongOfPSN = DecodeHex32( begin, hyphen );
			
			begin = hyphen + 1;
		}
		
		psn.lowLongOfPSN = DecodeHex32( begin, end );
		
		return psn;
	}
	
	
	bool sys_mac_proc_Details::KeyIsValid( const Key& key )
	{
		try
		{
			(void) N::GetProcessInformation( key );
		}
		catch ( const N::ProcNotFound& err )
		{
		#ifdef __MWERKS__
			
			if ( err.Get() != procNotFound )
			{
				throw;
			}
			
		#endif
			
			return false;
		}
		
		return true;
	}
	
	class sys_mac_proc_PSN_name_Query
	{
		private:
			typedef ProcessSerialNumber Key;
			
			Key itsKey;
		
		public:
			sys_mac_proc_PSN_name_Query( const Key& key ) : itsKey( key )
			{
			}
			
			std::string operator()() const
			{
				Str255 name;
				
				ProcessInfoRec processInfo;
				
				Nucleus::Initialize< ProcessInfoRec >( processInfo );
				
				processInfo.processName = name;
				
				N::GetProcessInformation( itsKey, processInfo );
				
				std::string output = NN::Convert< std::string >( name ) + "\n";
				
				return output;
			}
	};
	
	class FSTree_sys_mac_proc_PSN_exe : public FSTree
	{
		private:
			typedef ProcessSerialNumber Key;
			
			Key itsKey;
		
		public:
			FSTree_sys_mac_proc_PSN_exe( const Key& key ) : itsKey( key )  {}
			
			bool IsLink() const  { return true; }
			
			std::string Name() const  { return "exe"; }
			
			FSTreePtr Parent() const  { return MakeFSTree( new FSTree_sys_mac_proc_PSN( itsKey ) ); }
			
			std::string ReadLink() const  { return ResolveLink()->Pathname(); }
			
			FSTreePtr ResolveLink() const  { return FSTreeFromFSSpec( N::GetProcessAppSpec( itsKey ) ); }
	};
	
	
	FSTreePtr sys_mac_proc_Details::Parent() const
	{
		return GetSingleton< FSTree_sys_mac >();
	}
	
	
	FSTree_sys_mac_proc_PSN::FSTree_sys_mac_proc_PSN( const Key& key ) : itsKey( key )
	{
	}
	
	void FSTree_sys_mac_proc_PSN::Init()
	{
		Map( FSTreePtr( new FSTree_QueryFile< sys_mac_proc_PSN_name_Query >( Pathname(),
		                                                                     "name",
		                                                                     sys_mac_proc_PSN_name_Query( itsKey ) ) ) );
		
		Map( FSTreePtr( new FSTree_sys_mac_proc_PSN_exe( itsKey ) ) );
	}
	
}

