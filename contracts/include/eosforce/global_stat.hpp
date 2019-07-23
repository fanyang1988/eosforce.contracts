#pragma once

#include <string>
#include <variant>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

namespace eosforce {
   using eosio::name;
   
   struct gsvote {
      static constexpr uint64_t type_name = ("vote"_n).value;
      
      int64_t total_staked = -1;

      EOSLIB_SERIALIZE( gsvote, ( total_staked ) )
   };
   
   struct [[eosio::table, eosio::contract("eosio.system")]] global_stat_info {
      eosio::account_name    typ;
      std::variant< gsvote > data;
      
      uint64_t primary_key() const { return typ; }

      EOSLIB_SERIALIZE( global_stat_info, ( typ )( data ) )
   };
   typedef eosio::multi_index<"globalstat"_n,  global_stat_info> global_stat_table;
   
   template< typename T >
   bool get_globalstat( const name& self, T& res ) {
      global_stat_table t( self, self.value );
      
      const auto it = t.find( T::type_name );
      if( it == t.end() ) {
         return false;
      }
      
      eosio::check( std::holds_alternative<T>(it->data), 
         "get_globalstat get data error" );
      res = std::get<T>(it->data);
      
      return true;
   }

   template< typename T >
   bool is_has_globalstat( const name& self ) {
      global_stat_table t( self, self.value );
      
      const auto it = t.find( T::type_name );
      if( it == t.end() ) {
         return false;
      }
      
      if( !std::holds_alternative<T>(it->data) ) {
         return false;
      }
      
      return true;
   }
   
   template< typename T >
   void set_globalstat( const name& self, std::function<void(T&)>&& updater ) {
      eosio::print_f("set_globalstat %\n", T::type_name);
      global_stat_table t( self, self.value );
      const auto it = t.find( T::type_name );
      if( it == t.end() ) {
         t.emplace( name{0}, [&]( global_stat_info& info ) {
            info.typ = T::type_name;
            T data;
            updater(data);
            info.data = data;
            eosio::print_f("data %\n", T::type_name);
         } );
      } else {
         t.modify( it, name{0}, [&]( global_stat_info& info ) {
            eosio::check( std::holds_alternative<T>(info.data), 
               "set_globalstat get data error" );
            auto& data = std::get<T>(info.data);
            updater(data);
            info.data = data;
         } );
      }
   }
   
} // namespace eosforce