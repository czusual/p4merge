/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>



/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/


struct metadata {
    /* empty */
}

struct headers {
    /* empty */
}


/*************************************************************************
*********************** P A R S E R  ***********************************
*************************************************************************/

parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        
        transition select(standard_metadata.ingress_port) {
            1:start_0_v1;
            2:start_0_v2;
        }
    }
    state start_0_v1 {
        
    }
    state start_0_v2 {
        
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
        if(standard_metadata.ingress_port==1)
        {
            ipv4_lpm.apply();
        }
        if(standard_metadata.ingress_port==2)
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
