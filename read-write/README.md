# TCP Delayed Ack and Nagle algorithm Interaction

TCP delayed ackとNagleアルゴリズムの相互作用による
遅延の観測。

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

- クライアントはheaderとbodyをサーバーに送る。ここではそれぞれでwrite()する。
- サーバーはheaderとbodyを読んだらreplyを1回write()する

##

- PC1でserverプログラムを作動
- PC2でtcpdumpでキャプチャしつつclientを作動

させたときのclientのログ:

```
 1  0.000094 0.000000 client: will write header
 2  0.000145 0.000051 client: wrote header
 3  0.000149 0.000004 client: will write body
 4  0.000152 0.000003 client: wrote body
 5  0.000153 0.000001 client: will read reply
 6  0.000321 0.000168 client: got reply

 7  1.000384 1.000063 client: will write header
 8  1.000402 0.000018 client: wrote header
 9  1.000405 0.000003 client: will write body
10  1.000408 0.000003 client: wrote body
11  1.000410 0.000002 client: will read reply
12  1.040998 0.040588 client: got reply

13  2.041059 1.000061 client: will write header
14  2.041075 0.000016 client: wrote header
15  2.041078 0.000003 client: will write body
16  2.041080 0.000002 client: wrote body
17  2.041082 0.000002 client: will read reply
18  2.081995 0.040913 client: got reply

19  3.082056 1.000061 client: will write header
20  3.082071 0.000015 client: wrote header
21  3.082074 0.000003 client: will write body
22  3.082077 0.000003 client: wrote body
23  3.082078 0.000001 client: will read reply
24  3.122994 0.040916 client: got reply

25  4.123054 1.000060 client: will write header
26  4.123070 0.000016 client: wrote header
27  4.123073 0.000003 client: will write body
28  4.123076 0.000003 client: wrote body
29  4.123078 0.000002 client: will read reply
30  4.163992 0.040914 client: got reply

31  5.164052 1.000060 client: will write header
32  5.164067 0.000015 client: wrote header
33  5.164070 0.000003 client: will write body
34  5.164073 0.000003 client: wrote body
35  5.164074 0.000001 client: will read reply
36  5.204991 0.040917 client: got reply
```

最初のreplyはすぐに返ってきているが、2回目以降は40ミリ秒よけいに
時間がかかっている。

tcpdumpのログ[tcpdump.txt](tcpdump.txt)
をみると、2回目以降は11, 17行め, ...のようにサーバー側から
ackが返るのに40ミリ秒かかっていて、ackを受信後、2個目の
リクエストデータを送っていることが確認できる。
