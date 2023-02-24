#pragma once

#include <vsm/unit_test/backend/branch.hpp>

#include <type_traits>

namespace vsm::unit_test {
namespace detail {

template<typename T>
class generator
{
public:
	virtual advance_result advance(uint32_t count) = 0;
	virtual T const& get() const = 0;
};

template<typename T>
class generator_ptr
{
	generator<T>* const m_ptr;

public:
	generator_ptr(T* const ptr)
		: m_ptr(ptr)
	{
	}
	
	generator_ptr(generator_ptr const&) = delete;
	generator_ptr& operator=(generator_ptr const&) = delete;
	
	~generator_ptr()
	{
		m_ptr->~generator();
	}

	generator<T>* operator->()
	{
		return m_ptr;
	}

	generator<T> const* operator->() const
	{
		return m_ptr;
	}
};


auto generate(auto const create_generator, std::source_location const& source = std::source_location::current())
{
	uint32_t const position = backend::get_branch_position(source);

	
}


struct generator_factory {};

template<typename T>
class single_generator : public generator<T>
{
	T const m_value;
	uint8_t m_index = 0;

public:
	explicit single_generator(auto&& value)
		: m_value(vsm_forward(value))
	{
	}

	advance_result advance(uint32_t count) override
	{
		count = std::min(count, static_cast<uint32_t>(1) - m_index);
		m_index += count;
		return { m_index < 1, count };
	}
	
	T const& get() const override
	{
		vsm_assert(m_index == 0);
		return m_value;
	}
};

template<typename T>
class multi_generator final : public generator<T>
{
	generator_ptr<T> const* const m_beg;
	generator_ptr<T> const* const m_end;
	generator_ptr<T> const* m_current;

public:
	explicit multi_generator(std::span<generator<T>* const> const generators)
		: m_beg(generators.data())
		, m_end(m_beg + generators.size())
		, m_current(m_beg)
	{
	}
	
	~multi_generator() override
	{
		for (auto it = m_beg; it != m_end; ++it)
		{
			it->~generator_ptr();
		}
	}
	
	advance_result advance(uint32_t const count) override
	{
		uint32_t advance_count = 0;
		while (m_current != m_end && advance_count < count)
		{
			auto const r = m_current->advance(count - advance_count);
			advance_count += r.count;
			
			if (!r.value)
			{
				++m_current;
			}
		}
		return { m_current != m_end, advance_count };
	}
	
	T const& get() const override
	{
		vsm_assert(m_current != m_end);
		return m_current->get();
	}
};

template<typename T, std::convertible_to<T> U>
class cast_generator final : public generator<T>
{
	generator_ptr<U> const m_generator;
	std::optional<T> m_result;

public:
	explicit cast_generator(generator<U>* const generator)
		: m_generator(generator)
	{
	}

	advance_result advance(uint32_t const count) override
	{
		if (count > 0)
		{
			m_result.reset();
		}

		return m_generator->advance(count);
	}

	T const& get() const override
	{
		if (!m_result)
		{
			m_result = m_generator->get();
		}
		return *m_result;
	}
};

template<typename T>
class when_generator final : public generator<T>
{
	generator_ptr<T> const m_generator;
	bool const m_condition;

public:
	explicit when_generator(generator<T>* const generator, bool const condition)
		: m_generator(generator)
		, m_condition(condition)
	{
	}

	advance_result advance(uint32_t const count) override
	{
		return m_condition
			? m_generator->advance(count)
			: advance_result{ false, 0 };
	}
	
	T const& get() const override
	{
		vsm_assert(m_condition);
		return m_generator->get();
	}
};

template<typename T, typename Predicate>
class where_generator final : public generator<T>
{
	generator_ptr<T> const m_generator;
	Predicate const m_predicate;

public:
	explicit where_generator(generator<T>* const generator, auto&& predicate)
		: m_generator(generator)
		, m_predicate(vsm_forward(predicate))
	{
	}

	advance_result advance(uint32_t const count) override
	{
		uint32_t advance_count = 0;
		while (advance_count < count && (m_result = nullptr, m_generator.advance(1).value))
		{
			T const& result = m_generator.get();
			if (m_predicate(result))
			{
				m_result = &result;
				++advance_count;
			}
		}
		return { m_result != nullptr, advance_count };
	}
	
	T const& get() override
	{
		vsm_assert(m_result != nullptr);
		return *m_result;
	}
};

template<std::integral T>
class enumerate_generator final : public generator<T>
{
	using unsigned_type = std::make_unsigned_t<T>;
	using common_type = decltype(uint32_t() + unsigned_type());

	unsigned_type m_value;
	unsigned_type m_max;

public:
	explicit enumerate_generator(T const min, T const max)
		: m_value(static_cast<unsigned_type>(min))
		, m_max(static_cast<unsigned_type>(max))
	{
	}

	advance_result advance(uint32_t count) override
	{
		common_type const difference = m_max - m_value;
		
	}
	
	T const& get() const override
	{
		return m_value;
	}
};

class boolean_generator final : public generator<bool>
{
	bool m_value = false;
	uint8_t m_index = 0;

public:
	advance_result advance(uint32_t count) override
	{
		if (count > 0)
		{
			m_value = true;
		}
		
		count = std::min(count, static_cast<uint32_t>(2) - m_index);
		m_index += count;
		
		return { m_index < 2, count };
	}
	
	bool const& get() const override
	{
		return m_value;
	}
};

class boolean_generator_factory : public generator_factory
{
public:
	vsm_static_operator boolean_generator operator()() vsm_static_operator_const
	{
		return boolean_generator();
	}
};


template<typename T = void, typename U>
auto make_single_generator(generator<U>* const generator)
{
	if constexpr (std::is_void_v<T> || std::is_same_v<T, U>)
	{
		return generator;
	}
	else
	{
		return new (branch_allocator) cast_generator<T>(generator);
	}
}

template<typename T = void>
auto make_single_generator(std::derived_from<generator_factory> auto const factory)
{
	return make_single_generator<T>(factory());
}

template<typename T = void>
auto make_single_generator(auto&& value)
{
	using type = conditional_t<std::is_void_v<T>, std::remove_cvref_t<decltype(value)>, T>;
	return make_single_generator<T>(new (branch_allocator) single_generator<type>(vsm_forward(value)));
}


template<typename T>
struct as_tag {};

template<typename T = void>
void make_generator() = delete;

template<typename T = void, typename First, typename... Rest>
auto make_generator(First&& first, Rest&&... rest)
{
	using type = conditional_t<std::is_void_v<T>, remove_cvref_t<First>, T>;

	if constexpr (sizeof...(Rest) == 0)
	{
		return make_single_generator<type>(vsm_forward(first));
	}
	else
	{
		static constexpr size_t size = 1 + sizeof...(Rest);
		auto const generators = new (branch_allocator) generator_ptr<type>[size] =
		{
			make_single_generator<type>(vsm_forward(first)),
			make_single_generator<type>(vsm_forward(rest))...
		};
		return new (branch_allocator) multi_generator<type>(generators, size);
	}
}

template<typename T = void, typename As>
auto make_generator(as_tag<As>, auto&&... args)
{
	static_assert(std::is_void_v<T>,
		"as<T> may not be specified multiple times.");

	return make_generator<As>(vsm_forward(args)...);
}

} // namespace detail
namespace generators {

template<typename T>
inline constexpr detail::as_tag<T> as = {};

auto when(bool const condition, auto&&... args)
{
	return detail::when_generator(detail::make_generator(vsm_forward(args)...), condition);
}

auto where(auto&& predicate, auto&&... args)
{
	return detail::where_generator(detail::make_generator(vsm_forward(args)...), vsm_forward(predicate));
}

template<std::integral T>
detail::enumerate_generator<T> enumerate(T const max)
{
	return detail::enumerate_generator<T>(static_cast<T>(0), max);
}

template<std::integral T>
detail::enumerate_generator<T> enumerate(T const min, T const max)
{
	return detail::enumerate_generator<T>(min, max);
}

inline constexpr detail::boolean_generator_factory boolean = {};

} // namespace generators
} // namespace vsm::unit_test

#define vsm_detail_unit_test_generator(...) ([&]() \
	{ \
		using namespace ::vsm::unit_test::generators; \
		return ::vsm::unit_test::make_generator(__VA_ARGS__); \
	})

#define vsm_unit_test_generate(...) \
	(::vsm::unit_test::detail::generate(vsm_detail_unit_test_generator(__VA_ARGS__)))
