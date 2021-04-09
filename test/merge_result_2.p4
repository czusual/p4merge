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

header vlan_t {
    bit<3>  pcp;
    bit<1>  cfi;
    bit<12> vlanid;
    bit<16> ether_type;
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
    vlan_t     vlan;
    myTunnel_t myTunnel;
    ipv4_t     ipv4;
}

parser MyParser(packet_in packet, out headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    state start {
        packet.extract<ethernet_t>(hdr.ethernet);
        packet.extract<vlan_t>(hdr.vlan);
        transition select(hdr.vlan.vlanid) {
            12w0x1: parse_vlan_0_v1_0_v1;
            12w0x2: parse_vlan_0_v2_0_v1;
            12w0x3: parse_vlan_0_v2;
        }
    }
    state parse_vlan_0_v1_0_v1 {
        packet.extract<vlan_t>(hdr.vlan);
        transition select(hdr.vlan.ether_type) {
            16w0x800: parse_ipv4_0_v1_0_v1;
            default: accept;
        }
    }
    state parse_ipv4_0_v1_0_v1 {
        packet.extract<ipv4_t>(hdr.ipv4);
        transition accept;
    }
    state parse_vlan_0_v2_0_v1 {
        packet.extract<vlan_t>(hdr.vlan);
        transition select(hdr.vlan.ether_type) {
            16w0x1212: parse_myTunnel_0_v2_0_v1;
            16w0x800: parse_ipv4_0_v2_0_v1;
            default: accept;
        }
    }
    state parse_myTunnel_0_v2_0_v1 {
        packet.extract<myTunnel_t>(hdr.myTunnel);
        transition select(hdr.myTunnel.proto_id) {
            16w0x800: parse_ipv4_0_v2_0_v1;
            default: accept;
        }
    }
    state parse_ipv4_0_v2_0_v1 {
        packet.extract<ipv4_t>(hdr.ipv4);
        transition accept;
    }
    state reject_0_v1 {
    }
    state parse_vlan_0_v2 {
        packet.extract<vlan_t>(hdr.vlan);
        transition select(hdr.vlan.ether_type) {
            16w0x1212: parse_myTunnel_0_v2;
            16w0x800: parse_ipv4_0_v2;
            default: accept;
        }
    }
    state parse_myTunnel_0_v2 {
        packet.extract<myTunnel_t>(hdr.myTunnel);
        transition select(hdr.myTunnel.proto_id) {
            16w0x800: parse_ipv4_0_v2;
            default: accept;
        }
    }
    state parse_ipv4_0_v2 {
        packet.extract<ipv4_t>(hdr.ipv4);
        transition accept;
    }
    state reject_0_v2 {
    }
}

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {
    apply {
    }
}

control MyIngress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    action NoAction_1_v1_v1() {
    }
    action drop_1_v1_v1() {
        mark_to_drop();
    }
    action ipv4_forward_v1_v1(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl + 8w255;
    }
    table ipv4_lpm_0_v1_0_v1 {
        key = {
            hdr.ipv4.dstAddr: lpm ;
        }
        actions = {
            ipv4_forward_v1_v1();
            drop_1_v1_v1();
            NoAction_1_v1_v1();
        }
        size = 1024;
        default_action = drop_1_v1_v1();
    }
    action NoAction_1_v2_v1() {
    }
    action drop_1_v2_v1() {
        mark_to_drop();
    }
    action drop_2_v2_v1() {
        mark_to_drop();
    }
    action ipv4_forward_v2_v1(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl + 8w255;
    }
    table ipv4_lpm_0_v2_0_v1 {
        key = {
            hdr.ipv4.dstAddr: lpm ;
        }
        actions = {
            ipv4_forward_v2_v1();
            drop_1_v2_v1();
            NoAction_1_v2_v1();
        }
        size = 1024;
        default_action = drop_1_v2_v1();
    }
    action myTunnel_forward_v2_v1(egressSpec_t port) {
        standard_metadata.egress_spec = port;
    }
    table myTunnel_exact_0_v2_0_v1 {
        key = {
            hdr.myTunnel.dst_id: exact ;
        }
        actions = {
            myTunnel_forward_v2_v1();
            drop_2_v2_v1();
        }
        size = 1024;
        default_action = drop_2_v2_v1();
    }
    action NoAction_1_v2() {
    }
    action drop_1_v2() {
        mark_to_drop();
    }
    action drop_2_v2() {
        mark_to_drop();
    }
    action ipv4_forward_v2(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl + 8w255;
    }
    table ipv4_lpm_0_v2 {
        key = {
            hdr.ipv4.dstAddr: lpm ;
        }
        actions = {
            ipv4_forward_v2();
            drop_1_v2();
            NoAction_1_v2();
        }
        size = 1024;
        default_action = drop_1_v2();
    }
    action myTunnel_forward_v2(egressSpec_t port) {
        standard_metadata.egress_spec = port;
    }
    table myTunnel_exact_0_v2 {
        key = {
            hdr.myTunnel.dst_id: exact ;
        }
        actions = {
            myTunnel_forward_v2();
            drop_2_v2();
        }
        size = 1024;
        default_action = drop_2_v2();
    }
    apply {
        if (hdr.vlan.vlanid != 12w0x3) {
            if (hdr.vlan.vlanid == 12w0x1) 
                if (hdr.ipv4.isValid()) 
                    ipv4_lpm_0_v1_0_v1.apply();
            if (hdr.vlan.vlanid == 12w0x2) {
                if (hdr.ipv4.isValid() && !hdr.myTunnel.isValid()) 
                    ipv4_lpm_0_v2_0_v1.apply();
                if (hdr.myTunnel.isValid()) 
                    myTunnel_exact_0_v2_0_v1.apply();
            }
        }
        if (hdr.vlan.vlanid == 12w0x3) {
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
        packet.emit<vlan_t>(hdr.vlan);
        packet.emit<myTunnel_t>(hdr.myTunnel);
        packet.emit<ipv4_t>(hdr.ipv4);
    }
}

V1Switch<headers, metadata>(MyParser(), MyVerifyChecksum(), MyIngress(), MyEgress(), MyComputeChecksum(), MyDeparser()) main;

