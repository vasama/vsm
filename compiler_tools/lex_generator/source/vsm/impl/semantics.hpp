#pragma once

#include <vsm/impl/parser.hpp>

#include <span>

#include <cstdint>

namespace vsm::lexgen {

struct transition
{
	uint32_t index : 31;
	bool accept : 1;
};

struct state
{
	rule const* rule;
	size_t index;
	transition transitions[];
};

struct state_machine
{
	size_t class_count;
	uint8_t input_to_class[0x100];
	
	size_t state_count;
	state const* states[];
};

state_machine const* build_state_machine(syntax_tree const& syntax);

} // namespace vsm::lexgen
