
#include <string>
#include <string_view>

namespace klang {

struct person_info {
    std::string name;
    unsigned age;
};

class person {
public:
    explicit person() = default;

    explicit person(std::string_view name, const unsigned age)
        : _info(name, age)
    {
    }

    ~person() = default;

private:
    person_info _info;
};

template <typename T>
struct collection {
};

class people : public collection<person> {
};

template <typename T>
struct is_void : std::false_type {
};

template <>
struct is_void<void> : std::true_type {
};

}