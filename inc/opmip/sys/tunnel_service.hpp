//=============================================================================
// Brief   : Tunnel Service for Linux
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010 Universidade de Aveiro
// Copyrigth (C) 2010 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_SYS_TUNNEL_SERVICE__HPP_
#define OPMIP_SYS_TUNNEL_SERVICE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/throw_exception.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <cstring>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

namespace ip = boost::asio::ip;

///////////////////////////////////////////////////////////////////////////////
namespace detail {

///////////////////////////////////////////////////////////////////////////////
inline void throw_on_error(boost::system::error_code& ec, const char* what)
{
	if (ec)
		boost::throw_exception(boost::system::system_error(ec, what));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace detail */

///////////////////////////////////////////////////////////////////////////////
class tunnel_service {
	static const std::size_t if_name_size = 16;
	static const int         ioctl_begin  = 0x89F0;
	static const int         ioctl_get    = ioctl_begin + 0;
	static const int         ioctl_add    = ioctl_begin + 1;
	static const int         ioctl_remove = ioctl_begin + 2;
	static const int         ioctl_change = ioctl_begin + 3;

public:
	class ip6_parameters;

	tunnel_service();
	tunnel_service(boost::system::error_code& ec);
	~tunnel_service();

	template<class Parameters>
	void get(Parameters& op, boost::system::error_code& ec)
	{
		io_control(op.name(), ioctl_get, op.data(), ec);
	}

	template<class Parameters>
	void get(Parameters& op)
	{
		boost::system::error_code ec;

		get(op, ec);
		detail::throw_on_error(ec, "opmip::sys::tunnel_service::get()");
	}

	template<class Parameters>
	void add(Parameters& op, boost::system::error_code& ec)
	{
		Parameters tmp = op;

		io_control("ip6tnl0", ioctl_add, op.data(), ec);
		if (!ec && (op != tmp))
			ec = boost::system::error_code(boost::system::errc::invalid_argument,
			                               boost::system::get_system_category());
	}

	template<class Parameters>
	void add(Parameters& op)
	{
		boost::system::error_code ec;

		add(op, ec);
		detail::throw_on_error(ec, "opmip::sys::tunnel_service::add()");
	}

	template<class Parameters>
	void remove(Parameters& op, boost::system::error_code& ec)
	{
		io_control(op.name(), ioctl_remove, op.data(), ec);
	}

	template<class Parameters>
	void remove(Parameters& op)
	{
		boost::system::error_code ec;

		remove(op, ec);
		detail::throw_on_error(ec, "opmip::sys::tunnel_service::remove()");
	}

	template<class Parameters>
	void change(Parameters& op, boost::system::error_code& ec)
	{
		io_control(op.name(), ioctl_change, op.data(), ec);
	}

	template<class Parameters>
	void change(Parameters& op)
	{
		boost::system::error_code ec;

		change(op, ec);
		detail::throw_on_error(ec, "opmip::sys::tunnel_service::change()");
	}

private:
	void init(boost::system::error_code& ec);
	void io_control(const char* name, int opcode, void* data, boost::system::error_code& ec);

	int _fd;
};

class tunnel_service::ip6_parameters {
	typedef ip::address_v6::bytes_type ip6_address;

public:
	static const uint default_encapsulation_limit = 4;

	enum flags {
		ignore_encapsulation_limit = 0x01, ///Don't add encapsulation limit if one isn't present in inner packet
		use_original_traffic_class = 0x02, ///Copy the traffic class field from the inner packet
		use_original_flowlabel     = 0x04, ///Copy the flowlabel from the inner packet
		use_original_dscp          = 0x10, ///Copy DSCP from the outer packet
	};

	friend bool operator!=(const ip6_parameters& rhr, const ip6_parameters& lhr);
	friend std::ostream& operator<<(std::ostream& os, const ip6_parameters& lhr);

public:
	ip6_parameters()
	{
		std::memset(this, 0, sizeof(*this));
		_proto = IPPROTO_IPV6;
		_encap_limit = default_encapsulation_limit;
		_hop_limit = 64;
	}

	void name(const char* str)                      { std::strncpy(_name, str, if_name_size); }
	void device(int index)                          { _link = index; }
	void hop_limit(uint8 val)                       { _hop_limit = val; }
	void local_address(const ip::address_v6& addr)  { _local_addr = addr.to_bytes(); }
	void remote_address(const ip::address_v6& addr) { _remote_addr = addr.to_bytes(); }

	const char*    name() const           { return _name; }
	int            device() const         { return _link; }
	uint8          hop_limit() const      { return _hop_limit; }
	ip::address_v6 local_address() const  { return ip::address_v6(_local_addr); }
	ip::address_v6 remote_address() const { return ip::address_v6(_remote_addr); }

	void* data() { return this; }

private:
	char        _name[if_name_size];
	int         _link;
	uint8       _proto;
	uint8       _encap_limit;
	uint8       _hop_limit;
	uint32      _flowinfo;
	uint32      _flags;
	ip6_address _local_addr;
	ip6_address _remote_addr;
};

bool          operator!=(const tunnel_service::ip6_parameters& rhr, const tunnel_service::ip6_parameters& lhr);
std::ostream& operator<<(std::ostream& os, const tunnel_service::ip6_parameters& lhr);

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_TUNNEL_SERVICE__HPP_ */