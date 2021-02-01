/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>



/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/
const bit<16> TYPE_IPV4 = 0x8100;
const bit<16> TYPE_tunnel = 0x8200;
const bit<12> USER1 = 0x1;
const bit<12> USER2 = 0x2;

typedef bit<9> egressSpec_t;
typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;

header ethernet_t {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16>   etherType;
}

header vlan_t {
    bit<3> pcp;
    bit<1> cfi;
    bit<12> vlanid;
    bit<16> ether_type;
}



struct metadata {
}

struct headers {
    ethernet_t ethernet;
	vlan_t     vlan;
}

/*************************************************************************
*********************** P A R S E R  ***********************************
*************************************************************************/

parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        
        packet.extract(hdr.ethernet);
        transition parse_vlan;
        
    }
    state parse_vlan{
        packet.extract(hdr.vlan);
        transition select(hdr.vlan.vlanid) {
            USER1:parse_vlan_0_v1;
            USER2:parse_vlan_0_v2;
        }
    }
    state parse_vlan_0_v1 {
        
    }
    state parse_vlan_0_v2 {
        
    }

}

/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {   
    apply {  }
}


/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {
    table ipv4_lpm {
        key = {
           
        }
        actions = {
            NoAction;
        }
        size = 1024;
    }

    apply {
        if(hdr.vlan.vlanid==USER1)
        {
            ipv4_lpm.apply();
        }
        if(hdr.vlan.vlanid==USER2)
        {
            ipv4_lpm.apply();
        }
       
    }
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {
    apply {  }
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control MyComputeChecksum(inout headers  hdr, inout metadata meta) {
     apply {
	
    }
}

/*************************************************************************
***********************  D E P A R S E R  *******************************
*************************************************************************/

control MyDeparser(packet_out packet, in headers hdr) {
    apply { }
        
}

/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

V1Switch(
MyParser(),
MyVerifyChecksum(),
MyIngress(),
MyEgress(),
MyComputeChecksum(),
MyDeparser()
) main;
