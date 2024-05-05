#include <string>
#include "info.h"

class RequestDispatcher {
    public:
        RequestDispatcher();
        std::string dispatch_request(RequestDispatcherInfo info);

    private:
        enum Handlers {static_file, echo};
        bool is_valid_request(std::string request);
        std::string get_first_line(std::string request);
        std::string get_path(std::string request);
        std::string response_;
};