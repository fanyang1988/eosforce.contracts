#pragma once

#include <string>
#include <variant>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

namespace eosforce {
   using eosio::name;
   
   struct globalstat_vote {
      static constexpr name type_name = "vote"_n;
      
      int64_t total_staked = -1;
   };
   
   struct [[eosio::table, eosio::contract("eosio.system")]] global_stat_info {
      name                            type_name;
      std::variant< globalstat_vote > data;
      
      uint64_t primary_key() const { return type_name.value; }
   };
   typedef eosio::multi_index<"globalstat"_n,  global_stat_info> global_stat_table;
   
   template< typename T >
   bool get_globalstat( const name& self, T& res ) {
      global_stat_table t( self, self.value );
      
      const auto it = t.find( T::type_name.value );
      if( it == t.end() ) {
         return false;
      }
      
      check( std::holds_alternative<T>(it->data), 
         "get_globalstat get data error" );
      res = std::get<T>(it->data);
      
      return true;
   }
   
   template< typename T >
   void set_globalstat( const name& self, std::function<void(T&)>&& updater ) {
      global_stat_table t( self, self.value );
      const auto it = t.find( T::type_name.value );
      if( it == t.end() ) {
         t.emplace( T::type_name, [&]( global_stat_info& info ) {
            info.type_name = T::type_name.value;
            T data;
            updater(data);
            info.data = data;
         } );
      } else {
         t.modify( it, name{}, [&]( global_stat_info& info ) {
            check( std::holds_alternative<T>(info.data), 
               "set_globalstat get data error" );
            auto& data = std::get<T>(info.data);
            updater(data);
            info.data = data;
         } );
      }
   }
   
} // namespace eosforce