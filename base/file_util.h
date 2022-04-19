#pragma once
#include"nocopyable.h"
#include"string_piece.h"
#include<sys/types.h>
namespace czy
{
namespace FileUtil
{

// read small file < 64KB
class ReadSmallFile : nocopyable
{
 public:
  ReadSmallFile(StringArg filename);
  ~ReadSmallFile();

  // return errno
  template<typename String>
  int read_to_string(int maxSize,
                   String* content,
                   int64_t* fileSize,
                   int64_t* modifyTime,
                   int64_t* createTime);

  /// Read at maxium kBufferSize into buf_
  // return errno
  int read_to_buffer(int* size);

  const char* buffer() const { return m_buf; }

  static const int k_buff_size = 64*1024;

 private:
  int m_fd;
  int m_err;
  char m_buf[k_buff_size];
};

// read the file content, returns errno if error happens.
template<typename String>
int read_file(StringArg filename,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
  ReadSmallFile file(filename);
  return file.read_to_string(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe
class AppendFile : nocopyable
{
 public:
  explicit AppendFile(StringArg filename);

  ~AppendFile();

  void append(const char* logline, size_t len);

  void flush();

  off_t written_bytes() const { return m_write_bytes; }

 private:

  size_t write(const char* logline, size_t len);

  FILE* m_fp;
  char m_buffer[64*1024];
  off_t m_write_bytes;
};

}  // namespace FileUtil
}   // namespace czy