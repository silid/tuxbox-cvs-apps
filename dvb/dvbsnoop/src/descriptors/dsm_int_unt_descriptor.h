/*
$Id: dsm_int_unt_descriptor.h,v 1.1.2.1 2003/11/17 07:07:41 coronas Exp $ 


  dvbsnoop
  (c) Rainer Scherg 2001-2003

  Private TAG Space  DSM-CC   INT, UNT, ...
  DSM-CC Descriptors  ISO 13818-6  // TR 102 006



$Log: dsm_int_unt_descriptor.h,v $
Revision 1.1.2.1  2003/11/17 07:07:41  coronas
Compilefix rel-branch/Update from HEAD

Revision 1.1  2003/10/29 20:56:18  rasc
more PES stuff, DSM descriptors, testdata



*/


int   descriptorDSMCC_INT_UNT_Private (u_char *b);
void  descriptorDSMCC_INT_UNT_any (u_char *b);



