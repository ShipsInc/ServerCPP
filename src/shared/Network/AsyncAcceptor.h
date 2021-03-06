/*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ASYNCACCEPT_H_
#define __ASYNCACCEPT_H_

#include "Define.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class AsyncAcceptor
{
public:
    typedef void(*ManagerAcceptHandler)(tcp::socket&& newSocket);

    AsyncAcceptor(boost::asio::io_service& ioService, std::string const& bindIp, uint16 port) :
        _acceptor(ioService, tcp::endpoint(boost::asio::ip::address::from_string(bindIp), port)),
        _socket(ioService), _closed(false)
    {
    }

    void AsyncAcceptManaged(ManagerAcceptHandler mgrHandler)
    {
        _acceptor.async_accept(_socket, [this, mgrHandler](boost::system::error_code error)
        {
            if (!error)
            {
                try
                {
                    _socket.non_blocking(true);
                    mgrHandler(std::move(_socket));
                }
                catch (boost::system::system_error const& err)
                {
                    std::cout << "Failed to initialize client's socket " << err.what() << std::endl;
                }
            }

            if (!_closed)
                AsyncAcceptManaged(mgrHandler);
        });
    }

    void Close()
    {
        if (_closed.exchange(true))
            return;

        if (!_acceptor.is_open())
            return;

        boost::system::error_code err;
        _acceptor.close(err);
    }

private:
    tcp::acceptor _acceptor;
    tcp::socket _socket;
    std::atomic<bool> _closed;
};

#endif /* __ASYNCACCEPT_H_ */
