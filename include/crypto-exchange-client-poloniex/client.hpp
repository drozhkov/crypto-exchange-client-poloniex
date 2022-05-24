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

/// client.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_POLONIEX__CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_POLONIEX__CLIENT__H


#include "crypto-exchange-client-core/httpClient.hpp"
#include "crypto-exchange-client-core/client.hpp"

#include "crypto-exchange-client-poloniex/apiMessage.hpp"


namespace as::cryptox::poloniex {

	class Client : public as::cryptox::Client {
	protected:
		as::Url m_httpApiUrlPublic;

		as::t_string m_apiKey;
		as::t_string m_apiSecret;

	private:
		void addAuthHeaders( HttpHeaderList & headers, as::t_string & body );

	protected:
		void wsErrorHandler(
			as::WsClient &, int, const as::t_string & ) override;

		void wsHandshakeHandler( as::WsClient & ) override;
		bool wsReadHandler( as::WsClient &, const char *, size_t ) override;

		void initSymbolMap() override;
		void initWsClient() override;

		void subscribe(
			const as::t_string & channelName, bool isPrivate = false );

	public:
		Client( const as::t_string & apiKey = AS_T( "" ),
			const as::t_string & apiSecret = AS_T( "" ),
			const as::t_string & httpApiUrl = AS_T(
				"https://poloniex.com/tradingApi" ),
			const as::t_string & httpApiUrlPublic = AS_T(
				"https://poloniex.com/public" ),
			const as::t_string & wsApiUrl = AS_T( "wss://api2.poloniex.com" ) )
			: as::cryptox::Client( httpApiUrl, wsApiUrl )
			, m_httpApiUrlPublic( httpApiUrlPublic )
			, m_apiKey( apiKey )
			, m_apiSecret( apiSecret )
		{
		}

		ApiResponseReturnTicker apiReqReturnTicker();

		void run( const t_exchangeClientReadyHandler & handler ) override;

		void subscribePriceBookTicker( as::cryptox::Symbol symbol,
			const t_priceBookTickerHandler & handler ) override;

		void subscribeOrderUpdate(
			const t_orderUpdateHandler & handler ) override;

		t_order placeOrder( Direction direction,
			as::cryptox::Symbol symbol,
			const FixedNumber & price,
			const FixedNumber & quantity ) override;
	};

}


#endif
