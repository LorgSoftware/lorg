#ifndef LORG_HPP
#define LORG_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace lorg
{

constexpr char NODE_DEFINITION_CHARACTER = '#';
constexpr char UNIT_DEFINITION_CHARACTER = '$';
constexpr char UNIT_NAME_VALUE_SEPARATOR = ':';

struct Unit
{
    // NOTE: would not it be better to reference the unit to a "unit
    // definition" which holds the unit name. Something like:
    //   int unit_type_reference;
    // The unit definitions would be a mapping or an array. After all the units
    // with the same "name" refers implicitely to a unit definition so we could
    // make in the code this association more explicit.
    std::string name;

    float value;
    bool is_real;
    bool is_ignored;
};

struct Node
{
    std::vector<std::unique_ptr<Node>> children;

    std::string title;

    // NOTE: mapping unit name with a unit is maybe not the best thing. If we
    // use something holding unit definitions and that each unit refers to
    // their unit definition, then the key could be the unit definition
    // reference. The definition here would be like:
    //   std::map<int, Unit> units;
    std::map<std::string, Unit> units;
};

struct ParserResult
{
    bool has_error;
    std::string error_message;

    // Hold the calculation for all the parsed nodes.
    std::unique_ptr<Node> total_node;
};

ParserResult parse(std::string const & content);
}

#endif
