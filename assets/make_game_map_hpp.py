TEMPLATE = """\
#include <vector>

namespace GameMap {{
    const size_t LEVEL_SEED = 42;
    const size_t MAP_W = 10;
    const size_t MAP_H = 7;
    const std::array<bool, 7*10> LEVEL_1 {{{{
{}
    }}}};
}}
"""


if __name__ == "__main__":
    with open("assets/map.txt", 'r') as f:
        lines = f.readlines()
    lines = [line for line in reversed(lines)]
    result = TEMPLATE.format(
        "\n".join([("\t\t" + ", ".join([c for c in line.strip()]) + ',') for line in lines])
    )
    with open("GameMap.hpp", 'w') as f:
        f.write(result)