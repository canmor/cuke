
#ifndef CUKE_SOURCE_LOCATION_H_
#define CUKE_SOURCE_LOCATION_H_

#include <string_view>
#include <string>

namespace cuke {
    class location {
    public:
        explicit location(std::string_view file = {"unknown"}, int line = 0) : file(file), line(line) {}

        [[nodiscard]]
        std::string to_string() const {
            return std::string(file) + ":" + std::to_string(line);
        }

    private:
        std::string_view file;
        int line;
    };
}

#endif //CUKE_SOURCE_LOCATION_H_
