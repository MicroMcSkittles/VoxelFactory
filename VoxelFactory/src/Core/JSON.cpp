#include "JSON.h"
#include "Core/Utils.h"

#include <map>
#include <fstream>

namespace JSON {
	Node::Node(): type(NodeType::ValueNull) { }
	Node::Node(NodeType type) : type(type) { }
	Node::Node(uint8_t v) : type(NodeType::Number), value(std::to_string(v)) { }
	Node::Node(uint32_t v) : type(NodeType::Number), value(std::to_string(v)) {}
	Node::Node(int v) : type(NodeType::Number), value(std::to_string(v)) {}
	Node::Node(float v) : type(NodeType::Number), value(std::to_string(v)) {}
	Node::Node(bool v) {
		if (v) type = NodeType::ValueTrue;
		else type = NodeType::ValueFalse;
	}
	Node::Node(const std::string& v) : type(NodeType::String), value(v) {}
	Node::Node(const char* v) : type(NodeType::String), value(v) { }

	Node::~Node() { }

	Node::operator uint8_t() const {
		ASSERT(type == NodeType::Number);
		return (uint8_t)std::stoi(value);
	}
	Node::operator uint32_t() const {
		ASSERT(type == NodeType::Number);
		return (uint32_t)std::stoi(value);
	}
	Node::operator int() const {
		ASSERT(type == NodeType::Number);
		return std::stoi(value);
	}
	Node::operator float() const {
		ASSERT(type == NodeType::Number);
		return std::stof(value);
	}
	Node::operator bool() const {
		ASSERT(type == NodeType::ValueTrue || type == NodeType::ValueFalse);
		return type == NodeType::ValueTrue;
	}
	Node::operator std::string() const {
		ASSERT(type == NodeType::String);
		return value;
	}
	Node::operator glm::vec2() const {
		ASSERT(type == NodeType::Array);
		ASSERT(children_array.size() == 2);
		glm::vec2 value;
		value.x = children_array[0];
		value.y = children_array[1];
		return value;
	}

	Node& Node::operator [](int index) {
		ASSERT(type == NodeType::Array);
		ASSERT(index >= 0 && index < children_array.size());
		return children_array[index];
	}
	Node& Node::operator [](const std::string& key) {
		ASSERT(type == NodeType::Object);
		if (!children_object.count(key)) children_object.insert({ key, NodeType::ValueNull });
		return children_object[key];
	}
	Node& Node::operator[](const char* key) {
		ASSERT(type == NodeType::Object);
		if (!children_object.count(key)) children_object.insert({ key, NodeType::ValueNull });
		return children_object[key];
	}
	const Node& Node::operator [](int index) const {
		ASSERT(type == NodeType::Array);
		ASSERT(index >= 0 && index < children_array.size());
		return children_array.at(index);
	}
	const Node& Node::operator [](const std::string& key) const {
		ASSERT(type == NodeType::Object);
		ASSERT(children_object.count(key));
		return children_object.at(key);
	}
	const Node& Node::operator[](const char* key) const {
		ASSERT(type == NodeType::Object);
		ASSERT(children_object.count(key));
		return children_object.at(key);
	}

	bool Node::IsNull() const {
		return type == NodeType::ValueNull;
	}

	size_t Node::Size() const {
		if (type == NodeType::Array) return children_array.size();
		if (type == NodeType::Object) return children_object.size();
		return 0;
	}

	Node Parser::Parse(const std::string& filename) {
		std::ifstream file;
		file.open(filename);
		ASSERT(file.is_open());

		std::stringstream source_buffer;
		source_buffer << file.rdbuf();
		Node tree = ParseFromString(source_buffer.str());

		file.close();

		return tree;
	}
	Node Parser::ParseFromString(const std::string& source) {
		Consumer consumer;
		consumer.source = source;

		consumer.SkipWhiteSpace();
		return ParseObject(consumer);
	}

	char Parser::Consumer::Eat()  { 
		if (cursor >= source.size()) return '\0';
		return source[cursor++];   
	}
	char Parser::Consumer::Peek() { 
		if (cursor >= source.size()) return '\0';
		return source[cursor];     
	}
	char Parser::Consumer::Next() { 
		if (cursor + 1 >= source.size()) return '\0';
		return source[cursor + 1]; 
	}
	void Parser::Consumer::SkipWhiteSpace() {
		cursor = source.find_first_not_of(" \r\n\t", cursor);
	}

	Node Parser::ParseObject(Consumer& consumer) {
		char c = consumer.Eat();
		ASSERT(c == '{');
		consumer.SkipWhiteSpace();

		Node object_node = NodeType::Object;

		while ((c = consumer.Peek()) != '}') {
			std::string key = ParseString(consumer);
			consumer.Eat();
			consumer.SkipWhiteSpace();
			
			c = consumer.Eat();
			ASSERT(c == ':');

			object_node.children_object.insert({ key, ParseValue(consumer) });

			consumer.Eat();
			consumer.SkipWhiteSpace();

			c = consumer.Peek();
			ASSERT(c == '}' || c == ',');
			if (c == ',') consumer.Eat();
			else continue;

			consumer.SkipWhiteSpace();
		}

		return object_node;
	}
	Node Parser::ParseArray(Consumer& consumer) {
		char c = consumer.Eat();
		ASSERT(c == '[');
		consumer.SkipWhiteSpace();

		Node array_node = NodeType::Array;

		while ((c = consumer.Peek()) != ']') {
			array_node.children_array.push_back(ParseValue(consumer));
			
			consumer.Eat();
			consumer.SkipWhiteSpace();

			c = consumer.Peek();
			ASSERT(c == ']' || c == ',');
			if (c == ',') consumer.Eat();
			else continue;

			consumer.SkipWhiteSpace();
		}

		return array_node;
	}
	std::string Parser::ParseString(Consumer& consumer) {
		char c = consumer.Eat();
		ASSERT(c == '\"');

		size_t end = consumer.cursor;
		while ((end = consumer.source.find_first_of("\"", end)) != std::string::npos) {
			if (consumer.source[end - 1] != '\\') break;
		}
		ASSERT(end != std::string::npos);
		std::string string_value = std::string(consumer.source.begin() + consumer.cursor, consumer.source.begin() + end);
		consumer.cursor = end;

		while ((end = string_value.find_first_of("\\")) != std::string::npos) {
			char next = string_value[end + 1];
			std::string actual;
			if (next == '\\') actual = "\\";
			else if (next == '/') actual = "/";
			else if (next == 'b') actual = "\b";
			else if (next == 'f') actual = "\f";
			else if (next == 'b') actual = "\b";
			else if (next == 'n') actual = "\n";
			else if (next == 'r') actual = "\r";
			else if (next == 't') actual = "\t";
			else if (next == 'u') {
				actual = "uXXXX"; // TODO: figure out how the hex stuff works
			}
			else { ASSERT(false); }

			std::string left = std::string(string_value.begin(), string_value.begin() + end);
			std::string right = std::string(string_value.begin() + end + actual.size(), string_value.end());
			string_value = left + right;
		}

		return string_value;
	}
	Node Parser::ParseNumber(Consumer& consumer) {
		size_t end = consumer.source.find_first_not_of("1234567890.+-Ee", consumer.cursor);
		std::string number = std::string(consumer.source.begin() + consumer.cursor, consumer.source.begin() + end);
		consumer.cursor = end - 1;
		return std::stof(number);
	}
	Node Parser::ParseBoolean(Consumer& consumer) {
		std::string keyword = std::string(consumer.source.begin() + consumer.cursor, consumer.source.begin() + consumer.cursor + 4);
		ASSERT(keyword == "true" || keyword == "false");
		consumer.cursor += 3;
		if (keyword == "true") return NodeType::ValueTrue;
		return NodeType::ValueFalse;
	}
	Node Parser::ParseNull(Consumer& consumer) {
		std::string keyword = std::string(consumer.source.begin() + consumer.cursor, consumer.source.begin() + consumer.cursor + 4);
		ASSERT(keyword == "null");
		consumer.cursor += 3;
		return NodeType::ValueNull;
	}
	Node Parser::ParseValue(Consumer& consumer) {

		consumer.SkipWhiteSpace();
		char c = consumer.Peek();
		if (c == '{')               return ParseObject(consumer);
		if (c == '[')               return ParseArray(consumer);
		if (c == '\"')              return ParseString(consumer);
		if (c == '-' || isdigit(c)) return ParseNumber(consumer);
		if (c == 't' || c == 'f')   return ParseBoolean(consumer);
		if (c == 'n')               return ParseNull(consumer);

		return Node();
	}

	void Parser::Serialize(const Node& tree, const std::string& filename) {
		std::ofstream file;
		file.open(filename);
		ASSERT(file.is_open());

		file << SerializeToString(tree);

		file.close();
	}
	std::string Parser::SerializeToString(const Node& tree) {
		ASSERT(tree.type == NodeType::Object);
		std::stringstream stream;
		SerializeObject(stream, tree, "");
		return stream.str();
	}
	void Parser::SerializeObject(std::stringstream& stream, const Node& tree, const std::string& tab_depth) {
		stream << "{\n";

		std::string next_depth = tab_depth + c_TabDepth;
		for (auto& [key, value] : tree.children_object) {
			stream << next_depth << "\"" << key << "\" : ";
			SerializeValue(stream, value, next_depth);
			if (std::prev(tree.children_object.end())->first != key) stream << ",";
			stream << "\n";
		}

		stream << tab_depth << "}";
	}
	void Parser::SerializeArray(std::stringstream& stream, const Node& tree, const std::string& tab_depth) {
		stream << "[\n";

		std::string next_depth = tab_depth + c_TabDepth;
		for (int i = 0; i < tree.children_array.size(); i++) {
			stream << next_depth;
			SerializeValue(stream, tree.children_array[i], next_depth);
			if (i != tree.children_array.size() - 1) stream << ",";
			stream << "\n";
		}

		stream << tab_depth << "]";
	}
	void Parser::SerializeValue(std::stringstream& stream, const Node& tree, const std::string& tab_depth) {
		switch (tree.type) {
		case NodeType::Object:     SerializeObject(stream, tree, tab_depth); break;
		case NodeType::Array:      SerializeArray(stream, tree, tab_depth);  break;
		case NodeType::String:     stream << "\"" << tree.value << "\"";     break;
		case NodeType::Number:     stream << tree.value;                     break;
		case NodeType::ValueTrue:  stream << "true";                         break;
		case NodeType::ValueFalse: stream << "false";                        break;
		case NodeType::ValueNull:  stream << "null";                         break;
		}
	}
	
};