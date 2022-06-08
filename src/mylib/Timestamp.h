#pragma once
#include<cstdint>
#include<ctime>
#include<string>

using std::int64_t;
//微妙级别的时间戳
class Timestamp{
public:

  //an invalid timestamp
  Timestamp()
    : microSecondsSinceEpoch_(0)
  {
  }

  explicit Timestamp(int64_t microSecondsSinceEpochArg)
    : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
  {
  }

  bool valid() const { return microSecondsSinceEpoch_ > 0; }

  std::string toFormattedString(bool showMicroseconds) const;

  // for internal usage.
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const
  { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

  // For Comparison
  bool operator<(const Timestamp &rhs) const{
    return this->microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
  }

  bool operator==(const Timestamp &rhs) const{
    return this->microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
  }

  ///
  /// Get time of now.
  ///
  static Timestamp now();
  static Timestamp invalid()
  {
    return Timestamp();
  }

  static Timestamp fromUnixTime(time_t t)
  {
    return fromUnixTime(t, 0);
  }

  static Timestamp fromUnixTime(time_t t, int microseconds)
  {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t microSecondsSinceEpoch_;

};

inline double timeDifference(Timestamp high, Timestamp low){ // inline函数的定义一般放在头文件内
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds){
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}