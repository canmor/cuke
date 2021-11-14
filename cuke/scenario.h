
#ifndef CUKE_SCENARIO_H_
#define CUKE_SCENARIO_H_

#include <memory>

namespace cuke {
    struct scenario_ref {
        void reset() {
            impl.reset();
        };

        template<typename T>
        T &ensure() {
            if (!impl) {
                impl = std::make_shared<T>();
            }
            return *static_cast<T *>(impl.get());
        }

    private:
        std::shared_ptr<void> impl;
    };
}

#endif //CUKE_SCENARIO_H_
