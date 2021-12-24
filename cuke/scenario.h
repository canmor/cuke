
#ifndef CUKE_SCENARIO_H_
#define CUKE_SCENARIO_H_

#include <memory>

namespace cuke {
    struct scenario_ref {
        void reset() {
            impl.reset();
        };

        template<typename T, typename U = std::remove_const_t<std::remove_reference_t<T>>>
        U &ensure() {
            if (!impl) {
                impl = std::make_shared<U>();
            }
            return *static_cast<U *>(impl.get());
        }

    private:
        std::shared_ptr<void> impl;
    };
}

#endif //CUKE_SCENARIO_H_
