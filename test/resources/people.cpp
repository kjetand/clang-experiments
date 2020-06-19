
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

    auto get_info() const noexcept -> const auto&
    {
        return _info;
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

constexpr auto add(const int a, const int b)
{
    return a + b;
}

template <typename T>
auto multiply(T a, T b) noexcept
{
    return a * b;
}

}