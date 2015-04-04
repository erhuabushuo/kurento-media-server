/*
 * Copyright (c) 2013, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef HTTP_PARSER_IMPL_HPP
#define HTTP_PARSER_IMPL_HPP

#include <algorithm>
#include <sstream>
#include <string>

namespace websocketpp
{
namespace http
{
namespace parser
{

inline void parser::set_version (std::string const &version)
{
  m_version = version;
}

inline std::string const &parser::get_header (std::string const &key) const
{
  header_list::const_iterator h = m_headers.find (key);

  if (h == m_headers.end() ) {
    return empty_header;
  } else {
    return h->second;
  }
}

inline bool parser::get_header_as_plist (std::string const &key,
    parameter_list &out) const
{
  header_list::const_iterator it = m_headers.find (key);

  if (it == m_headers.end() || it->second.size() == 0) {
    return false;
  }

  return this->parse_parameter_list (it->second, out);
}

inline void parser::append_header (std::string const &key, std::string const &
                                   val)
{
  if (std::find_if (key.begin(), key.end(), is_not_token_char) != key.end() ) {
    throw exception ("Invalid header name", status_code::bad_request);
  }

  if (this->get_header (key) == "") {
    m_headers[key] = val;
  } else {
    m_headers[key] += ", " + val;
  }
}

inline void parser::replace_header (std::string const &key, std::string const &
                                    val)
{
  m_headers[key] = val;
}

inline void parser::remove_header (std::string const &key)
{
  m_headers.erase (key);
}

inline void parser::set_body (std::string const &value)
{
  if (value.size() == 0) {
    remove_header ("Content-Length");
    m_body = "";
    return;
  }

  std::stringstream len;
  len << value.size();
  replace_header ("Content-Length", len.str() );
  m_body = value;
}

inline bool parser::parse_parameter_list (std::string const &in,
    parameter_list &out) const
{
  if (in.size() == 0) {
    return false;
  }

  std::string::const_iterator it;
  it = extract_parameters (in.begin(), in.end(), out);
  return (it == in.begin() );
}

inline bool parser::parse_headers (std::istream &s)
{
  std::string header;
  std::string::size_type end;

  // get headers
  while (std::getline (s, header) && header != "\r") {
    if (header[header.size() - 1] != '\r') {
      continue; // ignore malformed header lines?
    } else {
      header.erase (header.end() - 1);
    }

    end = header.find (header_separator, 0);

    if (end != std::string::npos) {
      append_header (header.substr (0, end), header.substr (end + 2) );
    }
  }

  return true;
}

inline void parser::process_header (std::string::iterator begin,
                                    std::string::iterator end)
{
  std::string::iterator cursor = std::search (
                                   begin,
                                   end,
                                   header_separator,
                                   header_separator + sizeof (header_separator) - 1
                                 );

  if (cursor == end) {
    throw exception ("Invalid header line", status_code::bad_request);
  }

  append_header (strip_lws (std::string (begin, cursor) ),
                 strip_lws (std::string (cursor + sizeof (header_separator) - 1, end) ) );
}

inline std::string parser::raw_headers() const
{
  std::stringstream raw;

  header_list::const_iterator it;

  for (it = m_headers.begin(); it != m_headers.end(); it++) {
    raw << it->first << ": " << it->second << "\r\n";
  }

  return raw.str();
}



} // namespace parser
} // namespace http
} // namespace websocketpp

#endif // HTTP_PARSER_IMPL_HPP
