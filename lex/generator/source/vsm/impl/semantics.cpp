#include <vsm/impl/semantics.hpp>

#include <vsm/defer.hpp>
#include <vsm/ordered_set_view.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/vector.hpp>

using namespace vsm;
using namespace vsm::lexgen;

namespace {

struct nfa_state
{
	using state_set = ordered_set_view<nfa_state*>;

	bool initial = false;
	rule const* accept = nullptr;

	size_t transition_count = 0;
	nfa_state* transitions[0x100] = {};

	vector<nfa_state*> epsilon;
	state_set epsilon_set_cache;


	void add_transition(uint8_t const input, nfa_state* const state)
	{
		vsm_assert(state != nullptr);
		vsm_assert(transitions[input] == nullptr);
		transitions[input] = state;
		++transition_count;
	}
	
	void add_epsilon(nfa_state* const state)
	{
		epsilon.push_back(state);
	}
};
using nfa_state_set = nfa_state::state_set;

struct nfa
{
	nfa_state* initial_state;
	nfa_state* accept_state;
};

static nfa_state* nfa_create_state()
{
	return new nfa_state();
}

static nfa nfa_construct_sub(expr const& expr)
{
	nfa nfa = {};

	switch (expr->kind)
	{
	case sk::reference_expr:
		{
			vsm_assert(false);
		}
		break;
	
	case sk::literal_sequence_expr:
		{
			nfa_state* prev_state = nfa_create_state();
			nfa.initial_state = prev_sate;
			
			for (uint8_t const input : static_cast<string_expr const&>(expr).string_token->content)
			{
				nfa_state* const next_state = nfa_create_state();
				prev_state->add_transition(input, next_state);
				prev_state = next_state;
			}
			
			nfa.accept_state = prev_state;
		}
		break;
		
	case sk::literal_alternative_expr:
		{
			nfa.initial_state = nfa_create_state();
			nfa.accept_state = nfa_create_state();
			
			//TODO: Check for duplicates, handle ranges.
			for (uint8_t const input : static_cast<string_expr const&>(expr).string_token->content)
			{
				nfa.initial_state->add_transition(input, nfa.accept_state);
			}
		}
		break;
		
	case sk::zero_or_one_expr:
		{
			nfa = nfa_construct_sub(*static_cast<operator_1_expr const&>(expr).operand);
			
			// Transition directly from initial to accept.
			nfa.initial_state->add_epsilon(nfa.accept_state);
		}
		break;
		
	case sk::zero_or_many_expr:
		{
			nfa const sub = nfa_construct_sub(*static_cast<operator_1_expr const&>(expr).operand);
			
			nfa.initial_state = sub.initial_state;
			nfa.accept_state = nfa_create_state();
			
			// Transition back to initial state.
			sub.accept_state->add_epsilon(nfa.initial_state);
			
			// Final transition to accept.
			nfa.initial_state->add_epsilon(nfa.accept_state);
		}
		break;
	
	case sk::one_or_many_expr:
		{
			nfa const sub = nfa_construct_sub(*static_cast<operator_1_expr const&>(expr).operand);
			
			nfa.initial_state = sub.initial_state;
			nfa.accept_state = nfa_create_state();
			
			// Transition back to initial state.
			sub.accept_state->add_epsilon(nfa.initial_state);
			
			// Final transition to accept.
			sub.accept_state->add_epsilon(nfa.accept_state);
		}
		break;
	
	case sk::sequence_expr:
		{
			nfa const lhs_sub = nfa_construct_sub(*static_cast<operator_1_expr const&>(expr).lhs_operand);
			nfa const rhs_sub = nfa_construct_sub(*static_cast<operator_1_expr const&>(expr).rhs_operand);
			
			nfa.initial_state = lhs_sub.initial_state;
			lhs_sub.accept_state->add_epsilon(rhs_sub.initial_state);
			nfa.accept_state = rhs_sub.accept_state;
		}
		break;
	
	case sk::sequence_expr:
		{
			nfa.initial_state = nfa_create_state();
			nfa.accept_state = nfa_create_state();

			auto const process_sub = [&](auto const& process_sub, expr const& e) -> void
			{
				if (e.kind == sk::alternative_expr)
				{
					process_sub(process_sub, *static_cast<operator_2_expr const&>(expr).lhs_operand);
					process_sub(process_sub, *static_cast<operator_2_expr const&>(expr).rhs_operand);
				}
				else
				{
					nfa const sub = nfa_construct_sub(e);
					nfa.initial_state->add_epsilon(sub.initial_state);
					sub.accept_state->add_epsilon(nfa.accept_state);
				}
			};
			
			process_sub(process_sub, *static_cast<operator_2_expr const&>(expr).lhs_operand);
			process_sub(process_sub, *static_cast<operator_2_expr const&>(expr).rhs_operand);
		}
		break;
	
	default:
		vsm_unreachable();
	}

	return nfa;
}

static nfa_state* nfa_construct(std::span<rule const* const> const rules)
{
	nfa_state* const root_state = nfa_create_state();
	
	for (rule const* const rule : rules)
	{
		nfa const sub = nfa_construct_sub(*rule.expr);
		
		sub.initial_state->initial = true;
		sub.accept_state->accept = rule;
		
		root_state->add_epsilon(sub.initial_state);
	}

	return root_state;
}

static nfa_state_set nfa_get_epsilon_set(nfa_state* const state)
{
	if (state->epsilon_set_cache.empty())
	{
		small_vector<nfa_state*, 32> set;
		set.push_back(state);

		for (nfa_state* const x : state->epsilon)
		{
			for (nfa_state* const y = nfa_get_epsilon_set(x))
			{
				set.push_back(y);
			}
		}
		
		std::ranges::sort(set);
		set.erase(std::ranges::unique(set), set.end());

		state->epsilon_set_cache = ;
	}
	return state->epsilon_set_cache;
}


struct dfa_state
{
	using transition = incomplete_tag_ptr<dfa_state, bool>;

	nfa_state_set nfa_set;
	rule const* accept = nullptr;
	uint32_t index = static_cast<uint32_t>(-1);
	
	//TODO: Rename
	dfa_state* equivalent = nullptr;
	dfa_state* partition_by = nullptr;

	dfa_transition transitions[0x100] = {};
};
using dfa_transition = dfa_state::transition;

struct dfa
{
	vector<dfa_state*> states;
	dfa_state* initial_state;
};

static void dfa_minimize(dfa& dfa)
{
	auto const are_equivalent = [&](dfa_state const& a, dfa_state const& b)
	{
		for (size_t i = 0; i < 0x100; ++i)
		{
			dfa_state* const at = a->transitions[i].ptr();
			dfa_state* const bt = b->transitions[i].ptr();
			
			if (at != bt && (at == nullptr || bt == nullptr || at->equivalent != bt->equivalent))
			{
				return false;
			}
		}
		
		if (a->accept == b->accept)
		{
			return true;
		}
		
		return std::ranges::all_of(a->transitions, [](dfa_state const* const state)
		{
			state != nullptr;
		});
	};

	std::queue<std::span<dfa_state*>> partitions;
	
	auto const push_partition = [&](std::span<dfa_state* const> const partition)
	{
		dfa_state* const primary_state = partition.front();
		primary->equivalent = primary_state;

		if (partition.size() > 1)
		{
			for (dfa_state* const state : partition.subspan(1))
			{
				state->equivalent = primary_state;
			}
			partitions.push(partition);
		}
	};
	
	// Initial partition into rejecting and accepting states.
	{
		auto const beg = dfa.states.data();
		auto const end = beg + dfa.states.size();
	
		auto const mid = std::partition(beg, end, [](dfa_state* const x)
		{
			return x->accept == nullptr;
		});

		push_partition(std::span(beg, mid));
		push_partition(std::span(mid, end));
	}

	small_vector<dfa_state*, 32> primary_states;

	while (!partitions.empty())
	{
		vsm_defer { primary_states.clear(); };
	
		std::span<dfa_state*> const partition = partitions.front();
		partitions.pop();

		for (dfa_state* const state : partition)
		{
			for (dfa_state* const primary : primary_states)
			{
				if (are_equivalent(state, primary))
				{
					state->partition_by = primary;
					goto got_equivalence;
				}
			}
			
			primary_states.push_back(state);
		got_equivalence: {}
		}
		
		if (primary_states.size() == 1)
		{
			continue;
		}
		
		std::ranges::sort(partition, [](dfa_state const* const a, dfa_state const* const b)
		{
			return std::less()(a->partition_by, b->partition_by);
		});
		
		auto beg = partition.begin();
		auto const const end = partition.end();

		while (beg != end)
		{
			dfa_state const* const beg_state = *beg;
		
			auto const mid = std::find(beg, end,
				[&](dfa_state const* const x) { return x != beg_state; });

			push_partition(std::span(beg, mid));

			beg = mid;
		}
	}

	size_t state_count = 0;
	erase_unstable(dfa.states, [](dfa_state const* const state)
	{
		bool const is_primary = state->equivalent == state;
	
		if (is_primary)
		{
			for (size_t i = 0; i < 0x100; ++i)
			{
				if (dfa_transition& transition = state->transitions[i])
				{
					transition.set_ptr(transition->equivalent);
				}
			}
			state->index = state_count++;
		}
	
		return !is_primary;
	});
}

static dfa dfa_construct(nfa_state* const nfa_initial_state)
{
	dfa dfa = {};

	std::queue<dfa_state*> queue;

	auto const create_state = [&]() -> dfa_state*
	{
		dfa_state* const state = new dfa_state;
		state->index = dfa.states.size();
		dfa.states.push_back(state);
		queue.push_back(state);
		return state;
	};
	
	dfa.initial_state = create_state();
	dfa.initial_state->nfa_set = nfa_get_epsilon_set(nfa_initial_state);

	hash_map<nfa_state_set, dfa_state*> nfa_to_dfa;
	small_vector<nfa_state_set*, 32> nfa_state_set_vector;
	
	while (!queue.empty())
	{
		dfa_state* const state = queue.front();
		queue.pop();

		for (size_t i = 0; i < 0x100; ++i)
		{
			rule const* accept = nullptr;
			
			for (nfa_state* const nfa_state : state->nfa_set)
			{
				nfa_state* const nfa_transition = nfa_state->transitions[i];

				if (nfa_transition == nullptr)
				{
					continue;
				}
				
				for (nfa_state* const x = nfa_get_epsilon_set(nfa_transition))
				{
					if (rule const* const rule = x->accept)
					{
						vsm_assert(accept == nullptr);
						accept = rule;
					}
					nfa_state_set_vector.push_back(x);
				}
			}

			if (nfa_state_set_vector.empty())
			{
				continue;
			}
			vsm_defer { nfa_state_set_vector.clear(); };
			
			std::ranges::sort(nfa_state_set_vector);
			nfa_state_set const nfa_set = nfa_state_set(nfa_state_set_vector);

			auto const r = nfa_to_dfa.insert(nfa_state_set);
			if (r.inserted)
			{
				dfa_state* const transition = create_state();
				transition->nfa_set = nfa_state_set;
				transition->accept = accept;
				r.element->value = transition;
			}
			state->transitions[i] = dfa_transition(r.element->value, true);
		}
	}

	dfa_minimize(dfa);

	return dfa;
}

static size_t build_equivalence_classes(dfa const& dfa, uint8_t* const input_to_class, uint8_t* const class_to_input)
{
	using dfa_transition_set = value_span<dfa_transition>;

	size_t const state_count = dfa.states.size();
	small_hash_map<dfa_transition_set, size_t, 0x100> map;

	size_t class_count = 0;

	for (size_t i = 0; i < 0x100; ++i)
	{
		dfa_transition_set transitions = dfa_transition_set(); //TODO

		for (size_t j = 0; j < state_count; ++j)
		{
			transitions[j] = dfa.states[j]->transitions[i];
		}
		
		auto const r = map.emplace(transitions);
		if (r.inserted)
		{
			vsm_assert(class_count < 0x100);
			r.element->value = class_count++;
		}
		input_to_class[i] = r.element->value;
		class_to_input[r.element->value] = i;
	}

	return class_count;
}

} // namespace

state_machine const* lexgen::build_state_machine(syntax_tree const& syntax)
{
	dfa const dfa = dfa_construct(nfa_construct(syntax.rules));
	size_t const state_count = dfa.states.size();


	static constexpr size_t machine_size = offsetof(state_machine, states);
	void* const machine_storage = operator new(machine_size + dfa.states.size() * sizeof(state*));
	state_machine* const machine = new (machine_storage) state_machine;
	std::uninitialized_default_construct_n(machine->states, state_count);
	machine->state_count = state_count;

	uint8_t* const input_to_class = machine->input_to_class;
	uint8_t class_to_input[0x100];

	size_t const class_count = build_equivalence_classes(dfa, input_to_class, class_to_input);
	machine->class_count = class_count;

	for (dfa_state const* const dfa_state : dfa.states)
	{
		static constexpr size_t state_size = offsetof(state, transitions);
		void* const state_storage = operator new(state_size + class_count * sizeof(transition));
		state* const state = new (state_storage) state{ .rule = dfa_state.accept };
		std::uninitialized_default_construct_n(state->transitions, class_count);

		for (size_t i = 0; i < class_count; ++i)
		{
			auto const t = dfa_state->transitions[class_to_input[i]];
			state->transitions[i] = transition{ t->index, t.tag() };
		}

		machine->states[state.index] = state;
	}

	return machine;
}
