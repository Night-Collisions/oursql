#ifndef OURSQL_SERVER_LOGIC_CALENDAR_DATEMANAGER_H_
#define OURSQL_SERVER_LOGIC_CALENDAR_DATEMANAGER_H_

#include <boost/date_time.hpp>

using t_date = boost::gregorian::date;

class DateManager {
   public:
    DateManager() = delete;

    bool isInPeriod(const t_date& from_date, const t_date& to_date);
};

#endif  // OURSQL_SERVER_LOGIC_CALENDAR_DATEMANAGER_H_
