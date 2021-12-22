
#ifndef CUKE_STEP_HUB_H_
#define CUKE_STEP_HUB_H_

#include "step_engine.h"

namespace cuke {
    class step_hub {
    public:
        static step_engine &engine() {
            static step_hub instance;
            return instance.a_engine;
        }

    private:
        step_hub() = default;

        step_engine a_engine;
    };

    struct definer {
        template<class Callable>
        definer(location location, std::string_view desc, Callable func) {
            step_hub::engine().define(desc, func, location);
        }

        template<class Callable>
        definer(std::string_view desc, Callable func) {
            step_hub::engine().define(desc, func);
        }
    };

    struct when : definer {
        using definer::definer;
    };
    struct then : definer {
        using definer::definer;
    };
    struct given : definer {
        using definer::definer;
    };
}

#endif //CUKE_STEP_HUB_H_
