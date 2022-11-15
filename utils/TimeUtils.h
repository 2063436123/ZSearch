#include "../typedefs.h"

class DateTime {
public:
    DateTime() = default;

    DateTime(struct timespec time) : date_time(Poco::Timestamp(time.tv_sec * 1000 + time.tv_nsec / 1000)) {    }

    // format like 2011-03-28 15:00:44
    DateTime(const std::string& date_time_str)
    {
        if (date_time_str.size() != strlen("2011-03-28 15:00:44"))
            throw DateTimeFormatException();
        int year = std::stoi(date_time_str.substr(0, 4));
        int month = std::stoi(date_time_str.substr(5, 2));
        int day = std::stoi(date_time_str.substr(8, 2));
        int hour = std::stoi(date_time_str.substr(11, 2));
        int minute = std::stoi(date_time_str.substr(14, 2));
        int second = std::stoi(date_time_str.substr(17, 2));
        date_time.assign(year, month, day, hour, minute, second);
    }

    Poco::DateTime internal() const {
        return date_time;
    }

    auto operator<(const DateTime& rhs) const {
        return date_time < rhs.date_time;
    }

private:
    Poco::DateTime date_time;
};