# Base64编解码类tc_base64


### code：
[tc_base64.h](https://github.com/Tencent/Tars/blob/master/cpp/util/include/util/tc_base64.h)   
[tc_base64.cpp](https://github.com/Tencent/Tars/blob/master/cpp/util/src/tc_base64.cpp)

## 知识点

Base64是一种基于64个可打印字符来表示二进制数据的表示方法。由于2的6次方等于64，所以每6位为一个单元，对应某个可打印字符。在Base64中的可打印字符包括字母A-Z、a-z、数字0-9，这样共有62个字符，此外两个可打印符号在不同的系统中而不同。

### base64 算法的由来

base64 最早是用作解决电子邮件的传输问题，由于历史原因，早期电子邮件只允许传输 ASCII 码字符，如果想传输一封带有非 ASCII 字符的邮件，在遇到一些老旧的网关就可能会对字符最高位进行调整，导致收到的邮件出现乱码。



### base 编码方法

二进制数据通过8位表示一个字符，Base64通过6位表示一个字符。所以将一个二进制数据转换为Base64编码，其长度要增加 (lengh*8/6)倍。
其规则如下： 
1)依次从Binary数据中取出6位，转换为一个Base64字符;
2)如果最后有剩余则以0补够6位。在补位的同时,需要在Base64结尾以‘=’进行标识，一个'='表示补2个0，两个'==',表示补4个0。
需要注意的是第二条规则，由于数据本身是8的倍数，所以每次取6位，最后的余数只能是2或4。这两个则通过在Base64结尾加‘=’和'=='标识。。

4.举例说明
1) 0 1 2编码为Base64
0 1 2 二进制表示为 0011 0000        0011  0001     0011  0010
转换为Base64为    001100          000011          000100              110010  
                           即    12                        3                   4                         50
从编码表中查找       M                          D                  E                          y
所以对应的Base64编码为 "MDEy"

2) 0123编码为Base64（需要补位）
0 1  2  3 二进制表示为 0011 0000        0011  0001     0011  0010        0011  0011
转换为Base64为    001100          000011          000100              110010   001100     110000（后面补4个0）
                           即    12                        3                   4                         50              12              48
从编码表中查找       M                          D                  E                          y                 M                w
所以对应的Base64编码为 "MDEyMw==" （由于补了4个0，因此需要加上'=='）

4.base64 索引表如下
将输入数据流每次取6 bit，用此6 bit的值(0-63)作为索引去查表，输出相应字符。这样，每3个字节将编码为4个字符(3×8 → 4×6)；不满4个字符的以'='填充。

0	A	16	Q	32	g	48	w   
1	B	17	R	33	h	49	x   
2	C	18	S	34	i	50	y   
3	D	19	T	35	j	51	z   
4	E	20	U	36	k	52	0   
5	F	21	V	37	l	53	1   
6	G	22	W	38	m	54	2   
7	H	23	X	39	n	55	3   
8	I	24	Y	40	o	56	4     
9	J	25	Z	41	p	57	5     
10	K	26	a	42	q	58	6  
11	L	27	b	43	r	59	7  
12	M	28	c	44	s	60	8  
13	N	29	d	45	t	61	9   
14	O	30	e	46	u	62	+   
15	P	31	f	47	v	63	/  


如果要编码的字节数不能被3整除，最后会多出1个或2个字节，那么可以使用下面的方法进行处理：先使用0字节值在末尾补足，使其能够被3整除，然后再进行base64的编码。在编码后的base64文本后加上一个或两个'='号，代表补足的字节数。也就是说，当最后剩余一个八位字节（一个byte）时，最后一个6位的base64字节块有四位是0值，最后附加上两个等号；如果最后剩余两个八位字节（2个byte）时，最后一个6位的base字节块有两位是0值，最后附加一个等号

```
int TC_Base64::encode(const unsigned char* pSrc, int nSrcLen, char* pDst, bool bChangeLine/* = false*/)
{
    unsigned char c1, c2, c3;   
    int nDstLen = 0;             
    int nLineLen = 0;         
    int nDiv = nSrcLen / 3;      
    int nMod = nSrcLen % 3;    
    // 每次取3个字节，编码成4个字符
    for (int i = 0; i < nDiv; i ++)
    {
        c1 = *pSrc++;
        c2 = *pSrc++;
        c3 = *pSrc++;
 
        *pDst++ = EnBase64Tab[c1 >> 2];
        *pDst++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f];
        *pDst++ = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f];
        *pDst++ = EnBase64Tab[c3 & 0x3f];
        nLineLen += 4;
        nDstLen += 4;
        // 相关RFC中每行超过76字符时需要添加回车换行
        if (bChangeLine && nLineLen > 72)
        {
            *pDst++ = '\r';
            *pDst++ = '\n';
            nLineLen = 0;
            nDstLen += 2;
        }
    }
    // 编码余下的字节
    if (nMod == 1)
    {
        c1 = *pSrc++;
        *pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
        *pDst++ = EnBase64Tab[((c1 & 0x03) << 4)];
        *pDst++ = '=';
        *pDst++ = '=';
        nLineLen += 4;
        nDstLen += 4;
    }
    else if (nMod == 2)
    {
        c1 = *pSrc++;
        c2 = *pSrc++;
        *pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
        *pDst++ = EnBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
        *pDst++ = EnBase64Tab[((c2 & 0x0f) << 2)];
        *pDst++ = '=';
        nDstLen += 4;
    }
    // 输出加个结束符
    *pDst = '\0';
 
    return nDstLen;
}

```