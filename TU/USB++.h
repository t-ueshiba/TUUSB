/*
 *  $Id: USB++.h,v 1.1.1.1 2012-09-15 08:03:09 ueshiba Exp $
 */
#ifndef __TUUSBPP_H
#define __USUSBPP_H
#include <sys/types.h>
#include <usb.h>
#include <iostream>

namespace TU
{
/************************************************************************
*  class USBHub								*
************************************************************************/
class USBHub
{
  private:
    struct Initializer
    {
	Initializer()	{usb_init(); usb_find_busses(); usb_find_devices();}
    };
    
  public:
    USBHub(uint16_t idVendor, uint16_t idProduct)			;
    ~USBHub()								;

    uint16_t	idVendor()					const	;
    uint16_t	idProduct()					const	;
    u_int	nports()					const	;
    USBHub&	setPower(u_int port, bool on)				;
    USBHub&	setLED(u_int port, u_int value)				;
    bool	isPowerOn(u_int port)				const	;
    u_int	getLED(u_int port)				const	;
    
    friend std::ostream&
		operator <<(std::ostream& out, const USBHub& hub)	;

    static void	listup(std::ostream& out)				;
    
  private:
    USBHub(usb_dev_handle* handle)					;
    
    void	initialize()						;
    USBHub&	setStatus(u_int request, u_int feature, u_int index)	;
    u_int32_t	getStatus(u_int port)				const	;
    
  private:
    usb_dev_handle* const	_handle;	//!< USBデバイスのハンドル
    u_int			_nports;	//!< USBハブのポート数

    static Initializer		_initializer;
};

inline
USBHub::~USBHub()
{
    if (_handle)
	usb_close(_handle);
}

inline uint16_t
USBHub::idVendor() const
{
    return usb_device(_handle)->descriptor.idVendor;
}
    
inline uint16_t
USBHub::idProduct() const
{
    return usb_device(_handle)->descriptor.idProduct;
}
    
inline u_int
USBHub::nports() const
{
    return _nports;
}

/************************************************************************
*  class USBPort							*
************************************************************************/
class USBPort
{
  public:
    USBPort(USBHub& hub, u_int port)	:_hub(hub), _port(port)		{}

    uint16_t	idVendor()					const	;
    uint16_t	idProduct()					const	;
    u_int	port()						const	;
    USBPort&	setPower(bool on)					;
    bool	isPowerOn()					const	;
    
  private:
    USBHub&	_hub;
    const u_int	_port;
};

inline uint16_t
USBPort::idVendor() const
{
    return _hub.idVendor();
}
    
inline uint16_t
USBPort::idProduct() const
{
    return _hub.idProduct();
}
    
inline u_int
USBPort::port() const
{
    return _port;
}
    
inline USBPort&
USBPort::setPower(bool on)
{
    _hub.setPower(_port, on);
    return *this;
}

inline bool
USBPort::isPowerOn() const
{
    return _hub.isPowerOn(_port);
}
    
}
#endif	// !__TUUSBPP_H
