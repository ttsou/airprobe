

struct _id_list list_ChannelRelease[] = {
{0, "00000000 RR-Cause (reason of event) = Normal event"},
{1, "00000001  RR-Cause (reason of event) = Abnormal release, unspecified"},
{2, "00000010  RR-Cause (reason of event) = Abnormal release, channel unacceptable"},
{3, "00000011  RR-Cause (reason of event) = Abnormal release, timer expired"},
{4, "00000100  RR-Cause (reason of event) = Abnormal release, no activity on the radio path"},
{5, "00000101  RR-Cause (reason of event) = Preemptive release"},
{8, "00001000  RR-Cause (reason of event) = Handover impossible, timing advance out of range"},
{9, "00001001  RR-Cause (reason of event) = Channel mode unacceptable"},
{10, "00001010  RR-Cause (reason of event) = Frequency not implemented"}, 
{0x81, "01000001  RR-Cause (reason of event) = Call already cleared"}, 
{0x5f, "01011111  RR-Cause (reason of event) = Semantically incorrect message"},
{0x60, "01100000  RR-Cause (reason of event) = Invalid mandatory information"},
{0x61, "01100001  RR-Cause (reason of event) = Message type non-existent or not implemented"},
{0x62, "01100010  RR-Cause (reason of event) = Message type not compatible with protocol state"},
{0x64, "01100100  RR-Cause (reason of event) = Conditional IE error"},
{0x65, "01100101  RR-Cause (reason of event) = Nocell allocation available"},
{0x6f, "01101111  RR-Cause (reason of event) = Protocol error unspecified"}, 
{-1, NULL}
};

struct _id_list list_RequestServiceType[] = {
{1, "----0001 Request Service Type: MS originated call"},
{2, "----0010 Request Service Type: Emergency call"},
{4, "----0100 Request Service Type: SMS"},
{8, "----1000 Request Service Type: Supplementary Service Activation"},
{9, "----1001 Request Service Type: Voice Group call"},
{10, "----1010 Request Service Type: Voice Broadcast call"},
{11, "----1011 Request Service Type: Location Services"},
{-1, NULL}
};

struct _id_list list_SMSCAddressType[] = {
{0, "-000---- Unknown Number Type"},
{1, "-001---- International Number"},
{2, "-010---- National Number"},
{3, "-011---- Network specific number"},
{4, "-100---- Subscriber number"},
{5, "-101---- Alphanumeric number"},
{6, "-110---- Abbreviated number"},
{-1, NULL}
};

struct _id_list list_SMSCAddressNumberingPlan[] = {
{0, "----0000 Numbering plan: Unknown"},
{1, "----0001 Numbering plan: ISDN/telephone (E164/E.163)"},
{3, "----0011 Numbering plan: Data(X.121)"},
{4, "----0100 Numbering plan: Telex"},
{8, "----1000 Numbering plan: National"},
{9, "----1001 Numbering plan: Private"},
{10, "----1010 Numbering plan: ERMES (ESTI DE/PS3 01-3)"},
{-1, NULL}
};

struct _id_list list_ChannelDescriptionTwo[] = {
{0x01, "TCH/F + ACCHs"},
{0x02, "TCH/H + ACCHs, subchannel 0"},
{0x03, "TCH/H + ACCHs, subchannel 1"},
{0x04, "SDCCH/4 + SACCH/C4 or CBCH (SDCCH/4), SC0"},
{0x05, "SDCCH/4 + SACCH/C4 or CBCH (SDCCH/4), SC1"},
{0x06, "SDCCH/4 + SACCH/C4 or CBCH (SDCCH/4), SC2"},
{0x07, "SDCCH/4 + SACCH/C4 or CBCH (SDCCH/4), SC3"},
{0x08, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC0"},
{0x09, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC1"},
{0x0a, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC2"},
{0x0b, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC3"},
{0x0c, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC4"},
{0x0d, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC5"},
{0x0e, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC6"},
{0x0f, "SDCCH/8 + SACCH/C8 or CBCH (SDCCH/8), SC7"},
{-1, NULL}
};

struct _id_list list_ChannelMode[] = {
{0x00, "00000000 Channel Mode: signaling only"},
{0x01, "00000001 Channel Mode: TCH/F or TCH/H rev 1"},
{0x21, "00100001 Channel Mode: TCH/F or TCH/H rev 2"},
{0x41, "01000001 Channel Mode: TCH/F or TCH/H rev 3"},
{0x0f, "00001111 Channel Mode: Data, 14.5 kbit/s"},
{0x03, "00000011 Channel Mode: Data, 12.0 kbit/s"},
{0x0b, "00001011 Channel Mode: Data, 6.0 kbit/s"},
{0x13, "00010011 Channel Mode: Data, 3.6 kbit/s"},
{-1, NULL}
};

struct _id_list list_CodingStandard[] = {
{0x00, "CCITT"},
{0x01, "Reserved for international standards"},
{0x02, "National standard"},
{0x03, "GSM-PLMNS"},
{-1, NULL}
};


struct _id_list list_Location[] = {
{0x00, "User"},
{0x01, "Private network serving local user"},
{0x02, "Public network serving local user"},
{0x04, "Public network serving remote user"},
{0x05, "Private network serving remote user"},
{0x0a, "Network beyong interworking point"},
{-1, NULL}
};

struct _id_list list_Progress[] = {
{0x01, "-0000001 Progress: Call is not end-to-end PLMN/ISDN"},
{0x02, "-0000010 Progress: Destination address in non-PLMN/ISDN"},
{0x03, "-0000011 Progress: Origination address in non-PLMN/ISDN"},
{0x04, "-0000100 Progress: Call has returned to the PLMN/ISDN"},
{0x08, "-0001000 Progress: In-band information or appr. pattern available"},
{0x20, "-0100000 Progress: Call is end-to-end PLMN/ISDN"},
{0x40, "-1000000 Progress: Queueing"},
{-1, NULL}
};

struct _id_list list_Cause[] = {
{0x01, "Unassigned number"},
{0x03, "No route to destination"},
{0x06, "Channel unacceptable"},
{0x08, "Operator determined barring"},
{0x10, "Normal call clearing"},
{0x11, "User busy"},
{0x12, "No user responding"},
{0x13, "User alerting, no answer"},
{0x15, "Call rejected"},
{0x16, "Number changed, New destination"},
{0x19, "Pre-emption"},
{0x1a, "Non selected user clearing"},
{0x1b, "Destination out of order"},
{0x1c, "Invalid number format (incomplete number)"},
{0x1d, "Fascility rejected"},
{0x1e, "Response to Status Enquiry"},
{0x1f, "Normal"},
{0x22, "No channel available"},
{0x26, "Network out of order"},
/* FIXME: fill in others.. */
{-1, NULL}
};

struct _id_list list_RadioChannelReq[] = {
{0x00, "reserved"},
{0x01, "full rate MS"},
{0x02, "dual rate MS/half rate preferred"},
{0x03, "dual rate MS/full rate preferred"},
{-1, NULL}
};

struct _id_list list_TransferCap[] = {
{0x00, "speech"},
{0x01, "unrestricted digital information"},
{0x02, "3.1 kHz audio, exx PLMN"},
{0x03, "facsimile group 3"},
{0x05, "Other ITC"},
{-1, NULL}
};

