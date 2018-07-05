/******************************************************************************
* Copyright (c) 2000-2018 Ericsson Telecom AB
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v2.0
* which accompanies this distribution, and is available at
* https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
*
* Contributors:
*   Gabor Szalai - initial implementation and initial documentation
*   Peter Kremer
*   Sandor Palugyai
*   Tibor Harai
******************************************************************************/
//
//  File:               RTP_EncDec.cc
//  Description:        RTP
//  Rev:                R5A
//  Prodnr:             CNL 113 392
//
#include "RTP_Types.hh"

TTCN_Module RTP__EncDec("RTP__EncDec", __DATE__, __TIME__);
namespace RTP__Types{
OCTETSTRING f__RTP__enc(RTP__messages__union const &pdu){
  TTCN_Buffer bb;
  if(pdu.get_selection()==RTP__messages__union::ALT_rtcp)
      pdu.rtcp().encode(PDU__RTCP_descr_,bb,TTCN_EncDec::CT_RAW);
  else pdu.rtp().encode(PDU__RTP_descr_,bb,TTCN_EncDec::CT_RAW);
  
  return OCTETSTRING(bb.get_len(),bb.get_data());
}

RTP__messages__union f__RTP__dec(OCTETSTRING const &data){
  const unsigned char *raw_data=(const unsigned char*)data;
  int data_length=data.lengthof();
  int pt=raw_data[1]&0x7f;
  RTP__messages__union ret_val;
  TTCN_Buffer bb;
  if(raw_data[0]&0x20){
    data_length-=raw_data[data_length-1];
  }
  bb.clear();
  bb.put_s(data_length,raw_data);
  if(pt>71 && pt<77){
  // Magic numbers. See rfc3550 12. Summary of Protocol Constants 
    PDU__RTCP pdu;
    pdu.decode(PDU__RTCP_descr_,bb,TTCN_EncDec::CT_RAW);
    ret_val.rtcp()=pdu;
  }
  else{
    PDU__RTP pdu;
    pdu.decode(PDU__RTP_descr_,bb,TTCN_EncDec::CT_RAW);
    ret_val.rtp()=pdu;
  }
  return ret_val;
}

OCTETSTRING f__RTP__packet__enc ( RTP__packet const &pdu ) {
  TTCN_Buffer bb;

  pdu.encode ( RTP__packet_descr_, bb, TTCN_EncDec::CT_RAW );
  
  return OCTETSTRING ( bb.get_len(), bb.get_data() );
}

RTP__packet f__RTP__packet__dec ( OCTETSTRING const &data )
{
  const unsigned char *raw_data = (const unsigned char*)data;
  int data_length = data.lengthof();
  RTP__packet pdu;
  TTCN_Buffer bb;
  
  bb.clear();
  bb.put_s ( data_length, raw_data );
  
  pdu.decode ( RTP__packet_descr_, bb, TTCN_EncDec::CT_RAW );
  return pdu;
}
}
