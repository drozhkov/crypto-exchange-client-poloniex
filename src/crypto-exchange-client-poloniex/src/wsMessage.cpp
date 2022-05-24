/*
MIT License
Copyright (c) 2022 Denis Rozhkov <denis@rozhkoff.com>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/// wsMessage.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "crypto-exchange-client-poloniex/wsMessage.hpp"


namespace as::cryptox::poloniex {

	std::shared_ptr<::as::cryptox::ApiMessageBase> WsMessage::deserialize(
		const char * data, size_t size )
	{

		auto v = boost::json::parse( { data, size } );

		if ( !v.is_array() ) {
			return s_unknown;
		}

		auto & o = v.get_array();

		WsMessage * r = nullptr;

		auto channelId = o[0].get_int64();

		if ( ChannelIdTickerData == channelId ) {
			if ( o[1].is_null() ) {
				r = new WsMessagePriceBookTicker;
			}
		}
		else if ( ChannelIdAccountNotifications == channelId ) {
			if ( o[1].is_null() || o[1].is_string() ) {
				r = new WsMessageAccountNotifications;
			}
		}

		if ( nullptr == r ) {
			return s_unknown;
		}

		r->deserialize( o[2].get_array() );

		return std::shared_ptr<::as::cryptox::WsMessage>( r );
	}

	//

	void WsMessagePriceBookTicker::deserialize( boost::json::array & o )
	{
		m_symbol = static_cast<as::cryptox::Symbol>( o[0].get_int64() );
		m_askPrice.Value( o[2].get_string() );
		m_bidPrice.Value( o[3].get_string() );
	}

	//

	void WsMessageAccountNotifications::deserialize( boost::json::array & o )
	{
		for ( const auto & item : o ) {
			const auto & typeName = item.get_array()[0].get_string();

			if ( "p" == typeName || "n" == typeName || "o" == typeName ) {
			}
			else {
			}
		}
	}

}
