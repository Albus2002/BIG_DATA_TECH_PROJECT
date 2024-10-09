/*
* author: Wenrui Liu
* last edit time: 2023-4-13
*/
#ifndef _MINITAP_PROTOCOLTYPE_H_
#define _MINITAP_PROTOCOLTYPE_H_

#include<stdint.h>

namespace ProtoType{
    //referenece: https://github.com/seladb/PcapPlusPlus/blob/master/Packet%2B%2B/header/ProtocolType.h
    /**
	 * @typedef ProtocolType
	 * Representing all protocols supported by PcapPlusPlus
	 */

	typedef uint64_t ProtocolType;
	/**
	 * Unknown protocol (or unsupported by PcapPlusPlus)
	 */
	const ProtocolType UnknownProtocol = 0x00;

	/**
	 * Ethernet protocol
	 */
	const ProtocolType Ethernet = 0x01;

	/**
	 * IPv4 protocol
	 */
	const ProtocolType IPv4 = 0x02;

	/**
	 * IPv6 protocol
	 */
	const ProtocolType IPv6 = 0x04;

	/**
	 * IP protocol (aggregation bitmask of IPv4 and IPv6 protocols)
	 */
	const ProtocolType IP = 0x06;

	/**
	 * TCP protocol
	 */
	const ProtocolType TCP = 0x08;
	const ProtocolType TCPData = 0x09;

	/**
	 * UDP protocol
	 */
	const ProtocolType UDP = 0x10;
	const ProtocolType UDPData = 0x11;

	/**
	 * HTTP request protocol
	 */
	const ProtocolType HTTPRequest = 0x20;

	/**
	 * HTTP response protocol
	 */
	const ProtocolType HTTPResponse = 0x40;

	/**
	 * HTTP protocol (aggregation bitmask of HTTP request and HTTP response protocols)
	 */
	const ProtocolType HTTP = 0x60;

	const ProtocolType HTTPS = 0x61;

	const ProtocolType IMAP = 0x65;

	const ProtocolType SMTP = 0x70;

	const ProtocolType POP3 = 0x75;

	/**
	 * ARP protocol
	 */
	const ProtocolType ARP = 0x80;

	/**
	 * VLAN protocol
	 */
	const ProtocolType VLAN = 0x100;

	/**
	 * ICMP protocol
	 */
	const ProtocolType ICMP = 0x200;

	/**
	 * PPPoE session protocol
	 */
	const ProtocolType PPPoESession = 0x400;

	/**
	 * PPPoE discovery protocol
	 */
	const ProtocolType PPPoEDiscovery = 0x800;

	/**
	 * PPPoE protocol (aggregation bitmask of PPPoESession and PPPoEDiscovery protocols)
	 */
	const ProtocolType PPPoE = 0xc00;

	/**
	 * DNS protocol
	 */
	const ProtocolType DNS = 0x1000;

	/**
	 * MPLS protocol
	 */
	const ProtocolType MPLS = 0x2000;

	/**
	 * GRE version 0 protocol
	 */
	const ProtocolType GREv0 = 0x4000;

	/**
	 * GRE version 1 protocol
	 */
	const ProtocolType GREv1 = 0x8000;

	/**
	 * GRE protocol (aggregation bitmask of GREv0 and GREv1 protocols)
	 */
	const ProtocolType GRE = 0xc000;

	/**
	 * PPP for PPTP protocol
	 */
	const ProtocolType PPP_PPTP = 0x10000;

	/**
	 * SSL/TLS protocol
	 */
	const ProtocolType SSL = 0x20000;

	/**
	 * SLL (Linux cooked capture) protocol
	 */
	const ProtocolType SLL = 0x40000;

	/**
	 * DHCP/BOOTP protocol
	 */
	const ProtocolType DHCP = 0x80000;

	/**
	 * Null/Loopback protocol
	 */
	const ProtocolType NULL_LOOPBACK = 0x100000;

	/**
	 * IGMP protocol
	 */
	const ProtocolType IGMP = 0xE00000;

	/**
	 * IGMPv1 protocol
	 */
	const ProtocolType IGMPv1 = 0x200000;

	/**
	 * IGMPv2 protocol
	 */
	const ProtocolType IGMPv2 = 0x400000;

	/**
	 * IGMPv3 protocol
	 */
	const ProtocolType IGMPv3 = 0x800000;

	/**
	 * Generic payload (no specific protocol)
	 */
	const ProtocolType GenericPayload = 0x1000000;

	/**
	 * VXLAN protocol
	 */
	const ProtocolType VXLAN = 0x2000000;

	/**
	 * SIP request protocol
	 */
	const ProtocolType SIPRequest = 0x4000000;

	/**
	 * SIP response protocol
	 */
	const ProtocolType SIPResponse = 0x8000000;

	/**
	 * SIP protocol (aggregation bitmask of SIPRequest and SIPResponse protocols)
	 */
	const ProtocolType SIP = 0xc000000;

	/**
	 * SDP protocol
	 */
	const ProtocolType SDP = 0x10000000;

	/**
	 * Packet trailer
	 */
	const ProtocolType PacketTrailer = 0x20000000;

	/**
	 * RADIUS protocol
	 */
	const ProtocolType Radius = 0x40000000;

	/**
	 * GTPv1 protocol
	 */
	const ProtocolType GTPv1 = 0x80000000;

	/**
	 * GTP protocol (currently the same as GTPv1)
	 */
	const ProtocolType GTP = 0x80000000;

	/**
	 * IEEE 802.3 Ethernet protocol
	 */
	const ProtocolType EthernetDot3 = 0x100000000;

	/**
	 * Border Gateway Protocol (BGP) version 4 protocol
	 */
	const ProtocolType BGP = 0x200000000;

	/**
	 * SSH version 2 protocol
	 */
	const ProtocolType SSH = 0x400000000;

	/**
	 * IPSec Authentication Header (AH) protocol
	 */
	const ProtocolType AuthenticationHeader = 0x800000000;

	/**
	 * IPSec Encapsulating Security Payload (ESP) protocol
	 */
	const ProtocolType ESP = 0x1000000000;

	/**
	 * IPSec protocol (aggregation bitmask of AH and ESP protocols)
	 */
	const ProtocolType IPSec = 0x1800000000;

	/**
	 * Dynamic Host Configuration Protocol version 6 (DHCPv6) protocol
	 */
	const ProtocolType DHCPv6 = 0x2000000000;

	/**
	 * Network Time (NTP) Protocol
	 */
	const ProtocolType NTP = 0x4000000000;

	/**
	 * Telnet Protocol
	 */
	const ProtocolType Telnet = 0x8000000000;

  	/**
   	 * File Transfer (FTP) Protocol
	 */
	const ProtocolType FTP = 0x10000000000;

	/**
	 * ICMPv6 protocol
	 */
	const ProtocolType ICMPv6 = 0x20000000000;

	/**
	 * Spanning Tree Protocol
	 */
	const ProtocolType STP = 0x40000000000;

	/**
	 * Logical Link Control (LLC)
	 */
	const ProtocolType LLC = 0x80000000000;

	/**
	 * SOME/IP Base protocol
	 */
	const ProtocolType SomeIP = 0x100000000000;

	/**
	 * Wake On LAN (WOL) Protocol
	 */
	const ProtocolType WakeOnLan = 0x200000000000;

	/**
	 * NFLOG (Linux Netfilter NFLOG) Protocol
	 */
	const ProtocolType NFLOG = 0x400000000000;

	/**
	 * An enum representing OSI model layers
	 */
	enum MY_OsiModelLayer
	{
		/** Physical layer (layer 1) */
		MY_OsiModelPhysicalLayer = 1,
		/** Data link layer (layer 2) */
		MY_OsiModelDataLinkLayer = 2,
		/** Network layer (layer 3) */
		MY_OsiModelNetworkLayer = 3,
		/** Transport layer (layer 4) */
		MY_OsiModelTransportLayer = 4,
		/** Session layer (layer 5) */
		MY_OsiModelSesionLayer = 5,
		/** Presentation layer (layer 6) */
		MY_OsiModelPresentationLayer = 6,
		/** Application layer (layer 7) */
		MY_OsiModelApplicationLayer = 7,
		/** Unknown / null layer */
		MY_OsiModelLayerUnknown = 8
	};

	enum IPProtocolTypes
	{
		/** Dummy protocol for TCP		*/
		MY_IPv4PROTOIP = 0,
		/** IPv6 Hop-by-Hop options		*/
		MY_IPv4PROTOHOPOPTS = 0,
		/** Internet Control Message Protocol	*/
		MY_IPv4PROTOICMP = 1,
		/** Internet Gateway Management Protocol */
		MY_IPv4PROTOIGMP = 2,
		/** IPIP tunnels (older KA9Q tunnels use 94) */
		MY_IPv4PROTOIPIP = 4,
		/** Transmission Control Protocol	*/
		MY_IPv4PROTOTCP = 6,
		/** Exterior Gateway Protocol		*/
		MY_IPv4PROTOEGP = 8,
		/** PUP protocol				*/
		MY_IPv4PROTOPUP = 12,
		/** User Datagram Protocol		*/
		MY_IPv4PROTOUDP = 17,
		/** XNS IDP protocol			*/
		MY_IPv4PROTOIDP = 22,
		/** IPv6 header				*/
		MY_IPv4PROTOIPV6 = 41,
		/** IPv6 Routing header			*/
		MY_IPv4PROTOROUTING = 43,
		/** IPv6 fragmentation header		*/
		MY_IPv4PROTOFRAGMENT = 44,
		/** GRE protocol */
		MY_IPv4PROTOGRE = 47,
		/** encapsulating security payload	*/
		MY_IPv4PROTOESP = 50,
		/** authentication header		*/
		MY_IPv4PROTOAH = 51,
		/** ICMPv6				*/
		MY_IPv4PROTOICMPV6 = 58,
		/** IPv6 no next header			*/
		MY_IPv4PROTONONE = 59,
		/** IPv6 Destination options		*/
		MY_IPv4PROTODSTOPTS = 60,
		/** Raw IP packets			*/
		MY_IPv4PROTOUDPData = 253,
		MY_IPv4PROTOTCPData = 254,
		MY_IPv4PROTORAW = 255,
		/** Maximum value */
		MY_IPv4PROTOMAX
	};

	enum ETHProtocolTypes{
		/* IPv4 protocol*/
		MY_ETHPROTO_IPV4 = 0x0800,
		/* ARP protocol*/
		MY_ETHPROTO_ARP = 0x0806,
		/* PPPoE protocol */
		MY_ETHPROTO_PPPOE = 0x8864,
		/* 802.1 Q tag */
		MY_ETHPROTO_8021Q = 0x8100,
		/* IPv6 protocol*/
		MY_ETHPROTO_IPV6 = 0x86dd,
		/* MPLS Label */
		MY_ETHPROTO_MPLS = 0x8847,
		/* VLAN */
		MY_ETHPROTO_VLAN1 = 0x9100,
		MY_ETHPROTO_VLAN2 = 0x9200
	};

	enum IPv6ExtensionTypes {
		/** Hop-By-Hop extension type */
		MY_IPv6HopByHop = 0,
		/** Routing extension type */
		MY_IPv6Routing = 43,
		/** IPv6 fragmentation extension type */
		MY_IPv6Fragmentation = 44,
		/** Authentication Header extension type */
		MY_IPv6AuthenticationHdr = 51,
		/** Destination extension type */
		MY_IPv6NoNextHeader = 59,
		MY_IPv6Destination = 60,

		MY_IPv6NonFirstFragUDP = 253,
		MY_IPv6NonFirstFragTCP = 254,
		/** Unknown or unsupported extension type */
		My_IPv6ExtensionUnknown = 255
	};
}

#endif