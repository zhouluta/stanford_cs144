#include "byte_stream.hh"
#include <iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , error_( false )
  , container_( "" )
  , is_closed_( false )
  , stream_error_( "" )
  , pushed_bytes_( 0 )
  , poped_bytes_( 0 )
{}
void ByteStream::push(std::string data){
  if ( is_closed_ ) {
    std::cout << "Writer stream has been closed.\n";
    return;
  }
  uint64_t dataLength = data.size();
  uint64_t usedCapacity = container_.size();
  uint64_t currentContainerCapacity = capacity_ - usedCapacity;
  if ( usedCapacity >= capacity_ ) {
    std::cout << "The writer stram is full of data.\n";
    return;
  }
  uint64_t toWriteDataLength = std::min( dataLength, currentContainerCapacity );
  container_ += data.substr( 0, toWriteDataLength );
  pushed_bytes_ = pushed_bytes_ + toWriteDataLength;
  return;
}

void ByteStream::close(){
  if ( is_closed_ ) {
    std::cout << "The writer stream has been closed.\n";
  }
  is_closed_ = true;
}

uint64_t ByteStream::available_capacity() const {
  return capacity_ - container_.size();
}

bool Writer::is_closed() const
{
  return is_closed_;
}

void Writer::push( string data )
{
  if ( is_closed_ ) {
    std::cout << "Writer stream has been closed.\n";
    return;
  }
  uint64_t dataLength = data.size();
  uint64_t usedCapacity = container_.size();
  uint64_t currentContainerCapacity = capacity_ - usedCapacity;
  if ( usedCapacity >= capacity_ ) {
    std::cout << "The writer stram is full of data.\n";
    return;
  }
  uint64_t toWriteDataLength = std::min( dataLength, currentContainerCapacity );
  container_ += data.substr( 0, toWriteDataLength );
  pushed_bytes_ = pushed_bytes_ + toWriteDataLength;
  return;
}

void Writer::close()
{
  if ( is_closed_ ) {
    std::cout << "The writer stream has been closed.\n";
  }
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - container_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes_;
}

bool Reader::is_finished() const
{
  return is_closed_ && pushed_bytes_ == poped_bytes_;
}

uint64_t Reader::bytes_popped() const
{
  return poped_bytes_;
}

string_view Reader::peek() const
{
  if ( !container_.empty() ) {
    return string_view( container_ );
  }
  return {};
}

void Reader::pop( uint64_t len )
{
  uint64_t currentContainerLength = container_.size();
  if ( !currentContainerLength ) {
    std::cout << "The reader stream is empty.\n";
    return;
  }
  uint64_t toPopDataLength = std::min( len, currentContainerLength );
  container_ = container_.substr( toPopDataLength );
  poped_bytes_ += toPopDataLength;
  return;
}

uint64_t Reader::bytes_buffered() const
{
  return container_.size();
}
