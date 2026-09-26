#pragma once
#include "Core/Core.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <glm/glm.hpp>

// This is like the 5th json parser I've writen and it still kinda sucks

namespace JSON {
	enum class NodeType {
		Object,
		Array,
		String,
		Number,
		ValueTrue,
		ValueFalse,
		ValueNull
	};

	struct Node {
		NodeType type;
		std::string value;
		std::map<std::string, Node> children_object;
		std::vector<Node> children_array;

		Node();
		Node(NodeType type);
		Node(uint8_t v);
		Node(uint32_t v);
		Node(int v);
		Node(float v);
		Node(bool v);
		Node(const std::string& v);
		Node(const char* v);
		~Node();

		operator uint8_t() const;
		operator uint32_t() const;
		operator int() const;
		operator float() const;
		operator bool() const;
		operator std::string() const;
		operator glm::vec2() const;

		Node& operator [](int index);
		Node& operator [](const std::string& key);
		Node& operator [](const char* key);
		const Node& operator [](int index) const;
		const Node& operator [](const std::string& key) const;
		const Node& operator [](const char* key) const;

		bool IsNull() const;
		size_t Size() const;
	};

	class Parser {
	private:
		struct Consumer {
			std::string source;
			size_t cursor = 0;

			char Eat();
			char Peek();
			char Next();

			void SkipWhiteSpace();
		};

	public:
		static Node Parse(const std::string& filename);
		static Node ParseFromString(const std::string& source);

		static void Serialize(const Node& tree, const std::string& filename);
		static std::string SerializeToString(const Node& tree);

	private:
		static Node ParseObject(Consumer& consumer);
		static Node ParseArray(Consumer& consumer);
		static std::string ParseString(Consumer& consumer);
		static Node ParseNumber(Consumer& consumer);
		static Node ParseBoolean(Consumer& consumer);
		static Node ParseNull(Consumer& consumer);
		static Node ParseValue(Consumer& consumer);

		static void SerializeObject(std::stringstream& stream, const Node& tree, const std::string& tab_depth);
		static void SerializeArray(std::stringstream& stream, const Node& tree, const std::string& tab_depth);
		static void SerializeValue(std::stringstream& stream, const Node& tree, const std::string& tab_depth);

	private:
		const static inline std::string c_TabDepth = "\t";
	};
};