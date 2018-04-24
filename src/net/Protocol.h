#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

class NOTI 
{
public:
    static const int JOB_ID = 0;
    static const int BLOB = 1;
    static const int TARGET = 2;
    static const int JOB_UNIT = 3;
};

class LEN
{
public:
    static const int BLOB_HEX = 144;
    static const int PREHASH_HEX = 128;
    static const int NONCE_HEX = 16;
    static const int DIFF_HEX = 4;
    static const int RESULT_HEX = 64;
    
    static const int BLOB = 72;
    static const int PREHASH = 64;
    static const int NONCE = 8;
    static const int DIFF = 2;
    static const int RESULT = 32;
};
#endif