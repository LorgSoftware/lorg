#include "lorg.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <stack>

using namespace lorg;

bool is_char_in_vector(char const & c, std::vector<char> const & v);

// Streams to a given string, and keeps the column and the line position
// synchronized with the current character the stream is currently pointing to.
//
// NOTE(nales, 2023-01-06): Some characters are completely ignored by Lorg so we skip them here.
struct StringStream
{
	// The line of the last character returned by `get()`.
	int line;

	// The column of the last character returned by `get()`.
	int column;

	// The line of the character returned by `peek()`.
	int peek_line;

	// The column of the character returned by `peek()`.
	int peek_column;

	// The current look up character index in the string.
	size_t index;

	std::string const & s;

	StringStream(std::string const & string_reference):
		line(0),
		column(0),
		peek_line(1),
		peek_column(1),
		index(0),
		s(string_reference)
	{
		if(s.empty())
		{
			peek_line = 0;
			peek_column = 0;
			return;
		}
		while(
			index < s.size() &&
			is_char_in_vector(s[index], IGNORED_CHARACTERS)
		)
		{
			index++;
			peek_column++;
		}
		if(!eof())
		{
			if(s[index] == '\n')
			{
				peek_line++;
				peek_column = 0;
			}
		}
	}

	bool eof() noexcept
	{
		return index >= s.size();
	}

	char get() noexcept
	{
		if(eof())
		{
			return '\0';
		}
		char c = s[index];
		line = peek_line;
		column = peek_column;

		index++;
		peek_column++;

		while(
			index < s.size() &&
			is_char_in_vector(s[index], IGNORED_CHARACTERS)
		)
		{
			index++;
			peek_column++;
		}

		if(!eof())
		{
			if(s[index] == '\n')
			{
				peek_line++;
				peek_column = 0;
			}
		}
		return c;
	}

	char peek() noexcept
	{
		if(eof())
		{
			return '\0';
		}
		else
		{
			return s[index];
		}
	}
};

struct ConvertStringToNodesResult
{
	ParserResult parser_result;
	std::set<std::string> existing_units;
};

bool is_char_in_vector(char const & c, std::vector<char> const & v)
{
	for(char const & v_c : v)
	{
		if(c == v_c)
		{
			return true;
		}
	}
	return false;
}

inline bool is_whitespace(char const & c)
{
	return c == ' ' || c == '\t';
}

inline bool is_end_of_line(char const & c)
{
	return c == '\n' || c == '\0';
}

// Move the stream so the next time `get()` is called it returns something else
// than a white space.
void skip_whitespaces(StringStream & stream)
{
	// No need to check EOF because stream returns '\0' when EOF.
	while(is_whitespace(stream.peek()))
	{
		stream.get();
	}
}

// Move the stream so the next time `get()` is called it returns the first
// character after the current line.
void skip_line(StringStream & stream)
{
	while(!is_end_of_line(stream.peek()))
	{
		stream.get();
	}
}

inline bool is_digit(char const & c)
{
	return (48 <= c && c <= 57);
}

// The value should be in the format `/[-+]?\d+(\.\d+)?/`.
bool is_unit_value_ok(std::string const & value)
{
	if(value.size() == 0)
	{
		return false;
	}

	size_t i = 0;
	// Check if has sign.
	if(value[0] == '-' || value[0] == '+')
	{
		i = 1;
	}

	// Check if has only digits or digits then a decimal point for floats.
	for(; i < value.size(); i++)
	{
		char const & c = value[i];
		if(is_digit(c))
		{
			continue;
		}
		else if(c == '.')
		{
			break;
		}
		else
		{
			return false;
		}
	}

	// Check if this is the definition of an integer.
	if(i == value.size() && value[value.size()-1] != '.')
	{
		return true;
	}

	// Trailing points are not allowed.
	if(i == value.size() - 1 && value[i] == '.')
	{
		return false;
	}

	// Check the decimals of the supposedly float.
	for(i = i + 1; i < value.size(); i++)
	{
		if(is_digit(value[i]))
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

// `first_char` is needed because we often detect the need of getting the rest
// of the line after checking the first character.
// After this function ran, `stream.get()` returns the first character after
// the line.
std::string get_rest_of_line_without_trailing_spaces(
	StringStream stream, const char first_char
)
{
	std::string content;
	char c = first_char;
	int trailing_space_count = 0;
	while(!is_end_of_line(c))
	{
		content.push_back(c);
		if(is_whitespace(c))
		{
			trailing_space_count++;
		}
		else
		{
			trailing_space_count = 0;
		}
		c = stream.get();
	}
	if(trailing_space_count > 0)
	{
		content.resize(content.size() - trailing_space_count);
	}
	return content;
}

// Returns a substring from `start` included to `end` excluded. Trim the spaces.
std::string get_substring_without_leading_trailing_spaces(
	std::string const & str, size_t start, size_t end
)
{
	assert(start <= end);
	assert(start < str.size());
	assert(end <= str.size());
	std::string substring;
	// Skip leading spaces.
	while(is_whitespace(str[start]) && start < end)
	{
		start++;
	}
	// All characters were white spaces.
	if(start == end)
	{
		return substring;
	}
	// Skip trailing spaces.
	while(is_whitespace(str[end-1]) && start < end)
	{
		end--;
	}
	// Return the substring.
	for(size_t i = start; i < end; i++)
	{
		substring.push_back(str[i]);
	}
	return substring;
}

std::string format_error(std::string const message, int line, int column = 0)
{
	std::string error_message = "Line " + std::to_string(line);
	if(column != 0)
	{
		error_message += ", column " + std::to_string(column);
	}
	error_message += ": " + message;
	return error_message;
}

std::string get_error_message_node_without_title(int line)
{
	return format_error(
		"The node has no title.", line
	);
}

std::string get_error_message_node_without_direct_parent(int line)
{
	return format_error(
		"The node is not a direct descendant to any other node.", line
	);
}

std::string get_error_message_unit_definition_ill_formed(int line)
{
	std::string error_message = format_error(
		"The unit definition is ill-formed.", line
	);
	error_message += "\nThe unit definition should follow this format:";
	error_message += "\n    $ UNIT_NAME : UNIT_VALUE";
	return error_message;
}

std::string get_error_message_unit_value_incorrect(int line)
{
	return format_error(
		"The unit value is incorrect.", line
	);
}

std::string get_error_message_unit_outside_node(int line)
{
	return format_error(
		"The unit definition is outsite of a node.", line
	);
}

ConvertStringToNodesResult create_ConvertStringToNodesResult_error(
	std::string error_message
)
{
	ConvertStringToNodesResult result;
	result.parser_result.has_error = true;
	result.parser_result.error_message = error_message;
	return result;
}

ConvertStringToNodesResult convert_string_to_nodes(std::string const & content)
{
	ConvertStringToNodesResult result;
	result.parser_result.has_error = false;
	result.parser_result.total_node = std::make_unique<Node>();

	Node & total_node = *(result.parser_result.total_node);
	total_node.title = "TOTAL";

	StringStream stream(content);

	// Contain the node currently being parsed. We use a stack to avoid
	// unnecessary recursion. The stack size represents the level of the node
	// on top. The node below it is its direct parent.
	std::stack<std::unique_ptr<Node>> nodes_to_add;

	while(!stream.eof())
	{
		// Skip useless possible white spaces at the beginning of the line.
		if(stream.column == 0 && is_whitespace(stream.peek()))
		{
			skip_whitespaces(stream);
			if(stream.eof())
			{
				break;
			}
		}

		char c = stream.get();

		if(c == NODE_DEFINITION_CHARACTER)
		{
			// Keep the current line because maybe the node definition is
			// ill-formed, and when we detect it the stream is pointing to the
			// next line.
			int current_line = stream.line;

			// Get node level.
			size_t level = 0;
			while(c == NODE_DEFINITION_CHARACTER)
			{
				level++;
				c = stream.get();
			}

			// Get node title.
			if(is_whitespace(c))
			{
				skip_whitespaces(stream);
			}
			c = stream.get();
			if(is_end_of_line(c))
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_node_without_title(current_line)
				);
			}
			std::string title = get_rest_of_line_without_trailing_spaces(stream, c);

			// Manage hierarchy.
			if(level > nodes_to_add.size() + 1)
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_node_without_direct_parent(current_line)
				);
			}
			while(level < nodes_to_add.size() + 1)
			{
				// Moving the siblings and nephews until the top of the stack
				// is the direct parent of the current node.
				std::unique_ptr<Node> other = std::move(nodes_to_add.top());
				nodes_to_add.pop();
				if(nodes_to_add.size() > 0)
				{
					nodes_to_add.top()->children.push_back(std::move(other));
				}
				else
				{
					total_node.children.push_back(std::move(other));
				}
			}
			auto current_node = std::make_unique<Node>();
			current_node->title = title;
			nodes_to_add.push(std::move(current_node));
		}
		else if(c == UNIT_DEFINITION_CHARACTER)
		{
			// Keep the current line because maybe the node definition is
			// ill-formed, and when we detect it the stream is pointing to the
			// next line.
			int current_line = stream.line;

			// We get all the line immediately because unit names can contain
			// `UNIT_NAME_VALUE_SEPARATOR`.
			skip_whitespaces(stream);
			std::string definition = get_rest_of_line_without_trailing_spaces(stream, stream.get());
			if(definition.empty())
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_definition_ill_formed(current_line)
				);
			}

			// We get the last `UNIT_NAME_VALUE_SEPARATOR` index so it is sure
			// that everything before it is part of the unit name.
			size_t separator_index = definition.size() - 1;
			{
				for(; separator_index > 0; separator_index--)
				{
					if(definition[separator_index] == UNIT_NAME_VALUE_SEPARATOR)
					{
						break;
					}
				}
				if(definition[separator_index] != UNIT_NAME_VALUE_SEPARATOR)
				{
					return create_ConvertStringToNodesResult_error(
						get_error_message_unit_definition_ill_formed(current_line)
					);
				}
			}
			// The only thing in the node definition is `UNIT_DEFINITION_CHARACTER`.
			if(definition.size() == 1)
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_definition_ill_formed(current_line)
				);
			}

			// Get name.
			std::string name = get_substring_without_leading_trailing_spaces(
				definition, 0, separator_index
			);
			if(name.empty())
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_definition_ill_formed(current_line)
				);
			}

			// Get value.
			std::string value_string = get_substring_without_leading_trailing_spaces(
				definition, separator_index + 1, definition.size()
			);
			if(value_string.empty())
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_definition_ill_formed(current_line)
				);
			}
			if(!is_unit_value_ok(value_string))
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_definition_ill_formed(current_line)
				);
			}

			// Check the unit definition is not outside of a node. We prefer to
			// do that after checking the syntax of the unit definition.
			if(nodes_to_add.empty())
			{
				return create_ConvertStringToNodesResult_error(
					get_error_message_unit_outside_node(current_line)
				);
			}

			Unit unit;
			unit.name = name;
			unit.value = std::stof(value_string);
			unit.is_real = true;
			unit.is_ignored = false;
			nodes_to_add.top()->units[unit.name] = unit;
			result.existing_units.insert(unit.name);
		}
		else if(c == '\n')
		{
			continue;
		}
		else
		{
			skip_line(stream);
		}
	}

	if(!nodes_to_add.empty())
	{
		while(nodes_to_add.size() > 1)
		{
			std::unique_ptr<Node> other = std::move(nodes_to_add.top());
			nodes_to_add.pop();
			nodes_to_add.top()->children.push_back(std::move(other));
		}
		total_node.children.push_back(std::move(nodes_to_add.top()));
		nodes_to_add.pop();
	}

	return result;
}

void update_node_unit_values(
	Node & node, std::set<std::string> existing_units,
	std::set<std::string> units_to_ignore
)
{
	std::map<std::string, Unit> & units = node.units;

	// Add all missing units and define which ones need to be calculated.
	std::set<std::string> units_to_calculate;
	for(auto const & unit_name : existing_units)
	{
		if(units.find(unit_name) == units.end())
		{
			Unit unit;
			unit.name = unit_name;
			unit.value = 0.0f;
			unit.is_ignored = false;
			unit.is_real = false;
			units[unit_name] = unit;
			units_to_calculate.insert(unit_name);
		}
	}

	// Ignore all units to ignore and set the one the children should ignore.
	std::set<std::string> children_units_to_ignore;
	for(auto & unit_pair : units)
	{
		Unit & unit = unit_pair.second;
		if(units_to_ignore.find(unit.name) != units_to_ignore.end())
		{
			unit.is_ignored = true;
			children_units_to_ignore.insert(unit.name);
		}
		if(unit.is_real)
		{
			children_units_to_ignore.insert(unit.name);
		}
	}

	// Update the children.
	for(std::unique_ptr<Node> & child : node.children)
	{
		update_node_unit_values(*child, existing_units, children_units_to_ignore);
	}

	// Update the units to calculate.
	for(std::unique_ptr<Node> & child : node.children)
	{
		for(auto & unit_pair : units)
		{
			Unit & unit = unit_pair.second;
			if(!unit.is_real)
			{
				unit.value += child->units[unit.name].value;
			}
		}
	}
}

ParserResult lorg::parse(std::string const & content)
{
	ConvertStringToNodesResult result = convert_string_to_nodes(content);
	if(result.parser_result.has_error)
	{
		return std::move(result.parser_result);
	}
	update_node_unit_values(
		*(result.parser_result.total_node), result.existing_units, {}
	);
	return std::move(result.parser_result);
}
