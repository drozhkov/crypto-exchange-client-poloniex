#include <iostream>
#include <exception>

#include "crypto-exchange-client-poloniex/client.hpp"

#include "api-secret.hpp"


int main()
{
	try {
		as::cryptox::poloniex::Client client( as::cryptox::poloniex::ApiKey(),
			as::cryptox::poloniex::ApiSecret() );

		as::cryptox::Direction direction = as::cryptox::Direction::BUY;

		client.run( [&direction]( as::cryptox::Client & c ) {
			std::cout << "ready" << std::endl;

			c.subscribeOrderUpdate(
				[]( as::cryptox::Client & c, as::cryptox::t_order_update & u ) {
					std::cout << "order update: " << u.orderId << std::endl;
				} );

			c.subscribePriceBookTicker( as::cryptox::Symbol::A_ALL,
				[&direction]( as::cryptox::Client & c,
					as::cryptox::t_price_book_ticker & t ) {
					const auto & pair = c.Pair( t.symbol );

					if ( pair.Base() == as::cryptox::Coin::TRX &&
						pair.Quote() == as::cryptox::Coin::USDT ) {

						std::cout
							<< "price book ticker: " << t.askPrice.toString()
							<< '/' << t.bidPrice.toString() << std::endl;

						if ( as::cryptox::Direction::BUY == direction ) {
							direction = as::cryptox::Direction::SELL;
							auto qty = as::FixedNumber( "1000.0" );

							c.placeOrder( as::cryptox::Direction::BUY,
								t.symbol,
								t.bidPrice,
								qty );
						}
					}
				} );
		} );
	}
	catch ( const std::exception & x ) {
		std::cerr << x.what() << std::endl;
	}
	catch ( ... ) {
		std::cerr << "error" << std::endl;
	}

	return 0;
}
