/*
$Id: dsmcc_str.h,v 1.3.2.1 2003/10/28 19:33:22 coronas Exp $ 

  dvbsnoop
  (c) Rainer Scherg 2001-2003

  dsmcc decoder helper functions


$Log: dsmcc_str.h,v $
Revision 1.3.2.1  2003/10/28 19:33:22  coronas
Compilefix rel-branch/Update from HEAD

Revision 1.3  2003/10/26 21:36:20  rasc
private DSM-CC descriptor Tags started,
INT-Section completed..

Revision 1.2  2003/10/25 19:11:50  rasc
no message

Revision 1.1  2003/10/16 19:02:28  rasc
some updates to dvbsnoop...
- small bugfixes
- tables updates from ETR 162



*/

char *dsmccStrDSMCCPrivateDescriptorTAG (u_int id);
char *dsmccStrMHPOrg (u_int id);
char *dsmccStrAction_Type (u_int id);
char *dsmccStrProcessing_order (u_int id);
char *dsmccStrPayload_scrambling_control (u_int id);
char *dsmccStrAddress_scrambling_control (u_int id);
char *dsmccStrLinkage0CTable_TYPE (u_int i);
char *dsmccStrMultiProtEncapsMACAddrRangeField (u_int i);
char *dsmccStrPlatform_ID (u_int id);




