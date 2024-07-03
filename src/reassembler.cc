#include "reassembler.hh"
#include <ranges>
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( data.empty() ) {
    if ( is_last_substring ) {
      output_.close();
    }
    return;
  }
  auto end_index = first_index + data.size();                  // data: [first_index, end_index)
  auto last_index = next_index_ + output_.available_capacity(); // 可用范围: [next_index_, last_index)
  if ( end_index < next_index_ || first_index >= last_index ) {
    return; // 不在可用范围内, 直接返回
  }

  // 调整data的范围
  if ( last_index < end_index ) {
    end_index = last_index;
    data.resize( end_index - first_index );
    is_last_substring = false;
  }
  if ( first_index < next_index_ ) {
    data = data.substr( next_index_ - first_index );
    first_index = next_index_;
  }

  // 若data可以直接写入output, 则直接写入
  if ( first_index == next_index_ && ( buffer_.empty() || end_index < get<1>( buffer_.front() ) + 2 ) ) {
    if ( buffer_.size() ) { // 若重叠, 则调整data的范围
      data.resize( min( end_index, get<0>( buffer_.front() ) ) - first_index );
    }
    push_to_output( move( data ), output_ );
  } else { // 否则, 将data插入buffer_
    buffer_push( first_index, end_index - 1, data );
  }
  had_last_ |= is_last_substring;
  
  // 尝试将buffer_中的数据写入output
  buffer_pop(output_);
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_size_;
}

void Reassembler::push_to_output(std::string data, ByteStream& output){
  next_index_ += data.size();
  output.push(std::move(data));
}

void Reassembler::buffer_push(uint64_t first_index, uint64_t last_index, std::string data){
  // 合并区间
  auto l = first_index, r = last_index;
  auto beg = buffer_.begin(), end = buffer_.end();
  auto lef = lower_bound( beg, end, l, []( auto& a, auto& b ) { return get<1>( a ) < b; } );
  auto rig = upper_bound( lef, end, r, []( auto& b, auto& a ) { return get<0>( a ) > b; } );
  if (lef != end) l = min( l, get<0>( *lef ) );
  if (rig != beg) r = max( r, get<1>( *prev( rig ) ) );
  
  // 当data已在buffer_中时，直接返回
  if ( lef != end && get<0>( *lef ) == l && get<1>( *lef ) == r ) {
    return;
  }

  buffer_size_ += 1 + r - l;
  if ( data.size() == r - l + 1 && lef == rig ) { // 当buffer_中没有data重叠的部分
	buffer_.emplace( rig, l, r, move( data ) );
	return;
  }
  string s( 1 + r - l, 0 );

  for ( auto&& it : views::iota( lef, rig ) ) {
	auto& [a, b, c] = *it;
	buffer_size_ -= c.size();
    ranges::copy(c, s.begin() + a - l);
  }
  ranges::copy(data, s.begin() + first_index - l);
  buffer_.emplace( buffer_.erase( lef, rig ), l, r, move( s ) );
}

void Reassembler::buffer_pop(ByteStream& output){
  while ( !buffer_.empty() && get<0>( buffer_.front() ) == next_index_ ) {
    auto& [a, b, c] = buffer_.front();
    buffer_size_ -= c.size();
    push_to_output( move( c ), output ); 
    buffer_.pop_front();
  }

  if ( had_last_ && buffer_.empty() ) {
    output.close();
  }
}
