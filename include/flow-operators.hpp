#pragma once

#if !defined(FLOW_OPERATORS_HPP)
#define FLOW_OPERATORS_HPP

#include <rxcpp/rx-includes.hpp>

namespace rxcpp {

struct select_tag {
    template<class Included>
    struct include_header {
        static_assert(Included::value, "mssing include: please #include flow-select.hpp");
    };
};

struct project_tag {
    template<class Included>
    struct include_header{
        static_assert(Included::value, "missing include: please #include flow-project.hpp");
    };
};

}

#endif

