#include <chrono>
#include <string>
#include <ostream>

// a helper class that will time and report time
class time_and_report{
public:
  typedef typename std::chrono::steady_clock clocktype;
  typedef typename clocktype::duration durationtype;
  typedef typename clocktype::time_point timepointtype;

  time_and_report(std::string name_, int reportmax_=1, bool enabled_=true): \
      name(name_), reportmax(reportmax_), enabled(enabled_){};
  int start(){
    timestart=clocktype::now();


protected:
  std::string name;
  bool enabled;
  bool ifstart=false;
  int reportcnt=0;
  int reportmax;
  dutationtype timesum(0);
  timepointtype timestart;
};
