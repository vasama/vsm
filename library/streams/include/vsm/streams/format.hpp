#pragma once

#include <vsm/exceptions.hpp>
#include <vsm/streams/buffered_stream.hpp>
#include <vsm/streams/print.hpp>
#include <vsm/utility.hpp>

#include <format>

namespace vsm::streams {
namespace detail {

template<direct_sink Sink>
class format_output
{
	Sink& m_sink;

	std::byte* m_buffer_beg;
	std::byte* m_buffer_pos;
	std::byte* m_buffer_end;

	size_t m_total_size = 0;

	std::byte m_discard_buffer[1];
	bool m_buffer_increment = 1;

	std::error_code m_error = {};

	class iterator
	{
		format_output* m_output;

		class proxy
		{
			format_output& m_output;
	
		public:
			explicit proxy(format_output& output) noexcept
				: m_output(output)
			{
			}
	
			proxy operator=(char const character) const noexcept
			{
				m_output.push(character);
				return *this;
			}
		};

	public:
		using difference_type = ptrdiff_t;

		explicit iterator(format_output& output)
			: m_output(&output)
		{
		}

		[[nodiscard]] proxy operator*() const noexcept
		{
			return proxy(*m_output);
		}
	
		iterator& operator++() & noexcept
		{
			return *this;
		}
	
		[[nodiscard]] iterator operator++(int) & noexcept
		{
			return *this;
		}
	
		[[nodiscard]] friend bool operator==(iterator const&, iterator const&) noexcept
		{
			return false;
		}
	};

public:
	explicit format_output(Sink& sink)
		: m_sink(sink)
	{
		set_buffer(m_sink.direct_write_acquire(static_cast<size_t>(1)));
	}

	[[nodiscard]] iterator begin() &
	{
		return iterator(*this);
	}

	void push(char const character) &
	{
		if (m_buffer_pos == m_buffer_end)
		{
			set_buffer(streams::direct_write_refresh(
				m_sink,
				static_cast<size_t>(m_buffer_end - m_buffer_beg),
				/* min_new_size: */ 1));
		}

		*m_buffer_pos = static_cast<std::byte>(character);
		m_buffer_pos += m_buffer_increment;
	}

	[[nodiscard]] vsm::result<size_t> finalize() const
	{
		if (m_error)
		{
			return vsm::unexpected(m_error);
		}

		return {};
	}

private:
	void set_buffer(vsm::result<std::span<std::byte>> const r)
	{
		if (r)
		{
			vsm_assert(r->size() != 0);

			m_buffer_beg = r->data();
			m_buffer_pos = r->data();
			m_buffer_end = r->data() + r->size();
		}
		else
		{
			m_error = r.error();

			m_buffer_beg = m_discard_buffer;
			m_buffer_pos = m_discard_buffer;
			m_buffer_end = m_discard_buffer + 1;

			m_buffer_increment = 0;
		}
	}
};

} // namespace detail

template<sink Sink, typename... Args>
[[nodiscard]] vsm::result<size_t> format(
	Sink&& sink,
	std::format_string<Args...> const format,
	Args&&... args)
{
	static_assert(streams::sink<Sink&>);

#if 0 //TODO: Figure out why this causes infinite recursion...
	if constexpr (non_cv<remove_ref_t<Sink>> && streams::sink<Sink const&>)
	{
		return streams::format(
			static_cast<Sink const&>(sink),
			format,
			vsm_forward(args)...);
	}
	else
#endif
	{
		if constexpr (direct_sink<Sink&>)
		{
			detail::format_output<Sink&> output(sink);

			std::format_to(
				output.begin(),
				format,
				vsm_forward(args)...);

			return output.finalize();
		}
		else
		{
			using buffer_type = std::array<std::byte, 1024>;
			basic_buffered_sink<Sink&, buffer_type> buffered_sink(sink);

			return streams::format(
				buffered_sink,
				format,
				vsm_forward(args)...);
		}
	}
}

} // namespace vsm::streams
