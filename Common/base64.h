#ifndef __Base64_H__
#define __Base64_H__

int base64_encode(const char *data,int dlen,char* ret,int space);
int base64_decode(const char *bdata,char* ret,int space);

#endif //__Base64_H__