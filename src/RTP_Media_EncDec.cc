/******************************************************************************
* Copyright (c) 2005, 2014  Ericsson AB
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
//  File:           RTP_Media_EncDec.cc
//  Description:    Encode/decode functions for the media file operations
//  Rev:            R5A
//  Prodnr:         CNL 113 392

#include "RTP_File_Types.hh"

namespace RTP__File__Types{
OCTETSTRING f__RTP__Hdr__enc(Media__RTP__Header const &hdr)
{
    TTCN_Buffer buf;
    if (hdr.get_selection()==Media__RTP__Header::ALT_jpeg__rtp__hdr)
        hdr.jpeg__rtp__hdr().encode(JPEG__RTP__Header_descr_,buf,TTCN_EncDec::CT_RAW);
    else if (hdr.get_selection()==Media__RTP__Header::ALT_h263__rtp__hdr)
        hdr.h263__rtp__hdr().encode(H263__RTP__Header_descr_,buf,TTCN_EncDec::CT_RAW);
    else
        TTCN_error("RTP_Hdr_enc: The incoming parameter (hdr) is unbound!");

    return OCTETSTRING(buf.get_len(),buf.get_data());
}

JPEG__RTP__Header f__JPEG__RTP__Hdr__dec(OCTETSTRING const &stream)
{
    TTCN_Buffer buf;
    buf.put_os(stream);
    JPEG__RTP__Header ret_val;
    ret_val.decode(JPEG__RTP__Header_descr_, buf, TTCN_EncDec::CT_RAW);

    return ret_val;
}

H263__RTP__Header f__H263__RTP__Hdr__dec(OCTETSTRING const &stream)
{
    TTCN_Buffer buf;
    buf.put_os(stream);
    H263__RTP__Header ret_val;
    ret_val.decode(H263__RTP__Header_descr_, buf, TTCN_EncDec::CT_RAW);

    return ret_val;
}

OCTETSTRING f__RTP__Data__enc(Media__RTP__Data const &rtp_data)
{
    TTCN_Buffer buf;
    if (rtp_data.get_selection()==Media__RTP__Data::ALT_jpeg__rtp)
        rtp_data.jpeg__rtp().encode(JPEG__RTP_descr_,buf,TTCN_EncDec::CT_RAW);
    else if (rtp_data.get_selection()==Media__RTP__Data::ALT_h263__rtp)
        rtp_data.h263__rtp().encode(H263__RTP_descr_,buf,TTCN_EncDec::CT_RAW);
    else
        TTCN_error("RTP_Data_enc: The incoming parameter (rtp_data) is unbound!");

  return OCTETSTRING(buf.get_len(),buf.get_data());
}

JPEG__RTP f__JPEG__RTP__Data__dec(OCTETSTRING const &stream)
{
    TTCN_Buffer buf;
    buf.put_os(stream);
    JPEG__RTP ret_val;
    ret_val.decode(JPEG__RTP_descr_, buf, TTCN_EncDec::CT_RAW);

    return ret_val;
}

H263__RTP f__H263__RTP__Data__dec(OCTETSTRING const &stream)
{
    TTCN_Buffer buf;
    buf.put_os(stream);
    H263__RTP ret_val;
    ret_val.decode(H263__RTP_descr_, buf, TTCN_EncDec::CT_RAW);

    return ret_val;
}
}
