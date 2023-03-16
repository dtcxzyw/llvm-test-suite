// This file is part of Asteria.
// Copyleft 2018 - 2022, LH_Mouse. All wrongs reserved.

#include "../precompiled.ipp"
#include "runtime_error.hpp"
#include "enums.hpp"
namespace asteria {

static_assert(
    ::std::is_nothrow_copy_constructible<Runtime_Error>::value &&
    ::std::is_nothrow_move_constructible<Runtime_Error>::value &&
    ::std::is_nothrow_copy_assignable<Runtime_Error>::value &&
    ::std::is_nothrow_move_assignable<Runtime_Error>::value);

Runtime_Error::
~Runtime_Error()
  {
  }

void
Runtime_Error::
do_backtrace(Backtrace_Frame&& new_frm)
  {
    // Unpack nested exceptions, if any.
    try {
      auto eptr = ::std::current_exception();
      if(eptr)
        ::std::rethrow_exception(eptr);
    }
    catch(Runtime_Error& nested) {
      this->m_frames.append(nested.m_frames.move_begin(), nested.m_frames.move_end());
    }
    catch(...) { }

    // Push a new frame.
    this->do_insert_frame(::std::move(new_frm));
  }

void
Runtime_Error::
do_insert_frame(Backtrace_Frame&& new_frm)
  {
    // Insert the frame. Note exception safety.
    this->m_frames.insert(this->m_ins_at, ::std::move(new_frm));
    this->m_ins_at++;

    // Rebuild the message using new frames. The storage may be reused.
    // Strings are written verbatim. All the others are formatted.
    this->m_fmt.clear_string();
    this->m_fmt << "runtime error: ";

    if(this->m_value.is_string())
      this->m_fmt << this->m_value.as_string();
    else
      this->m_fmt << this->m_value;

    // Get the width of the frame number colomn.
    ::rocket::ascii_numput nump;
    nump.put_DU(this->m_frames.size());

    static_vector<char, 24> sbuf(nump.size(), ' ');
    sbuf.emplace_back();

    // Append stack frames.
    this->m_fmt << "\n[backtrace frames:\n";
    for(size_t k = 0;  k < this->m_frames.size();  ++k) {
      const auto& frm = this->m_frames[k];
      nump.put_DU(k + 1);
      ::std::copy_backward(nump.begin(), nump.end(), sbuf.mut_end() - 1);
      format(this->m_fmt, "  $1) $2 at '$3': ", sbuf.data(), frm.what_type(), frm.sloc());

      this->m_frame_fmt.clear_string();
      frm.value().print(this->m_frame_fmt);

      if(this->m_frame_fmt.length() > 120) {
        size_t nchars_omitted = this->m_frame_fmt.length() - 100;
        this->m_fmt.putn(this->m_frame_fmt.c_str(), this->m_frame_fmt.length() - nchars_omitted);
        nump.put_DU(nchars_omitted);
        format(this->m_fmt, " ... ($1 characters omitted)\n", nchars_omitted);
      }
      else
        this->m_fmt << this->m_frame_fmt.get_string() << '\n';
    }
    this->m_fmt << "  -- end of backtrace frames]";
  }

}  // namespace asteria
