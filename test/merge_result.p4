#include <core.p4>
#include <v1model.p4>

typedef bit<9> egressSpec_t;
typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;
header ethernet_t {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16>   etherType;
}

header myTunnel_t {
    bit<16> proto_id;
    bit<16> dst_id;
}

header ipv4_t {
    bit<4>    version;
    bit<4>    ihl;
    bit<8>    diffserv;
    bit<16>   totalLen;
    bit<16>   identification;
    bit<3>    flags;
    bit<13>   fragOffset;
    bit<8>    ttl;
    bit<8>    protocol;
    bit<16>   hdrChecksum;
    ip4Addr_t srcAddr;
    ip4Addr_t dstAddr;
}

struct metadata {
}

struct headers {
    ethernet_t ethernet;
    myTunnel_t myTunnel;
    ipv4_t     ipv4;
}

parser MyParser(packet_in packet, out headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    state start {
        transition select(standard_metadata.ingress_port) {
            9w1: start_0_v1;
            9w2: start_0_v2;
        }
    }
    @name("start") state start_0_v1 {
        packet.extract<ethernet_t>(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4_0_v1;
            default: accept;
        }
    }
    @name("parse_ipv4") state parse_ipv4_0_v1 {
        packet.extract<ipv4_t>(hdr.ipv4);
        transition accept;
    }
    @name("reject") state reject_0_v1 {
    }
    @name("start") state start_0_v2 {
        packet.extract<ethernet_t>(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x1212: parse_myTunnel_0_v2;
            16w0x800: parse_ipv4_0_v2;
            default: accept;
        }
    }
    @name("parse_myTunnel") state parse_myTunnel_0_v2 {
        packet.extract<myTunnel_t>(hdr.myTunnel);
        transition select(hdr.myTunnel.proto_id) {
            16w0x800: parse_ipv4_0_v2;
            default: accept;
        }
    }
    @name("parse_ipv4") state parse_ipv4_0_v2 {
        packet.extract<ipv4_t>(hdr.ipv4);
        transition accept;
    }
    @name("reject") state reject_0_v2 {
    }
}

