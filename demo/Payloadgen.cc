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
#include "Payloadgen_Types.hh"

namespace Payloadgen__Types{
OCTETSTRING f__generate__comfort__noise(const INTEGER& level, const Coefficient__list& coefficients)
{
	if(level<0 || level>127)
		TTCN_error("Invalid comfort noise level value: %d", (int)level);
		
	OCTETSTRING ret_val = int2oct(level, 1);
	for(int i=0;i<coefficients.size_of();i++) {
		if((double)coefficients[i]<-1 || (double)coefficients[i]>1) {
			TTCN_error("Invalid comfort noise coefficient value(%d): %f",
				i, (double)coefficients[i]);
		}
		ret_val = ret_val + int2oct(float2int((
			((double)coefficients[i]*32768)/258)+127.0), 1);
	}
	return ret_val;
}

OCTETSTRING f__generate__tones__events(const Tones__DTMFs& events_dtmfs)
{
	OCTETSTRING header(0,0), value(0,0);
	
	for(int i=0;i<events_dtmfs.size_of();i++) {
		const Tone__DTMF__Type & curr_descr = events_dtmfs[i];
		if(i==0)
			header = bit2oct(int2bit(0,1) + 
				int2bit(curr_descr.pt(), 7));
		else
			header = bit2oct(int2bit(1,1) + 
				int2bit(curr_descr.pt(), 7))
				+ bit2oct(int2bit(curr_descr.ts__offset(), 14) + 
				int2bit(4, 10)) + header;
				
		Tone__or__DTMF__Type& curr_type =
			(Tone__or__DTMF__Type&)curr_descr.tod();
		
		//RFC 2833 Section 3.
		if(curr_type.get_selection() == Tone__or__DTMF__Type::ALT_dtmf) {
			DTMF__Event__Type& dtmf_descr = curr_type.dtmf();
			if(dtmf_descr.E__bit().lengthof() != 1)
				TTCN_error("E_bit is not 1 byte long");
			value = value + (int2oct(dtmf_descr.event(), 1) +
				(bit2oct(dtmf_descr.E__bit() + int2bit(0, 1) +
				int2bit(dtmf_descr.volume(), 6)) + 
				int2oct(dtmf_descr.duration(), 2)));
		}	
		//RFC 2833 Section 4.
		else if(curr_type.get_selection() == Tone__or__DTMF__Type::ALT_tone) {
			Telephony__Tone__Type& tone_descr = curr_type.tone();
			if(tone_descr.T__bit().lengthof() != 1)
				TTCN_error("T_bit is not 1 byte long");
			value = value + (bit2oct(int2bit(tone_descr.modulation(), 9) +
				tone_descr.T__bit() + int2bit(tone_descr.volume(), 6)) + 
				int2oct(tone_descr.duration(), 2));
			for(int j=0;j<tone_descr.frequencies().size_of();j++)
				value = value + bit2oct(int2bit(0, 4) +
					int2bit(tone_descr.frequencies()[j], 12));
		}
		else TTCN_error("generate_tones_events: unexpected union alternative");
	}
	
	return header + value;
}

}
