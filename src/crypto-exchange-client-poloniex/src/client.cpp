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

/// client.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "boost/json.hpp"

#include "crypto-exchange-client-core/logger.hpp"
#include "crypto-exchange-client-core/exception.hpp"

#include "crypto-exchange-client-poloniex/client.hpp"
#include "crypto-exchange-client-poloniex/wsMessage.hpp"


namespace as::cryptox::poloniex {

	void Client::addAuthHeaders(
		HttpHeaderList & headers, ::as::t_string & body )
	{

		auto nonce = UnixTs<std::chrono::milliseconds>();

		body += ( body.empty() ? AS_T( "" ) : AS_T( "&" ) ) +
			as::t_string( AS_T( "nonce=" ) ) + AS_TOSTRING( nonce );

		auto sign = hmacSha512( m_apiSecret, body );
		auto signHex = toHex( as::t_buffer( sign.data(), sign.size() ) );

		headers.add( AS_T( "Key" ), m_apiKey );
		headers.add( AS_T( "Sign" ), signHex );

		headers.add( AS_T( "Content-Type" ),
			AS_T( "application/x-www-form-urlencoded " ) );
	}

	void Client::wsErrorHandler(
		WsClient & client, int code, const as::t_string & message )
	{
	}

	void Client::wsHandshakeHandler( WsClient & client )
	{
		AS_CALL( m_clientReadyHandler, *this );
		client.readAsync();
	}

	bool Client::wsReadHandler(
		WsClient & client, const char * data, size_t size )
	{

		try {
			// std::string s( data, size );
			// std::cout << s << std::endl;

			auto message = WsMessage::deserialize( data, size );

			switch ( message->TypeId() ) {
				case WsMessage::TypeIdPriceBookTicker: {
					auto m = static_cast<WsMessagePriceBookTicker *>(
						message.get() );

					as::cryptox::t_price_book_ticker t;
					t.symbol = m->Symbol();
					t.askPrice = std::move( m->AskPrice() );
					t.askQuantity = std::move( m->AskSize() );
					t.bidPrice = std::move( m->BidPrice() );
					t.bidQuantity = std::move( m->BidSize() );

					callSymbolHandler(
						t.symbol, m_priceBookTickerHandlerMap, t );
				}

				break;

				case WsMessage::TypeIdAccountNotifications: {
					auto m = static_cast<WsMessageAccountNotifications *>(
						message.get() );

					// as::cryptox::t_order_update u;
					// u.orderId = std::move( m->OrderId() );

					// AS_CALL( m_orderUpdateHandler, *this, u );
				}

				break;
			}
		}
		catch ( ... ) {
		}

		return true;
	}

	void Client::initSymbolMap()
	{
		as::cryptox::Client::initSymbolMap();

		auto apiRes = apiReqReturnTicker();
		m_pairList.resize( apiRes.MaxId() + 2 );

		m_pairList[0] = as::cryptox::Pair( as::cryptox::Coin::_undef,
			as::cryptox::Coin::_undef,
			AS_T( "undefined" ) );

		for ( const auto & p : apiRes.Pairs() ) {
			auto pos = p.name.find( AS_T( '_' ) );
			as::cryptox::Coin quote = toCoin( p.name.substr( 0, pos ).c_str() );
			as::cryptox::Coin base = toCoin( p.name.substr( pos + 1 ).c_str() );

			as::cryptox::Pair pair( base, quote, p.name );
			m_pairList[p.id] = pair;

			addSymbolMapEntry(
				p.name, static_cast<as::cryptox::Symbol>( p.id ) );
		}
	}

	void Client::initWsClient()
	{
		as::cryptox::Client::initWsClient();
	}

	ApiResponseReturnTicker Client::apiReqReturnTicker()
	{
		auto url = m_httpApiUrlPublic.add( ApiRequest::ReturnTicker() );
		auto res = m_httpClient.get( url, HttpHeaderList() );

		return ApiResponseReturnTicker::deserialize( res );
	}

	void Client::subscribe( const as::t_string & channelName, bool isPrivate )
	{
		auto buffer = WsMessage::Subscribe(
			channelName, m_apiKey, m_apiSecret, isPrivate );

		m_wsClient->write( buffer.c_str(), buffer.length() );
	}

	void Client::run( const t_exchangeClientReadyHandler & handler )
	{
		as::cryptox::Client::run( handler );

		while ( true ) {
			initWsClient();
			m_wsClient->run();
		}
	}

	void Client::subscribePriceBookTicker(
		as::cryptox::Symbol symbol, const t_priceBookTickerHandler & handler )
	{

		as::cryptox::Client::subscribePriceBookTicker( symbol, handler );
		subscribe( AS_TOSTRING( WsMessage::ChannelIdTickerData ) );
	}

	void Client::subscribeOrderUpdate( const t_orderUpdateHandler & handler )
	{
		as::cryptox::Client::subscribeOrderUpdate( handler );
		subscribe(
			AS_TOSTRING( WsMessage::ChannelIdAccountNotifications ), true );
	}

	t_order Client::placeOrder( Direction direction,
		as::cryptox::Symbol symbol,
		const FixedNumber & price,
		const FixedNumber & quantity )
	{

		as::t_string body;

		if ( Direction::BUY == direction ) {
			body.assign(
				ApiRequest::Buy( toName( symbol ), price, quantity ) );
		}
		else if ( Direction::SELL == direction ) {
			body.assign(
				ApiRequest::Sell( toName( symbol ), price, quantity ) );
		}

		HttpHeaderList headers;
		addAuthHeaders( headers, body );

		auto res = m_httpClient.post( m_httpApiUrl, headers, body );

		auto resOrders = ApiResponseOrders::deserialize( res );

		t_order result;
		result.id = std::move( resOrders.OrderId() );

		return result;
	}

}
