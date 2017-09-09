/*
	analyze.cc
	----------
*/

#include "vlib/analyze.hh"

// debug
#include "debug/assert.hh"

// vlib
#include "vlib/exceptions.hh"
#include "vlib/scope.hh"
#include "vlib/throw.hh"
#include "vlib/types/bareword.hh"
#include "vlib/types/string.hh"
#include "vlib/types/term.hh"


namespace vlib
{
	
	class Analyzer
	{
		private:
			lexical_scope_box its_scope;
			
			bool     it_is_a_module;
			unsigned its_export_count;
			
			Variable its_exports;
		
		private:
			Value enscope( const Value& block ) const;
			
			void visit( Value& syntree );
		
		public:
			Analyzer( lexical_scope* globals )
			:
				its_scope( globals ),
				it_is_a_module(),
				its_export_count(),
				its_exports( "__export__" )
			{
			}
			
			Value analyze( Value syntree );
	};
	
	Value Analyzer::enscope( const Value& block ) const
	{
		return Value( its_scope->symbols(), Op_scope, block );
	}
	
	void Analyzer::visit( Value& v )
	{
		if ( Expr* expr = v.expr() )
		{
			const op_type op = expr->op;
			
			if ( op == Op_module )
			{
				if ( it_is_a_module )
				{
					THROW( "duplicate `module` declaration" );
				}
				
				if ( its_export_count != 0 )
				{
					THROW( "`module` must precede `export`" );
				}
				
				it_is_a_module = true;
				
				Value exports( Op_export, empty_array );
				
				its_exports.sym()->deref() = exports;
				
				return;
			}
			else if ( op == Op_export )
			{
				if ( ! it_is_a_module  &&  its_export_count != 0 )
				{
					THROW( "only one export allowed in non-module" );
				}
				
				++its_export_count;
				
				expr->left = its_exports;
			}
			else if ( op == Op_block )
			{
				its_scope.push();
			}
			else if ( declares_symbols( op ) )
			{
				const Value* x = &expr->right;
				
				if ( op == Op_for )
				{
					Expr* e2 = x->expr();  // `do`
					ASSERT( e2 != NULL );
					
					e2 = e2->left.expr();  // `in`
					ASSERT( e2 != NULL );
					
					x = &e2->left;  // loop iteration variable
				}
				
				if ( ! x->is< Identifier >() )
				{
					THROW( "declared symbol must be an identifier" );
				}
				
				bool is_var = op == Op_var;
				symbol_type type = symbol_type( is_var );
				
				its_scope->declare( x->string(), type );
			}
			
			visit( expr->left );
			
			if ( op == Op_member )
			{
				return;
			}
			
			visit( expr->right );
			
			if ( op == Op_function )
			{
				if ( Expr* e2 = expr->left.expr() )
				{
					if ( declares_symbols( e2->op )  &&  e2->op != Op_def )
					{
						v = Value( expr->left, Op_denote, expr->right );
					}
				}
			}
			else if ( op == Op_block )
			{
				expr->right = enscope( expr->right );
				
				its_scope.pop();
			}
		}
		else if ( const Identifier* identifier = v.is< Identifier >() )
		{
			const plus::string& name = identifier->get();
			
			if ( const Value& sym = its_scope->resolve( name ) )
			{
				v = sym;
			}
			else
			{
				throw undeclared_symbol_error( name, source_spec() );
			}
		}
	}
	
	Value Analyzer::analyze( Value syntree )
	{
		try
		{
			visit( syntree );
			
			if ( its_export_count )
			{
				syntree = Value( syntree, Op_end, its_exports );
			}
			
			return enscope( syntree );
		}
		catch ( const exception& e )
		{
			throw user_exception( String( e.message ), source_spec() );
		}
		
		// Silence Metrowerks C++ warning
		return Value();
	}
	
	Value analyze( const Value& syntree, lexical_scope* globals )
	{
		Analyzer analyzer( globals );
		
		return analyzer.analyze( syntree );
	}
	
}
