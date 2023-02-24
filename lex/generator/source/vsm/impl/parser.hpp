#pragma once

#include <vsm/assert.h>
#include <vsm/vector.hpp>

#include <coroutine>

namespace vsm {
namespace detail {

class parser_base
{
	struct syntax_stack_frame
	{
		syntax* syntax_node;
		token* first_token;
	};

	small_vector<syntax_stack_frame, 32> m_syntax_stack;

	small_vector<token*, 8> m_token_lookahead;

public:
	struct frame_object {};
	using frame_tag = frame_object const*;

	void enter_frame(frame_tag const tag)
	{
	}

	void leave_frame(frame_tag const tag)
	{
	}

	void move_frame(frame_tag const old_tag, frame_tag const new_tag)
	{
	}

	void* allocate_frame(frame_tag const tag, size_t const size)
	{
		
	}


	void enter_syntax(syntax* const node)
	{
		m_syntax_stack.emplace_back(node, get_token(0));
	}

	void leave_syntax(syntax* const node)
	{
		auto const& frame = m_syntax_stack.back();
		vsm_assert(node == frame.syntax_node);

		token const* const tokens_beg = frame.first_token;
		token const* const tokens_end = get_token(-1);

		char const* const content_beg = tokens_beg->content.data();
		char const* const content_end = tokens_end->content.data() + tokens_end->content.size();

		node->content = std::string_view(content_beg, content_end);

		node->leading_trivia = tokens_beg->leading_trivia;
		node->trailing_trivia = tokens_end->trailing_trivia;

		m_syntax_stack.pop_back();
	}

	void* allocate_syntax(size_t const size)
	{
		
	}

private:
	token* get_token(ptrdiff_t const offset)
	{
		
	}
};

class parser_frame : parser_base::frame_object
{
	parser_base* m_parser;

public:
	explicit parser_frame(parser_base& parser)
		: m_parser(&parser)
	{
		m_parser->enter_frame(this);
	}
	
	parser_frame(parser_frame&& other)
		: m_parser(other.m_parser)
	{
		other.m_parser = nullptr;
		if (m_parser != nullptr)
		{
			m_parser->move(&other, this);
		}
	}

	parser_frame& operator=(parser_frame&&) = delete;

	~parser_frame()
	{
		if (m_parser != nullptr)
		{
			m_parser->leave(this);
		}
	}

	parser_base& parser() const
	{
		vsm_assert(m_parser != nullptr);
		return *m_parser;
	}

	void* allocate(size_t const size)
	{
		vsm_assert(m_parser != nullptr);
		m_parser->allocate(this, size);
	}
};

class transfer_to
{
	std::coroutine_handle<> m_continuation;

public:
	transfer_to(std::coroutine_handle<> const continuation)
		: m_continuation(continuation)
	{
	}

	bool await_ready() const
	{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const
	{
		return m_continuation;
	}

	void await_resume() const
	{
	}
};

template<typename T>
class recursive_promise;

template<typename T>
class recursive_awaiter;

template<typename T>
class recursive
{
	recursive_promise<T>& m_promise;

public:
	using promise_type = vsm::detail::recursive_promise<T>;

	explicit recursive(recursive_promise<T>& promise)
		: m_promise(promise)
	{
	}

	recursive(recursive const&) = delete;
	recursive& operator=(recursive const&) = delete;

	~recursive()
	{
		recursive_promise<T>::get_handle(m_promise).destroy();
	}

private:
	friend class recursive_promise<T>;
	friend class recursive_awaiter<T>;
};

template<typename T>
class recursive_awaiter
{
	recursive_promise<T>& m_promise;

public:
	explicit recursive_awaiter(recursive<T> const& recursive)
		: m_promise(recursive.m_promise)
	{
	}

	bool await_ready() const
	{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> const continuation)
	{
		m_promise.set_continuation(continuation);
		return recursive_promise<T>::get_handle(m_promise);
	}

	T await_resume()
	{
		return vsm_move(*m_promise.m_result);
	}
};

class recursive_promise_base
{
	struct allocation : parser_frame
	{
		using parser_frame::parser_frame;

		std::byte buffer alignas(std::max_align_t)[];
	};

	static constexpr size_t buffer_offset = offsetof(allocation, buffer);

public:
	static void* operator new(size_t const size, parser_base& parser, auto&&...)
	{
		parser_frame frame(parser);
		size_t const storage_size = buffer_offset + size;
		return (new (frame.allocate(storage_size)) allocation(vsm_move(frame)))->buffer;
	}

	static void operator delete(void* const buffer)
	{
		std::destroy_at(reinterpret_cast<allocation*>(static_cast<std::byte*>(buffer) - buffer_offset));
	}
};

template<typename T>
class recursive_promise : public recursive_promise_base
{
	using handle_type = std::coroutine_handle<recursive_promise>;

	std::optional<T> m_result;
	std::coroutine_handle<> m_continuation = {};

public:
	recursive<T> get_return_object()
	{
		return recursive<T>(m_result);
	}

	std::suspend_always initial_suspend() noexcept
	{
		return {};
	}

	transfer_to final_suspend() noexcept
	{
		return { m_continuation ? m_continuation : std::noop_coroutine() };
	}

	void return_value(std::convertible_to<T> auto&& value)
	{
		m_result = vsm_forward(value);
	}

	void unhandled_exception()
	{
	}

	template<typename U>
	recursive_awaiter<U> await_transform(recursive<U>& recursive) const
	{
		return recursive_awaiter<U>(recursive);
	}
	
	void set_continuation(std::coroutine_handle<> const continuation)
	{
		vsm_assert(!m_continuation);
		m_continuation = continuation;
	}

	static handle_type get_handle(recursive_promise& promise)
	{
		return handle_type::from_promise(promise);
	}
};

template<typename Syntax>
class node_builder : parser_frame
{
	Syntax* m_syntax;

public:
	explicit node_builder(parser_base& parser, auto&&... args)
		: parser_frame(parser)
	{
		m_syntax = new (allocate_syntax(sizeof(Syntax))) Syntax(vsm_forward(args)...);
	}

	Syntax* leave()
	{
		vsm_assert(m_syntax != nullptr);
		return std::exchange(m_syntax, nullptr);
	}

	Syntax& operator*() const
	{
		return *m_syntax;
	}

	Syntax* operator->() const
	{
		return m_syntax;
	}
};

template<typename Syntax>
class list_builder : parser_frame
{
	struct block
	{
		block* next = nullptr;
		size_t size = 0;
		size_t const capacity;
		Syntax* data[];

		explicit block(size_t const capacity)
			: capacity(capacity)
		{
			std::uninitialized_default_construct_n(data, capacity);
		}

		block(block const&) = delete;
		block& operator=(block const&) = delete;

		~block()
		{
			std::destroy_n(data, capacity);
		}
	};

	block* m_first = nullptr;
	block* m_last = nullptr;
	size_t m_size = 0;

public:
	explicit list_builder(parser_base& parser)
		: parser_frame(parser)
	{
	}

	void push(Syntax* const syntax)
	{
	}

	list<Syntax> leave()
	{
		using object_type = source_element_ptr<Syntax>;

		void* const storage = parser().allocate_syntax(m_size * sizeof(object_type));
		object_type* const data = static_cast<object_type*>(storage);

		size_t const size = std::exchange(m_size, 0);
		block* first = std::exchange(m_first, nullptr);
		block* const last = std::exchange(m_last, nullptr);

		// Move elements from blocks to the final array.
		{
			object_type* pos = data;
			do
			{
				pos = std::uninitialized_move_n(first->data, first->size, pos);
			}
			while ((first = first->next) != last);
			vsm_assert(static_cast<size_t>(pos - data) == size);
		}
		
		return list<Syntax>(data, size);
	}
};

} // namespace detail

using detail::recursive;
using detail::node_builder;
using detail::list_builder;

class parser : detail::parser_base
{
protected:
	token const* peek_token()
	{
	}

	token const* consume_token()
	{
	}

	token const* consume_token(token_kind const kind)
	{
	}

	token const* try_consume_token(token_kind const kind)
	{
	}

	template<typename T>
	T recurse(recursive<T>&& recursive)
	{
		vsm_assert(!recursive.m_promise.m_result);
		recursive_promise<T>::get_handle(recursive.m_promise).resume();
		return vsm_move(recursive.m_promise.m_result);
	}

	template<typename Syntax>
	node_builder<Syntax> node(auto&&... args)
	{
		return node_builder<Syntax>(*this, vsm_forward(args)...);
	}

	template<typename Syntax>
	list_builder<Syntax> list()
	{
		return list_builder<Syntax>(*this);
	}
};

} // namespace vsm
