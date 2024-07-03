#include "reassembler.hh"
#include <ranges>
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(data.empty()){
    if(is_last_substring){
      output_.close();
    }
    return;
  }

  auto end_index = first_index + data.size();
  auto last_index = next_index_ + output_.available_capacity();
  if(end_index < next_index_ || first_index >= last_index) return;

  if(last_index < end_index){
    end_index = last_index;
    data.resize(end_index - first_index);
    is_last_substring = false;
  }
  if(first_index < next_index_){
    data = data.substr(next_index_ - first_index);
    first_index = next_index_;
  }

  if(first_index == next_index_ && (buffer_.empty() || end_index < get<1>(buffer_.front()) + 2)){
    if(buffer_.size()){
      data.resize(std::min(end_index, get<0>(buffer_.front())) - first_index);
    }
    push_to_output(std::move(data), output_);
  }else{
    buffer_push(first_index, end_index - 1, data);
  }

  had_last_ |= is_last_substring;
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
  auto left = first_index, right = last_index;
  auto begin = buffer_.begin(), end = buffer_.end();
  auto lef = lower_bound(begin, end, left, [](auto& a, auto& b) {return get<1>(a) < b; });
  auto rig = upper_bound(lef, end, right, [](auto& b, auto& a) {return get<0>(a) > b; });
  if(lef != end) left = std::min(left, get<0>(*lef));
  if(rig != begin) right = std::max(right, get<1>(*prev(rig)));

  if(lef != end && get<0>(*lef) == left && get<1>(*lef) == right) return;

  buffer_size_ += 1 + right - left;
  if(data.size() == right - left + 1 && lef == rig){
    buffer_.emplace(rig, left, right, move(data));
    return;
  }
  std::string s(1 + right - left, 0);

  for(auto&& it : views::iota(lef, rig)){
    auto& [a, b, c] = *it;
    buffer_size_ -= c.size();
    ranges::copy(c, s.begin() + a - left);
  }
  ranges::copy(data, s.begin() + first_index - left);
  buffer_.emplace(buffer_.erase(lef, rig), left, right, std::move(s));
}

void Reassembler::buffer_pop(ByteStream& output){
  while(!buffer_.empty() && get<0>(buffer_.front()) == next_index_){
    auto& [a, b, c] = buffer_.front();
    buffer_size_ -= c.size();
    push_to_output(std::move(c), output);
    buffer_.pop_front();
  }

  if(had_last_ && buffer_.empty()){
    output.close();
  }
}
