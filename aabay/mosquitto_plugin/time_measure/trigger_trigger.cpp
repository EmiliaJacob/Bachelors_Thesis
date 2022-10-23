#include <iostream>
#include <chrono>
#include <string>
#include <ydb-global.h>

int main(int argc, char** argv)
{
  if(argc > 2) {
    std::cout << "ERROR: Too many arguments" << std::endl;
    return -1;
  }

  std::string article_id = std::string(argv[1]);

  using namespace std::chrono; // TODO: Refactoring

  high_resolution_clock::time_point start_point;
  duration<double> start_duration;
  c_ydb_global _articles("^articles");

  start_point = high_resolution_clock::now();
  start_duration = start_point.time_since_epoch();
  _articles[article_id]["bid"] = std::to_string(start_duration.count());// INFO: Die Reihenfolge der Nachrichten ist aktuell nicht messbar

  return 0;
}
