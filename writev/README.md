# TCP Delayed Ack and Nagle algorithm Interaction

TCP delayed ackとNagleアルゴリズムの相互作用による
遅延の観測。

## プロトコル

```
クライアント                                 サーバー

header_byte_size (512) バイト ---->
body_byte_size   (512) バイト ---->
                              <---- reply_byte_size (1448) バイト

(スリープusleep_sec秒)
header_byte_size (512) バイト ---->
body_byte_size   (512) バイト ---->
                              <---- reply_byte_size (1448) バイト
```

## writev()を使う

サーバーはread-writeディレクトリにあるものとかわりはない。

クライアント側でwritev()を使って1回のwriteで
headerとbodyを書いてみる。


writev()の使い方、必要な院クルードファイル、および引数に指定する
struct iovecはman writevするとでてくる。
```
#include <sys/uio.h>
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};
```

## プログラムのログとtcpdumpキャプチャデータ

```
 1  0.000000 0.000000 client: writev done
 2  0.000382 0.000382 client: got reply
 3  1.000455 1.000073 client: writev done
 4  1.000514 0.000059 client: got reply
 5  2.000587 1.000073 client: writev done
 6  2.000653 0.000066 client: got reply
 7  3.000721 1.000068 client: writev done
 8  3.000780 0.000059 client: got reply
 9  4.000857 1.000077 client: writev done
10  4.000924 0.000067 client: got reply
11  5.000992 1.000068 client: writev done
12  5.001059 0.000067 client: got reply
13  6.001128 1.000069 client: writev done
14  6.001186 0.000058 client: got reply
15  7.001256 1.000070 client: writev done
16  7.001311 0.000055 client: got reply
17  8.001380 1.000069 client: writev done
18  8.001433 0.000053 client: got reply
19  9.001501 1.000068 client: writev done
20  9.001555 0.000054 client: got reply
21  10.001623 1.000068 client: writev done
22  10.001681 0.000058 client: got reply
```

writev()してから40msの遅延なく、replyがきている。

tcpdumpのキャプチャ[tcpdump.txt](tcpdump.txt)