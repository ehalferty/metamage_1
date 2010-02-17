/*	==========
 *	SetData.cp
 *	==========
 */

#ifndef AEOBJECTMODEL_SETDATA_H
#include "AEObjectModel/SetData.h"
#endif


namespace Nitrogen
{
	
	void SetData( const AEDesc_Token& obj, const AEDesc_Data& data )
	{
		return TheGlobalDataSetter().SetData( obj, data );
	}
	
	DataSetter::DataSetter()
	{
	}
	
	void DataSetter::SetData( const AEDesc_Token& obj, const AEDesc_Data& data )
	{
		Map::const_iterator found = map.find( DescType( obj.descriptorType ) );
		
		if ( found == map.end() )
		{
			ThrowOSStatus( errAEEventNotHandled );
		}
		
		return found->second( obj, data );
	}
	
	DataSetter& TheGlobalDataSetter()
	{
		static DataSetter theGlobalDataSetter;
		
		return theGlobalDataSetter;
	}
	
}

