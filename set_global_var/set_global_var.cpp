#include <iostream>
#include <chrono>
#include <string>
#include <ydb-global.h>
#include <unistd.h>

#include <unistd.h>

int main(int argc, char** argv)
{

  if(argc != 3) {
    std::cout << "ERROR: Wrong amount of args" << std::endl;
    return -1;
  }

  std::string article_id = std::string(argv[1]);
  int number_of_set_calls = stoi(argv[2]);

  using namespace std::chrono; 

  c_ydb_global _articles("^articles");

  system_clock::time_point start_point;
  duration<double> start_duration;


  for (int i=0; i<number_of_set_calls; i++)
  { 
    start_point = system_clock::now();
    start_duration = start_point.time_since_epoch();

    _articles[article_id]["bid"] = std::to_string(start_duration.count());
  }

  return 0;
}