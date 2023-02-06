#pragma once

#include "../typedefs.h"
#include "StringUtils.h"

class DateTime {
public:
    DateTime() = default;

    DateTime(int64_t since_epoch_seconds) : date_time(Poco::Timestamp(since_epoch_seconds * 1000 * 1000)) {}

    DateTime(struct timespec time) : date_time(Poco::Timestamp(time.tv_sec * 1000 * 1000 + time.tv_nsec / 1000)) {}

    // format like 2011-03-28 15:00:44
    DateTime(const std::string& date_time_str)
    {
        if (date_time_str.size() != strlen("0000-00-00 00:00:00"))
            THROW(DateTimeFormatException("not enough len of " + date_time_str));
        try
        {
            int year = restrictStoi<int>(date_time_str.substr(0, 4));
            int month = restrictStoi<int>(date_time_str.substr(5, 2));
            int day = restrictStoi<int>(date_time_str.substr(8, 2));
            int hour = restrictStoi<int>(date_time_str.substr(11, 2));
            int minute = restrictStoi<int>(date_time_str.substr(14, 2));
            int second = restrictStoi<int>(date_time_str.substr(17, 2));
            date_time.assign(year, month, day, hour, minute, second);
        }
        catch (std::exception &e)
        {
            THROW(DateTimeFormatException("in " + date_time_str));
        }
    }

    std::string string(bool beijing_time_zone = false) const {
        if (beijing_time_zone)
        {
            return DateTime(*this).adjustTime(8 * 3600).string();
        }
        return Poco::format("%04d-%02d-%02d %02d:%02d:%02d", date_time.year(), date_time.month(), date_time.day(), date_time.hour(), date_time.minute(), date_time.second());
    }

    Poco::DateTime internal() const {
        return date_time;
    }

    DateTime& adjustTime(int64_t adjust_time_seconds)
    {
        date_time = Poco::Timestamp(date_time.timestamp().epochMicroseconds() + adjust_time_seconds * 1000 * 1000);
        return *this;
    }

    bool operator<(const DateTime& rhs) const {
        return date_time < rhs.date_time;
    }

    bool operator==(const DateTime& rhs) const {
        return date_time == rhs.date_time;
    }

    bool operator>(const DateTime& rhs) const {
        return date_time > rhs.date_time;
    }

    bool operator>=(const DateTime& rhs) const {
        return date_time >= rhs.date_time;
    }

    bool operator<=(const DateTime& rhs) const {
        return date_time <= rhs.date_time;
    }

    bool operator!=(const DateTime& rhs) const {
        return date_time != rhs.date_time;
    }

private:
    Poco::DateTime date_time;
};

DateTime getModifiedLastDateTime(const std::filesystem::path& path)
{
    return duration_cast<std::chrono::seconds>(last_write_time(path).time_since_epoch()).count();
}

template <typename T>
void httpLog(const T& msg)
{
    static std::mutex log_lock;
    std::lock_guard lg(log_lock);
    std::cout << '[' << DateTime().string(true) << "] " << msg << std::endl;
}