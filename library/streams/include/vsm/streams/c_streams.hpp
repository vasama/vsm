#pragma once

#include <vsm/streams/streams.hpp>

namespace vsm::streams {
namespace detail {

vsm::result<size_t> c_write_some(FILE* file, std::span<std::byte const> data);
vsm::result<void> c_flush(FILE* file);

vsm::result<size_t> c_read_some(FILE* file, std::span<std::byte> data);

} // namespace detail

template<typename FilePointer>
class basic_c_sink
{
	FilePointer m_file;

public:
	template<std::convertible_to<FilePointer> P = FilePointer>
	basic_c_sink(P&& file)
		: m_file(vsm_forward(file))
	{
	}

	[[nodiscard]] vsm::result<size_t> write_some(
		std::span<std::byte const> const data) const noexcept
	{
		return detail::c_write_some(std::to_address(m_file), data);
	}

	[[nodiscard]] vsm::result<void> flush() const noexcept
	{
		return detail::c_flush(std::to_address(m_file));
	}
};

template<typename FilePointer>
class basic_c_source
{
	FilePointer m_file;

public:
	template<std::convertible_to<FilePointer> P = FilePointer>
	basic_c_source(P&& file)
		: m_file(vsm_forward(file))
	{
	}

	[[nodiscard]] vsm::result<size_t> read_some(std::span<std::byte> const data) const noexcept
	{
		return detail::c_read_some(std::to_address(m_file), data);
	}
};

extern basic_c_source<FILE*> cin;
extern basic_c_sink<FILE*> cout;
extern basic_c_sink<FILE*> cerr;

} // namespace vsm::streams
