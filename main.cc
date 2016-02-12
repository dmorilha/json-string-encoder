#include <assert.h>
#include <iostream>
#include <string>

/*
 * references:
 * https://en.wikipedia.org/wiki/JSON#Data_portability_issues
 * https://en.wikipedia.org/wiki/UTF-8#Description
 * https://en.wikipedia.org/wiki/UTF-16#U.2B10000_to_U.2B10FFFF
 * http://stackoverflow.com/questions/3911536/utf-8-unicode-whats-with-0xc0-and-0x80/3911566#3911566
 */

struct MultiByteParser {
  int count_;
  unsigned char next_;
  std::string & output_;
  bool flag_;

  MultiByteParser(std::string & o) :
    count_(0), next_('\0'), output_(o), flag_(false) { }

  void parse(const unsigned char c) {
    if ((c & 0xC0) == 0x80) {
      assert(count_ > 0);
      --count_;
      switch (count_) {
      case 0:
        {
          assert( ! flag_);
          next_ |= c & 0x3F;
          const unsigned char a = (next_ >> 4) & 0xF;
          const unsigned char b = next_ & 0xF;
          output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
          output_ += b <= 9 ? '0' + b : 'A' - 10 + b;
        }
        break;
      case 1:
        {
          if (flag_) {
            flag_ = false;
            next_ |= c & 0x3F;
            const unsigned char a = (next_ >> 4) & 0xF;
            output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
            output_ += "\\uD";
            const unsigned char b = 0xC | ((next_ >> 2) & 0x3);
            output_ += b <= 9 ? '0' + b : 'A' - 10 + b;
          } else {
            const unsigned char a = (c >> 2) & 0x0F;
            output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
          }
          next_ = (c & 0x3) << 6;
        }
        break;
      case 2:
        {
          assert( ! flag_);
          next_ |= (c >> 2) & 0xF;
          next_ -= 0x4;
          const unsigned char a = 0x8 | ((next_ >> 4) & 0x3);
          output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
          const unsigned char b = next_ & 0xF;
          output_ += b <= 9 ? '0' + b : 'A' - 10 + b;
          next_ = (c & 0x3) << 6;
          flag_ = true;
        }
        break;
      default:
        assert(false);
      }
    } else {
      assert(count_ == 0);
      assert( ! flag_);
      if ((c & 0x80) == 0) {
        //7 bits
        output_ += c;
      } else {
        const unsigned char d = c & 0xF8;
        if (d == 0xF0) {
          //21 bits
          count_ = 3;
          output_ += "\\uD";
          next_ = (c & 0x7) << 4;
        } else if ((d & 0xE0) == 0xE0) {
          //16 bits
          count_ = 2;
          output_ += "\\u";
          const unsigned char a = c & 0xF;
          output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
        } else if ((d & 0xC0) == 0xC0) {
          //11 bits
          count_ = 1;
          next_ = (c & 0x3) << 6;
          output_ += "\\u0";
          const unsigned char a = (c >> 2) & 0x3;
          output_ += a <= 9 ? '0' + a : 'A' - 10 + a;
        } else {
          assert(false); //unsuported
        }
      }
    }
  }
};

std::string escape(const char * const s) {
  std::string result;
  result.reserve(strlen(s) * 6);
  MultiByteParser parser(result);
  const unsigned char * c = reinterpret_cast< const unsigned char * >(s);
  for (; *c != '\0'; ++c) {
    parser.parse(*c);
  }
  return result;
}

int main(void) {
  const char * const myString = "maçã 😂 🍺 ";
  std::cout << myString << " " << escape(myString) << std::endl;
  return 0;
}
