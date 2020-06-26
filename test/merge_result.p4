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

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {
    }
}

control MyIngress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    @name(".NoAction") action NoAction_1_v1() {
    }
    @name("MyIngress.drop") action drop_1_v1() {
        mark_to_drop();
    }
    @name("MyIngress.ipv4_forward") action ipv4_forward_v1(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl + 8w255;
    }
    @name("MyIngress.ipv4_lpm") table ipv4_lpm_0_v1 {
        key = {
            hdr.ipv4.dstAddr: lpm @name("hdr.ipv4.dstAddr") ;
        }
        actions = {
            ipv4_forward_v1();
            drop_1_v1();
            NoAction_1_v1();
        }
        size = 1024;
        default_action = drop_1_v1();
    }
    @name(".NoAction") action NoAction_1_v2() {
    }
    @name("MyIngress.drop") action drop_1_v2() {
        mark_to_drop();
    }
    @name("MyIngress.drop") action drop_2_v2() {
        mark_to_drop();
    }
    @name("MyIngress.ipv4_forward") action ipv4_forward_v2(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl + 8w255;
    }
    @name("MyIngress.ipv4_lpm") table ipv4_lpm_0_v2 {
        key = {
            hdr.ipv4.dstAddr: lpm @name("hdr.ipv4.dstAddr") ;
        }
        actions = {
            ipv4_forward_v2();
            drop_1_v2();
            NoAction_1_v2();
        }
        size = 1024;
        default_action = drop_1_v2();
    }
    @name("MyIngress.myTunnel_forward") action myTunnel_forward_v2(egressSpec_t port) {
        standard_metadata.egress_spec = port;
    }
    @name("MyIngress.myTunnel_exact") table myTunnel_exact_0_v2 {
        key = {
            hdr.myTunnel.dst_id: exact @name("hdr.myTunnel.dst_id") ;
        }
        actions = {
            myTunnel_forward_v2();
            drop_2_v2();
        }
        size = 1024;
        default_action = drop_2_v2();
    }
    apply {
        if (standard_metadata.ingress_port == 9w1) {
            if (hdr.ipv4.isValid()) 
                ipv4_lpm_0_v1.apply();
        }
        if (standard_metadata.ingress_port == 9w2) {
            if (hdr.ipv4.isValid() && !hdr.myTunnel.isValid()) 
                ipv4_lpm_0_v2.apply();
            if (hdr.myTunnel.isValid()) 
                myTunnel_exact_0_v2.apply();
        }
    }
}

control MyEgress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    apply {
    }
}

control MyComputeChecksum(inout headers hdr, inout metadata meta) {
    apply {
        update_checksum<tuple<bit<4>, bit<4>, bit<8>, bit<16>, bit<16>, bit<3>, bit<13>, bit<8>, bit<8>, bit<32>, bit<32>>, bit<16>>(hdr.ipv4.isValid(), { hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr }, hdr.ipv4.hdrChecksum, HashAlgorithm.csum16);
    }
}

control MyDeparser(packet_out packet, in headers hdr) {
    apply {
        packet.emit<ethernet_t>(hdr.ethernet);
        packet.emit<myTunnel_t>(hdr.myTunnel);
        packet.emit<ipv4_t>(hdr.ipv4);
    }
}

V1Switch<headers, metadata>(MyParser(), MyVerifyChecksum(), MyIngress(), MyEgress(), MyComputeChecksum(), MyDeparser()) main;

