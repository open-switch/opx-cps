#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdint.h>
using namespace std;

/*
bool cps_api_attr_create_escaped(void *buff, size_t len, const void *attr, size_t max_len, size_t *attr_len) {
  char special_chars[] = {'*', '?'};

  size_t count = 0;
  for(size_t ix = 0; ix < len; ++ix) {
    for(size_t x = 0; x < sizeof(special_chars); ++x) {
      if(((char *)buff)[ix] == special_chars[x]) {
        ((char *)attr)[count] = '\\';
        ++count;
      }
       ((char *)attr)[count] = ((char *)buff)[ix]; 
    }
    ++count;
  }
  *attr_len = count;
  if(max_len < count)
    return false;

  std::cout << max_len << " " << count << std::endl;
  return true;
}
*/

bool cps_api_attr_create_escaped(void *buff, size_t len, const void *attr, size_t *attr_len) {
  std::cout << sizeof(uint16_t)  << " : " << sizeof(uint32_t) << " : " << sizeof(uint64_t) << std::endl;;
  if(*attr_len < 2*len)
      return false;
  size_t count = 0;
  for(size_t ix = 0; ix < len; ++ix, ++count) {
    if(((char *)buff)[ix] == '*' || ((char *)buff)[ix] == '?' || ((char *)buff)[ix] == '[' || ((char *)buff)[ix] == ']' || ((char *)buff)[ix] == '\\') {
      ((char *)attr)[count] = '\\';
      ++count;
    }
    ((char *)attr)[count] = ((char *)buff)[ix];
  }
  //std::cout << count << std::endl;
  *attr_len = count;

  //std::cout << max_len << " " << count << std::endl;
  return true;
}


int main() {
  char buff[10] = "a*b\\dc";
  char attr[20] = "";
  size_t attr_len = 20;

  bool res;
  //std::cout << strlen(buff) << std::endl;
  res = cps_api_attr_create_escaped(buff, strlen(buff), attr, &attr_len);
  if(res)
      std::cout << attr << " : " << attr_len << std::endl;
  else
      std::cout << "False" << std::endl;
  return 0;
}
