/*	=================
 *	ProjectCatalog.cc
 *	=================
 */

#include "CompileDriver/ProjectCatalog.hh"

// Standard C++
#include <string>
#include <vector>

// Iota
#include "iota/strings.hh"

// Io
#include "io/files.hh"
#include "io/walk.hh"

// Nucleus
#include "Nucleus/Convert.h"

// POSeven
#include "POSeven/Directory.hh"
#include "POSeven/FileDescriptor.hh"
#include "POSeven/Pathnames.hh"
#include "POSeven/Stat.hh"

// MoreFunctional
#include "PointerToFunction.hh"

// Io
#include "Io/TextInput.hh"

// A-line
#include "A-line/Exceptions.hh"


namespace tool
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	
	
	using namespace io::path_descent_operators;
	
	
	template < class Data >
	struct project_data
	{
		// A map from platform requirements to project data
		typedef std::map< PlatformDemands, Data > platform_map;
		
		// A map from project name to config file set
		typedef std::map< std::string, platform_map > name_map;
	};
	
	typedef project_data< ProjectConfig >::platform_map  ProjectConfigCandidates;
	typedef project_data< ProjectConfig >::name_map      ProjectCatalog;
	
	
	static ProjectCatalog gProjectCatalog;
	
	
	static bool ends_with( const std::string& string, const char* substring, std::size_t length )
	{
		return std::equal( string.end() - length, string.end(), substring );
	}
	
	static std::string get_project_dir_from_config_file( const std::string& config_pathname )
	{
		std::string config_dir = io::get_preceding_directory( config_pathname );
		
		const bool has_confd = ends_with( config_dir, STR_LEN( "A-line.confd/" ) );
		
		return has_confd ? io::get_preceding_directory( config_dir )
		                 :                              config_dir;
	}
	
	
	void AddProjectConfigFile( const std::string&      name,
	                           const PlatformDemands&  demands,
	                           const ProjectConfig&    config )
	{
		gProjectCatalog[ name ][ demands ] = config;
	}
	
	struct platform_compatibility
	{
		Platform platform;
		
		platform_compatibility() : platform()
		{
		}
		
		platform_compatibility( Platform p ) : platform( p )
		{
		}
		
		bool operator()( const PlatformDemands& demands ) const
		{
			return demands.Test( platform );
		}
		
		template < class value_type >
		bool operator()( const value_type& map_value ) const
		{
			return operator()( map_value.first );
		}
	};
	
	static ProjectConfigCandidates& find_project_config_candidates( const std::string& project_name )
	{
		ProjectCatalog::iterator it = gProjectCatalog.find( project_name );
		
		if ( it == gProjectCatalog.end() )
		{
			throw NoSuchProject( project_name );
		}
		
		return it->second;
	}
	
	const ProjectConfig& GetProjectConfig( const std::string& name, Platform targetPlatform )
	{
		ProjectConfigCandidates& candidates = find_project_config_candidates( name );
		
		ProjectConfigCandidates::iterator it = std::find_if( candidates.begin(),
		                                                     candidates.end(),
		                                                     platform_compatibility( targetPlatform ) );
		if ( it == candidates.end() )
		{
			// FIXME:  Indicate that there are projects with this name,
			// but that they're not compatible with the current target
			throw NoSuchProject( name );
		}
		
		ProjectConfig& result = it->second;
		
		if ( result.get_config_data().empty() )
		{
			try
			{
				result.load_config();
			}
			catch ( const p7::enoent& err )
			{
			#ifdef __MWERKS__
				
				if ( err != ENOENT )
				{
					throw;
				}
				
			#endif
				
				throw missing_project_config( name, result.get_pathname() );
			}
		}
		
		return result;
	}
	
	static void add_cached_config( const std::string&      project_name,
	                               const PlatformDemands&  demands,
	                               const std::string&      pathname )
	{
		ProjectConfig& config = gProjectCatalog[ project_name ][ demands ];
		
		if ( config.get_pathname().empty() )
		{
			config.set_pathname( pathname );
		}
	}
	
	void ProjectConfig::load_config()
	{
		its_project_dir = get_project_dir_from_config_file( its_pathname );
		
		DotConfData data;
		
		ReadProjectDotConf( its_pathname, data );
		
		its_config_data = MakeConfData( data );
	}
	
	void ScanDirForProjects( const std::string&                                       dirPath,
	                         std::back_insert_iterator< std::vector< std::string > >  configs,
	                         std::back_insert_iterator< std::vector< std::string > >  folders )
	{
		if ( !io::directory_exists( dirPath ) )
		{
			return;
		}
		
		std::string conf = dirPath / "A-line.conf";
		
		if ( io::file_exists( conf ) )
		{
			*configs++ = conf;
			
			return;
		}
		
		typedef io::directory_contents_traits< std::string >::container_type directory_container;
		
		std::string confd = dirPath / "A-line.confd";
		
		bool has_confd = io::directory_exists( confd );
		
		directory_container contents = io::directory_contents( has_confd ? confd : dirPath );
		
		std::copy( contents.begin(),
		           contents.end(),
		           has_confd ? configs : folders );
	}
	
	
	void write_catalog_cache( p7::fd_t output )
	{
		typedef ProjectCatalog::const_iterator name_iter;
		
		typedef ProjectConfigCandidates::const_iterator demands_iter;
		
		for ( name_iter the_name = gProjectCatalog.begin();  the_name != gProjectCatalog.end();  ++the_name )
		{
			const std::string&              name       = the_name->first;
			const ProjectConfigCandidates&  candidates = the_name->second;
			
			for ( demands_iter the_demands = candidates.begin();  the_demands != candidates.end();  ++the_demands )
			{
				const PlatformDemands&  demands = the_demands->first;
				const ProjectConfig&    config  = the_demands->second;
				
				std::string record = name;
				
				record += '\t';
				
				record += NN::Convert< std::string >( demands.Required  () );
				
				record += '/';
				
				record += NN::Convert< std::string >( demands.Prohibited() );
				
				record += '\t';
				
				record += config.get_pathname();
				
				record += '\n';
				
				p7::write( output, record.data(), record.length() );
			}
		}
	}
	
	void read_catalog_cache( p7::fd_t input_fd )
	{
		Io::TextInputAdapter< p7::fd_t > input( input_fd );
		
		while ( input.Ready() )
		{
			std::string text = input.Read();
			
			const char* begin = text.c_str();
			
			if ( const char* tab1 = std::strchr( begin, '\t' ) )
			{
				if ( const char* slash = std::strchr( tab1 + 1, '/' ) )
				{
					if ( const char* tab2 = std::strchr( slash + 1, '\t' ) )
					{
						std::string project_name( begin, tab1 );
						
						std::string requirements( tab1  + 1, slash );
						std::string prohibitions( slash + 1, tab2  );
						
						std::string config_pathname( tab2 + 1 );
						
						PlatformDemands demands( Platform( std::atoi( requirements.c_str() ) ),
						                         Platform( std::atoi( prohibitions.c_str() ) ) );
						
						add_cached_config( project_name, demands, config_pathname );
					}
				}
			}
		}
	}
	
}

