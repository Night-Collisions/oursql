#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_SYSTIME_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_SYSTIME_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include "../../../Core/Exception.h"
#include "Node.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

enum class RangeType { all, from_to };

class SysTime : public Node {
   public:
    SysTime(const std::string& from, const std::string& to,
            std::unique_ptr<exc::Exception>& e)
        : Node(NodeType::sys_time), rtype_(RangeType::from_to) {
        try {
            from_ = time_from_string(from);
            to_ = time_from_string(to);
        } catch (boost::bad_lexical_cast& b) {
            e.reset(new exc::DateCastFromStrExc());
            return;
        } catch (std::out_of_range& e) {
            from_ = ptime(from_string(from));
            to_ = ptime(from_string(to));
        }
    }
    SysTime() : Node(NodeType::sys_time), rtype_(RangeType::all) {}

    std::pair<ptime, ptime> getRange() const { return {from_, to_}; }

    RangeType getRangeType() const { return rtype_; }

    std::string getName() override { return std::string(); }

   private:
    RangeType rtype_;
    ptime from_;
    ptime to_;
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_SYSTIME_H_
